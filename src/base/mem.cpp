#include "mem.h"
#include "logger.h"
#include "os/os_core.h"

////////////////////////////////////////////////////////////////////////
// Global allocator (segregated pow2)

struct MemCtx {
  Arena arena;
  AllocSegList seglist;
};

global MemCtx ctx;

void global_alloc_init() {
  ctx.arena = arena_init();
  ctx.seglist.init(ctx.arena);
}

u8* global_alloc(u64 size, u64 align) {
  return seglist_alloc(&ctx.seglist, size, align);
}

u8* global_alloc_zero(u64 size, u64 align) {
  return seglist_alloc_zero(&ctx.seglist, size, align);
}

u8* global_realloc(void* ptr, u64 old_size, u64 new_size, u64 align) {
  return seglist_realloc(&ctx.seglist, ptr, old_size, new_size, align);
}

u8* global_realloc_zero(void* ptr, u64 old_size, u64 new_size, u64 align) {
  return seglist_realloc_zero(&ctx.seglist, ptr, old_size, new_size, align);
}

void global_free(void* ptr) {
  seglist_free(&ctx.seglist, ptr);
}

////////////////////////////////////////////////////////////////////////
// Arena

Arena arena_init() {
  u64 reserve_size = ARENA_DEFAULT_RESERVE_SIZE;

  u8* base = os_reserve(reserve_size);

  Arena result = {
    .base = base,
    .cap = reserve_size,
  };
  return result;
}

void arena_deinit(Arena* arena) {
  os_release(arena->base, arena->cap);
}

void arena_clear(Arena* arena) { 
  arena->pos = 0;
  AsanPoisonMemRegion(arena->base, arena->cmt);
};

u8* arena_alloc(Arena* arena, u64 size, u64 align) {
  u64 pos = AlignUp(arena->pos, align);
  u64 pad = pos - arena->pos;

  if (pos + size > arena->cmt) {
    u64 commit_size = AlignUp(pad + size, ARENA_DEFAULT_COMMIT_SIZE);
    
    Assert((pos + commit_size) <= arena->cap && "Arena is out of memory");

    os_commit(Offset(arena->base, arena->cmt), commit_size);
    MemGuardDealloc(Offset(arena->base, arena->cmt), commit_size);

    AsanPoisonMemRegion(Offset(arena->base, arena->cmt), commit_size);
    arena->cmt += commit_size;
  }

  AsanUnpoisonMemRegion(Offset(arena->base, arena->pos), size + pad);
  MemGuardAlloc(Offset(arena->base, arena->pos), size + pad);
  u8* result = Offset(arena->base, pos);
  arena->pos = pos + size;

  return result;
}

u8* arena_alloc_zero(Arena* arena, u64 size, u64 align) {
  u8* result = arena_alloc(arena, size, align);
  MemZero(result, size);
  return result;
}

u8* arena_realloc(Arena* arena, void* ptr, u64 old_size, u64 new_size, u64 align) {
  u8* result = arena_alloc(arena, new_size, align);
  MemCopy(result, ptr, old_size);
  return result;
}

u8* arena_realloc_zero(Arena* arena, void* ptr, u64 old_size, u64 new_size, u64 align) {
  u8* result = arena_alloc(arena, new_size, align);
  MemCopy(result, ptr, old_size);
  MemZero(Offset(result, old_size), new_size - old_size);
  return result;
}

////////////////////////////////////////////////////////////////////////
// ArenaList
#define ARENA_LIST_BLOCK_SIZE KB(64)

void ArenaList::clear() {
  for (ArenaBlock* b = first;; b = b->next) {
    u8* base = Offset(b, sizeof(ArenaBlock));
    MemGuardDealloc(base, b->pos);
    AsanPoisonMemRegion(base, ARENA_LIST_BLOCK_SIZE);
    b->pos = 0;
    if (b == current)
      break;
  }
  current = first;
};

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

