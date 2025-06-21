#pragma once
#include "lib.h"

#include "render/r_types.h"

#define DefaultTextureName "default"

void texture_init(Arena* arena);

KAPI void texture_load(String name);
KAPI u32 texture_get(String name);
