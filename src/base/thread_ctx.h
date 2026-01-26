#pragma once
#include "mem.h"

KAPI Temp tctx_get_scratch();
KAPI Temp tctx_get_scratch(Allocator conflict);

struct Scratch {
  Temp temp;
  INLINE operator Allocator()        { return {.type = AllocatorType_Arena, .ctx = temp.arena}; }
  INLINE Scratch()                   { temp = tctx_get_scratch(); }
  INLINE Scratch(Allocator conflict) { temp = tctx_get_scratch(conflict); }
  INLINE ~Scratch()                  { temp.arena->pos = temp.pos; };
};

KAPI void tctx_init();
