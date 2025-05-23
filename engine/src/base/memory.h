#pragma once

#include "defines.h"

#define MEMORY_ALLOCATED_GUARD
#ifdef MEMORY_ALLOCATED_GUARD
  #define MEMORY_ALLOCATED_GUARD_SIZE 8
#else
  #define MEMORY_ALLOCATED_GUARD_SIZE 0
#endif

struct MemRange {
  u64 offset; 
  u64 size; 
};

////////////////////////////////
// Arena

#define DEFAULT_ALIGNMENT (sizeof(void*))
#define ARENA_HEADER sizeof(Arena)

struct Arena {
  u64 size;
  u64 used;
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
  
  INLINE operator Arena*() { return this->arena; }

  INLINE Scratch(Arena** conflics = null, u64 counts = 0);
  INLINE ~Scratch() { arena->used = pos; };
};

KAPI Arena* arena_alloc(Arena *a, u64 size, u64 align = DEFAULT_ALIGNMENT);

INLINE u64 arena_pos(Arena* arena) { return ARENA_HEADER + arena->used; };
INLINE void arena_clear(Arena* arena) { arena->used = 0; };

KAPI void* _arena_push(Arena* arena, u64 size, u64 align = DEFAULT_ALIGNMENT);

INLINE Temp temp_begin(Arena* arena) { return Temp{arena, arena->used}; };
INLINE void temp_end(Temp temp) { temp.arena->used = temp.pos; }

void tctx_init(Arena* arena);
KAPI Temp tctx_get_scratch(Arena** conflics, u64 counts);

#define push_array(a, T, c) (T*)_arena_push(a, sizeof(T)*c, Max(DEFAULT_ALIGNMENT, alignof(T)))
#define push_struct(a, T) (T*)_arena_push(a, sizeof(T), Max(DEFAULT_ALIGNMENT, alignof(T)))
#define push_buffer(a, c, ...) (u8*)_arena_push(a, c, ##__VA_ARGS__)

void _arena_move(Arena* arena, u64 size, u64 align);
#define arena_move_array(a, T, c) _arena_move(a, sizeof(T)*c, Max(DEFAULT_ALIGNMENT, alignof(T)))

#define GetScratch(conflicts, count) (tctx_get_scratch((conflicts), (count)))
#define ReleaseScratch(scratch) temp_end(scratch)

INLINE Scratch::Scratch(Arena** conflics, u64 counts) {
  Temp temp = tctx_get_scratch(conflics, counts);
  *this = *(Scratch*)&temp;
}

////////////////////////////////
// Pool

struct PoolFreeNode {
	PoolFreeNode *next;
};

struct Pool {
  u8* data;
	u64 chunk_count;
	u64 chunk_size;

	PoolFreeNode *head;

#ifdef MEMORY_ALLOCATED_GUARD
  u64 guard_size;
#endif
};

KAPI u8* pool_alloc(Pool& p);

KAPI Pool pool_create(Arena* arena, u64 chunk_count, u64 chunk_size, u64 chunk_alignment = DEFAULT_ALIGNMENT);
KAPI void pool_free(Pool& p, void* ptr);
KAPI void pool_free_all(Pool& pool);

////////////////////////////////
// Free list

#define DebugMagic 0xDEADBEEF
struct FreeListAllocationHeader {
  u64 block_size;
  u64 padding;
#ifdef MEMORY_ALLOCATED_GUARD
  u64 guard;
#endif
};

struct FreeListNode {
  FreeListNode* next;
  u64 block_size;
};

struct FreeList {
  u8* data;
  u64 size;
  u64 used;

  FreeListNode* head;
};

struct MetaData {
  u64 offset;
  u64 size;
  u64 padding;
  u64 next;
  u64 next_size;
};

struct FreeListGpu {
  MetaData free_blocks[128];
  u32 free_count;
  u32 used_count;
  MetaData used_blocks[128];
  u64 size;
  u64 used;
  u32 count = 128;
  u32 head;
};

KAPI u8* free_list_alloc(FreeList& fl, u64 size, u64 alignment = DEFAULT_ALIGNMENT);
KAPI u64 free_list_alloc_block(FreeList& fl, u64 size, u64 alignment = DEFAULT_ALIGNMENT);

KAPI FreeList free_list_create(Arena* arena, u64 size, u64 alignment = DEFAULT_ALIGNMENT);
KAPI void free_list_free(FreeList& fl, void* ptr);
KAPI void free_list_free_all(FreeList& fl);

// FreeListGpu free_list_gpu_create(u64 size, u64 alignment = DEFAULT_ALIGNMENT);
// u32 free_list_gpu_alloc(FreeListGpu& fl, u64 size, u64 alignment = DEFAULT_ALIGNMENT);
// void free_list_gpu_free(FreeListGpu& fl, void* ptr);
// void free_list_gpu_free_all(FreeListGpu& fl);

////////////////////////////////
// Global Allocator

KAPI void global_allocator_init();
KAPI u8* mem_alloc(u64 size);
KAPI u8* mem_realoc(void* origin, u64 size);
KAPI void mem_free(void* ptr);

////////////////////////////////
// Ring Buffer

u64 ring_write(u8* ring_base, u64 ring_size, u64 ring_pos, void* src_data, u64 src_data_size);
u64 ring_read(u8* ring_base, u64 ring_size, u64 ring_pos, void* dst_data, u64 read_size);
#define ring_write_struct(ring_base, ring_size, ring_pos, ptr) ring_write((ring_base), (ring_size), (ring_pos), (ptr), sizeof(*(ptr)))
#define ring_read_struct(ring_base, ring_size, ring_pos, ptr) ring_read((ring_base), (ring_size), (ring_pos), (ptr), sizeof(*(ptr)))
