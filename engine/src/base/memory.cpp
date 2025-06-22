#include "memory.h"

#include "os.h"
#include "logger.h"

global thread_local TCTX tctx_thread_local;

////////////////////////////////
// Arena

Arena* arena_alloc(Arena* arena, u64 size, u64 align) {
  PtrInt curr_ptr = (PtrInt)arena + ARENA_HEADER + arena->used;
  PtrInt offset = AlignPow2(curr_ptr, align);
  PtrInt temp = (PtrInt)arena + ARENA_HEADER;
	offset -= temp; // Change to relative offset

  Assert(offset+size <= arena->size && "Arena is out of memory");
  
  Arena* result; Assign(result, Offset(arena, ARENA_HEADER + offset));
  *result = { .size = size, };
  arena->used = offset + size;

  FillAlloc(Offset(result, ARENA_HEADER), size);
  return result;
}

Arena* mem_arena_alloc(u64 size) {
  Arena* arena; Assign(arena, mem_alloc(size));
  *arena = {.size = size};
  return arena;
}

void* _arena_push(Arena* arena, u64 size, u64 align) {
  PtrInt curr_ptr = (PtrInt)arena + ARENA_HEADER + arena->used;
  u64 offset = AlignPow2(curr_ptr, align);
  u64 temp = (PtrInt)arena + ARENA_HEADER;
	offset -= temp;

  Assert(offset+size <= arena->size && "Arena is out of memory");
  
  u8* result = Offset(arena, ARENA_HEADER + offset);
  arena->used = offset + size;

  FillAlloc(result, size);
  return result;
}

void _arena_move(Arena* arena, u64 size, u64 align) {
  PtrInt curr_ptr = (PtrInt)arena + ARENA_HEADER + arena->used;
  u64 offset = AlignPow2(curr_ptr, align);
  u64 temp = (PtrInt)arena + ARENA_HEADER;
	offset -= temp;
  Assert(offset + size <= arena->size && "Arena is out of memory");
  arena->used = offset+size;
}

void tctx_init() {
  tctx_thread_local.arenas[0] = mem_arena_alloc(MB(8));
  tctx_thread_local.arenas[1] = mem_arena_alloc(MB(8));
}

Temp tctx_get_scratch(Arena** conflics, u64 counts) {
  TCTX* tctx = &tctx_thread_local;
  
  Loop (i, ArrayCount(tctx->arenas)) {
    b32 is_conflicting_arena = false;
    Loop (z, counts) {
      if (tctx->arenas[i] == conflics[z]) {
        is_conflicting_arena = true;
      }
    }

    if (!is_conflicting_arena) {
      return temp_begin(tctx->arenas[i]);
    }
  }
  return {};
}

////////////////////////////////
// Pool
#ifdef GUARD_MEMORY

MemPool pool_create(Arena* arena, u64 chunk_count, u64 chunk_size, u64 chunk_alignment) {
  chunk_size = AlignPow2(chunk_size, chunk_alignment);
  u64 stride = chunk_size + chunk_alignment;
  u8* data = push_buffer(arena, chunk_count*stride, chunk_alignment);

  MemPool result = {
    .data = data,
    .chunk_count = chunk_count,
    .chunk_size = chunk_size,
    .head = null,
    .guard_size = chunk_alignment,
  };

  pool_free_all(result);
  return result;
}

u8* pool_alloc(MemPool& p) {
  PoolFreeNode* result = p.head;
  Assert(result && "Pool allocator has no free memory");

  *(u64*)(Offset(result, -p.guard_size)) = ALLOC_HEADER_GUARD;

  p.head = p.head->next;

  FillAlloc(result, p.chunk_size);
  return (u8*)result;
}

void pool_free(MemPool& p, void* ptr) {
  Assert(*(u64*)(Offset(ptr, -p.guard_size)) == ALLOC_HEADER_GUARD && "Double free memory");
  *(u64*)(Offset(ptr, -p.guard_size)) = DEALLOC_HEADER_GUARD;

	void* start = p.data;
	void* end = p.data + p.chunk_count*(p.chunk_size + p.guard_size);
  Assert((start <= ptr && ptr < end) && "Memory is out of bounds");
  FillDealoc(ptr, p.chunk_size);

  PoolFreeNode* node; Assign(node, ptr);
	node->next = p.head;
	p.head = node;
}

