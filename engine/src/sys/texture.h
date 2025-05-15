#pragma once
#include "lib.h"

#include "render/r_types.h"

struct TextureSystemConfig {
  u32 max_texture_count;
};

#define DefaultTextureName "default"_

void texture_system_init(Arena* arena, TextureSystemConfig config);
void texture_system_shutdown();

Texture* texture_system_acquire(String name, b32 auto_release);
void texture_system_release(String name);

Texture* texture_system_get_default_texture();

KAPI void texture_load(String name);
