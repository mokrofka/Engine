#pragma once
#include "mem.h"

KAPI Temp tctx_get_scratch(Arena** conflics, u64 counts);

struct Scratch {
  Arena* arena;
  u64 pos;
  
  INLINE operator Arena*() { return this->arena; }

  INLINE Scratch() {
    Temp temp = tctx_get_scratch(null, 0);
    *this = As(Scratch)&temp;
  }
  INLINE Scratch(Arena** conflics) {
    Temp temp = tctx_get_scratch(conflics, 1);
    *this = As(Scratch)&temp;
  }
  INLINE ~Scratch() { arena->pos = pos; };
};

KAPI void tctx_init();

// deprecated
#define GetScratch(conflicts, count) (tctx_get_scratch((conflicts), (count)))
#define ReleaseScratch(scratch) temp_end(scratch)
