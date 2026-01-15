#include "mem.h"

struct TCTX {
  Arena arenas[2];
};

global thread_local TCTX tctx;

void tctx_init() {
  tctx.arenas[0] = arena_init();
  tctx.arenas[1] = arena_init();
}

Temp tctx_get_scratch() {
  return temp_begin(&tctx.arenas[0]);
}

Temp tctx_get_scratch(Allocator conflict) {
  Arena* arena_conflict = (Arena*)conflict.ctx;
  Arena* arena_result = {};

  for (Arena& arena : tctx.arenas) {
    b32 is_conflicting_arena = false;
    if (arena.base == arena_conflict->base) {
      is_conflicting_arena = true;
    }

    if (!is_conflicting_arena) {
      arena_result = &arena;
      break;
    }
  }

  return temp_begin(arena_result);
}

