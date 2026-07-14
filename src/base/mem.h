#pragma once
#include "base.h"

#define MEM_TRACK 1

const u32 MEM_DEFAULT_ALIGNMENT = sizeof(void*);

enum AllocatorType {
  AllocatorType_None,
  AllocatorType_Arena,
  AllocatorType_ArenaList,
  AllocatorType_Alloc,
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
void mem_free(Allocator alloc, void* ptr, u64 size);

template<typename T> T* mem_realloc_array(Allocator a, T* ptr, u32 old_c, u32 c)      { return (T*)mem_realloc(a, ptr, sizeof(T)*old_c, sizeof(T)*c, alignof(T)); }
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
#define push_slice_zero(a, T, c) Slice(push_array_zero(a, T, c), c)
#define push_buffer_slice(a, z) Slice(push_buffer(a, z), z)

////////////////////////////////////////////////////////////////////////
// Mem track

struct AllocatorInfo {
  AllocatorInfo* first;
  AllocatorInfo* last;
  AllocatorInfo* next;
  AllocatorInfo* prev;
  AllocatorInfo* parent;
  u32 child_count;

  String64 name;
  AllocatorType type;
  u32 thread_idx;

  u64 pos;
  u64 exclusive_pos;
  u64 cap;
  u64 res;
  u64 allocs;
  u64 frees;
  u64 current_allocs;
  u64 allocs_per_frame;
};

struct AllocatorInfoList {
  AllocatorInfo* first;
  AllocatorInfo* last;
  u32 count;
};

AllocatorInfoList mem_track_info();
void mem_track_init();

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

#define arena_make(...) arena_make_(__func__)
Arena arena_make_named(String name);
Arena arena_make_(String name);
void  arena_destroy(Arena& arena);
void  arena_clear(Arena& arena);

struct Temp {
  Arena* arena;
  u64 pos;

#if MEM_TRACK
  u64 temp_exclusive_pos;
#endif
};

Temp temp_begin(Arena* arena);
void temp_end(Temp temp);

struct Scratch {
  Temp temp;
  NO_DEBUG operator Allocator();
  Scratch();
  NO_DEBUG Scratch(Allocator conflict);
  NO_DEBUG ~Scratch();
};

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
  operator Allocator();
};

ArenaList alloc_arena_list_make(Allocator alloc);
void alloc_arena_list_clear(ArenaList& arena);

////////////////////////////////////////////////////////////////////////
// Segregated pow2 list

struct MemNode {
  MemNode* next;
};

struct Alloc {
#if MEM_TRACK
  AllocatorInfo* info;
#endif
  Allocator alloc;
  MemNode pools[32];
  operator Allocator();
};

Alloc alloc_make(Allocator alloc);
void alloc_destroy(Allocator alloc);

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
// GPU segregated pow2 list

typedef u32 GpuMemId;

struct GpuBlockList {
  u32 next;
  b32 is_allocated;
  Region range;
};

struct GpuAllocSegList {
  u64 pos;
  u64 cap;
  Allocator alloc;
  GpuBlockList* data;
  u32 range_count;
  u32 range_cap;
  u32 heads[32];
};

GpuAllocSegList gpu_alloc_seglist_make(Allocator alloc);
GpuMemId gpu_alloc_seglist_alloc(GpuAllocSegList& a, u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
void gpu_alloc_seglist_free(GpuAllocSegList& a, GpuMemId h);
u64 gpu_alloc_seglist_get(GpuAllocSegList& a, GpuMemId h);

////////////////////////////////////////////////////////////////////////
// Misc

struct SoA_Field {
  void** dst_ptr;
  u32 elem_size;
  u32 align;
};
#define SoA_push_field(ptr) {(void**)(&ptr), sizeof(*ptr), alignof(*ptr)}

u64 mem_soa_size(u32 count, Slice<SoA_Field> fields);
u8* mem_alloc_soa(Allocator alloc, u32 count, Slice<SoA_Field> fields);
u8* mem_realloc_soa(Allocator alloc, u32 old_count, u32 new_count, Slice<SoA_Field> fields);
u8* mem_alloc_soa_zero(Allocator alloc, u32 count, Slice<SoA_Field> fields);
u8* mem_realloc_soa_zero(Allocator alloc, u32 old_count, u32 new_count, Slice<SoA_Field> fields);

u8* offset_ptr_push(void*& offset, u64 size, u64 align = 1);
#define offset_ptr_push_struct(a, T)  (T*)offset_ptr_push(a, sizeof(T), alignof(T))
#define offset_ptr_push_array(a, T, c)(T*)offset_ptr_push(a, sizeof(T)*(c), alignof(T))
u64 offset_push(u64& offset, u64 size, u64 align = 1);
#define offset_push_struct(a, T)          offset_push(a, sizeof(T), alignof(T))
#define offset_push_array(a, T, c)        offset_push(a, sizeof(T)*(c), alignof(T))

struct MemFormatSize {
  String format;
  u32 format_id;
  f32 size;
};
MemFormatSize mem_format_size(f32 value);

