#pragma once
#include "defines.h"

#define MEM_ALLOC_HEADER_GUARD   0xA110C8
#define MEM_DEALLOC_HEADER_GUARD 0xDE1E7E
#define MEM_ALLOC_GUARD          0xA1
#define MEM_DEALLOC_GUARD        0xDE
#define PAGE_SIZE                4096

#define MEM_GUARD 1

#if MEM_GUARD
  #define MemGuardAlloc(d, c)         MemSet(d, MEM_ALLOC_GUARD, c)
  #define MemGuardAllocStruct(d)      MemSet(d, MEM_ALLOC_GUARD, sizeof(*(d)))
  #define MemGuardAllocTyped(d, c)    MemSet(d, MEM_ALLOC_GUARD, sizeof(*(d) * (c)))
  #define MemGuardDealloc(d, c)       MemSet(d, MEM_DEALLOC_GUARD, c)
  #define MemGuardDeallocTyped(d, c)  MemSet(d, MEM_DEALLOC_GUARD, sizeof(*(d) * (c)))
  #define MemGuardDeallocStruct(d)    MemSet(d, MEM_DEALLOC_GUARD, sizeof(*(d)))
#else
  #define MemGuardAlloc(d, c)
  #define MemGuardAllocStruct(d)
  #define MemGuardAllocTyped(d, c)
  #define MemGuardDealloc(d, c)
  #define MemGuardDeallocStruct(d)
  #define MemGuardDeallocTyped(d, c)
#endif

#define DEFAULT_ALIGNMENT sizeof(void*)

#define ARENA_DEFAULT_RESERVE_SIZE MB(64)
#define ARENA_DEFAULT_COMMIT_SIZE  KB(4)

struct Range {
  u64 offset; 
  u64 size; 
};

////////////////////////////////////////////////////////////////////////
// Global Allocator

KAPI void global_allocator_init();
KAPI u8*  mem_alloc(u64 size, u64 alignment = DEFAULT_ALIGNMENT);
KAPI u8*  mem_alloc_zero(u64 size);
#define   mem_alloc_struct(T)        (T*)mem_alloc(sizeof(T))
#define   mem_alloc_typed(T, c)      (T*)mem_alloc(sizeof(T)*c)
#define   mem_alloc_typed_zero(T, c) (T*)mem_alloc_zero(sizeof(T)*c)

KAPI u8*  mem_realloc(void* ptr, u64 size);
KAPI u8*  mem_realloc_zero(void* ptr, u64 size);
#define   mem_realloc_typed(a, T, c)      (T*)mem_realloc(a, sizeof(T)*c)
#define   mem_realloc_typed_zero(a, T, c) (T*)mem_realloc_zero(a, sizeof(T)*c)

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
INLINE void arena_clear(Arena* arena) { arena->pos = 0; };

INLINE Temp temp_begin(Arena* arena) { return Temp{arena, arena->pos}; };
INLINE void temp_end(Temp temp)      { temp.arena->pos = temp.pos; }

KAPI void* _arena_push(Arena* arena, u64 size, u64 align = DEFAULT_ALIGNMENT);
#define    push_array(a, T, c)    (T*) _arena_push(a, sizeof(T)*c, alignof(T))
#define    push_struct(a, T)      (T*) _arena_push(a, sizeof(T), alignof(T))
#define    push_buffer(a, c, ...) (u8*)_arena_push(a, c, ##__VA_ARGS__)

KAPI void arena_release(Arena* arena);

////////////////////////////////////////////////////////////////////////
// General allocator

struct PoolFreeNode {
  PoolFreeNode* next;
};

struct MemPoolPow2 {
  PoolFreeNode* head;
};

struct Allocator{
  Arena* arena;
  MemPoolPow2 pools[32];
  Allocator() = default;
  Allocator(Arena* arena_) { arena = arena_; }
};

u8*  mem_alloc(Allocator& allocator, u64 size);
void mem_free(Allocator& allocator, void* ptr);

////////////////////////////////////////////////////////////////////////
// Pool

struct MemPool {
  Arena* arena;
  u8* data;
  u64 cap;
	u64 count;
	u64 chunk_size;
  PoolFreeNode* head;
};

KAPI MemPool mem_pool_create(Arena* arena, u64 chunk_size, u64 chunk_alignment = DEFAULT_ALIGNMENT);
KAPI u8*     mem_pool_alloc(MemPool& p);
KAPI void    mem_pool_free(MemPool& p, void* ptr);
KAPI void    mem_pool_free_all(MemPool& pool);
KAPI void    mem_pool_grow(MemPool& p);

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
  
#ifdef MEM_GUARD
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
