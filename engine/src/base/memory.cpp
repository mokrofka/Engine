#include "memory.h"

#include "os.h"
#include "logger.h"
#include <string.h>

#define ZERO_MEMORY 1

#ifdef ZERO_MEMORY
  #define ClearMemory(ptr, size) MemZero(ptr, size)
#else
  #define ClearMemory(ptr, size)
#endif

global thread_local TCTX tctx_thread_local;

void _memory_zero(void* block, u64 size) {
  memset(block, 0, size);
}

void _memory_copy(void* dest, const void* source, u64 size) {
  memcpy(dest, source, size);
}

void _memory_set(void* dest, i32 value, u64 size) {
  memset(dest, value, size);
}

b32 _memory_match(void* a, void* b, u64 size) {
  return memcmp(a, b, size) ? 0 : 1;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Utils

internal b32 is_power_of_two(u64 x) {
	return (x & (x-1)) == 0;
}

u64 calc_padding_with_header(PtrInt ptr, u64 alignment, u64 header_size) {
  Assert(is_power_of_two(alignment));

  u64 p = ptr;
  u64 a = alignment;
  u64 modulo = p & (a - 1); // (p % a) as it assumes alignment is a power of two

  u64 padding = 0;
  u64 needed_space = 0;

  if (modulo != 0) { // Same logic as 'align_forward'
    padding = a - modulo;
  }

  needed_space = header_size;

  if (padding < needed_space) {
    needed_space -= padding;

    if ((needed_space & (a - 1)) != 0) {
      padding += a * (1 + (needed_space / a));
    } else {
      padding += a * (needed_space / a);
    }
  }
  return padding;
}

internal u64 align_forward(PtrInt ptr, u64 align) {
	u64 p, a, modulo;
  
  Assert(is_power_of_two(align));

	p = ptr;
	a = align;
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

Arena* arena_alloc(Arena* arena, u64 size, u64 align) {
  // Align 'curr_offset' forward to the specified alignment
  PtrInt curr_ptr = (PtrInt)arena + ARENA_HEADER + arena->pos;
  PtrInt offset = align_forward(curr_ptr, align);
  PtrInt temp = (PtrInt)arena + ARENA_HEADER;
	offset -= temp; // Change to relative offset

  Assert(offset+size <= arena->res && "Arena is out of memory");
  
  Arena* ptr = (Arena*)(((PtrInt)arena + ARENA_HEADER) + offset);
  ptr->pos = 0;
  ptr->res = size;
  arena->pos = offset+size;

  ClearMemory((u8*)ptr+ARENA_HEADER, size);
  return ptr;
}

void* _arena_push(Arena* arena, u64 size, u64 align) {
  // Align 'curr_offset' forward to the specified alignment
  PtrInt curr_ptr = (PtrInt)arena + ARENA_HEADER + arena->pos;
  u64 offset = align_forward(curr_ptr, align);
  u64 temp = (u64)arena + ARENA_HEADER;
	offset -= temp; // Change to relative offset

  Assert(offset+size <= arena->res && "Arena is out of memory");
  
  u8* buffer = (u8*)((PtrInt)arena + ARENA_HEADER) + offset;
  arena->pos = offset+size;

  ClearMemory(buffer, size);
  return buffer;
}

void tctx_init(Arena* arena) {
  tctx_thread_local.arenas[0] = arena_alloc(arena, MB(8));
  tctx_thread_local.arenas[1] = arena_alloc(arena, MB(8));
}

Temp tctx_get_scratch(Arena** conflics, u32 counts) {
  TCTX* tctx = &tctx_thread_local;
  
  Loop (i, ArrayCount(tctx->arenas)) {
    b32 isConflictingArena = false;
    Loop (z, counts) {
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

u64 align_forward_uintptr(PtrInt ptr, u64 align) {
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

Pool pool_create(Arena* arena, u64 chunk_count, u64 chunk_size, u64 chunk_alignment) {
  Pool p;

  // Align chunk size up to the required chunk_alignment
  chunk_size = align_forward_size(chunk_size, chunk_alignment);
  
  PtrInt init_start = (PtrInt)arena + arena_pos(arena);
  PtrInt start = align_forward_uintptr(init_start, chunk_alignment);
  
  u8* data = push_buffer(arena, u8, chunk_count*chunk_size, chunk_alignment);
  
  // Assert that the parameters passed are valid
  Assert(chunk_size >= sizeof(PoolFreeNode) && "Chunk size is too small");

  // Store the adjusted parameters
  p.data = data;
  p.chunk_count = chunk_count;
  p.chunk_size = chunk_size;
  p.head = null; // Free List Head

  // Set up the free list for free chunks
  pool_free_all(p);
  return p;
}

u8* pool_alloc(Pool& p) {
  // Get latest free node
  PoolFreeNode* node = p.head;

  Assert(node && "Pool allocator has no free memory");

  // Pop free node
  p.head = p.head->next;

  ClearMemory(node, p.chunk_size);
  return (u8*)node;
}

void pool_free(Pool& p, void *ptr) {
	PoolFreeNode *node;

	void *start = p.data;
	void *end = &(p.data)[p.chunk_count*p.chunk_size];

  Assert((start <= ptr && ptr < end) && "Memory is out of bounds of the buffer in this pool");

	// Push free node
	node = (PoolFreeNode *)ptr;

  Assert(!(start <= node->next && ptr < end) && "Memomy chunk is already free");
	node->next = p.head;
	p.head = node;
}

void pool_free_all(Pool& p) {
	// Set all chunks to be free
  Loop (i, p.chunk_count) {
		void* ptr = &p.data[i * p.chunk_size];
		PoolFreeNode *node = (PoolFreeNode*)ptr;
		// Push free node onto thte free list
		node->next = p.head;
		p.head = node;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Free list

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

FreeList free_list_create(Arena* arena, u64 size) {
  FreeList fl;
  fl.data = push_buffer(arena, u8, size);
  fl.size = size;
  free_list_free_all(fl);
  return fl;
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
      prev_node->next = new_node;
      new_node->next = null;
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
  // Iterates the list and finds the first free block with enough space
  FreeListNode* node = fl.head;
  FreeListNode* prev_node = null;

  u64 padding = 0;

  while (node != null) {
    padding = calc_padding_with_header((PtrInt)node, alignment, sizeof(FreeListAllocationHeader));
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

u8* free_list_alloc(FreeList& fl, u64 size, u64 alignment) {
  u64 padding = 0;
  FreeListNode* prev_node = null;
  FreeListNode* node = null;

  if (size < sizeof(FreeListNode)) {
    size = sizeof(FreeListNode);
  }

  node = free_list_find_first(fl, size, alignment, &padding, &prev_node);
  Assert(node && "Free list has no free memory");

  u64 alignment_padding = padding - sizeof(FreeListAllocationHeader);
  u64 required_space = size + padding;
  u64 remaining = node->block_size - required_space;

  if (remaining > 0) {
    FreeListNode* new_node = (FreeListNode*)((PtrInt)node + required_space);
    new_node->block_size = remaining;
    free_list_node_insert(&fl.head, node, new_node);
  }

  free_list_node_remove(&fl.head, prev_node, node);

  FreeListAllocationHeader* header_ptr = (FreeListAllocationHeader*)((PtrInt)node + alignment_padding);
  header_ptr->block_size = required_space;
  header_ptr->padding = alignment_padding;
  header_ptr->magic_value = DebugMagic;

  fl.used += required_space;

  return (u8*)header_ptr + sizeof(FreeListAllocationHeader);
}

void FreeList_coalescence(FreeList& fl, FreeListNode* prev_node, FreeListNode* free_node);

void free_list_free(FreeList& fl, void* ptr) {
  Assert(fl.data <= ptr && ptr < fl.data+fl.size && "out of bounce");
  
  FreeListAllocationHeader* header;
  FreeListNode* free_node;
  FreeListNode* node;
  FreeListNode* prev_node = null;

  header = (FreeListAllocationHeader*)((PtrInt)ptr - sizeof(FreeListAllocationHeader));
#if _DEBUG
  Assert(header->magic_value == DebugMagic);
  header->magic_value = 0;
#endif

  free_node = (FreeListNode*)((PtrInt)header - header->padding);
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

  FreeList_coalescence(fl, prev_node, free_node);
}

void FreeList_coalescence(FreeList& fl, FreeListNode* prev_node, FreeListNode* free_node) {
  if (free_node->next != null && (void*)((PtrInt)free_node + free_node->block_size) == free_node->next) {
    free_node->block_size += free_node->next->block_size;
    free_list_node_remove(&fl.head, free_node, free_node->next);
  }

  if (prev_node == null) {
    return;
  } else if (prev_node->next != null && (void*)((PtrInt)prev_node + prev_node->block_size) == free_node) {
    // prev_node->block_size += free_node->next->block_size;
    prev_node->block_size += free_node->block_size;
    free_list_node_remove(&fl.head, prev_node, free_node);
  }
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Global Allocator

struct SegPool {
	u64 chunk_count;
	u64 chunk_size;
  u64 used;
  u8* data;

	PoolFreeNode *head;
};

struct MemCtx {
  u8* start;
  SegPool pools[32];
};

global MemCtx mem_ctx;

#define PAGE_SIZE 4096
#define MIN_CHUNKS_PER_COMMIT 32

internal void pool_commit(SegPool& p, u64 commit_size) {
  u8* end_pool = (u8*)(p.chunk_size * p.chunk_count + (PtrInt)p.data);
  os_commit(end_pool, commit_size);
}

internal void pool_init_new_chunk(SegPool& p, u64 chunks_add) {
	// Set all chunks to be free
  Loop (i, chunks_add) {
		void* ptr = (u8*)&p.data[i * p.chunk_size] + p.chunk_count*p.chunk_size;
		PoolFreeNode *node = (PoolFreeNode*)ptr;
		node->next = p.head;
		p.head = node;
	}
}

internal void* segregated_pool_alloc(SegPool& p) {
  if (p.head == null) {
    if (p.used + 1 > p.chunk_count) {
      u64 commit_size;
      if (p.chunk_size > MB(1)) {
        commit_size = p.chunk_size;
      } else {
        commit_size = Clamp(PAGE_SIZE, p.chunk_size * MIN_CHUNKS_PER_COMMIT, MB(1));
      }
      u64 chunks_add = commit_size / p.chunk_size;
      pool_commit(p, commit_size);
      pool_init_new_chunk(p, chunks_add);
      p.chunk_count += chunks_add;
    }
  }
  ++p.used;
  
  void* result = p.head;
  Assert(result && "Pool allocator has no free memory");
  p.head = p.head->next;
  return result;
}

internal void segregated_pool_free(SegPool& p, void* ptr) {
	void *start = p.data;
	void *end = &(p.data)[p.chunk_count*p.chunk_size];

  Assert((start <= ptr && ptr < end) && "Memory is out of bounds of the buffer in this pool");

	PoolFreeNode* node = (PoolFreeNode*)ptr;
  // Assert(!(start <= node->next && ptr < end) && "Memomy chunk is already free");
	node->next = p.head;
	p.head = node;
  --p.used;
}

u8* mem_alloc(u64 size) {
  // Minimum size is 8 bytes
  u64 base_size = 8;

  Loop (i, ArrayCount(mem_ctx.pools)) {
    if (size <= base_size << i) {
      return (u8*)segregated_pool_alloc(mem_ctx.pools[i]);
    }
  }

  // Too large for any of our pools
  Assert(0 && "Too large for any of our pools");
  return null;
}

void mem_free(void* ptr) {
  u64 offset_num = GB(10);
  u8* base = mem_ctx.start;

  Loop (i, ArrayCount(mem_ctx.pools)) {
    u8* start = base + i * offset_num;
    u8* end = start + offset_num;

    if ((u8*)ptr >= start && (u8*)ptr < end) {
      segregated_pool_free(mem_ctx.pools[i], ptr);
      return;
    }
  }

  Assert(0 && "global_free: pointer does not belong to any pool");
}

void global_allocator_init() {
  mem_ctx.start = (u8*)os_reserve(TB(1));
  u64 offset = 0;
  u64 offset_num = GB(10);
  
  u64 pow_num = 1;
  Loop (i, ArrayCount(mem_ctx.pools)) {
    mem_ctx.pools[i].data = (u8*)mem_ctx.start + offset;
    mem_ctx.pools[i].chunk_size = 8 * pow_num;
    pow_num *= 2;
    offset += offset_num;
  };
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
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

u64 ring_read(u8 *ring_base, u64 ring_size, u64 ring_pos, void *dst_data, u64 read_size) {
  Assert(read_size <= ring_size);
  
  u64 ring_off = ring_pos % ring_size;
  u64 bytes_before_split = ring_size - ring_off;
  u64 pre_split_bytes = Min(bytes_before_split, read_size);
  u64 pst_split_bytes = read_size - pre_split_bytes;
  MemCopy(dst_data, ring_base + ring_off, pre_split_bytes);
  MemCopy((u8*)dst_data + pre_split_bytes, ring_base + 0, pst_split_bytes);
  return read_size;
}
