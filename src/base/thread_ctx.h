#pragma once
#include "mem.h"

KAPI Temp tctx_get_scratch();
KAPI Temp tctx_get_scratch(Allocator conflict);

struct Scratch {
  Temp temp;
  // NO_DEBUG operator Allocator();
  // NO_DEBUG Scratch();
  // NO_DEBUG Scratch(Allocator conflict);
  // NO_DEBUG ~Scratch();
  NO_DEBUG operator Allocator()        { return {.type = AllocatorType_Arena, .ctx = temp.arena}; }
  NO_DEBUG Scratch()                   { temp = tctx_get_scratch(); }
  NO_DEBUG Scratch(Allocator conflict) { temp = tctx_get_scratch(conflict); }
  NO_DEBUG ~Scratch()                  { temp.arena->pos = temp.pos; };
};

KAPI void tctx_init();
