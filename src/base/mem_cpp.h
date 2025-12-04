#include "base_inc.h"

////////////////////////////////////////////////////////////////////////
// Global Allocator

#define MIN_CHUNKS_PER_COMMIT 32
#define POOL_STEP GB(10)
#define MEM_POOL_BATCH 16

struct SegPool {
  u8* data;
	u32 count;
	u32 chunk_size;
  PoolFreeNode* head;
};

struct MemCtx {
  u8* data;
  SegPool pools[32];
};

global MemCtx mem_ctx;

void global_alloc_init() {
  mem_ctx.data = os_reserve(TB(1));
  u64 offset = 0;
  u64 chunk_size = 8;
  Loop (i, ArrayCount(mem_ctx.pools)) {
    mem_ctx.pools[i].data = Offset(mem_ctx.data, offset);
    mem_ctx.pools[i].chunk_size = chunk_size;
    offset += POOL_STEP;
    chunk_size *= 2;
  };
}

intern u8* global_pool_alloc(SegPool& p) {
  if (p.head == null) {
    u32 commit_size;
    if (p.chunk_size > MB(1)) 
      commit_size = p.chunk_size;
    else 
      commit_size = Clamp(PAGE_SIZE, p.chunk_size * MIN_CHUNKS_PER_COMMIT, MB(1));

    u8* end_of_pool = Offset(p.data, p.count*p.chunk_size);
    os_commit(end_of_pool, commit_size);

    u32 chunks_add = DivPow2(commit_size, p.chunk_size);
    for (u32 i = p.count; i < p.count + chunks_add; ++i) {
      u8* chunk =  Offset(p.data, i*p.chunk_size);
      PoolFreeNode* node = (PoolFreeNode*)chunk;
      node->next = p.head;
      p.head = node;
      AsanPoisonMemRegion(Offset(chunk, sizeof(void*)), p.chunk_size - sizeof(void*));
    }
    p.count += chunks_add;
  }
  
  u8* result = (u8*)p.head;
  p.head = p.head->next;
  AsanUnpoisonMemRegion(Offset(result, sizeof(void*)), p.chunk_size - sizeof(void*));
  return result;
}

intern void global_pool_free(SegPool& p, void* ptr) {
	PoolFreeNode* node = (PoolFreeNode*)ptr;
	node->next = p.head;
  AsanPoisonMemRegion(Offset(node, sizeof(void*)), p.chunk_size - sizeof(void*));
	p.head = node;
}

// NOTE: Header Guard(u32 header_size -> u32 header_guard) -> u8* memory
u8* global_alloc(u64 size, u64 alignment) {
  Assert(size > 0);
#if MEM_GUARD
  u64 alloc_size = size + alignment;
  u64 pow2 = next_pow2(alloc_size);
  u64 pool_idx = ctz(pow2) - ctz((u32)8);
  SegPool& p = mem_ctx.pools[pool_idx];
  u8* result = global_pool_alloc(p);
  result = Offset(result, alignment);
  u32* header_guard = (u32*)OffsetBack(result, sizeof(u32));
  *header_guard = MEM_ALLOC_HEADER_GUARD;
  u32* header_size = (u32*)OffsetBack(header_guard, sizeof(u32));
  *header_size = alignment;
  MemGuardAlloc(result, p.chunk_size - *header_size);
#else
  u64 pow2 = next_pow2(size);
  u64 pool_idx = ctz64(pow2) - ctz64(8);
  u8* result = global_pool_alloc(mem_ctx.pools[pool_idx]);
  MemGuardAlloc(result, size);
#endif
  return result;
}

u8* global_alloc_zero(u64 size) {
  u8* result = global_alloc(size);
  MemZero(result, size);
  return result;
}

