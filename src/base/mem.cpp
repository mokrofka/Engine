#include "base_impl.h"

const u32 MEM_ALLOC_HEADER_GUARD   = 0xA110C8;
const u32 MEM_DEALLOC_HEADER_GUARD = 0xDE1E7E;
const u32 MEM_ALLOC_GUARD          = 0xA1;
const u32 MEM_DEALLOC_GUARD        = 0xDE;
const u32 MEM_ALLOC_TAIL_GUARD     = 0xdeedbeef;

#define MEM_GUARD 1

#if MEM_GUARD
  #define MemGuardAlloc(d, z)   MemSet(d, MEM_ALLOC_GUARD, z)
  #define MemGuardDealloc(d, z) MemSet(d, MEM_DEALLOC_GUARD, z)
#else
  #define MemGuardAlloc(d, c)
  #define MemGuardDealloc(d, c)
#endif

const u32 ARENA_DEFAULT_RESERVE_SIZE = MB(64);
const u32 ARENA_DEFAULT_COMMIT_SIZE  = KB(4);
const u32 ARENA_LIST_BLOCK_SIZE      = KB(64);

////////////////////////////////////////////////////////////////////////
// Mem track

#if MEM_TRACK

const u32 MaxAllocators = 128;

struct MemState {
  Arena arena;
  Mutex mutex;
  AllocatorInfoList roots;

  AllocatorInfo* free;
  AllocatorInfoList list;
  Array<AllocatorInfo, MaxAllocators> infos;
  u32 count_alloc;
};

global MemState mem_track;

void mem_track_init() {
  mem_track.mutex = os_mutex_make();
}

AllocatorInfo* mem_track_get_info(Allocator alloc) {
  switch (alloc.type) {
    default: return null;
    case AllocatorType_Arena: return ((Arena*)alloc.ctx)->info;
    case AllocatorType_Alloc: return ((Alloc*)alloc.ctx)->info;
  }
}

AllocatorInfo* mem_track_register(Allocator parent_alloc, AllocatorType type, String name) {
  AllocatorInfo* info = mem_track.free;
  AllocatorInfo* parent = mem_track_get_info(parent_alloc);
  if (info) {
    sll_stack_pop(mem_track.free);
  } else {
    info = &mem_track.infos[mem_track.count_alloc++];
  }
  MemZeroStruct(info);
  
  info->type = type;
  info->thread_idx = tctx_get_idx();
  str_copy(info->name, name);
  info->parent = parent;

  LockScope(mem_track.mutex);
  if (parent) {
    dll_push_back(parent->first, parent->last, info);
    ++parent->child_count;
  } else {
    dll_push_back(mem_track.roots.first, mem_track.roots.last, info);
    ++mem_track.roots.count;
  }
  return info;
}

void mem_track_unregister(Allocator alloc) {
  AllocatorInfo* info = mem_track_get_info(alloc);
  LockScope(mem_track.mutex);
  if (info->parent) {
    dll_remove(info->parent->first, info->parent->last, info);
    --info->parent->child_count;
  } else {
    dll_remove(mem_track.roots.first, mem_track.roots.last, info);
    --mem_track.roots.count;
  }
}

void mem_track_on_alloc(AllocatorInfo* info, u64 size) {
  atomic_add(&info->pos, size);
  atomic_add(&info->exclusive_pos, size);
  atomic_inc(&info->allocs);
  atomic_inc(&info->current_allocs);
  if (info->parent) {
    atomic_add(&info->parent->pos, size);
  }
}

void mem_track_on_free(AllocatorInfo* info, u64 size) {
  atomic_sub(&info->pos, size);
  atomic_sub(&info->exclusive_pos, size);
  atomic_inc(&info->frees);
  atomic_dec(&info->current_allocs);
  if (info->parent) {
    atomic_sub(&info->parent->pos, size);
  }
}

void mem_track_on_commit(AllocatorInfo* info, u64 size) {
  atomic_add(&info->cap, size);
}

AllocatorInfoList mem_track_info() {
  return mem_track.roots;
}

#else
AllocatorInfoList mem_track_info() { return {}; }
void mem_track_init() {}
#endif

////////////////////////////////////////////////////////////////////////
// Arena

Arena::operator Allocator() { return {.type = AllocatorType_Arena, .ctx = this}; }

Arena arena_make_named(String name) {
  return arena_make_(name);
}

