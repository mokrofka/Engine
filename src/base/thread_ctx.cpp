#include "base_impl.h"

global thread_local TCTX tctx;
global u32 _next_thread_id;

u32 tctx_get_id() { return tctx.id; }

void tctx_init() {
  tctx.arenas[0] = arena_make();
  tctx.arenas[1] = arena_make();
  tctx.id = atomic_inc(&_next_thread_id);
}

intern Temp tctx_get_scratch() {
  Arena* arena = &tctx.arenas[0];
  return temp_begin(arena);
}

intern Temp tctx_get_scratch_conflict(Allocator conflict) {
  Arena* arena_conflict = (Arena*)conflict.ctx;
  Arena* arena_result = {};
  if (arena_conflict == &tctx.arenas[0]) {
    arena_result = &tctx.arenas[1];
  } else {
    Assert(arena_conflict == &tctx.arenas[1]);
    arena_result = &tctx.arenas[0];
  }
  return temp_begin(arena_result);
}

Scratch::operator Allocator()        { return {.type = AllocatorType_Arena, .ctx = temp.arena}; }
Scratch::Scratch()                   { temp = tctx_get_scratch(); }
Scratch::Scratch(Allocator conflict) { temp = tctx_get_scratch_conflict(conflict); }
Scratch::~Scratch()                  { temp_end(temp); }