void global_free(void* ptr) {
  Assert(ptr);
  u8* base = mem_ctx.data;

  Loop (i, ArrayCount(mem_ctx.pools)) {
    u8* start = base + i*POOL_STEP;
    u8* end = start + POOL_STEP;
    u64 base_size = 8;
    base_size <<= i;
    if (IsInsideBounds(start, ptr, end)) {
#if MEM_GUARD
      u32* header_guard = (u32*)OffsetBack(ptr,sizeof(u32));
      AssertMsg(*header_guard == MEM_ALLOC_HEADER_GUARD, "Double free memory");
      *header_guard = MEM_DEALLOC_HEADER_GUARD;
      u32* header_size = (u32*)OffsetBack(header_guard, sizeof(u32));
      MemGuardDealloc(ptr, base_size - *header_size);
      ptr = OffsetBack(ptr, *header_size);
#endif
      global_pool_free(mem_ctx.pools[i], ptr);
      return;
    }
  }

  Assert(!"pointer doesn't belong to any pool");
}

u8* global_realloc(void* ptr, u64 size) {
  Assert(ptr);
  Assert(size > 0);
  Info("%i", size);
  u8* result = 0;
  u8* base = mem_ctx.data;

  Loop (i, ArrayCount(mem_ctx.pools)) {
    u8* start = base + i*POOL_STEP;
    u8* end = start + POOL_STEP;
    
    u64 base_size = 8;
    base_size <<= i;
    if (IsInsideBounds(start, ptr, end)) {
#if MEM_GUARD
      result = global_alloc(size);
      u32* header_size = (u32*)OffsetBack(result, sizeof(u32)*2);
      MemCopy(result, ptr, base_size - *header_size);
      MemGuardDealloc(ptr, base_size - *header_size);
      global_free(ptr);
#else
      result = mem_alloc(size);
      MemCopy(result, ptr, base_size);
      global_pool_free(mem_ctx.pools[i], ptr);
#endif
      break;
    }
  }

  Assert(result && "pointer doesn't belong to any pool");
  return result;
}

u8* global_realloc_zero(void* ptr, u64 size) {
  Assert(ptr);
  Assert(size > 0);
  u8* result = 0;
  u8* base = mem_ctx.data;

  Loop (i, ArrayCount(mem_ctx.pools)) {
    u8* start = base + i*POOL_STEP;
    u8* end = start + POOL_STEP;

    u64 base_size = 8;
    base_size <<= i;
    if ((u8*)ptr >= start && (u8*)ptr < end) {
#if MEM_GUARD
      result = global_alloc_zero(size);
      u32* header_size = (u32*)OffsetBack(result, sizeof(u32)*2);
      MemCopy(result, ptr, base_size - *header_size);
      MemGuardDealloc(ptr, base_size - *header_size);
      global_free(ptr);
#else
      result = mem_alloc_zero(size);
      MemCopy(result, ptr, base_size);
      global_pool_free(mem_ctx.pools[i], ptr);
#endif
      break;
    }
  }

  Assert(result && "pointer doesn't belong to any pool");
  return result;
}

////////////////////////////////////////////////////////////////////////
// Arena

Arena* arena_alloc() {
  u64 reserve_size = ARENA_DEFAULT_RESERVE_SIZE;
  u64 commit_size = ARENA_DEFAULT_COMMIT_SIZE;

  u8* base = os_reserve(reserve_size);
  os_commit(base, commit_size);
  
  Arena* arena = (Arena*)base;
  *arena = {
    .pos = 0,
    .cmt = commit_size,
    .cap = reserve_size,
  };
  AsanPoisonMemRegion(base, commit_size);
  AsanUnpoisonMemRegion(base, sizeof(Arena));
  return arena;
}

void arena_clear(Arena* arena) { 
  arena->pos = 0;
  AsanPoisonMemRegion(Offset(arena, sizeof(Arena)), arena->cmt);
};

