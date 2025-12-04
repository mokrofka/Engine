#pragma once
#include "mem.h"

KAPI Temp tctx_get_scratch(Arena** conflics, u64 counts);

struct Scratch {
  Temp temp;
  
  INLINE operator Arena*() { return temp.arena; }

  INLINE Scratch() {
    temp = tctx_get_scratch(null, 0);
  }
  INLINE Scratch(Arena** conflics) {
    temp = tctx_get_scratch(conflics, 1);
  }
  INLINE ~Scratch() { temp.arena->pos = temp.pos; };
};

KAPI void tctx_init();

// deprecated
#define GetScratch(conflicts, count) (tctx_get_scratch((conflicts), (count)))
#define ReleaseScratch(scratch) temp_end(scratch)
