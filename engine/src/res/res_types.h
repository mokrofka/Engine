#pragma once
#include "lib.h"

struct Texture {
  u32 id;
  u32 width;
  u32 height;
  u8 channel_count;
  b8 has_transparency;
  u32 generation;
  String64 name;
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

struct Material {
  u32 id;
  u32 generation;
  u32 internal_id;
  String64 name;
  v4 diffuse_color;
  TextureMap diffuse_map;
};

struct Geometry {
  u32 id;
  u32 internal_id;
  u32 generation;
  String64 name;
  Material* material;
};
