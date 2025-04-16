#include "memory.h"

#include "os.h"
#include "logger.h"
#include <string.h>

global thread_local TCTX tctx_thread_local;

void* _memory_zero(void* block, u64 size) {
  return memset(block, 0, size);
}

void* _memory_copy(void* dest, const void* source, u64 size) {
  return memcpy(dest, source, size);
}

void* _memory_set(void* dest, i32 value, u64 size) {
  return memset(dest, value, size);
}

b32 _memory_match(void* a, void* b, u64 size) {
  return memcmp(a, b, size) ? 0 : 1;
}

internal b32 is_power_of_two(u64 x) {
	return (x & (x-1)) == 0;
}

internal u64 align_forward(u64 ptr, u64 align) {
	u64 p, a, modulo;
  
	if (!is_power_of_two(align)) {
    Error("isn't power of two");
  }

	p = ptr;
	a = (u64)align;
	// Same as (p % a) but faster as 'a' is a power of two
	modulo = p & (a-1);

	if (modulo != 0) {
		// If 'p' address is not aligned, push the address to the
		// next value which is aligned
		p += a - modulo;
	}
	return p;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Arena

Arena* arena_alloc(Arena *arena, u64 size, u64 align) {
	// Align 'curr_offset' forward to the specified alignment
	u64 curr_ptr = (u64)arena + ARENA_HEADER + arena->pos;
	u64 offset = align_forward(curr_ptr, align);
  u64 temp = (u64)arena + ARENA_HEADER;
	offset -= temp; // Change to relative offset

	// Check to see if the backing memory has space left
	if (offset+size <= arena->res) {
		Arena* ptr = (Arena*)((u8*)((u64)arena + ARENA_HEADER) + offset);
    ptr->pos = 0;
    ptr->res = size;
		arena->pos = offset+size;

		// Zero new memory by default
    // MemZero((u8*)(ptr) + ARENA_HEADER, size);
		return ptr;
	}
	// Return null if the arena is out of memory (or handle differently)
  Error("Arena is out of memory!");
  Assert(true);
  return 0;
}

Arena* arena_alloc(u64 size) {
  Arena* arena = (Arena*)os_allocate(size, true);
  arena->pos = 0;
  arena->res = size;
  return arena;
}

void* _arena_push(Arena* arena, u64 size, u64 align) {
  // Align 'curr_offset' forward to the specified alignment
  u64 curr_ptr = (u64)arena + ARENA_HEADER + arena->pos;
  u64 offset = align_forward(curr_ptr, align);
  u64 temp = (u64)arena + ARENA_HEADER;
	offset -= temp; // Change to relative offset

	// Check to see if the backing memory has space left
	if (offset+size <= arena->res) {
		u8* buffer = (u8*)((u64)arena + ARENA_HEADER) + offset;
		arena->pos = offset+size;

    // MemZero(buffer, size);
		return buffer;
	}
	// Return null if the arena is out of memory (or handle differently)
  AssertMsg(false, "Arena is out of memory");
  return 0;
}

void tctx_init(Arena* arena) {
  tctx_thread_local.arenas[0] = arena_alloc(arena, MB(32));
  tctx_thread_local.arenas[1] = arena_alloc(arena, MB(32));
}

Temp tctx_get_scratch(Arena** conflics, u32 counts) {
  TCTX* tctx = &tctx_thread_local;
  
  for (u32 i = 0; i < ArrayCount(tctx->arenas); i++) {
    b32 isConflictingArena = false;
    for (u32 z = 0; z < counts; z++) {
      if (tctx->arenas[i] == conflics[z]) {
        isConflictingArena = true;
      }
    }

    if (!isConflictingArena) {
      return temp_begin(tctx->arenas[i]);
    }
  }
  Temp temp = {null, 0};

  return temp;
}

u64 align_forward_uintptr(u64 ptr, u64 align) {
	u64 a, p, modulo;

	Assert(is_power_of_two(align));

	a = align;
	p = ptr;
	modulo = p & (a-1);
	if (modulo != 0) {
		p += a - modulo;
	}
	return p;
}

u64 align_forward_size(u64 ptr, u64 align) {
	u64 a, p, modulo;

	Assert(is_power_of_two((u64)align));

	a = align;
	p = ptr;
	modulo = p & (a-1);
	if (modulo != 0) {
		p += a - modulo;
	}
	return p;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Pool

Pool* pool_init(Arena* arena, u64 chunk_count, u64 chunk_size, u64 chunk_alignment) {

  // Align chunk size up to the required chunk_alignment
  chunk_size = align_forward_size(chunk_size, chunk_alignment);

  u8* header = push_buffer(arena, u8, POOL_HEADER);
  
  u64 init_start = u64((u8*)arena + arena_pos(arena));
  u64 start = align_forward_uintptr(init_start, (u64)chunk_alignment);
  u64 check = chunk_count;
  check -= (u64)(start - init_start);
  u64 gap = (u64)(start - init_start);
  
  arena->pos += gap;
  u8* buff = push_buffer(arena, u8, chunk_count*chunk_size);
  
  // Assert that the parameters passed are valid
  Assert(chunk_size >= sizeof(PoolFreeNode) &&
         "Chunk size is too small");
  Assert(check >= chunk_size &&
         "Backing buffer length is smaller than the chunk size");

  // Store the adjusted parameters
  Pool* pool = (Pool*)header;
  pool->buff = buff;
  pool->chunk_count = chunk_count;
  pool->chunk_size = chunk_size;
  pool->head = null; // Free List Head

  // Set up the free list for free chunks
  pool_free_all(pool);
  return pool;
}

void* _pool_alloc(Pool* p) {
  // Get latest free node
  PoolFreeNode* node = p->head;

  if (node == null) {
    Assert(0 && "Pool allocator has no free memory");
    return null;
  }

  // Pop free node
  p->head = p->head->next;

  // Zero memory by default
  // return MemSet(node, 0, p->chunk_size);
  return node;
}

void pool_free(Pool *p, void *ptr) {
	PoolFreeNode *node;

	void *start = p->buff;
	void *end = &(p->buff)[p->chunk_count*p->chunk_size];

	if (ptr == null) {
		// Ignore null pointers
		return;
	}

	if (!(start <= ptr && ptr < end)) {
		Assert(0 && "Memory is out of bounds of the buffer in this pool");
		return;
	}

	// Push free node
	node = (PoolFreeNode *)ptr;
	node->next = p->head;
	p->head = node;
}

void pool_free_all(Pool *pool) {
	// Set all chunks to be free
  Loop (i, pool->chunk_count) {
		void* ptr = &pool->buff[i * pool->chunk_size];
		PoolFreeNode *node = (PoolFreeNode*)ptr;
		// Push free node onto thte free list
		node->next = pool->head;
		pool->head = node;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Free list
u64 calc_padding_with_header(u64 ptr, u64 alignment, u64 header_size) {
  u64 p, a, modulo, padding, needed_space;

  Assert(is_power_of_two(alignment));

  p = ptr;
  a = alignment;
  modulo = p & (a - 1); // (p % a) as it assumes alignment is a power of two

  padding = 0;
  needed_space = 0;

  if (modulo != 0) { // Same logic as 'align_forward'
    padding = a - modulo;
  }

  needed_space = (u64)header_size;

  if (padding < needed_space) {
    needed_space -= padding;

    if ((needed_space & (a - 1)) != 0) {
      padding += a * (1 + (needed_space / a));
    } else {
      padding += a * (needed_space / a);
    }
  }
  return (u64)padding;
}

// Unlike our trivial stack allocator, this header needs to store the
// block size along with the padding meaning the header is a bit
// larger than the trivial stack allocator
void free_list_free_all(FreeList& fl) {
  fl.used = 0;
  FreeListNode* first_node = (FreeListNode*)fl.data;
  first_node->block_size = fl.size;
  first_node->next = null;
  fl.head = first_node;
}

void free_list_init(FreeList& fl, void* data, u64 size) {
  fl.data = data;
  fl.size = size;
  free_list_free_all(fl);
}

void FreeList_node_insert(FreeListNode** phead, FreeListNode* prev_node, FreeListNode* new_node) {
  if (prev_node == null) {
    if (*phead != null) {
      new_node->next = *phead;
    } else {
      *phead = new_node;
    }
  } else {
    if (prev_node->next == null) {
      prev_node->next = new_node;
      new_node->next = null;
    } else {
      new_node->next = prev_node->next;
      prev_node->next = new_node;
    }
  }
}

void FreeList_node_remove(FreeListNode** phead, FreeListNode* prev_node, FreeListNode* del_node) {
  if (prev_node == null) {
    *phead = del_node->next;
  } else {
    prev_node->next = del_node->next;
  }
}

u64 calc_padding_with_header(u64 ptr, u64 alignment, u64 header_size);

FreeListNode* FreeList_find_first(FreeList& fl, u64 size, u64 alignment, u64* padding_, FreeListNode** prev_node_) {
  // Iterates the list and finds the first free block with enough space
  FreeListNode* node = fl.head;
  FreeListNode* prev_node = null;

  u64 padding = 0;

  while (node != null) {
    padding = calc_padding_with_header((u64)node, (u64)alignment, sizeof(free_list_allocation_Header));
    u64 required_space = size + padding;
    if (node->block_size >= required_space) {
      break;
    }
    prev_node = node;
    node = node->next;
  }
  if (padding_)
    *padding_ = padding;
  if (prev_node_)
    *prev_node_ = prev_node;
  return node;
}

FreeListNode* FreeList_find_best(FreeList& fl, u64 size, u64 alignment, u64* padding_, FreeListNode** prev_node_) {
  // This iterates the entire list to find the best fit
  // O(n)
  u64 smallest_diff = ~(u64)0;

  FreeListNode* node = fl.head;
  FreeListNode* prev_node = null;
  FreeListNode* best_node = null;

  u64 padding = 0;

  while (node != null) {
    padding = calc_padding_with_header((u64)node, (u64)alignment, sizeof(free_list_allocation_Header));
    u64 required_space = size + padding;
    // if (node->block_size >= required_space && (it.block_size - required_space < smallest_diff)) {
    //   best_node = node;
    // }
    prev_node = node;
    node = node->next;
  }
  if (padding_)
    *padding_ = padding;
  if (prev_node_)
    *prev_node_ = prev_node;
  return best_node;
}

void* free_list_alloc(FreeList& fl, u64 size, u64 alignment) {
  u64 padding = 0;
  FreeListNode* prev_node = null;
  FreeListNode* node = null;
  u64 alignment_padding, required_space, remaining;
  free_list_allocation_Header* header_ptr;

  if (size < sizeof(FreeListNode)) {
    size = sizeof(FreeListNode);
  }
  if (alignment < 8) {
    alignment = 8;
  }

  if (fl.policy == PlacementPolicy_FindBest) {
    node = FreeList_find_best(fl, size, alignment, &padding, &prev_node);
  } else {
    node = FreeList_find_first(fl, size, alignment, &padding, &prev_node);
  }
  if (node == null) {
    Assert(0 && "Free list has no free memory");
    return null;
  }

  alignment_padding = padding - sizeof(free_list_allocation_Header);
  required_space = size + padding;
  remaining = node->block_size - required_space;

  if (remaining > 0) {
    FreeListNode* new_node = (FreeListNode*)((u8*)node + required_space);
    new_node->block_size = remaining;
    FreeList_node_insert(&fl.head, node, new_node);
  }

  FreeList_node_remove(&fl.head, prev_node, node);

  header_ptr = (free_list_allocation_Header*)((char*)node + alignment_padding);
  header_ptr->block_size = required_space;
  header_ptr->padding = alignment_padding;

  fl.used += required_space;

  return (void*)((char*)header_ptr + sizeof(free_list_allocation_Header));
}

void FreeList_coalescence(FreeList& fl, FreeListNode* prev_node, FreeListNode* free_node);

void* free_list_free(FreeList& fl, void* ptr) {
  free_list_allocation_Header* header;
  FreeListNode* free_node;
  FreeListNode* node;
  FreeListNode* prev_node = null;

  if (ptr == null) {
    return null;
  }

  header = (free_list_allocation_Header*)((char*)ptr - sizeof(free_list_allocation_Header));
  free_node = (FreeListNode*)header;
  free_node->block_size = header->block_size + header->padding;
  free_node->next = null;

  node = fl.head;
  while (node != null) {
    if (ptr < node) {
      FreeList_node_insert(&fl.head, prev_node, free_node);
      break;
    }
    prev_node = node;
    node = node->next;
  }

  fl.used -= free_node->block_size;

  FreeList_coalescence(fl, prev_node, free_node);
  return 0; // suspicious
}

void FreeList_coalescence(FreeList& fl, FreeListNode* prev_node, FreeListNode* free_node) {
  if (free_node->next != null && (void*)((char*)free_node + free_node->block_size) == free_node->next) {
    free_node->block_size += free_node->next->block_size;
    FreeList_node_remove(&fl.head, free_node, free_node->next);
  }

  if (prev_node->next != null && (void*)((char*)prev_node + prev_node->block_size) == free_node) {
    prev_node->block_size += free_node->next->block_size;
    FreeList_node_remove(&fl.head, prev_node, free_node);
  }
}
