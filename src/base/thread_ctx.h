#pragma once
#include "mem.h"

struct TCTX {
	Arena arenas[2];
	u32 id;
};

struct Scratch {
	Temp temp;
	NO_DEBUG operator Allocator();
	Scratch();
	NO_DEBUG Scratch(Allocator conflict);
	NO_DEBUG ~Scratch();
};

void tctx_init();
u32 tctx_get_id();