Arena arena_make_(String name) {
  u64 reserve_size = ARENA_DEFAULT_RESERVE_SIZE;
  u8* base = os_reserve(reserve_size);
  Arena result = {
    .base = base,
    .cap = reserve_size,
  };
#if MEM_TRACK
  result.info = mem_track_register({}, AllocatorType_Arena, name);
#endif
  return result;
}

void arena_destroy(Arena& arena) {
  os_release(arena.base, arena.cap);
#if MEM_TRACK
  // TODO: traverse tree
  mem_track_unregister(arena);
#endif
}

void arena_clear(Arena& arena) { 
  arena.pos = 0;
  AsanPoisonMemRegion(arena.base, arena.cmt);
#if MEM_TRACK
  // TODO: traverse tree
#endif
};

intern u8* arena_alloc(Arena* arena, u64 size, u64 align) {
  u64 pos = AlignUp(arena->pos, align);
  u64 pad = pos - arena->pos;
  if (pos + size > arena->cmt) {
    u64 commit_size = AlignUp(pad + size, ARENA_DEFAULT_COMMIT_SIZE);
    Assert((pos + commit_size) <= arena->cap && "Arena is out of memory");
    os_commit(Offset(arena->base, arena->cmt), commit_size);
    MemGuardDealloc(Offset(arena->base, arena->cmt), commit_size);
    AsanPoisonMemRegion(Offset(arena->base, arena->cmt), commit_size);
    arena->cmt += commit_size;

#if MEM_TRACK
    mem_track_on_commit(arena->info, commit_size);
#endif
  }
  AsanUnpoisonMemRegion(Offset(arena->base, arena->pos), size + pad);
  MemGuardAlloc(Offset(arena->base, arena->pos), size + pad);
  u8* result = Offset(arena->base, pos);
  arena->pos = pos + size;
#if MEM_TRACK
  mem_track_on_alloc(arena->info, size);
#endif
  return result;
}

intern u8* arena_alloc_zero(Arena* arena, u64 size, u64 align) {
  u8* result = arena_alloc(arena, size, align);
  MemZero(result, size);
  return result;
}

intern u8* arena_realloc(Arena* arena, void* ptr, u64 old_size, u64 new_size, u64 align) {
  u8* result = arena_alloc(arena, new_size, align);
  MemCopy(result, ptr, old_size);
  return result;
}

intern u8* arena_realloc_zero(Arena* arena, void* ptr, u64 old_size, u64 new_size, u64 align) {
  u8* result = arena_alloc(arena, new_size, align);
  MemCopy(result, ptr, old_size);
  MemZero(Offset(result, old_size), new_size - old_size);
  return result;
}

Temp temp_begin(Arena* arena) {
#if MEM_TRACK
  return Temp{arena, arena->pos, arena->info->exclusive_pos};
#endif
  return Temp{arena, arena->pos};
};
void temp_end(Temp temp) {
  temp.arena->pos = temp.pos;
#if MEM_TRACK
  temp.arena->info->pos = temp.arena->pos;
  temp.arena->info->exclusive_pos = temp.temp_exclusive_pos;
#endif
}

////////////////////////////////////////////////////////////////////////
// ArenaList

ArenaList::operator Allocator() { return {.type = AllocatorType_ArenaList, .ctx = this}; }

ArenaList alloc_arena_list_make(Allocator alloc) {
  ArenaList res = {
    .alloc = alloc,
  };
  return res;
}

void alloc_arena_list_clear(ArenaList& arena) {
  for (ArenaBlock* b = arena.first;; b = b->next) {
    u8* base = Offset(b, sizeof(ArenaBlock));
    MemGuardDealloc(base, b->pos);
    AsanPoisonMemRegion(base, ARENA_LIST_BLOCK_SIZE);
    b->pos = 0;
    if (b == arena.current)
      break;
  }
  arena.current = arena.first;
}

intern ArenaBlock* arena_list_new_block(ArenaList* arena) {
  ArenaBlock* b = (ArenaBlock*)mem_alloc(arena->alloc, sizeof(ArenaBlock) + ARENA_LIST_BLOCK_SIZE);
  *b = {
    .cap = ARENA_LIST_BLOCK_SIZE,
  };
  u8* base = Offset(b, sizeof(ArenaBlock));
  MemGuardDealloc(base, ARENA_LIST_BLOCK_SIZE);
  AsanPoisonMemRegion(base, ARENA_LIST_BLOCK_SIZE);
  return b;
}

