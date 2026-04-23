#pragma once
#include "base.h"

#define MEM_TRACK 1

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
// Allocator Interface

u8*  mem_alloc(Allocator alloc, u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
u8*  mem_alloc_zero(Allocator alloc, u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
u8*  mem_realloc(Allocator alloc, void* ptr, u64 old_size, u64 new_size, u64 align = MEM_DEFAULT_ALIGNMENT);
u8*  mem_realloc_zero(Allocator alloc, void* ptr, u64 old_size, u64 new_size, u64 align = MEM_DEFAULT_ALIGNMENT);
void mem_free(Allocator alloc, void* ptr);

template<typename T> T* mem_realloc_array(Allocator a, T* ptr, u32 old_c, u32 c)      { return (T*)mem_realloc(a, ptr,      sizeof(T)*old_c, sizeof(T)*c, alignof(T)); }
template<typename T> T* mem_realloc_array_zero(Allocator a, T* ptr, u32 old_c, u32 c) { return (T*)mem_realloc_zero(a, ptr, sizeof(T)*old_c, sizeof(T)*c, alignof(T)); }

#define push_buffer(a, z, ...)           mem_alloc(a,      z, ##__VA_ARGS__)
#define push_buffer_zero(a, z, ...)      mem_alloc_zero(a, z, ##__VA_ARGS__)
#define push_struct(a, T)            (T*)mem_alloc(a,      sizeof(T),     alignof(T))
#define push_struct_zero(a, T)       (T*)mem_alloc_zero(a, sizeof(T),     alignof(T))
#define push_array(a, T, c)          (T*)mem_alloc(a,      sizeof(T)*(c), alignof(T))
#define push_array_zero(a, T, c)     (T*)mem_alloc_zero(a, sizeof(T)*(c), alignof(T))

template<typename T> Slice<T> slice_free(Allocator alloc, Slice<T> slice) {
  mem_free(alloc, slice.data);
}
template<typename T> Slice<T> slice_clone(Allocator alloc, Slice<T> slice) {
  T* data = push_array(alloc, T, slice.count);
  MemCopyArray(data, slice.data, slice.count);
  return {data, slice.count};
}
#define push_slice(a, T, c) Slice(push_array(a, T, c), c)

////////////////////////////////////////////////////////////////////////
// Mem track

struct AllocatorInfo {
  AllocatorType type;
  AllocatorInfo* first;
  AllocatorInfo* last;
  AllocatorInfo* next;
  AllocatorInfo* prev;
  AllocatorInfo* parent;
  u64 exclusive_pos;
  u64 pos;
  u64 cmt;
  u64 cap;
  u64 res;
  u64 temp_pos;
  u64 temp_exclusive_pos;
  String name;
};

struct AllocatorInfoList {
  AllocatorInfo* first;
  AllocatorInfo* last;
  u32 count;
};

AllocatorInfoList get_allocators_info();

////////////////////////////////////////////////////////////////////////
// Global allocator

void global_allocator_init();

////////////////////////////////////////////////////////////////////////
// Arena (page allocator)

struct Arena {
#if MEM_TRACK
  AllocatorInfo* info;
#endif
  u8* base;
  u64 pos;
  u64 cmt;
  u64 cap;
  operator Allocator();
};

#define arena_init(...) arena_init_(__func__)
Arena arena_init_named(String name);
Arena arena_init_(String name);
void  arena_deinit(Arena* arena);
void  arena_clear(Arena* arena);

struct Temp {
  Arena* arena;
  u64 pos;
};

Temp temp_begin(Arena* arena);
void temp_end(Temp temp);

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
// Segregated pow2 list

struct AllocSegList {
#if MEM_TRACK
  AllocatorInfo* info;
#endif
  Allocator alloc;
  u8* pools[32];
  AllocSegList() = default;
  AllocSegList(Allocator alloc_);
  void init(Allocator alloc_, String name = {});
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
// u8* tlsf_alloc(TLSF_Allocator& allocator, u64 size, u64 alignment = MEM_DEFAULT_ALIGNMENT);

////////////////////////////////////////////////////////////////////////
// atlas TODO: implement

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
    BufferRegion range;
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

u8* mem_alloc_soa(Allocator alloc, u32 count, Slice<SoA_Field> fields);
u8* mem_realloc_soa(Allocator alloc, u32 old_count, u32 new_count, Slice<SoA_Field> fields);

void* offset_pusher_mem_push(void*& offset, u64 size, u64 align);
u64 offset_pusher_push(u64& offset, u64 size, u64 align);
#define offset_mem_push_struct(a, T)  (T*)offset_pusher_mem_push(a, sizeof(T), alignof(T))
#define offset_mem_push_array(a, T, c)(T*)offset_pusher_mem_push(a, sizeof(T)*(c), alignof(T))
#define offset_push_struct(a, T)          offset_pusher_push(a, sizeof(T), alignof(T))
#define offset_push_array(a, T, c)        offset_pusher_push(a, sizeof(T)*(c), alignof(T))

struct MemFormatSize {
  String format;
  u32 format_id;
  f32 size;
};
MemFormatSize mem_format_size(f32 value);
