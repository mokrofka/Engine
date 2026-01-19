#pragma once
#include "lib.h"

enum RenderpassType{
  RenderpassType_World,
  RenderpassType_UI,
  RenderpassType_Screen,
};

struct PushConstant {
  // mat4 model;
  u32 entity_idx;
  u32 texture_id;
};

struct ShaderEntity {
  alignas(16) mat4 model;
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
  alignas(16) mat4 projection;
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
  u32 width;
  u32 height;
  u8 channel_count;
  u8* data;
};

#define VertexAttributeCount 3
struct Vertex {
  v3 pos;
  v3 norm;
  v2 uv;
};

struct Mesh {
  Vertex* vertices;
  u32* indexes;
  u32 vert_count;
  u32 index_count;
};

struct RenderEntity {
  u32 shader_handle;
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
  ShaderType_Drawing_COUNT,

  // Screen
  ShaderType_Screen,
  ShaderType_Screen_COUNT,

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

  // Screen
  [ShaderType_Screen] = {
    .use_depth = false,
  },

  // Compute
  [ShaderType_Compute] = {},
};
