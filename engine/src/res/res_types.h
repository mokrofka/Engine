#pragma once

#include "math/math_types.h"

#define TEXTURE_NAME_MAX_LENGTH 512

struct Texture {
  u32 id;
  u32 width;
  u32 height;
  u8 channel_count;
  b8 has_transparency;
  u32 generation;
  char name[TEXTURE_NAME_MAX_LENGTH];
  void* internal_data;
};

enum TextureUse {
  TEXTURE_USE_UNKNOWN = 0x00,
  TEXTURE_USE_MAP_DIFFUSE = 0x01,
};

struct TextureMap {
  Texture* texture;
  TextureUse use;
};

#define MATERIAL_NAME_MAX_LENGTH 256

struct Material {
  u32 id;
  u32 generation;
  u32 internal_id;
  char name[MATERIAL_NAME_MAX_LENGTH];
  v4 diffuse_color;
  TextureMap diffuse_map;
};

