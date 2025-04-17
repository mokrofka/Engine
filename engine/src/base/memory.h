#pragma once

#include "defines.h"

KAPI void* _memory_zero(void* block, u64 size);
KAPI void* _memory_copy(void* dest, const void* source, u64 size);
KAPI void* _memory_set(void* dest, i32 value, u64 size);
KAPI b32   _memory_match(void* a, void* b, u64 size);

// Arena

#define DEFAULT_ALIGNMENT (sizeof(void*))
#define ARENA_HEADER sizeof(Arena)
#define POOL_HEADER sizeof(Pool)

struct Arena {
  u64 res;
  u64 pos;
};

struct TCTX {
  Arena* arenas[2];
};

struct Temp {
  Arena* arena;
  u64 pos;
};

struct Scratch {
  Arena* arena;
  u64 pos;
  
  Inline operator Arena*() {
    return this->arena;
  }

  Inline Scratch(Arena** conflics = null, u32 counts = 0);
  Inline ~Scratch() { arena->pos = pos; };
};

KAPI Arena* arena_alloc(Arena *a, u64 size = MB(1), u64 align = 8);
KAPI Arena* arena_alloc(u64 size = MB(1));

Inline u64 arena_pos(Arena* arena) { return ARENA_HEADER + arena->pos; };
Inline void arena_clear(Arena* arena) { arena->pos = 0; };

KAPI void* _arena_push(Arena* arena, u64 size, u64 align = 8);

Inline Temp temp_begin(Arena* arena) { return Temp{arena, arena->pos}; };
Inline void temp_end(Temp temp) { temp.arena->pos = temp.pos; }

void tctx_init(Arena* arena);
KAPI Temp tctx_get_scratch(Arena** conflics, u32 counts);

#define push_array(a, T, c) (T*)_arena_push(a, sizeof(T)*c, Max(8, alignof(T)))
#define push_struct(a, T) (T*)_arena_push(a, sizeof(T), Max(8, alignof(T)))
#define push_buffer(a, T, c, ...) (T*)_arena_push(a, c, ## __VA_ARGS__)
  
#define GetScratch(conflicts, count) (tctx_get_scratch((conflicts), (count)))
#define ReleaseScratch(scratch) temp_end(scratch)

Inline Scratch::Scratch(Arena** conflics, u32 counts) {
  Temp temp = tctx_get_scratch(conflics, counts);
  *this = *(Scratch*)&temp;
}

// Pool

struct PoolFreeNode {
	PoolFreeNode *next;
};

struct Pool {
	u64 chunk_count;
	u64 chunk_size;
  u8* data;

	PoolFreeNode *head;
};

KAPI void* _pool_alloc(Pool& p);

KAPI Pool pool_create(Arena* arena, u64 chunk_count, u64 chunk_size, u64 chunk_alignment = DEFAULT_ALIGNMENT);
KAPI void pool_free(Pool& p, void* ptr);
KAPI void pool_free_all(Pool& pool);

#define pool_alloc(p, T) (T*)_pool_push(p)

// Free list

struct free_list_allocation_Header {
  u64 block_size;
  u64 padding;
};

// An intrusive linked list for the free memory blocks
struct FreeListNode {
  FreeListNode* next;
  u64 block_size;
};

enum PlacementPolicy {
  PlacementPolicy_FindFirst,
  PlacementPolicy_FindBest
};

struct FreeList {
  void* data;
  u64 size;
  u64 used;

  FreeListNode* head;
  PlacementPolicy policy;
};

KAPI void* _free_list_alloc(FreeList& fl, u64 size, u64 alignment);

KAPI FreeList push_free_list(Arena* arena, u64 size);
KAPI void* free_list_free(FreeList& fl, void* ptr);
KAPI void free_list_free_all(FreeList& fl);