void pool_free_all(MemPool& p) {
  FillDealoc(p.data, p.chunk_count*(p.chunk_size + p.guard_size));
  Loop (i, p.chunk_count) {
		u64* free_guard; Assign(free_guard, p.data + i*(p.chunk_size + p.guard_size));
    *free_guard = DEALLOC_HEADER_GUARD;
		PoolFreeNode *node; Assign(node, p.data + i*(p.chunk_size + p.guard_size) + p.guard_size);
		node->next = p.head;
		p.head = node;
	}
}

#else

Pool pool_create(Arena* arena, u64 chunk_count, u64 chunk_size, u64 chunk_alignment) {
  Assert(chunk_size >= sizeof(PoolFreeNode) && "Chunk size is too small");

  chunk_size = AlignPow2(chunk_size, chunk_alignment);
  u64 size = chunk_count* chunk_size;
  u8* data = push_buffer(arena, size, chunk_alignment);

  Pool result {
    .data = data,
    .chunk_count = chunk_count,
    .chunk_size = chunk_size,
    .head = null,
  };

  pool_free_all(result);
  return result;
}

u8* pool_alloc(Pool& p) {
  PoolFreeNode* result = p.head;
  Assert(result && "Pool allocator has no free memory");

  p.head = p.head->next;

  FillAlloc(result, p.chunk_size);
  return (u8*)result;
}

void pool_free(Pool& p, void* ptr) {
  FillDealoc(ptr, p.chunk_size);

  PoolFreeNode* node; Assign(node, ptr);
	node->next = p.head;
	p.head = node;
}

void pool_free_all(Pool& p) {
  FillDealoc(p.data, p.chunk_count*p.chunk_size);
  Loop (i, p.chunk_count) {
		PoolFreeNode *node; Assign(node, p.data + i*p.chunk_size);
		node->next = p.head;
		p.head = node;
	}
}

#endif

////////////////////////////////
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
    padding = sizeof(FreeListAllocationHeader) + AlignPadPow2((PtrInt)node+sizeof(FreeListAllocationHeader), alignment);
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

  if (size < sizeof(FreeListNode)) {
    size = sizeof(FreeListNode);
  }

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
#ifdef GUARD_MEMORY
  header_ptr->guard = ALLOC_HEADER_GUARD;
#endif

  fl.used += required_space;
  u8* result = (u8*)header_ptr + sizeof(FreeListAllocationHeader);
  FillAlloc(result, size);

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
#ifdef GUARD_MEMORY
  Assert(header->guard == ALLOC_HEADER_GUARD && "double free memory");
  header->guard = DEALLOC_HEADER_GUARD;
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

  FillDealoc(ptr, free_node->block_size - padding - sizeof(FreeListAllocationHeader));
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

////////////////////////////////
// Global Allocator

struct SegPool {
  u8* data;
	u64 chunk_count;
	u64 chunk_size;
  u64 chunk_count_used;

  PoolFreeNode* head;
};

struct MemCtx {
  u8* data;
  SegPool pools[32];
};

global MemCtx mem_ctx;

#define PAGE_SIZE 4096
#define MIN_CHUNKS_PER_COMMIT 32
#define PoolStep GB(10)

internal void pool_commit(SegPool& p, u64 commit_size) {
  u8* end_pool = Offset(p.data, p.chunk_count*p.chunk_size);
  os_commit(end_pool, commit_size);
}

internal void pool_init_new_chunk(SegPool& p, u64 chunks_add) {
  Loop (i, chunks_add) {
    u8* chunk = Offset(p.data, (p.chunk_count + i)*p.chunk_size);
    PoolFreeNode* node; Assign(node, chunk);
    node->next = p.head;
    p.head = node;
	}
}

internal u8* segregated_pool_alloc(SegPool& p) {
  u64 commit_size = 0;
  if (p.head == null) {
    Assert(p.chunk_count_used + 1 > p.chunk_count);
    if (p.chunk_size > MB(1)) {
      commit_size = p.chunk_size;
    } else {
      commit_size = Clamp(PAGE_SIZE, p.chunk_size*MIN_CHUNKS_PER_COMMIT, MB(1));
    }
    u64 chunks_add = commit_size / p.chunk_size;
    pool_commit(p, commit_size);
    pool_init_new_chunk(p, chunks_add);
    p.chunk_count += chunks_add;
  }
  ++p.chunk_count_used;
  
  u8* result; Assign(result, p.head);
  p.head = p.head->next;
  return result;
}

