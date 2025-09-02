#pragma once
#include "lib.h"

enum {
  Renderpass_World,
  Renderpass_UI,
  Renderpass_Screen,
};

struct PushConstant {
  mat4 model;
  u32 entity_index;
};

struct ShaderEntity {
  alignas(16) v3 color;
  // alignas(16) v3 ambient;
  // alignas(16) v3 diffuse;
  // alignas(16) v3 specular;
  // alignas(4) f32 shininess;
};

struct PointLight {
  v3 color;
  v3 pos;
  f32 intensity;
  f32 rad;
};

struct ShaderPointLight {
  alignas(16) v3 color;
  alignas(16) v3 pos;
  alignas(4) f32 intensity;
  alignas(4) f32 rad;
};

struct DirLight {
  v3 color;
  v3 direction;
  f32 intensity;
};

struct ShaderDirLight {
  alignas(16) v3 color;
  alignas(16) v3 direction;
  alignas(4) f32 intensity;
};

struct SpotLight {
  v3 color;
  v3 pos;
  v3 dir;
  f32 intensity;
  f32 inner_cutoff;
  f32 outer_cutoff;
};

struct ShaderSpotLight {
  alignas(16) v3 color;
  alignas(16) v3 pos;
  alignas(16) v3 dir;
  alignas(4) f32 intensity;
  alignas(4) f32 inner_cutoff;
  alignas(4) f32 outer_cutoff;
};

struct ShaderGlobalState {
  alignas(16) mat4 projection_view;
  alignas(16) mat4 view;
  alignas(16) v4 ambient_color;
  alignas(4) f32 time;
  alignas(4) u32 point_light_count;
  alignas(4) u32 dir_light_count;
  alignas(4) u32 spot_light_count;
};

struct Vertex3D {
  v3 pos;
  v3 normal;
  v2 texcoord;
};

struct Vertex2D {
  v2 position;
  v2 texcoord;
};


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
