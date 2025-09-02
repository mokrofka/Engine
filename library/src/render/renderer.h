#pragma once
#include "lib.h"
#include "render/r_types.h"
#include "vulkan/vk_api.h"

struct Vertex {
  v3 pos;
  v3 norm;
  v3 texcoord;
};

struct Mesh {
  Vertex* vertices;
  u32* indexes;
};

////////////////////////////////////////////////////////////////////////
// Renderer
KAPI void r_init();
KAPI void r_shutdown();
KAPI void r_begin_draw_frame();
KAPI void r_end_draw_frame();

////////////////////////////////////////////////////////////////////////
// Geometry
KAPI void geometry_init();
KAPI void geometry_create(Geometry geometry);
KAPI void geometry_destroy(u32 id);
KAPI Geometry& geometry_get(String name);
KAPI void mesh_upload(Mesh mesh);

////////////////////////////////////////////////////////////////////////
// Resource
KAPI void res_sys_init(String asset_path);
String res_sys_base_path();
Buffer res_binary_load(Arena* arena, String filename);

////////////////////////////////////////////////////////////////////////
// Shader
KAPI Shader& shader_get(String name);
KAPI void shader_init();
KAPI void shader_create(Shader shader);

////////////////////////////////////////////////////////////////////////
// Texture
#define DefaultTextureName "default"
KAPI void texture_init();
Texture res_texture_load(String name);
KAPI void texture_load(String name);
KAPI Texture& texture_get(String name);
