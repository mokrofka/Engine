#include "memory.h"

#include "os.h"
#include "logger.h"

global thread_local TCTX tctx_thread_local;

void* _memory_zero(void* block, u64 size) {
  return _platform_memory_zero(block, size);
}

void* _memory_copy(void* dest, const void* source, u64 size) {
  return _platform_memory_copy(dest, source, size);
}

void* _memory_set(void* dest, i32 value, u64 size) {
  return _platform_memory_set(dest, value, size);
}

b8 _memory_compare(void* a, void* b, u64 size) {
  return _platform_memory_compare(a, b, size);
}

internal b8 is_power_of_two(u64 x) {
	return (x & (x-1)) == 0;
}

// Arena

internal u64 align_forward(u64 ptr, u64 align) {
	u64 p, a, modulo;
  
	if (!is_power_of_two(align)) {
    Fatal("isn't power of two");
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
	// Return NULL if the arena is out of memory (or handle differently)
  Fatal("Arena is out of memory!");
  Assert(true);
  return 0;
}

Arena* arena_alloc(u64 size) {
  Arena* arena = (Arena*)os_allocate(size, true);
  arena->pos = 0;
  arena->res = size;
  return arena;
}

void arena_clear(Arena *arena) {
	arena->pos = 0;
}

u64 arena_pos(Arena *arena) {
  Arena *current = arena;
  // u64 pos = current->base_pos + current->pos;
  u64 pos = ARENA_HEADER + current->pos;
  return pos;
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
	// Return NULL if the arena is out of memory (or handle differently)
  Fatal("Arena is out of memory!");
  Assert(true);
  return 0;
}

void* _arena_push(Temp arena, u64 size, u64 align) {
  // Align 'curr_offset' forward to the specified alignment
  u64 curr_ptr = (u64)arena.arena + ARENA_HEADER + arena.arena->pos;
  u64 offset = align_forward(curr_ptr, align);
  u64 temp = (u64)arena.arena + ARENA_HEADER;
	offset -= temp; // Change to relative offset

	// Check to see if the backing memory has space left
	if (offset+size <= arena.arena->res) {
		u8* buffer = (u8*)((u64)arena.arena + ARENA_HEADER) + offset;
		arena.arena->pos = offset+size;

    // MemZero(buffer, size);
		return buffer;
	}
	// Return NULL if the arena is out of memory (or handle differently)
  Fatal("Arena is out of memory!");
  Assert(true);
  return 0;
}

void* _arena_push(Scratch arena, u64 size, u64 align) {
  // Align 'curr_offset' forward to the specified alignment
  u64 curr_ptr = (u64)arena.arena + ARENA_HEADER + arena.arena->pos;
  u64 offset = align_forward(curr_ptr, align);
  u64 temp = (u64)arena.arena + ARENA_HEADER;
	offset -= temp; // Change to relative offset

	// Check to see if the backing memory has space left
	if (offset+size <= arena.arena->res) {
		u8* buffer = (u8*)((u64)arena.arena + ARENA_HEADER) + offset;
		arena.arena->pos = offset+size;

    // MemZero(buffer, size);
		return buffer;
	}
	// Return NULL if the arena is out of memory (or handle differently)
  Fatal("Arena is out of memory!");
  Assert(true);
  return 0;
}

Temp temp_begin(Arena* arena) {
  Temp temp = {arena, arena->pos};
  return temp;
}

void temp_end(Temp temp) {
  temp.arena->pos = temp.pos;
}

void tctx_initialize(Arena* arena) {
  tctx_thread_local.arenas[0] = arena_alloc(arena, MB(32));
  tctx_thread_local.arenas[1] = arena_alloc(arena, MB(32));
}

Scratch tctx_get_scratch_test(Arena** conflics, u32 counts) {
  TCTX* tctx = &tctx_thread_local;
  
  for (u32 i = 0; i < ArrayCount(tctx->arenas); i++) {
    b8 isConflictingArena = false;
    for (u32 z = 0; z < counts; z++) {
      if (tctx->arenas[i] == conflics[z]) {
        isConflictingArena = true;
      }
    }

    if (!isConflictingArena) {
      Temp temp = temp_begin(tctx->arenas[i]);
      Scratch* scratch = (Scratch*)&temp;
      return *scratch;
    }
  }
  // Temp temp = {null, 0};
  Scratch scratch = {};

  return scratch;
}

Scratch::Scratch(Arena** conflics, u32 counts) {
  *this = tctx_get_scratch_test(conflics, counts);
}
Scratch::~Scratch() {
  arena->pos = pos; 
}

Temp tctx_get_scratch(Arena** conflics, u32 counts) {
  TCTX* tctx = &tctx_thread_local;
  
  for (u32 i = 0; i < ArrayCount(tctx->arenas); i++) {
    b8 isConflictingArena = false;
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