void* _arena_push(Arena* arena, u64 size, u64 align) {
  u64 pos = AlignUp(arena->pos, align);
  u64 padd = pos - arena->pos;

  if (sizeof(Arena) + pos + size > arena->cmt) {
    u64 commit_size = AlignUp(padd + size, ARENA_DEFAULT_COMMIT_SIZE);
    
    Assert((sizeof(Arena) + pos + commit_size) <= arena->cap && "Arena is out of memory");

    b32 err = os_commit(Offset(arena, arena->cmt), commit_size);
    Assert(err == 0);

    AsanPoisonMemRegion(Offset(arena, arena->cmt), commit_size);
    arena->cmt += commit_size;
  }

  u8* result = Offset(arena, pos + sizeof(Arena));
  AsanUnpoisonMemRegion(Offset(arena, sizeof(Arena) + arena->pos), size + padd);
  arena->pos = pos + size;

  return result;
}

void arena_release(Arena* arena) {
  os_release(arena, arena->cap);
}

////////////////////////////////////////////////////////////////////////
// General allocator

// NOTE: Header: Guard(u32 header_size -> u32 header_guard) -> u32 pool_index -> u8* memory
u8* seglist_alloc(AllocSegList& allocator, u64 size, u64 alignment) {
  Assert(size > 0);
#if MEM_GUARD
  u32 header_size = AlignUp(sizeof(u32)*3, alignment);
  size = size + header_size;
  u64 pow2_size = next_pow2(size);
  u64 pool_index = ctz(pow2_size) - ctz(8);
  MemPoolPow2& p = allocator.pools[pool_index];

  if (p.head == null) {
    u8* result = push_buffer(allocator.arena, pow2_size, alignment);
    result = Offset(result, header_size);
    u32* index = (u32*)OffsetBack(result, sizeof(u32));
    *index = pool_index;
    u32* header_guard = (u32*)OffsetBack(index, sizeof(u32));
    *header_guard = MEM_ALLOC_HEADER_GUARD;
    u32* header_size_ = (u32*)OffsetBack(header_guard, sizeof(u32));
    *header_size_ = header_size;
    MemGuardAlloc(result, pow2_size - header_size);
    return result;
  }
  PoolFreeNode* result = p.head;
  p.head = p.head->next;

  AsanUnpoisonMemRegion(result, pow2_size - header_size);
  MemGuardAlloc(result, pow2_size - header_size);
  u32* header_guard = (u32*)OffsetBack(result, sizeof(u32)*2);
  Assert(*header_guard == MEM_DEALLOC_HEADER_GUARD);
  *header_guard = MEM_ALLOC_HEADER_GUARD;
  
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

  return (u8*)result;
};

void seglist_free(AllocSegList& allocator, void* ptr) {
  Assert(ptr);
  u32* pool_index = (u32*)OffsetBack(ptr, sizeof(u32));
  MemPoolPow2& p = allocator.pools[*pool_index];

#if MEM_GUARD
  u32* header_guard = (u32*)OffsetBack(pool_index, sizeof(u32));
  Assert(*header_guard == MEM_ALLOC_HEADER_GUARD);
  *header_guard = MEM_DEALLOC_HEADER_GUARD;
  u32* header_size = (u32*)OffsetBack(header_guard, sizeof(u32));
  u64 size = 8 << *pool_index;
  MemGuardDealloc(ptr, size - *header_size);
  AsanPoisonMemRegion(ptr, size - *header_size);
#endif

  PoolFreeNode* node = (PoolFreeNode*)ptr;
	node->next = p.head;
	p.head = node;
}

////////////////////////////////////////////////////////////////////////
// Offset

// u8* offset_push(OffsetBuffer& offset, u64 size, u64 alignment) {
//   size = AlignUp(size, alignment);
//   u8* result = Offset(offset.base, offset.pos);
//   offset.pos += size;
//   return result;
// }

// u64 offset_push(OffsetMark& arena, u64 size, u64 alignment) {
//   size = AlignUp(size, alignment);
//   u8* result = 
//   u64 offset = arena.pos;
//   arena.pos += size;
//   return offset;
// }

////////////////////////////////////////////////////////////////////////
// General allocator (segregated pow2 list but for gpu)

