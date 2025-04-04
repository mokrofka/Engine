#pragma once

#include "defines.h"

KAPI void* _memory_zero(void* block, u64 size);
KAPI void* _memory_copy(void* dest, const void* source, u64 size);
KAPI void* _memory_set(void* dest, i32 value, u64 size);
KAPI b8    _memory_compare(void* a, void* b, u64 size);

#define ARENA_HEADER sizeof(Arena)

struct Arena {
  u64 res;
  u64 pos;
};

struct TCTX {
  Arena* arenas[2];
};

struct Temp {
  Arena* arena;
  u64 pos;
};

struct Scratch {
  Arena* arena;
  u64 pos;
  
  KAPI Scratch(Arena** conflics = null, u32 counts = 0);
  KAPI ~Scratch();
};

#define DEFAULT_ALIGNMENT (sizeof(void*))

KAPI Arena* arena_alloc(Arena *a, u64 size = MB(1), u64 align = DEFAULT_ALIGNMENT);
KAPI Arena* arena_alloc(u64 size = MB(1));

KAPI u64 arena_pos(Arena* arena);

KAPI void* _arena_push(Arena* arena, u64 size, u64 align = DEFAULT_ALIGNMENT);
KAPI void* _arena_push(Temp arena, u64 size, u64 align = DEFAULT_ALIGNMENT);
KAPI void* _arena_push(Scratch arena, u64 size, u64 align = DEFAULT_ALIGNMENT);

KAPI Temp temp_begin(Arena* arena);
KAPI void temp_end(Temp temp);

KAPI void tctx_init(struct Arena* arena);
KAPI Temp tctx_get_scratch(Arena** conflics, u32 counts);

#define push_array(a, T, c) (T*)_arena_push(a, sizeof(T)*c, Max(8, alignof(T)))
#define push_struct(a, T) (T*)_arena_push(a, sizeof(T), Max(8, alignof(T)))
#define push_buffer(a, T, c) (T*)_arena_push(a, c, 8)
  
#define GetScratch(conflicts, count) (tctx_get_scratch((conflicts), (count)))
#define ReleaseScratch(scratch) temp_end(scratch)


