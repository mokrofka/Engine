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

void global_allocator_init() {
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

    u32 chunks_add = u32DivPow2(commit_size, p.chunk_size);
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

// NOTE: Header: header_size -> u32 header_guard -> u8* memory
u8* mem_alloc(u64 size, u64 alignment) {
  Assert(size > 0);
#if MEM_GUARD
  u64 alloc_size = size + alignment;
  u64 pow2 = next_pow2(alloc_size);
  u64 pool_idx = ctz64(pow2) - ctz64(8);
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

u8* mem_alloc_zero(u64 size) {
  u8* result = mem_alloc(size);
  MemZero(result, size);
  return result;
}

void mem_free(void* ptr) {
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

u8* mem_realloc(void* ptr, u64 size) {
  Assert(ptr);
  Assert(size > 0);
  u8* result = 0;
  u8* base = mem_ctx.data;

  Loop (i, ArrayCount(mem_ctx.pools)) {
    u8* start = base + i*POOL_STEP;
    u8* end = start + POOL_STEP;
    
    u64 base_size = 8;
    base_size <<= i;
    if (IsInsideBounds(start, ptr, end)) {
#if MEM_GUARD
      result = mem_alloc(size);
      u32* header_size = (u32*)OffsetBack(result, sizeof(u32)*2);
      MemCopy(result, ptr, base_size - *header_size);
      MemGuardDealloc(ptr, base_size - *header_size);
      mem_free(ptr);
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

u8* mem_realloc_zero(void* ptr, u64 size) {
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
      result = mem_alloc(size);
      u32* header_size = (u32*)OffsetBack(result, sizeof(u32)*2);
      MemCopy(result, ptr, base_size - *header_size);
      MemGuardDealloc(ptr, base_size - *header_size);
      mem_free(ptr);
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
  
  Arena* arena; Assign(arena, base);
  *arena = {
    .pos = 0,
    .cmt = commit_size,
    .res = reserve_size,
  };
  AsanPoisonMemRegion(base, commit_size);
  AsanUnpoisonMemRegion(base, sizeof(Arena));
  return arena;
}

void* _arena_push(Arena* arena, u64 size, u64 align) {
  size = AlignUp(size, align);

  if (arena->pos + size + sizeof(Arena) > arena->cmt) {
    u64 commit_size = AlignUp(size, ARENA_DEFAULT_COMMIT_SIZE);
    
    Assert((arena->pos + commit_size + sizeof(Arena)) <= arena->res && "Arena is out of memory");

    b32 err = os_commit(Offset(arena, arena->cmt), commit_size);
    Assert(err == 0);
    arena->cmt += commit_size;
    AsanPoisonMemRegion(Offset(arena, arena->pos + sizeof(Arena)), commit_size);
  }

  u8* result = Offset(arena, arena->pos + sizeof(Arena));
  AsanUnpoisonMemRegion(Offset(arena, arena->pos + sizeof(Arena)), size);
  arena->pos += size;

  return result;
}

void arena_release(Arena* arena) {
  os_release(arena, arena->res);
}

////////////////////////////////////////////////////////////////////////
// General allocator

// NOTE: Header: u32 header_size -> u32 header_guard -> u32 pool_index -> u8* memory
u8* mem_alloc(Allocator& allocator, u64 size, u64 alignment) {
  Assert(size > 0);
#if MEM_GUARD
  u32 header_size = AlignUp(sizeof(u32)*3, alignment);
  size = size + header_size;
  u64 pow2_size = next_pow2(size);
  u64 pool_index = ctz64(pow2_size) - ctz64(8);
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

void mem_free(Allocator& allocator, void* ptr) {
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
// Pool

MemPool mem_pool_create(Arena* arena, u64 chunk_size, u64 chunk_alignment) {
  IfDo(MEM_GUARD, chunk_size += sizeof(u32));
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
    PoolFreeNode* node; Assign(node, chunk);
    node->next = p.head;
    p.head = node;
    IfDo(MEM_GUARD,
      u32* guard; Assign(guard, Offset(chunk, p.chunk_size - sizeof(u32))); 
      *guard = MEM_DEALLOC_HEADER_GUARD;
    );
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
    IfDo(MEM_GUARD,
      u32* guard; Assign(guard, Offset(chunk, p.chunk_size - sizeof(u32))); 
      *guard = MEM_DEALLOC_HEADER_GUARD
    );
  }
  p.cap = new_cap;
}

////////////////////////////////////////////////////////////////////////
// Free list

FreeList freelist_create(Arena* arena, u64 size, u64 alignment) {
  FreeList fl;
  fl.data = push_buffer(arena, size, alignment);
  fl.size = size;
  freelist_free_all(fl);
  return fl;
}

void freelist_free_all(FreeList& fl) {
  fl.used = 0;
  FreeListNode* first_node = (FreeListNode*)fl.data;
  first_node->block_size = fl.size;
  first_node->next = null;
  fl.head = first_node;
}

void free_list_node_insert(FreeListNode** phead, FreeListNode* prev_node, FreeListNode* new_node) {
  if (prev_node == null) {
    if (*phead != null) {
      new_node->next = *phead;
      *phead = new_node;
    } else {
      *phead = new_node;
    }
  } else {
    if (prev_node->next == null) {
      new_node->next = null;
      prev_node->next = new_node;
    } else {
      new_node->next = prev_node->next;
      prev_node->next = new_node;
    }
  }
}

void free_list_node_remove(FreeListNode** phead, FreeListNode* prev_node, FreeListNode* del_node) {
  if (prev_node == null) {
    *phead = del_node->next;
  } else {
    prev_node->next = del_node->next;
  }
}

FreeListNode* free_list_find_first(FreeList& fl, u64 size, u64 alignment, u64* padding_, FreeListNode** prev_node_) {
  FreeListNode* node = fl.head;
  FreeListNode* prev_node = null;

  u64 padding = 0;

  while (node != null) {
    padding = sizeof(FreeListAllocationHeader) + AlignPadUp((u64)node+sizeof(FreeListAllocationHeader), alignment);
    u64 required_space = size + padding;
    if (node->block_size >= required_space) {
      break;
    }
    prev_node = node;
    node = node->next;
  }
  *padding_ = padding;
  *prev_node_ = prev_node;
  return node;
}

u8* freelist_alloc(FreeList& fl, u64 size, u64 alignment) {
  u64 padding = 0;
  FreeListNode* prev_node = null;
  FreeListNode* node = null;

  Assert(size < sizeof(FreeListNode) && "Don't do this");

  node = free_list_find_first(fl, size, alignment, &padding, &prev_node);
  Assert(node && "Free list has no free memory");

  u64 required_space = size + padding;
  u64 remaining = node->block_size - required_space;

  if (remaining > 0) {
    FreeListNode* new_node; Assign(new_node, Offset(node, required_space));
    new_node->block_size = remaining;
    free_list_node_insert(&fl.head, node, new_node);
  }

  free_list_node_remove(&fl.head, prev_node, node);

  u64 mem_alignment = padding - sizeof(FreeListAllocationHeader);
  FreeListAllocationHeader* header_ptr; Assign(header_ptr, Offset(node, mem_alignment));
  header_ptr->block_size = required_space;
  header_ptr->padding = mem_alignment;
#ifdef MEM_GUARD
  header_ptr->guard = MEM_ALLOC_HEADER_GUARD;
#endif

  fl.used += required_space;
  u8* result = (u8*)header_ptr + sizeof(FreeListAllocationHeader);
  MemGuardAlloc(result, size);

  return result;
}

void free_list_coalescence(FreeList& fl, FreeListNode* prev_node, FreeListNode* free_node);

void freelist_free(FreeList& fl, void* ptr) {
  Assert(fl.data <= ptr && ptr < fl.data+fl.size && "out of bounce");
  
  FreeListAllocationHeader* header;
  FreeListNode* free_node;
  FreeListNode* node;
  FreeListNode* prev_node = null;

  Assign(header, (u8*)ptr-sizeof(FreeListAllocationHeader));
#ifdef MEM_GUARD
  Assert(header->guard == MEM_ALLOC_HEADER_GUARD && "double free memory");
  header->guard = MEM_DEALLOC_HEADER_GUARD;
#endif

  u64 padding = header->padding;
  Assign(free_node, Offset(header, -header->padding));
  free_node->block_size = header->block_size;
  free_node->next = null;

  if (fl.head == null) {
    fl.head = free_node;
  }
  node = fl.head;
  while (node != null) {
    if (ptr < node) {
      free_list_node_insert(&fl.head, prev_node, free_node);
      break;
    }
    prev_node = node;
    node = node->next;
  }

  fl.used -= free_node->block_size;

  free_list_coalescence(fl, prev_node, free_node);

  MemGuardDealloc(ptr, free_node->block_size - padding - sizeof(FreeListAllocationHeader));
}

void free_list_coalescence(FreeList& fl, FreeListNode* prev_node, FreeListNode* free_node) {
  if (free_node->next != null && PtrMatch(Offset(free_node, free_node->block_size), free_node->next)) {
    free_node->block_size += free_node->next->block_size;
    free_list_node_remove(&fl.head, free_node, free_node->next);
  }

  if (prev_node == null) {
    return;
  } 
  else if (prev_node->next != null && PtrMatch(Offset(prev_node, prev_node->block_size), free_node)) {
    prev_node->block_size += free_node->block_size;
    free_list_node_remove(&fl.head, prev_node, free_node);
  }
}

////////////////////////////////////////////////////////////////////////
// Gpu free list

FreelistGpuNode* get_node(FreelistGpu& list);
void return_node(FreelistGpuNode* node);
u64 freelist_free_space(FreelistGpu& list);

FreelistGpu freelist_gpu_create(Arena* arena, u64 size) {
  // Enough space to hold state, plus array for all nodes.
  u64 max_entries = size / 1024; // NOTE: This might have a remainder, but that's ok.

  if (max_entries < 20) {
    max_entries = 20;
  }

  // The block's layout is head* first, then array of available nodes.
  FreelistGpu result;
  result.nodes = push_array(arena, FreelistGpuNode, max_entries);
  result.max_entries = max_entries;
  result.total_size = size;

  MemZero(result.nodes, sizeof(FreelistGpuNode)*result.max_entries);

  result.head = &result.nodes[0];
  result.head->offset = 0;
  result.head->size = size;
  result.head->next = 0;

  return result;
}

u64 freelist_gpu_alloc(FreelistGpu& list, u64 size) {
  FreelistGpuNode* node = list.head;
  FreelistGpuNode* previous = 0;
  u64 result;
  while (node) {
    if (node->size == size) {
      // Exact match. Just return the node.
      result = node->offset;
      FreelistGpuNode* node_to_return = 0;
      if (previous) {
        previous->next = node->next;
        node_to_return = node;
      } else {
        // This node is the head of the list. Reassign the head
        // and return the previous head node.
        node_to_return = list.head;
        list.head = node->next;
      }
      return_node(node_to_return);
      return result;
    } else if (node->size > size) {
      // Node is larger. Deduct the memory from it and move the offset
      // by that amount.
      result = node->offset;
      node->size -= size;
      node->offset += size;
      return result;
    }

    previous = node;
    node = node->next;
  }

  u64 free_space = freelist_free_space(list);
  Warn("freelist_find_block, no block with enough free space found (requested: %lluB, available: %lluB).", size, free_space);
  return -1;
}

void freelist_gpu_free(FreelistGpu& list, u64 size, u64 offset) {
  FreelistGpuNode* node = list.head;
  FreelistGpuNode* previous = 0;
  if (!node) {
    // Check for the case where the entire thing is allocated.
    // In this case a new node is needed at the head.
    FreelistGpuNode* new_node = get_node(list);
    new_node->offset = offset;
    new_node->size = size;
    new_node->next = 0;
    list.head = new_node;
    return;
  } else {
    while (node) {
      if (node->offset + node->size == offset) {
        // Can be appended to the right of this node.
        node->size += size;

        // Check if this then connects the range between this and the next
        // node, and if so, combine them and return the second node..
        if (node->next && node->next->offset == node->offset + node->size) {
          node->size += node->next->size;
          FreelistGpuNode* next = node->next;
          node->next = node->next->next;
          return_node(next);
        }
        return;
      } else if (node->offset == offset) {
        // If there is an exact match, this means the exact block of memory
        // that is already free is being freed again.
        Error("Attempting to free already-freed block of memory at offset %llu", node->offset);
        return;
      } else if (node->offset > offset) {
        // Iterated beyond the space to be freed. Need a new node.
        FreelistGpuNode* new_node = get_node(list);
        new_node->offset = offset;
        new_node->size = size;

        // If there is a previous node, the new node should be inserted between this and it.
        if (previous) {
          previous->next = new_node;
          new_node->next = node;
        } else {
          // Otherwise, the new node becomes the head.
          new_node->next = node;
          list.head = new_node;
        }

        // Double-check next node to see if it can be joined.
        if (new_node->next && new_node->offset + new_node->size == new_node->next->offset) {
          new_node->size += new_node->next->size;
          FreelistGpuNode* rubbish = new_node->next;
          new_node->next = rubbish->next;
          return_node(rubbish);
        }

        // Double-check previous node to see if the new_node can be joined to it.
        if (previous && previous->offset + previous->size == new_node->offset) {
          previous->size += new_node->size;
          FreelistGpuNode* rubbish = new_node;
          previous->next = rubbish->next;
          return_node(rubbish);
        }

        return;
      }

      // If on the last node and the last node's offset + size < the free offset,
      // a new node is required.
      if (!node->next && node->offset + node->size < offset) {
        FreelistGpuNode* new_node = get_node(list);
        new_node->offset = offset;
        new_node->size = size;
        new_node->next = 0;
        node->next = new_node;

        return;
      }

      previous = node;
      node = node->next;
    }
  }

  Warn("Unable to find block to be freed. Corruption possible?");
  return;
}


void freelist_clear(FreelistGpu& list) {
  // Invalidate the offset and size for all but the first node. The invalid
  // value will be checked for when seeking a new node from the list.
  for (u64 i = 1; i < list.max_entries; ++i) {
    list.nodes[i].offset = INVALID_ID;
    list.nodes[i].size = INVALID_ID;
  }

  // Reset the head to occupy the entire thing.
  list.head->offset = 0;
  list.head->size = list.total_size;
  list.head->next = 0;
}

u64 freelist_free_space(FreelistGpu& list) {
  u64 running_total = 0;
  FreelistGpuNode* node = list.head;
  while (node) {
    running_total += node->size;
    node = node->next;
  }

  return running_total;
}

FreelistGpuNode* get_node(FreelistGpu& list) {
  for (u64 i = 1; i < list.max_entries; ++i) {
    if (list.nodes[i].size == 0) {
      list.nodes[i].next = 0;
      list.nodes[i].offset = 0;
      return &list.nodes[i];
    }
  }

  // Return nothing if no nodes are available.
  return 0;
}

void return_node(FreelistGpuNode* node) {
  node->offset = INVALID_ID;
  node->size = INVALID_ID;
  node->next = 0;
}
