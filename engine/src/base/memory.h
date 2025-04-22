#pragma once

#include "defines.h"

KAPI void _memory_zero(void* block, u64 size);
KAPI void _memory_copy(void* dest, const void* source, u64 size);
KAPI void _memory_set(void* dest, i32 value, u64 size);
KAPI b32  _memory_match(void* a, void* b, u64 size);

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Arena

#define DEFAULT_ALIGNMENT (sizeof(void*))
#define ARENA_HEADER sizeof(Arena)

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

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
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

KAPI u8* pool_alloc(Pool& p);

KAPI Pool pool_create(Arena* arena, u64 chunk_count, u64 chunk_size, u64 chunk_alignment = DEFAULT_ALIGNMENT);
KAPI void pool_free(Pool& p, void* ptr);
KAPI void pool_free_all(Pool& pool);

#define PoolAlloc(p, a) *((u8**)(&(a))) = (u8*)pool_alloc(p)

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Free list

struct FreeListAllocationHeader {
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
  u8* data;
  u64 size;
  u64 used;

  FreeListNode* head;
  PlacementPolicy policy;
};

KAPI u8* free_list_alloc(FreeList& fl, u64 size, u64 alignment = 8);

KAPI FreeList free_list_create(Arena* arena, u64 size);
KAPI void* free_list_free(FreeList& fl, void* ptr);
KAPI void free_list_free_all(FreeList& fl);

#define FreeListAlloc(p, a, size) *((u8**)(&(a))) = (u8*)free_list_alloc(p, size)

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Global Allocator

KAPI void global_allocator_init();
KAPI u8* global_alloc(u64 size);

#define Global_Alloc(a,size) *((u8**)(&(a))) = (u8*)global_alloc(size)

KAPI void global_free(void* ptr);

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Ring Buffer

internal u64 ring_write(u8 *ring_base, u64 ring_size, u64 ring_pos, void *src_data, u64 src_data_size);
internal u64 ring_read(u8 *ring_base, u64 ring_size, u64 ring_pos, void *dst_data, u64 read_size);
#define ring_write_struct(ring_base, ring_size, ring_pos, ptr) ring_write((ring_base), (ring_size), (ring_pos), (ptr), sizeof(*(ptr)))
#define ring_read_struct(ring_base, ring_size, ring_pos, ptr) ring_read((ring_base), (ring_size), (ring_pos), (ptr), sizeof(*(ptr)))
