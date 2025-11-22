#include "base_inc.h"

struct TCTX {
  Arena* arenas[2];
};

global thread_local TCTX tctx_thread_local;

void tctx_init() {
  tctx_thread_local.arenas[0] = arena_alloc();
  tctx_thread_local.arenas[1] = arena_alloc();
}

Temp tctx_get_scratch(Arena** conflics, u64 counts) {
  TCTX& tctx = tctx_thread_local;
  
  Loop (i, ArrayCount(tctx.arenas)) {
    b32 is_conflicting_arena = false;
    Loop (z, counts) {
      if (tctx.arenas[i] == conflics[z]) {
        is_conflicting_arena = true;
      }
    }

    if (!is_conflicting_arena) {
      return temp_begin(tctx.arenas[i]);
    }
  }
  return {};
}