intern u8* arena_list_alloc(ArenaList* arena, u64 size, u64 align) {
  Assert(AlignUp(size, align) <= KB(64));
  if (!arena->current) {
    arena->first = arena->current = arena_list_new_block(arena);
  }
  u8* base = Offset(arena->current, sizeof(ArenaBlock));
  u64 pos = AlignUp((u64)base + arena->current->pos, align) - (u64)base;
  if (pos + size > arena->current->cap) {
    if (arena->current->next) {
      arena->current = arena->current->next;
    } else {
      arena->current = arena->current->next = arena_list_new_block(arena);
    }
    base = Offset(arena->current, sizeof(ArenaBlock));
    pos = AlignUp((u64)base, align) - (u64)base;
  }
  u64 pad = pos - arena->current->pos;
  AsanUnpoisonMemRegion(Offset(base, arena->current->pos), size + pad);
  MemGuardAlloc(Offset(base, arena->current->pos), size + pad);
  u8* result = Offset(base, pos);
  arena->current->pos = pos + size;
  return result;
}

intern u8* arena_list_alloc_zero(ArenaList* arena, u64 size, u64 align) {
  u8* result = arena_list_alloc(arena, size, align);
  MemZero(result, size);
  return result;
}

intern u8* arena_list_realloc(ArenaList* arena, void* ptr, u64 old_size, u64 new_size, u64 align) {
  u8* result = arena_list_alloc(arena, new_size, align);
  MemCopy(result, ptr, old_size);
  return result;
}

intern u8* arena_list_realloc_zero(ArenaList* arena, void* ptr, u64 old_size, u64 new_size, u64 align) {
  u8* result = arena_list_alloc(arena, new_size, align);
  MemCopy(result, ptr, old_size);
  MemZero(Offset(result, old_size), new_size - old_size);
  return result;
}

////////////////////////////////////////////////////////////////////////
// Segregated pow2 list

struct AllocHeader {
#if BUILD_DEBUG
  u32 head_guard;
  u32 size;
#endif
  u32 off;
};

Alloc::operator Allocator() { return {.type = AllocatorType_Alloc, .ctx = this};}

Alloc alloc_make(Allocator alloc) {
  Alloc res = {
    .alloc = alloc,
  };
#if MEM_TRACK
  res.info = mem_track_register(alloc, AllocatorType_Alloc, {});
#endif
  return res;
}

void alloc_destroy(Allocator alloc) {
#if MEM_TRACK
  mem_track_unregister(alloc);
#endif
}

u8* intern_alloc(Alloc* alloc, u64 size) {
  Assert(size > 7);
  u64 pow2_size = next_pow2(size);
  u64 pool_idx = ctz(pow2_size) - ctz(8);
  MemNode* p = alloc->pools[pool_idx].next;
  if (p == null) {
    u8* buf = mem_alloc(alloc->alloc, pow2_size);
#if MEM_TRACK
    mem_track_on_commit(alloc->info, pow2_size);
    mem_track_on_alloc(alloc->info, pow2_size);
#endif
    return buf;
  }
  sll_stack_pop(alloc->pools[pool_idx].next);
  AsanUnpoisonMemRegion(p, pow2_size);
  MemGuardAlloc(p, pow2_size);
#if MEM_TRACK
  mem_track_on_alloc(alloc->info, size);
#endif
  return (u8*)p;
}

void intern_free(Alloc* alloc, void* ptr, u64 size) {
  Assert(size > 7);
  u64 pow2_size = next_pow2(size);
  u64 pool_idx = ctz(pow2_size) - ctz(8);
  MemNode* p = (MemNode*)ptr;
  sll_stack_push(alloc->pools[pool_idx].next, p);
  MemGuardDealloc(Offset(p, sizeof(void*)), pow2_size - sizeof(void*));
  AsanPoisonMemRegion(Offset(p, sizeof(void*)), pow2_size - sizeof(void*));
#if MEM_TRACK
  mem_track_on_free(alloc->info, size);
#endif
}

// mem block: align_pad -> header -> mem
u8* intern_alloc_align(Alloc* alloc, u64 size, u64 align) {
  u64 alloc_size = align+sizeof(AllocHeader)+size;
  u8* raw = intern_alloc(alloc, alloc_size);
  u8* user = PtrAlignUp(raw+sizeof(AllocHeader), align);
  AllocHeader* h = OffsetBackStruct(user, AllocHeader);
  h->off = user - raw;
  return user;
}

