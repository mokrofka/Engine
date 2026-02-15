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

struct DrawLine {
  v3 a;
  v3 b;
  v3 color;
};
struct ShaderDrawLine {
  alignas(16) mat4 model;
  alignas(16) v3 color;
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
  u32 stage_count = 2;
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

u32 vk_mesh_load(Mesh mesh);

void vk_init();
void vk_shutdown();

void vk_begin_frame();
void vk_end_frame();
void vk_begin_renderpass(RenderpassType renderpass);
void vk_end_renderpass(RenderpassType renderpass);

KAPI u32 vk_shader_load(String name, ShaderType type);
KAPI u32 vk_texture_load(Texture t);
KAPI u32 vk_material_load(Material material);
KAPI u32 vk_cubemap_load(Texture* textures);

void vk_draw();
void vk_draw_screen();
void vk_draw_compute();

KAPI void vk_update_transform(u32 entity_id, Transform trans);

// Entity
KAPI u32 vk_make_renderable(u32 entity_id, u32 mesh_id, u32 shader_id, u32 material_id);
KAPI void vk_remove_renderable(u32 entity_id);
KAPI ShaderEntity& vk_get_entity(u32 entity_id);

// Point light
KAPI void vk_point_light_create(u32 entity_id);
KAPI void vk_point_light_remoove(u32 entity_id);
KAPI PointLight& vk_get_point_light_shader(u32 entity_id);

// Directional light
KAPI void vk_dir_light_make(u32 entity_id);
KAPI void vk_dir_light_remove(u32 entity_id);
KAPI DirLight& vk_dir_light_get(u32 entity_id);

// Spot light
KAPI void vk_spot_light_create(u32 entity_id);
KAPI void vk_spot_light_destroy(u32 entity_id);
KAPI SpotLight& vk_spot_light_get(u32 entity_id);

// Util
KAPI PushConstant& vk_get_push_constant(u32 id);
KAPI ShaderGlobalState* vk_get_shader_state();
KAPI void vk_shader_reload(String name, u32 id);

////////////////////////////////////////////////////////////////////////
// Debug drawing

KAPI void debug_draw_line(v3 a, v3 b, v3 color);

