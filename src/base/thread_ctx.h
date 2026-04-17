#pragma once
#include "mem.h"

struct Scratch {
  Temp temp;
  NO_DEBUG operator Allocator();
  NO_DEBUG Scratch();
  NO_DEBUG Scratch(Allocator conflict);
  NO_DEBUG ~Scratch();
};

void tctx_init();