void intern_free_align(Alloc* alloc, void*ptr, u64 size) {
  AllocHeader* h = OffsetBackStruct(ptr, AllocHeader);
  u8* raw = OffsetBack(ptr, h->off);
  u64 raw_size = size + PtrDiff(ptr, raw);
  intern_free(alloc, raw, raw_size);
}

// mem block: align_pad -> header -> mem -> u32 tail guard
u8* alloc_alloc(Alloc* alloc, u64 size, u64 align) {
  u64 alloc_size = size + sizeof(u32);
  u8* res = intern_alloc_align(alloc, alloc_size, align);
  AllocHeader* h = OffsetBackStruct(res, AllocHeader);
  h->head_guard = MEM_ALLOC_HEADER_GUARD;
  h->size = size;
  *OffsetAs(res, u32, size) = MEM_ALLOC_TAIL_GUARD;
  return res;
}

u8* alloc_alloc_zero(Alloc* alloc, u64 size, u64 align)  {
  u8* result = alloc_alloc(alloc, size, align);
  MemZero(result, size);
  return result;
}

void alloc_free(Alloc* alloc, void* ptr, u64 size) {
  AllocHeader* h = OffsetBackStruct(ptr, AllocHeader);
  Assert(h->head_guard == MEM_ALLOC_HEADER_GUARD);
  Assert(h->size == size);
  u32* tail = OffsetAs(ptr, u32, size);
  Assert(*tail == MEM_ALLOC_TAIL_GUARD);
  h->head_guard = MEM_DEALLOC_HEADER_GUARD;
  intern_free_align(alloc, ptr, size + sizeof(u32));
}

u8* alloc_realloc(Alloc* alloc, void* ptr, u64 old_size, u64 new_size, u64 align) {
  u8* res = alloc_alloc(alloc, new_size, align);
  MemCopy(res, ptr, old_size);
  alloc_free(alloc, ptr, old_size);
  return res;
}

u8* alloc_realloc_zero(Alloc* alloc, void* ptr, u64 old_size, u64 new_size, u64 align) {
  u8* res = alloc_alloc(alloc, new_size, align);
  MemCopy(res, ptr, old_size);
  MemZero(Offset(res, old_size), new_size - old_size);
  alloc_free(alloc, ptr, old_size);
  return res;
}

////////////////////////////////////////////////////////////////////////
// tlsf

// TLSF_Allocator tlsf_init() {
//   return {};
// }

// struct BinmapInfo {
//   u32 bin_idx;
//   u32 sub_bin_idx;
//   u64 rounded_size;
// };

// struct BlockMap {
//   u32 bin_idx;
//   u32 sub_bin_idx;
//   u64 rounded_size;
//   u32 idx;
// };

// intern BinmapInfo binmap_up(u64 size) {
//   u64 bin_idx = most_significant_bit(size | MinAllocation);
//   u64 subbin_size = 1 << (bin_idx - ctz(SubbinCount));
//   u64 rounded_size = AlignUp(size, subbin_size);
//   u32 sub_bin_idx = (rounded_size - (1 << bin_idx)) / subbin_size;

//   u32 adjusted_bin_idx = (bin_idx - ctz(MinAllocation)) + (sub_bin_idx / SubbinCount);
//   u32 adjusted_subbin_idx = sub_bin_idx % SubbinCount;

//   return {
//     .bin_idx = adjusted_bin_idx,
//     .sub_bin_idx = adjusted_subbin_idx,
//     .rounded_size = rounded_size,
//   };
// }

// intern BlockMap tlsf_find_free_block(TLSF_Allocator& a, u64 size) {
//   BinmapInfo map = binmap_up(size);
//   u32 sub_bin_bitmap = a.sub_bin_bitmaps[map.bin_idx] & (~0 << map.sub_bin_idx);
//   if (sub_bin_bitmap == 0) {
//     u32 bin_bitmap = a.bin_bitmap & (~0 << (map.bin_idx + 1));
//     if (bin_bitmap == 0) return {};
//     map.bin_idx = ctz(bin_bitmap);
//     sub_bin_bitmap = a.sub_bin_bitmaps[map.sub_bin_idx];
//   }

//   Assert(sub_bin_bitmap != 0);
//   map.sub_bin_idx = ctz(sub_bin_bitmap);
//   u32 idx = map.bin_idx * SubbinCount + map.sub_bin_idx;

//   return {
//     .bin_idx = map.bin_idx,
//     .sub_bin_idx = map.sub_bin_idx,
//     .rounded_size = map.rounded_size,
//     .idx = idx,
//   };
// }

