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

struct BlockHeader {
  u32 guard;
  u32 size;
  u16 align;
  u16 offset;
};

////////////////////////////////////////////////////////////////////////
// Global allocator (segregated pow2)

KAPI void global_alloc_init();
KAPI u8*  global_alloc(u64 size, u64 alignment = DEFAULT_ALIGNMENT);
KAPI u8*  global_alloc_zero(u64 size);
#define   global_alloc_struct(T)        (T*)global_alloc(sizeof(T))
#define   global_alloc_array(T,c)       (T*)global_alloc(sizeof(T) * (c))
#define   global_alloc_array_zero(T,c)  (T*)global_alloc(sizeof(T) * (c))

KAPI u8*  global_realloc(void* ptr, u64 size);
KAPI u8*  global_realloc_zero(void* ptr, u64 size);
#define   global_realloc_array(ptr,T,c)       (T*)global_realloc(ptr,sizeof(T) * (c))
#define   global_realloc_array_zero(ptr,T,c)  (T*)global_realloc_zero(ptr,sizeof(T) * (c))

KAPI void global_free(void* ptr);

////////////////////////////////////////////////////////////////////////
// Arena

struct Arena {
  u64 pos;
  u64 cmt;
  u64 cap;
};

struct Temp {
  Arena* arena;
  u64 pos;
};

KAPI Arena* arena_alloc();
KAPI void   arena_clear(Arena* arena);

INLINE Temp temp_begin(Arena* arena) { return Temp{arena, arena->pos}; };
INLINE void temp_end(Temp temp)      { temp.arena->pos = temp.pos; }

KAPI void* _arena_push(Arena* arena, u64 size, u64 align = DEFAULT_ALIGNMENT);
#define    push_array(a, T, c)    (T*) _arena_push(a, sizeof(T) * c, alignof(T))
#define    push_struct(a, T)      (T*) _arena_push(a, sizeof(T), alignof(T))
#define    push_buffer(a, z, ...) (u8*)_arena_push(a, z, ##__VA_ARGS__)

KAPI void arena_release(Arena* arena);

////////////////////////////////////////////////////////////////////////
// General allocator (segregated pow2)

struct PoolFreeNode {
  PoolFreeNode* next;
};

struct MemPoolPow2 {
  PoolFreeNode* head;
};

struct AllocSegList {
  Arena* arena = null;
  MemPoolPow2 pools[32] = {};
  AllocSegList() = default;
  AllocSegList(Arena* arena_) { arena = arena_; }
};

u8*  seglist_alloc(AllocSegList& allocator, u64 size, u64 alignment = DEFAULT_ALIGNMENT);
void seglist_free(AllocSegList& allocator, void* ptr);

////////////////////////////////////////////////////////////////////////
// Offset

struct OffsetBuffer {
  u64 pos;
  u8* base;
};

struct OffsetMark {
  u64 pos;
};

u8* push_offset(OffsetBuffer& arena, u64 size, u64 alignment = DEFAULT_ALIGNMENT);
u64 push_offset(OffsetMark& arena, u64 size, u64 alignment = DEFAULT_ALIGNMENT);

////////////////////////////////////////////////////////////////////////
// General GPU allocator (segregated pow2)

struct AllocGpuHandle {
  u64 offset;
  u64 size;
};

struct AllocGpu {
  AllocGpuHandle* pools[32] = {};
  AllocGpuHandle* handlers;
};

AllocGpuHandle* gpu_seglist_alloc(AllocGpu& allocator, u64 size, u64 alignment = DEFAULT_ALIGNMENT);
void gpu_seglist_free(AllocGpu& allocator, AllocGpuHandle* handle);

////////////////////////////////////////////////////////////////////////
// Pool

// NOTE: realocates memory to keep indexes valid
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
// tlsf TODO: implement

#define BinCount      32
#define SubbinCount   32
#define MinAllocation 128

struct TLSF_Allocator {
  u32 bin_bitmap;
  u32 sub_bin_bitmaps[BinCount];
};

TLSF_Allocator tlsf_create();
KAPI u8* tlsf_alloc(TLSF_Allocator& allocator, u64 size, u64 alignment = DEFAULT_ALIGNMENT);

////////////////////////////////////////////////////////////////////////
// atlas TODO: implement

////////////////////////////////////////////////////////////////////////
// Allocator Interface

enum AllocType {
  AllocType_General,
  AllorType_Arena,
  AllorType_SegList,
};
struct Allocator {
  AllocType type;
  void* ctx;
};
inline u8* mem_alloc(Allocator alloc, u64 size, u64 alignment = DEFAULT_ALIGNMENT) {
  switch (alloc.type) {
    case AllocType_General: return global_alloc(size, alignment);
    case AllorType_Arena: return push_buffer((Arena*)alloc.ctx, size, alignment);
    case AllorType_SegList: return seglist_alloc(*(AllocSegList*)alloc.ctx, size, alignment);
  }
}
inline void mem_free(Allocator alloc, void* ptr) {
  switch (alloc.type) {
    case AllocType_General: return global_free(ptr);
    case AllorType_Arena: return;
    case AllorType_SegList: return seglist_free(*(AllocSegList*)alloc.ctx, ptr);
  }
}

