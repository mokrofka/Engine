#pragma once
#include "lib.h"
#include "r_types.h"
#include "vk.h"

////////////////////////////////////////////////////////////////////////
// Renderer
KAPI void r_init();
KAPI void r_shutdown();
KAPI void r_begin_draw_frame();
KAPI void r_end_draw_frame();

////////////////////////////////////////////////////////////////////////
// Mesh
KAPI void mesh_init();
KAPI u32 mesh_create(String name);
KAPI void mesh_destroy(u32 id);
KAPI Mesh mesh_get(String name);

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
