#pragma once
#include "base.h"

const u32 MEM_DEFAULT_ALIGNMENT = sizeof(void*);

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
// Global allocator

KAPI void global_allocator_init();
KAPI Allocator mem_get_global_allocator();

////////////////////////////////////////////////////////////////////////
// Arena (page allocator)

struct Arena {
  u8* base;
  u64 pos;
  u64 cmt;
  u64 cap;
  operator Allocator();
};

KAPI Arena arena_init();
KAPI void  arena_deinit(Arena* arena);
KAPI void  arena_clear(Arena* arena);

struct Temp {
  Arena* arena;
  u64 pos;
};

KAPI Temp temp_begin(Arena* arena);
KAPI void temp_end(Temp temp);

////////////////////////////////////////////////////////////////////////
// ArenaList

struct ArenaBlock {
  u64 pos;
  u64 cap;
  ArenaBlock* next;
};

struct ArenaList {
  Allocator alloc;
  ArenaBlock* current;
  ArenaBlock* first;
  ArenaList() = default;
  ArenaList(Allocator alloc_);
  void init(Allocator alloc_);
  void clear();
  operator Allocator();
};

////////////////////////////////////////////////////////////////////////
// General allocator (segregated pow2 list)

struct AllocSegList {
  Allocator alloc;
  u8* pools[32];
  AllocSegList() = default;
  AllocSegList(Allocator alloc_);
  void init(Allocator alloc_);
  operator Allocator();
};

////////////////////////////////////////////////////////////////////////
// tlsf TODO: implement

// #define BinCount      32
// #define SubbinCount   32
// #define MinAllocation 128
// struct TLSF_Allocator {
//   u32 bin_bitmap;
//   u32 sub_bin_bitmaps[BinCount];
// };
// TLSF_Allocator tlsf_init();
// KAPI u8* tlsf_alloc(TLSF_Allocator& allocator, u64 size, u64 alignment = MEM_DEFAULT_ALIGNMENT);

////////////////////////////////////////////////////////////////////////
// atlas TODO: implement

////////////////////////////////////////////////////////////////////////
// Allocator Interface

KAPI u8*  mem_alloc(Allocator alloc, u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI u8*  mem_alloc_zero(Allocator alloc, u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI u8*  mem_realloc(Allocator alloc, void* ptr, u64 old_size, u64 new_size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI u8*  mem_realloc_zero(Allocator alloc, void* ptr, u64 old_size, u64 new_size, u64 align = MEM_DEFAULT_ALIGNMENT);
KAPI void mem_free(Allocator alloc, void* ptr);

template<typename T> T* mem_alloc_struct(Allocator a)                                 { return (T*)mem_alloc(a,             sizeof(T),                    alignof(T)); }
template<typename T> T* mem_alloc_struct_zero(Allocator a)                            { return (T*)mem_alloc_zero(a,        sizeof(T),                    alignof(T)); }
template<typename T> T* mem_alloc_array(Allocator a, u32 c)                           { return (T*)mem_alloc(a,             sizeof(T)*c,                  alignof(T)); }
template<typename T> T* mem_alloc_array_zero(Allocator a, u32 c)                      { return (T*)mem_alloc_zero(a,        sizeof(T)*c,                  alignof(T)); }
template<typename T> T* mem_realloc_array(Allocator a, T* ptr, u32 old_c, u32 c)      { return (T*)mem_realloc(a, ptr,      sizeof(T)*old_c, sizeof(T)*c, alignof(T)); }
template<typename T> T* mem_realloc_array_zero(Allocator a, T* ptr, u32 old_c, u32 c) { return (T*)mem_realloc_zero(a, ptr, sizeof(T)*old_c, sizeof(T)*c, alignof(T)); }

// I like push name
#define push_buffer(a, z, ...)           mem_alloc(a,      z, ##__VA_ARGS__)
#define push_buffer_zero(a, z, ...)      mem_alloc_zero(a, z, ##__VA_ARGS__)
#define push_struct(a, T)            (T*)mem_alloc(a,      sizeof(T),     alignof(T))
#define push_struct_zero(a, T)       (T*)mem_alloc_zero(a, sizeof(T),     alignof(T))
#define push_array(a, T, c)          (T*)mem_alloc(a,      sizeof(T)*(c), alignof(T))
#define push_array_zero(a, T, c)     (T*)mem_alloc_zero(a, sizeof(T)*(c), alignof(T))

////////////////////////////////////////////////////////////////////////
// General GPU allocator (segregated pow2)

typedef u32 GpuMemHandler;

struct GpuAllocSegList {
  u64 pos;
  u64 cap;
  Allocator allocator;
  struct RangeList {
    u32 next;
    b32 is_allocated;
    Range range;
  };
  RangeList* data;
  u32 range_count;
  u32 range_cap;
  u32 heads[32];
  void init(Allocator alloc_);
  GpuMemHandler alloc(u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
  void free(u32 idx);
  u64 get(u32 idx);
};

////////////////////////////////////////////////////////////////////////
// Utils

struct SoA_Field {
  void** dst_ptr;
  u32 elem_size;
  u32 align;
};
#define SoA_push_field(ptr,T) {(void**)(ptr), sizeof(T), alignof(T)}

KAPI u8* mem_alloc_soa(Allocator alloc, u32 count, SoA_Field* fields, u32 fields_count);
KAPI u8* mem_realloc_soa(Allocator alloc, void* ptr, u32 old_count, u32 new_count, SoA_Field* fields, u32 fields_count);

struct OffsetMemPusher {
  void* offset;
  void* push(u64 size, u64 align);
};
struct OffsetPusher {
  u64 offset;
  u64 push(u64 size, u64 align);
};
#define offset_push_struct(a, T) (T*)a.push(sizeof(T), alignof(T))
#define offset_push_array(a, T, c) (T*)a.push(sizeof(T)*(c), alignof(T))