// u8* tlsf_alloc(TLSF_Allocator& a, u64 size, u64 alignment) {
//   BlockMap block_map = tlsf_find_free_block(a, size);

//   return {};
// }

////////////////////////////////////////////////////////////////////////
// Atlas allocator

////////////////////////////////////////////////////////////////////////
// Allocator Interface

u8* mem_alloc(Allocator alloc, u64 size, u64 align) {
  switch (alloc.type) {
    case AllocatorType_None: InvalidPath;
    // case AllocatorType_Global:    return global_alloc(size, align);
    case AllocatorType_Arena:     return arena_alloc((Arena*)alloc.ctx, size, align);
    case AllocatorType_ArenaList: return arena_list_alloc((ArenaList*)alloc.ctx, size, align);
    case AllocatorType_Alloc:     return alloc_alloc((Alloc*)alloc.ctx, size, align);
  }
}
u8* mem_alloc_zero(Allocator alloc, u64 size, u64 align) {
  switch (alloc.type) {
    case AllocatorType_None: InvalidPath;
    // case AllocatorType_Global:    return global_alloc_zero(size, align);
    case AllocatorType_Arena:     return arena_alloc_zero((Arena*)alloc.ctx, size, align);
    case AllocatorType_ArenaList: return arena_list_alloc((ArenaList*)alloc.ctx, size, align);
    case AllocatorType_Alloc:     return alloc_alloc_zero((Alloc*)alloc.ctx, size, align);
  }
}
u8* mem_realloc(Allocator alloc, void* ptr, u64 old_size, u64 new_size, u64 align) {
  switch (alloc.type) {
    case AllocatorType_None: InvalidPath;
    // case AllocatorType_Global:    return global_realloc(ptr, old_size, new_size, align);
    case AllocatorType_Arena:     return arena_realloc((Arena*)alloc.ctx, ptr, old_size, new_size, align);
    case AllocatorType_ArenaList: return arena_list_realloc((ArenaList*)alloc.ctx, ptr, old_size, new_size, align);
    case AllocatorType_Alloc:     return alloc_realloc((Alloc*)alloc.ctx, ptr, old_size, new_size, align);
  }
}
u8* mem_realloc_zero(Allocator alloc, void* ptr, u64 old_size, u64 new_size, u64 align) {
  switch (alloc.type) {
    case AllocatorType_None: InvalidPath;
    // case AllocatorType_Global:    return global_realloc_zero(ptr, old_size, new_size, align);
    case AllocatorType_Arena:     return arena_realloc_zero((Arena*)alloc.ctx, ptr, old_size, new_size, align);
    case AllocatorType_ArenaList: return arena_list_realloc_zero((ArenaList*)alloc.ctx, ptr, old_size, new_size, align);
    case AllocatorType_Alloc:     return alloc_realloc_zero((Alloc*)alloc.ctx, ptr, old_size, new_size, align);
  }
}
void mem_free(Allocator alloc, void* ptr, u64 size) {
  switch (alloc.type) {
    case AllocatorType_None: InvalidPath;
    // case AllocatorType_Global:    return global_free(ptr, size);
    case AllocatorType_Arena:     return;
    case AllocatorType_ArenaList: return;
    case AllocatorType_Alloc:     return alloc_free((Alloc*)alloc.ctx, ptr, size);
  }
}

////////////////////////////////////////////////////////////////////////
// General GPU allocator (segregated pow2)

GpuAllocSegList gpu_alloc_seglist_make(Allocator alloc) {
  GpuAllocSegList res = {
    .alloc = alloc,
  };
  res.range_cap = 8,
  res.data = push_array(alloc, GpuBlockList, res.range_cap);
  return res;
}
GpuMemId gpu_alloc_seglist_alloc(GpuAllocSegList& a, u64 size, u64 align) {
  Assert(size > 0);
  u64 alloc_size = align + size;
  u64 pow2_size = next_pow2(alloc_size);
  u64 pool_idx = ctz(pow2_size) - ctz(8);
  u32& p = a.heads[pool_idx];
  if (p == 0) {
    u64 cur_pos = AlignUp(a.pos, align);
    a.pos = cur_pos + pow2_size;
    GpuBlockList range = {0, true, cur_pos, alloc_size};
    u32 result = a.range_count;
    if (a.range_count >= a.range_cap) {
      a.range_cap *= 2;
      mem_realloc_array(a.alloc, a.data, a.range_count, a.range_cap) ;
    }
    a.data[a.range_count++] = range;
    return result;
  }
  u32 result = p;
  p = a.data[p].next;
  return result;
}
void gpu_alloc_seglist_free(GpuAllocSegList& a, GpuMemId h) {
  GpuBlockList& range = a.data[h];
  Assert(range.is_allocated == true);
  u64 pow2_size = next_pow2(range.range.size);
  u64 pool_idx = ctz(pow2_size) - ctz(8);
  u32& p = a.heads[pool_idx];
  range.next = p;
  p = h;
}

