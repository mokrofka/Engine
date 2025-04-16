#pragma once
#include "lib.h"

#include "render/r_types.h"

struct TextureSystemConfig {
  u32 max_texture_count;
};

#define DEFAULT_TEXTURE_NAME "default"

void texture_system_init(Arena* arena, TextureSystemConfig config);
void texture_system_shutdown();

Texture* texture_system_acquire(String name, b8 auto_release);
void texture_system_release(String name);

Texture* texture_system_get_default_texture();

