#pragma once

#include "defines.h"

KAPI void* zero_memory(void* block, u64 size);
KAPI void* copy_memory(void* dest, const void* source, u64 size);
KAPI void* set_memory(void* dest, i32 value, u64 size);
KAPI b8 compare_memory(void* a, void* b, u64 size);

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

#define DEFAULT_ALIGNMENT (sizeof(void*))

KAPI Arena* arena_alloc(Arena *a, u64 size = MB(1), u64 align = DEFAULT_ALIGNMENT);
KAPI Arena* arena_alloc(u64 size = MB(1));

KAPI u64 arena_pos(Arena* arena);

KAPI void *arena_push(Arena *arena, u64 size, u64 align = DEFAULT_ALIGNMENT);
KAPI void *arena_push(Temp arena, u64 size, u64 align = DEFAULT_ALIGNMENT);

KAPI Temp temp_begin(Arena* arena);

KAPI void temp_end(Temp temp);

KAPI void tctx_initialize(struct Arena* arena);
KAPI Temp tctx_get_scratch(Arena** conflics, u32 counts);

#define push_array(a, T, c) (T*)arena_push(a, sizeof(T)*c, Max(8, alignof(T)))
#define push_struct(a, T) (T*)arena_push(a, sizeof(T), Max(8, alignof(T)))
#define push_buffer(a, T, c) (T*)arena_push(a, c, 8)
  
#define GetScratch(conflicts, count) (tctx_get_scratch((conflicts), (count)))
#define ReleaseScratch(scratch) temp_end(scratch)