u64 gpu_alloc_seglist_get(GpuAllocSegList& a, GpuMemId h) {
  return a.data[h].range.offset;
}

////////////////////////////////////////////////////////////////////////
// Utils

struct SoALayout {
  u64 size;
  u64 offsets[8];
};

u64 mem_soa_size(u32 count, Slice<SoA_Field> fields) {
  u64 off = 0;
  Loop (i, fields.count) {
    SoA_Field field = fields[i];
    offset_push(off, field.elem_size*count, field.align);
  }
  return off;
}

SoALayout mem_soa_layout(u32 count, Slice<SoA_Field> fields) {
  SoALayout res = {};
  Loop (i, fields.count) {
    SoA_Field field = fields[i];
    res.offsets[i] = offset_push(res.size, field.elem_size*count, field.align);
  }
  return res;
}

u8* mem_alloc_soa(Allocator alloc, u32 count, Slice<SoA_Field> fields) {
  SoALayout layout = mem_soa_layout(count, fields);
  u8* buf = mem_alloc(alloc, layout.size, fields[0].align);
  Loop (i, fields.count) {
    *(fields[i].dst_ptr) = Offset(buf, layout.offsets[i]);
  }
  return buf;
}

u8* mem_realloc_soa(Allocator alloc, u32 old_count, u32 new_count, Slice<SoA_Field> fields) {
  void* old_ptr = *fields[0].dst_ptr;
  SoALayout layout = mem_soa_layout(new_count, fields);
  u8* buf = mem_alloc(alloc, layout.size, fields[0].align);
  Loop (i, fields.count) {
    void* old_ptr = *(fields[i].dst_ptr);
    void* new_ptr = Offset(buf, layout.offsets[i]);
    u64 old_ptr_size = fields[i].elem_size * old_count;
    MemCopy(new_ptr, old_ptr, old_ptr_size);
    *(fields[i].dst_ptr) = new_ptr;
  }
  mem_free(alloc, old_ptr, mem_soa_size(old_count, fields));
  return buf;
}

u8* mem_alloc_soa_zero(Allocator alloc, u32 count, Slice<SoA_Field> fields) {
  SoALayout layout = mem_soa_layout(count, fields);
  u8* buf = mem_alloc_zero(alloc, layout.size, fields[0].align);
  Loop (i, fields.count) {
    *(fields[i].dst_ptr) = Offset(buf, layout.offsets[i]);
  }
  return buf;
}

u8* mem_realloc_soa_zero(Allocator alloc, u32 old_count, u32 new_count, Slice<SoA_Field> fields) {
  void* old_ptr = *fields[0].dst_ptr;
  SoALayout layout = mem_soa_layout(new_count, fields);
  u8* buf = mem_alloc_zero(alloc, layout.size, fields[0].align);
  u64 old_size = mem_soa_size(old_count, fields);
  MemZero(Offset(buf, old_size), layout.size - old_size);
  Loop (i, fields.count) {
    void* old_ptr = *(fields[i].dst_ptr);
    void* new_ptr = Offset(buf, layout.offsets[i]);
    u64 old_ptr_size = fields[i].elem_size * old_count;
    MemCopy(new_ptr, old_ptr, old_ptr_size);
    *(fields[i].dst_ptr) = new_ptr;
  }
  mem_free(alloc, old_ptr, old_size);
  return buf;
}

u8* offset_ptr_push(void*& offset, u64 size, u64 align) {
  u8* result = PtrAlignUp(offset, align);
  offset = Offset(result, size);
  return result;
}

u64 offset_push(u64& offset, u64 size, u64 align) {
  u64 result = AlignUp(offset, align);
  offset = result + size;
  return result;
}

global String mem_units[] = {"B", "KB", "MB", "GB", "TB"};
MemFormatSize mem_format_size(f32 value) {
  u32 unit = 0;
  while (value >= 1024) {
    value /= 1024;
    ++unit;
  }
  MemFormatSize result = {
    .format = mem_units[unit],
    .format_id = unit,
    .size = value,
  };
  return result;
}

