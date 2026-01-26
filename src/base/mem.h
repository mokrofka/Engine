#pragma once
#include "base.h"

#define MEM_ALLOC_HEADER_GUARD   0xA110C8
#define MEM_DEALLOC_HEADER_GUARD 0xDE1E7E
#define MEM_ALLOC_GUARD          0xA1
#define MEM_DEALLOC_GUARD        0xDE
#define MEM_ALLOC_TAIL_GUARD     0xdeedbeef

#define MEM_GUARD 1

#if MEM_GUARD
  #define MemGuardAlloc(d, z)   MemSet(d, MEM_ALLOC_GUARD, z)
  #define MemGuardDealloc(d, z) MemSet(d, MEM_DEALLOC_GUARD, z)
#else
  #define MemGuardAlloc(d, c)
  #define MemGuardDealloc(d, c)
#endif

#define MEM_DEFAULT_ALIGNMENT sizeof(void*)

#define ARENA_DEFAULT_RESERVE_SIZE MB(64)
#define ARENA_DEFAULT_COMMIT_SIZE  KB(4)

enum AllocatorType {
  AllocatorType_Global,
  AllocatorType_Arena,
  AllocatorType_ArenaList,
  AllocatorType_SegList,
};

struct Allocator {
  AllocatorType type;
  void* ctx;
};

////////////////////////////////////////////////////////////////////////
// Global allocator (just seglist)

