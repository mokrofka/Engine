#pragma once
#include "defines.h"

#define ALLOC_HEADER_GUARD   0xA110C8
#define DEALLOC_HEADER_GUARD 0xDE1E7E
#define ALLOC_GUARD          0xA1
#define DEALLOC_GUARD        0xDE

#define GUARD_MEMORY 1

#if GUARD_MEMORY
  #define FillAlloc(ptr, size)  MemSet(ptr, ALLOC_GUARD, size)
  #define FillAllocStruct(ptr)  MemSet(ptr, ALLOC_GUARD, sizeof(*(ptr)))
  #define FillDealoc(ptr, size) MemSet(ptr, DEALLOC_GUARD, size)
  #define FillDealocStruct(ptr) MemSet(ptr, DEALLOC_GUARD, sizeof(*(ptr)))
#else
  #define FillAlloc(ptr, size)
  #define FillAllocStruct(ptr)
  #define FillDealoc(ptr, size)
  #define FillDealocStruct(ptr)
#endif

#define DEFAULT_ALIGNMENT (sizeof(void*))
#define ARENA_HEADER_SIZE sizeof(Arena)

#define ARENA_DEFAULT_RESERVE_SIZE MB(64)
#define ARENA_DEFAULT_COMMIT_SIZE  KB(4)

struct Range {
  u64 offset; 
  u64 size; 
};

////////////////////////////////////////////////////////////////////////
// Global Allocator

KAPI void global_allocator_init();
KAPI u8* mem_alloc(u64 size);
KAPI u8* mem_alloc_zero(u64 size);
#define mem_alloc_struct(T) (T*)mem_alloc(sizeof(T))
KAPI u8* mem_realloc(void* ptr, u64 size);
KAPI void mem_free(void* ptr);

////////////////////////////////////////////////////////////////////////
// Arena

struct Arena {
  u64 pos;
  u64 cmt;
  u64 res;
};

struct Temp {
  Arena* arena;
  u64 pos;
};

KAPI Arena* arena_alloc();
KAPI Arena* arena_shm_alloc(void* handler);
INLINE void arena_clear(Arena* arena) { arena->pos = 0; };

INLINE Temp temp_begin(Arena* arena) { return Temp{arena, arena->pos}; };
INLINE void temp_end(Temp temp) { temp.arena->pos = temp.pos; }

KAPI void* _arena_push(Arena* arena, u64 size, u64 align = DEFAULT_ALIGNMENT);
#define push_array(a, T, c) (T*)_arena_push(a, sizeof(T)*c, alignof(T))
#define push_struct(a, T) (T*)_arena_push(a, sizeof(T), alignof(T))
#define push_buffer(a, c, ...) (u8*)_arena_push(a, c, ##__VA_ARGS__)

KAPI void arena_release(Arena* arena);

// void _arena_move(Arena* arena, u64 size, u64 align);
// #define arena_move_array(a, T, c) _arena_move(a, sizeof(T)*c, alignof(T))

////////////////////////////////////////////////////////////////////////
// Pool

struct PoolFreeNode {
	PoolFreeNode *next;
};

struct MemPool {
  u8* data;
	u64 chunk_count;
	u64 chunk_size;

	PoolFreeNode *head;

#ifdef GUARD_MEMORY
  u64 guard_size;
#endif
};

KAPI u8* pool_alloc(MemPool& p);
KAPI MemPool pool_create(Arena* arena, u64 chunk_count, u64 chunk_size, u64 chunk_alignment = DEFAULT_ALIGNMENT);
KAPI void pool_free(MemPool& p, void* ptr);
KAPI void pool_free_all(MemPool& pool);

////////////////////////////////////////////////////////////////////////
// Free list

struct FreeListAllocationHeader {
  u64 block_size;
  u64 padding;
  
#ifdef GUARD_MEMORY
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

KAPI u8* freelist_alloc(FreeList& fl, u64 size, u64 alignment = DEFAULT_ALIGNMENT);
KAPI FreeList freelist_create(Arena* arena, u64 size, u64 alignment = DEFAULT_ALIGNMENT);
KAPI void freelist_free(FreeList& fl, void* ptr);
KAPI void freelist_free_all(FreeList& fl);

////////////////////////////////////////////////////////////////////////
// Gpu Freelist

struct FreelistGpuNode {
  u64 offset;
  u64 size;
  FreelistGpuNode* next;
};

struct FreelistGpu {
  u64 total_size;
  u64 max_entries;
  FreelistGpuNode* head;
  FreelistGpuNode* nodes;
};

FreelistGpu freelist_gpu_create(Arena* arena, u64 size);
u64 freelist_gpu_alloc(FreelistGpu& list, u64 size);
void freelist_gpu_free(FreelistGpu& list, u64 size, u64 offset);
