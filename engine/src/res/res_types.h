#pragma once
#include "lib.h"

enum ResType {
  ResType_Text,
  ResType_Binary,
  ResType_Image,
  ResType_Material,
  ResType_StaticMesh,
  ResType_Custom,
};

struct Res {
  u32 loader_id;
  String64 name64;
  String64 file_path64;
  u64 data_size;
  u8* data;
};

struct ImageResData {
  u8 channel_count;
  u32 width;
  u32 height;
  u8* pixels;
};

struct Texture {
  u32 id;
  u32 width;
  u32 height;
  u8 channel_count;
  b8 has_transparency;
  u32 generation;
  String64 name64;
  void* internal_data;
};

enum TextureUse {
  TextureUse_Unknown = 0x00,
  TextureUse_MapDiffuse  = 0x01,
};

struct TextureMap {
  Texture* texture;
  TextureUse use;
};

struct MaterialConfig {
  String64 name64;
  b8 auto_release;
  v4 diffuse_color;
  String64 diffuse_map_name64;
};

struct Material {
  u32 id;
  u32 generation;
  u32 internal_id;
  String64 name64;
  v4 diffuse_color;
  TextureMap diffuse_map;
};

struct Geometry {
  u32 id;
  u32 internal_id;
  u32 generation;
  String64 name64;
  Material* material;
};