KAPI void global_alloc_init();
KAPI u8*  global_alloc(u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI u8*  global_alloc_zero(u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
#define   global_alloc_struct(T)        (T*)global_alloc(     sizeof(T),     alignof(T))
#define   global_alloc_struct_zero(T)   (T*)global_alloc_zero(sizeof(T),     alignof(T))
#define   global_alloc_array(T,c)       (T*)global_alloc(     sizeof(T)*(c), alignof(T))
#define   global_alloc_array_zero(T,c)  (T*)global_alloc_zero(sizeof(T)*(c), alignof(T))
KAPI u8*  global_realloc(void* ptr, u64 old_size, u64 new_size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI u8*  global_realloc_zero(void* ptr, u64 old_size, u64 new_size, u64 align = MEM_DEFAULT_ALIGNMENT);
#define   global_realloc_array(ptr,old_c,T,c) (T*)global_realloc(ptr, sizeof(T)*(old_c), sizeof(T)*(c), alignof(T))
KAPI void global_free(void* ptr);

////////////////////////////////////////////////////////////////////////
// Arena (page allocator)

struct Arena {
  u8* base;
  u64 pos;
  u64 cmt;
  u64 cap;
  operator Allocator() { return {.type = AllocatorType_Arena, .ctx = this}; }
};

KAPI Arena arena_init();
KAPI void  arena_deinit(Arena* arena);
KAPI void  arena_clear(Arena* arena);
KAPI u8*   arena_alloc(Arena* arena, u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI u8*   arena_alloc_zero(Arena* arena, u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI u8*   arena_realloc(Arena* arena, void* ptr, u64 old_size, u64 new_size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI u8*   arena_realloc_zero(Arena* arena, void* ptr, u64 old_size, u64 new_size, u64 align = MEM_DEFAULT_ALIGNMENT);

struct Temp {
  Arena* arena;
  u64 pos;
};

INLINE Temp temp_begin(Arena* arena) { return Temp{arena, arena->pos}; };
INLINE void temp_end(Temp temp)      { temp.arena->pos = temp.pos; }

////////////////////////////////////////////////////////////////////////
// ArenaList

struct ArenaBlock {
  u64 pos;
  u64 cap;
  ArenaBlock* next;
};

struct ArenaList {
  Allocator alloc = {};
  ArenaBlock* current = null;
  ArenaBlock* first = null;
  ArenaList() = default;
  ArenaList(Allocator alloc_) { alloc = alloc_; }
  void init(Allocator alloc_) { alloc = alloc_; }
  void clear();
  operator Allocator() { return {.type = AllocatorType_ArenaList, .ctx = this}; }
};

KAPI u8* arena_list_alloc(ArenaList* arena, u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI u8* arena_list_alloc_zero(ArenaList* arena, u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI u8* arena_list_realloc(ArenaList* arena, void* ptr, u64 old_size, u64 new_size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI u8* arena_list_realloc_zero(ArenaList* arena, void* ptr, u64 old_size, u64 new_size, u64 align = MEM_DEFAULT_ALIGNMENT);

////////////////////////////////////////////////////////////////////////
// General allocator (segregated pow2 list)

struct PoolFreeNode {
  PoolFreeNode* next;
};

struct MemPoolPow2 {
  PoolFreeNode* head;
};

struct AllocSegList {
  Allocator alloc = {};
  MemPoolPow2 pools[32] = {};
  AllocSegList() = default;
  AllocSegList(Allocator alloc_) { alloc = alloc_; }
  void init(Allocator alloc_) { alloc = alloc_; }
  operator Allocator() { return {.type = AllocatorType_SegList, .ctx = this}; }
};

KAPI u8*  seglist_alloc(AllocSegList* alloc, u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI u8*  seglist_alloc_zero(AllocSegList* alloc, u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI u8*  seglist_realloc(AllocSegList* alloc, void* ptr, u64 old_size, u64 new_size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI u8*  seglist_realloc_zero(AllocSegList* alloc, void* ptr, u64 old_size, u64 new_size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI void seglist_free(AllocSegList* alloc, void* ptr);

////////////////////////////////////////////////////////////////////////
// tlsf TODO: implement

#define BinCount      32
#define SubbinCount   32
#define MinAllocation 128

struct TLSF_Allocator {
  u32 bin_bitmap;
  u32 sub_bin_bitmaps[BinCount];
};

TLSF_Allocator tlsf_init();
KAPI u8* tlsf_alloc(TLSF_Allocator& allocator, u64 size, u64 alignment = MEM_DEFAULT_ALIGNMENT);

////////////////////////////////////////////////////////////////////////
// atlas TODO: implement

////////////////////////////////////////////////////////////////////////
// Allocator Interface

KAPI u8*  mem_alloc(Allocator alloc, u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI u8*  mem_alloc_zero(Allocator alloc, u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
#define   mem_alloc_struct(a,T)        (T*)mem_alloc(a,      sizeof(T),     alignof(T))
#define   mem_alloc_struct_zero(a,T)   (T*)mem_alloc_zero(a, sizeof(T),     alignof(T))
#define   mem_alloc_array(a,T,c)       (T*)mem_alloc(a,      sizeof(T)*(c), alignof(T))
#define   mem_alloc_array_zero(a,T,c)  (T*)mem_alloc_zero(a, sizeof(T)*(c), alignof(T))
#define   push_buffer(a, z, ...)           mem_alloc(a,      z, ##__VA_ARGS__)
#define   push_buffer_zero(a, z, ...)      mem_alloc_zero(a, z, ##__VA_ARGS__)
#define   push_struct(a, T)            (T*)mem_alloc(a,      sizeof(T),     alignof(T))
#define   push_struct_zero(a, T)       (T*)mem_alloc_zero(a, sizeof(T),     alignof(T))
#define   push_array(a, T, c)          (T*)mem_alloc(a,      sizeof(T)*(c), alignof(T))
#define   push_array_zero(a, T, c)     (T*)mem_alloc_zero(a, sizeof(T)*(c), alignof(T))
KAPI u8*  mem_realloc(Allocator alloc, void* ptr, u64 old_size, u64 new_size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI u8*  mem_realloc_zero(Allocator alloc, void* ptr, u64 old_size, u64 new_size, u64 align = MEM_DEFAULT_ALIGNMENT);
#define   mem_realloc_array(a,ptr,old_c,T,c)      (T*)mem_realloc(a, ptr, sizeof(T)*(old_c), sizeof(T)*(c))
#define   mem_realloc_array_zero(a,ptr,old_c,T,c) (T*)mem_realloc_zero(a, ptr, sizeof(T)*(old_c), sizeof(T)*(c))
KAPI void mem_free(Allocator alloc, void* ptr);

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

AllocGpuHandle* gpu_seglist_alloc(AllocGpu& allocator, u64 size, u64 alignment = MEM_DEFAULT_ALIGNMENT);
void gpu_seglist_free(AllocGpu& allocator, AllocGpuHandle* handle);

////////////////////////////////////////////////////////////////////////
// Utils

// u64 _push_offset(u64* cur_pos, u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
// #define push_offset(cur_pos, T, c) _push_offset(cur_pos, sizeof(T)*(c), alignof(T))

struct SoA_Field {
  void** dst_ptr;
  u32 elem_size;
  u32 align;
};
#define SoA_push_field(ptr,T) {(void**)(ptr), sizeof(T), alignof(T)}

KAPI u8* mem_alloc_soa(Allocator alloc, u32 count, SoA_Field* fields, u32 fields_count);
KAPI u8* mem_realloc_soa(Allocator alloc, void* ptr, u32 old_count, u32 new_count, SoA_Field* fields, u32 fields_count);

