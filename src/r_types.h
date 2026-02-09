#pragma once
#include "lib.h"

enum RenderpassType{
  RenderpassType_World,
  RenderpassType_UI,
  RenderpassType_Screen,
};

union PushConstant {
  struct {
    u32 entity_idx;
    u32 texture_id;
  };
  u8 data[128];
};

#define MaxShaderEntity KB(1)

struct ShaderEntity {
  alignas(16) mat4 model;
  alignas(16) v4 color;
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
  f32 intensity;
  f32 rad;
};

struct DirLight {
  v3 color;
  v3 direction;
  f32 intensity;
};
struct ShaderDirLight {
  alignas(16) v3 color;
  alignas(16) v3 direction;
  f32 intensity;
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
  f32 intensity;
  f32 inner_cutoff;
  f32 outer_cutoff;
};

struct ShaderGlobalState {
  alignas(16) mat4 projection_view;
  alignas(16) mat4 projection;
  alignas(16) mat4 view;
  alignas(16) v4 ambient_color;
  f32 time;
  u32 point_light_count;
  u32 dir_light_count;
  u32 spot_light_count;
};

struct Material {
  v3 ambient;
  v3 diffuse;
  v3 specular;
  f32 shininess;
  u32 texture;
};
struct ShaderMaterial {
  alignas(16) v3 ambient;
  alignas(16) v3 diffuse;
  alignas(16) v3 specular;
  f32 shininess;
  u32 texture;
};

struct Texture {
  u32 width;
  u32 height;
  u8* data;
};

struct Vertex {
  v3 pos;
  v3 norm;
  v2 uv;
  v3 color;
};

struct Mesh {
  Vertex* vertices;
  u32* indexes;
  u32 vert_count;
  u32 index_count;
};

enum ShaderTopology {
  ShaderTopology_Triangle,
  ShaderTopology_Line,
  ShaderTopology_Point,
};

struct ShaderInfo {
  ShaderTopology primitive = ShaderTopology_Triangle;
  u8 stage_count = 2;
  b8 is_transparent = false;
  b8 use_depth = true;
};

enum ShaderType {
  // Drawing
  ShaderType_Drawing,
  ShaderType_DrawingTransparent,
  ShaderType_DrawingTransparentLine,
  ShaderType_Drawing_COUNT,

  // Screen
  ShaderType_Screen,
  ShaderType_Screen_COUNT,

  // Cubemap
  ShaderType_Cubemap,
  ShaderType_Cubemap_COUNT,

  // Compute
  ShaderType_Compute,
  ShaderType_Compute_COUNT,
};

struct Shader {
  String name;
  u32 id;
  ShaderType type;
};

inline ShaderInfo shader_type[] = {
  // Drawing
  [ShaderType_Drawing] = {},
  [ShaderType_DrawingTransparent] = {
    .is_transparent = true,
  },
  [ShaderType_DrawingTransparentLine] = {
    .primitive = ShaderTopology_Line,
    .is_transparent = true,
  },

  // Screen
  [ShaderType_Screen] = {
    .use_depth = false,
  },

  // Cubemap
  [ShaderType_Cubemap] = {},

  // Compute
  [ShaderType_Compute] = {},
};

