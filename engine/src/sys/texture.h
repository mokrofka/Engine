#pragma once
#include "lib.h"

#include "render/r_types.h"

struct TextureSystemConfig {
  u32 max_texture_count;
};

#define DefaultTextureName "default"_

void texture_system_init(Arena* arena, TextureSystemConfig config);

KAPI void texture_load(String name);
