#pragma once
#include "lib.h"

struct Texture {
  u32 id;
  String filepath;
  u32 width;
  u32 height;
  u8 channel_count;
  b8 has_transparency;
  u8* data;
};

struct Geometry {
  u32 id;
  String name;
  
  u32 vertex_count;
  u32 vertex_size;
  void* vertices;
  u32 index_size;
  u32 index_count;
  void* indices;
};

enum ShaderTopology {
  ShaderTopology_Triangle,
  ShaderTopology_Line,
  ShaderTopology_Point,
};

enum ShaderType {
  ShaderType_Gfx,
  ShaderType_Screen,
  ShaderType_Compute,
};

struct Shader {
  u32 id;
  String name;
  ShaderTopology primitive;
  ShaderType type;
  u8 stages;
  b8 is_transparent;
  u8 attribut[10];
};