AllocGpuHandle* gpu_seglist_alloc(AllocGpu& allocator, u64 size, u64 alignment) {
  
}

void mem_free(AllocGpu& allocator, AllocGpuHandle handle) {

}

////////////////////////////////////////////////////////////////////////
// Atlas allocator

////////////////////////////////////////////////////////////////////////
// Pool
// NOTE: Header Guard(header_guard) -> u8* memory
MemPool mem_pool_create(Arena* arena, u64 chunk_size, u64 chunk_alignment) {
#if MEM_GUARD
  chunk_size += sizeof(u32);
#endif
  chunk_size = AlignUp(chunk_size, chunk_alignment);
  u8* data = push_buffer(arena, DEFAULT_CAPACITY*chunk_size, chunk_alignment);

  MemPool result {
    .arena = arena,
    .data = data,
    .cap = DEFAULT_CAPACITY,
    .chunk_size = chunk_size,
  };

  mem_pool_free_all(result);
  return result;
}

u8* mem_pool_alloc(MemPool& p) {
  if (p.head == null) {
    mem_pool_grow(p);
  }

  PoolFreeNode* result = p.head;
  p.head = p.head->next;
  ++p.count;

  #if MEM_GUARD
    u32* guard = (u32*)Offset(result, p.chunk_size - sizeof(u32));
    Assert(*guard == MEM_DEALLOC_HEADER_GUARD);
    *guard = MEM_ALLOC_HEADER_GUARD;
    MemGuardAlloc(result, p.chunk_size - sizeof(u32));
  #else
    MemGuardAlloc(result, p.chunk_size);
  #endif

  return (u8*)result;
}

void mem_pool_free(MemPool& p, void* ptr) {
  #if (MEM_GUARD)
    u32* guard; Assign(guard, Offset(ptr, p.chunk_size - sizeof(u32)));
    Assert(*guard == MEM_ALLOC_HEADER_GUARD);
    *guard = MEM_DEALLOC_HEADER_GUARD;
    MemGuardDealloc(ptr, p.chunk_size - sizeof(u32));
  #else
    MemGuardDealloc(ptr, p.chunk_size);
  #endif

  PoolFreeNode* node; Assign(node, ptr);
	node->next = p.head;
	p.head = node;
  --p.count;
}

void mem_pool_free_all(MemPool& p) {
  MemGuardDealloc(p.data, p.cap*p.chunk_size);
  p.head = null;
  Loop (i, p.cap) {
    u8* chunk  = Offset(p.data, i*p.chunk_size);
    PoolFreeNode* node = (PoolFreeNode*)chunk;
    node->next = p.head;
    p.head = node;
#if MEM_GUARD
    u32* guard = (u32*)Offset(chunk, p.chunk_size - sizeof(u32)); 
    *guard = MEM_DEALLOC_HEADER_GUARD;
#endif
  }
  p.count = 0;
}

void mem_pool_grow(MemPool& p) {
  u64 old_cap = p.cap;
  u64 new_cap = old_cap * DEFAULT_RESIZE_FACTOR;
  u64 old_size = old_cap * p.chunk_size;
  u64 new_size = new_cap * p.chunk_size;

  u8* new_data = push_buffer(p.arena, new_size);
  MemCopy(new_data, p.data, old_size);
  p.data = new_data;

  MemGuardDealloc(p.data+old_cap, old_size);

  for (i32 i = old_cap; i < new_cap; ++i) {
    u8* chunk =  Offset(p.data, i*p.chunk_size);
    PoolFreeNode* node; Assign(node, chunk);
    node->next = p.head;
    p.head = node;
#if MEM_GUARD
    u32* guard = (u32*)Offset(chunk, p.chunk_size - sizeof(u32)); 
    *guard = MEM_DEALLOC_HEADER_GUARD;
#endif
  }
  p.cap = new_cap;
}

////////////////////////////////////////////////////////////////////////
// tlsf

TLSF_Allocator tlsf_create() {
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