internal void segregated_pool_free(SegPool& p, void* ptr) {
	PoolFreeNode* node; Assign(node, ptr);
	node->next = p.head;
	p.head = node;
  --p.chunk_count_used;
}

u8* mem_alloc(u64 size) {
  u8* result = 0;
  u64 base_size = 8;

  Loop (i, ArrayCount(mem_ctx.pools)) {
    if (size <= base_size << i) {
      result = segregated_pool_alloc(mem_ctx.pools[i]);
      FillAlloc(result, size);
      break;
    }
  }

  Assert(result);
  return result;
}

u8* mem_alloc_zero(u64 size) {
  u8* result = 0;
  u64 base_size = 8;

  Loop (i, ArrayCount(mem_ctx.pools)) {
    if (size <= base_size << i) {
      result = segregated_pool_alloc(mem_ctx.pools[i]);
      MemZero(result, size);
      break;
    }
  }

  Assert(result);
  return result;
}

void mem_free(void* ptr) {
  u8* base = mem_ctx.data;

  Loop (i, ArrayCount(mem_ctx.pools)) {
    u8* start = base + i*PoolStep;
    u8* end = start + PoolStep;

    u64 base_size = 8;
    base_size <<= i;
    if ((u8*)ptr >= start && (u8*)ptr < end) {
      FillDealoc(ptr, base_size);
      segregated_pool_free(mem_ctx.pools[i], ptr);
      return;
    }
  }

  Assert(0 && "pointer doesn't belong to any pool");
}

u8* mem_realoc(void* ptr, u64 size) {
  u8* result = 0;
  u8* base = mem_ctx.data;

  Loop (i, ArrayCount(mem_ctx.pools)) {
    u8* start = base + i*PoolStep;
    u8* end = start + PoolStep;

    u64 base_size = 8;
    base_size <<= i;
    if ((u8*)ptr >= start && (u8*)ptr < end) {
      result = mem_alloc(size);
      MemCopy(result, ptr, base_size);
      FillDealoc(ptr, base_size);
      segregated_pool_free(mem_ctx.pools[i], ptr);
    }
  }

  Assert(result && "pointer doesn't belong to any pool");
  return result;
}

void global_allocator_init() {
  mem_ctx.data = os_reserve(TB(1));
  u64 offset = 0;
  
  u64 pow_num = 1;
  Loop (i, ArrayCount(mem_ctx.pools)) {
    mem_ctx.pools[i].data = Offset(mem_ctx.data, offset);
    mem_ctx.pools[i].chunk_size = 8 * pow_num;
    pow_num *= 2;
    offset += PoolStep;
  };
}

////////////////////////////////
// Ring Buffer

u64 ring_write(u8* ring_base, u64 ring_size, u64 ring_pos, void* src_data, u64 src_data_size) {
  Assert(src_data_size <= ring_size);

  u64 ring_off = ring_pos % ring_size;
  u64 bytes_before_split = ring_size - ring_off;
  u64 pre_split_bytes = Min(bytes_before_split, src_data_size);
  u64 pst_split_bytes = src_data_size - pre_split_bytes;
  void* pre_split_data = src_data;
  void* pst_split_data = ((u8*)src_data + pre_split_bytes);
  MemCopy(ring_base + ring_off, pre_split_data, pre_split_bytes);
  MemCopy(ring_base + 0, pst_split_data, pst_split_bytes);
  return src_data_size;
}

u64 ring_read(u8* ring_base, u64 ring_size, u64 ring_pos, void* dst_data, u64 read_size) {
  Assert(read_size <= ring_size);

  u64 ring_off = ring_pos % ring_size;
  u64 bytes_before_split = ring_size - ring_off;
  u64 pre_split_bytes = Min(bytes_before_split, read_size);
  u64 pst_split_bytes = read_size - pre_split_bytes;
  MemCopy(dst_data, ring_base + ring_off, pre_split_bytes);
  MemCopy((u8*)dst_data + pre_split_bytes, ring_base + 0, pst_split_bytes);
  return read_size;
}

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
        Fatal("Attempting to free already-freed block of memory at offset %llu", node->offset);
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