u8* arena_list_alloc(ArenaList* arena, u64 size, u64 align) {
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

u8* arena_list_alloc_zero(ArenaList* arena, u64 size, u64 align) {
  u8* result = arena_list_alloc(arena, size, align);
  MemZero(result, size);
  return result;
}

u8* arena_list_realloc(ArenaList* arena, void* ptr, u64 old_size, u64 new_size, u64 align) {
  u8* result = arena_list_alloc(arena, new_size, align);
  MemCopy(result, ptr, old_size);
  return result;
}

u8* arena_list_realloc_zero(ArenaList* arena, void* ptr, u64 old_size, u64 new_size, u64 align) {
  u8* result = arena_list_alloc(arena, new_size, align);
  MemCopy(result, ptr, old_size);
  MemZero(Offset(result, old_size), new_size - old_size);
  return result;
}

////////////////////////////////////////////////////////////////////////
// General allocator (segregated pow2)

struct SegListHeader {
#if MEM_GUARD
  u64 size;
  u32 guard;
#endif
  u32 pad;
};

// NOTE: memory: header + pad -> u8* memory -> u32 tail_guard
u8* seglist_alloc(AllocSegList* alloc, u64 size, u64 align) {
  Assert(size > 0);

#if MEM_GUARD
  u64 alloc_size = sizeof(void*) + sizeof(SegListHeader) + align + size + sizeof(u32);
  
  u64 pow2_size = next_pow2(alloc_size);
  u64 pool_idx = ctz(pow2_size) - ctz(8);
  MemPoolPow2& p = alloc->pools[pool_idx];
  
  if (p.head == null) {
    u8* buff = mem_alloc(alloc->alloc, sizeof(u32) + pow2_size);
    *(u32*)buff = pool_idx;
    u8* result_no_aligned = Offset(buff+sizeof(u32)+sizeof(void*), sizeof(SegListHeader));
    u8* result = (u8*)AlignUp(((u64)result_no_aligned), align);
    SegListHeader* header = (SegListHeader*)OffsetBack(result, sizeof(SegListHeader));
    *header = {
      .size = size,
      .guard = MEM_ALLOC_HEADER_GUARD,
      .pad = (u32)MemDiff(result, buff),
    };
    u32* tail = (u32*)Offset(result, size);
    *tail = MEM_ALLOC_TAIL_GUARD;

    MemGuardDealloc(Offset(result, size+sizeof(u32)), pow2_size - alloc_size);
    AsanPoisonMemRegion(Offset(result, size+sizeof(u32)), pow2_size - alloc_size);
    return result;
  }

  PoolFreeNode* alloc_buff = p.head;
  p.head = p.head->next;
  AsanUnpoisonMemRegion(alloc_buff, alloc_size);

  u8* result_no_aligned = Offset(Offset(alloc_buff,sizeof(void*)), sizeof(SegListHeader));
  u8* result = (u8*)AlignUp(((u64)result_no_aligned), align);
  SegListHeader* header = (SegListHeader*)OffsetBack(result, sizeof(SegListHeader));
  *header = {
    .size = size,
    .guard = MEM_ALLOC_HEADER_GUARD,
    .pad = (u32)MemDiff(result, OffsetBack(alloc_buff, sizeof(u32))),
  };
  u32* tail = (u32*)Offset(result, size);
  *tail = MEM_ALLOC_TAIL_GUARD;
  MemGuardAlloc(result, size);
  return result;
  
#else
  size = size + alignment;
  u64 pow2_size = next_pow2(size);
  u64 pool_index = ctz64(pow2_size) - ctz64(8);
  MemPoolPow2& p = allocator.pools[pool_index];

  if (p.head == null) {
    u8* result = push_buffer(allocator.arena, pow2_size, alignment);
    *(u32*)result = pool_index;
    u32* index = (u32*)OffsetBack(result, sizeof(u32));
    *index = pool_index;
    return result;
  }
  PoolFreeNode* result = p.head;
  p.head = p.head->next;
#endif

};

u8* seglist_alloc_zero(AllocSegList* alloc, u64 size, u64 align) {
  u8* result = seglist_alloc(alloc, size, align);
  MemZero(result, size);
  return result;
}

u8* seglist_realloc(AllocSegList* alloc, void* ptr, u64 old_size, u64 new_size, u64 align) {
  u8* result = seglist_alloc(alloc, new_size, align);
  MemCopy(result, ptr, old_size);
  seglist_free(alloc, ptr);
  return result;
}

u8* seglist_realloc_zero(AllocSegList* alloc, void* ptr, u64 old_size, u64 new_size, u64 align) {
  u8* result = seglist_alloc(alloc, new_size, align);
  MemCopy(result, ptr, old_size);
  MemZero(Offset(result, old_size), new_size - old_size);
  seglist_free(alloc, ptr);
  return result;
}

void seglist_free(AllocSegList* allocator, void* ptr) {
  Assert(ptr);

#if MEM_GUARD
  SegListHeader* header = (SegListHeader*)OffsetBack(ptr, sizeof(SegListHeader));
  Assert(header->guard == MEM_ALLOC_HEADER_GUARD);
  header->guard = MEM_DEALLOC_HEADER_GUARD;
  u32* tail = (u32*)Offset(ptr, header->size);
  Assert(*tail == MEM_ALLOC_TAIL_GUARD);

  u32* pool_idx = (u32*)OffsetBack(ptr, header->pad);
  MemPoolPow2& p = allocator->pools[*pool_idx];

  u64 pow2_size = 8 << *pool_idx;
  MemGuardDealloc(ptr, header->size+sizeof(u32));
  AsanPoisonMemRegion(ptr, header->size+sizeof(u32));

  PoolFreeNode* node = (PoolFreeNode*)Offset(pool_idx, sizeof(u32));
	node->next = p.head;
	p.head = node;
#endif
}

////////////////////////////////////////////////////////////////////////
// tlsf

TLSF_Allocator tlsf_init() {
  return {};
}

struct BinmapInfo {
  u32 bin_idx;
  u32 sub_bin_idx;
  u64 rounded_size;
};

struct BlockMap {
  u32 bin_idx;
  u32 sub_bin_idx;
  u64 rounded_size;
  u32 idx;
};

intern BinmapInfo binmap_up(u64 size) {
  u64 bin_idx = most_significant_bit(size | MinAllocation);
  u64 subbin_size = 1 << (bin_idx - ctz(SubbinCount));
  u64 rounded_size = AlignUp(size, subbin_size);
  u32 sub_bin_idx = (rounded_size - (1 << bin_idx)) / subbin_size;

  u32 adjusted_bin_idx = (bin_idx - ctz(MinAllocation)) + (sub_bin_idx / SubbinCount);
  u32 adjusted_subbin_idx = sub_bin_idx % SubbinCount;

  return {
    .bin_idx = adjusted_bin_idx,
    .sub_bin_idx = adjusted_subbin_idx,
    .rounded_size = rounded_size,
  };
}

intern BlockMap tlsf_find_free_block(TLSF_Allocator& a, u64 size) {
  BinmapInfo map = binmap_up(size);
  u32 sub_bin_bitmap = a.sub_bin_bitmaps[map.bin_idx] & (~0 << map.sub_bin_idx);
  if (sub_bin_bitmap == 0) {
    u32 bin_bitmap = a.bin_bitmap & (~0 << (map.bin_idx + 1));
    if (bin_bitmap == 0) return {};
    map.bin_idx = ctz(bin_bitmap);
    sub_bin_bitmap = a.sub_bin_bitmaps[map.sub_bin_idx];
  }

  Assert(sub_bin_bitmap != 0);
  map.sub_bin_idx = ctz(sub_bin_bitmap);
  u32 idx = map.bin_idx * SubbinCount + map.sub_bin_idx;

  return {
    .bin_idx = map.bin_idx,
    .sub_bin_idx = map.sub_bin_idx,
    .rounded_size = map.rounded_size,
    .idx = idx,
  };
}

u8* tlsf_alloc(TLSF_Allocator& a, u64 size, u64 alignment) {
  BlockMap block_map = tlsf_find_free_block(a, size);

  return {};
}

////////////////////////////////////////////////////////////////////////
// Atlas allocator

////////////////////////////////////////////////////////////////////////
// Allocator Interface

u8* mem_alloc(Allocator alloc, u64 size, u64 align) {
  switch (alloc.type) {
    case AllocatorType_Global:    return global_alloc(size, align);
    case AllocatorType_Arena:     return arena_alloc((Arena*)alloc.ctx, size, align);
    case AllocatorType_ArenaList: return arena_list_alloc((ArenaList*)alloc.ctx, size, align);
    case AllocatorType_SegList:   return seglist_alloc((AllocSegList*)alloc.ctx, size, align);
  }
}
u8* mem_alloc_zero(Allocator alloc, u64 size, u64 align) {
  switch (alloc.type) {
    case AllocatorType_Global:    return global_alloc_zero(size, align);
    case AllocatorType_Arena:     return arena_alloc_zero((Arena*)alloc.ctx, size, align);
    case AllocatorType_ArenaList: return arena_list_alloc((ArenaList*)alloc.ctx, size, align);
    case AllocatorType_SegList:   return seglist_alloc_zero((AllocSegList*)alloc.ctx, size, align);
  }
}
u8* mem_realloc(Allocator alloc, void* ptr, u64 old_size, u64 new_size, u64 align) {
  switch (alloc.type) {
    case AllocatorType_Global:    return global_realloc(ptr, old_size, new_size, align);
    case AllocatorType_Arena:     return arena_realloc((Arena*)alloc.ctx, ptr, old_size, new_size, align);
    case AllocatorType_ArenaList: return arena_list_realloc((ArenaList*)alloc.ctx, ptr, old_size, new_size, align);
    case AllocatorType_SegList:   return seglist_realloc((AllocSegList*)alloc.ctx, ptr, old_size, new_size, align);
  }
}
u8* mem_realloc_zero(Allocator alloc, void* ptr, u64 old_size, u64 new_size, u64 align) {
  switch (alloc.type) {
    case AllocatorType_Global:    return global_realloc_zero(ptr, old_size, new_size, align);
    case AllocatorType_Arena:     return arena_realloc_zero((Arena*)alloc.ctx, ptr, old_size, new_size, align);
    case AllocatorType_ArenaList: return arena_list_realloc_zero((ArenaList*)alloc.ctx, ptr, old_size, new_size, align);
    case AllocatorType_SegList:   return seglist_realloc_zero((AllocSegList*)alloc.ctx, ptr, old_size, new_size, align);
  }
}
void mem_free(Allocator alloc, void* ptr) {
  switch (alloc.type) {
    case AllocatorType_Global:    return global_free(ptr);
    case AllocatorType_Arena:     return;
    case AllocatorType_ArenaList: return;
    case AllocatorType_SegList:   return seglist_free((AllocSegList*)alloc.ctx, ptr);
  }
}

////////////////////////////////////////////////////////////////////////
// General allocator (segregated pow2 list but for gpu)

AllocGpuHandle* gpu_seglist_alloc(AllocGpu& allocator, u64 size, u64 alignment) {
  return {};
}

void mem_free(AllocGpu& allocator, AllocGpuHandle handle) {

}

////////////////////////////////////////////////////////////////////////
// Utils

// u64 _push_offset(u64* cur_pos, u64 size, u64 align) {
//   u64 offset = AlignUp(*cur_pos, align);      // aligned start for this array
//   *cur_pos = offset + size;                   // advance current position
//   return offset;
// }

u8* mem_alloc_soa(Allocator alloc, u32 count, SoA_Field* fields, u32 fields_count) {
  u64 offset = 0;
  u64 offsets[10] = {};
  Loop (i, fields_count) {
    offset = AlignUp(offset, fields[i].align);
    offsets[i] = offset;
    offset += fields[i].elem_size * count;
  }

  u64 alloc_size = offset;
  u8* buff = mem_alloc(alloc, alloc_size, fields[0].align);

  Loop (i, fields_count) {
    void* new_ptr = Offset(buff, offsets[i]);
    *(fields[i].dst_ptr) = new_ptr;
  }

  return buff;
}

u8* mem_realloc_soa(Allocator alloc, void* ptr, u32 old_count, u32 new_count, SoA_Field* fields, u32 fields_count) {
  u64 offset = 0;
  u64 offsets[10] = {};
  Loop (i, fields_count) {
    offset = AlignUp(offset, fields[i].align);
    offsets[i] = offset;
    offset += fields[i].elem_size * new_count;
  }
  u64 new_size = offset;

  u8* buff = mem_alloc(alloc, new_size, fields[0].align);

  Loop (i, fields_count) {
    void* old_ptr = *(fields[i].dst_ptr);
    void* new_ptr = Offset(buff, offsets[i]);
    u64 old_ptr_size = fields[i].elem_size * old_count;
    MemCopy(new_ptr, old_ptr, old_ptr_size);
    *(fields[i].dst_ptr) = new_ptr;
  }

  mem_free(alloc, ptr);

  return buff;
}

