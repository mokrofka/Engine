#pragma once
#include "defines.h"

#define ALLOC_HEADER_GUARD   0xA110C8
#define DEALLOC_HEADER_GUARD 0xDE1E7E
#define ALLOC_GUARD          0xA1
#define DEALLOC_GUARD        0xDE
#define PAGE_SIZE            4096

#define MEMORY_GUARD 1

#if MEMORY_GUARD
  #define FillAlloc(d, c)        MemSet(d, ALLOC_GUARD, c)
  #define FillAllocStruct(d)     MemSet(d, ALLOC_GUARD, sizeof(*(d)))
  #define FillAllocTyped(d, c)   MemSet(d, ALLOC_GUARD, sizeof(*(d) * (c)))
  #define FillDealoc(d, c)       MemSet(d, DEALLOC_GUARD, c)
  #define FillDealocTyped(d, c)  MemSet(d, DEALLOC_GUARD, sizeof(*(d) * (c)))
  #define FillDealocStruct(d)    MemSet(d, DEALLOC_GUARD, sizeof(*(d)))
#else
  #define FillAlloc(d, c)
  #define FillAllocStruct(d)
  #define FillAllocTyped(d, c)
  #define FillDealoc(d, c)
  #define FillDealocStruct(d)
  #define FillDealocTyped(d, c)
#endif

#define DEFAULT_ALIGNMENT sizeof(void*)
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
#define mem_alloc_typed(T, c) (T*)mem_alloc(sizeof(T)*c)
#define mem_alloc_typed_zero(T, c) (T*)mem_alloc_zero(sizeof(T)*c)
KAPI u8* mem_realloc(void* ptr, u64 size);
KAPI u8* mem_realloc_zero(void* ptr, u64 size);
#define mem_realloc_typed(a, T, c) (T*)mem_realloc(a, sizeof(T)*c)
#define mem_realloc_typed_zero(a, T, c) (T*)mem_realloc_zero(a, sizeof(T)*c)
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

////////////////////////////////////////////////////////////////////////
// Pool

struct PoolFreeNode {
  PoolFreeNode* next;
};

struct MemPool {
  Arena* arena;
  u8* data;
  u64 cap;
	u64 count;
	u64 chunk_size;
  PoolFreeNode* head;
};

KAPI void mem_pool_free_all(MemPool& pool);
KAPI MemPool mem_pool_create(Arena* arena, u64 chunk_size, u64 chunk_alignment = DEFAULT_ALIGNMENT);
KAPI void mem_pool_grow(MemPool& p);
KAPI u8* mem_pool_alloc(MemPool& p);
KAPI void mem_pool_free(MemPool& p, void* ptr);

template<typename T>
struct ObjectPool {
  MemPool p;
  void init(Arena* arena) {
    p = mem_pool_create(arena, sizeof(T));
  }
  void grow() {
    mem_pool_grow(p);
  }
  void free_all() {
    mem_pool_free_all(p);
  }
  T* alloc() {
    return (T*)mem_pool_alloc(p);
  }
  void free(T* ptr) {
    mem_pool_free(p, ptr);
  }
  T& operator[](u32 index) {
    return ((T*)p.data)[index];
  }
};

////////////////////////////////////////////////////////////////////////
// Free list

struct FreeListAllocationHeader {
  u64 block_size;
  u64 padding;
  
#ifdef MEMORY_GUARD
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
