#pragma once
#include "lib.h"
#include "r_types.h"
#include "vk.h"

#define MaxLights KB(1)
#define MaxEntities KB(4)
#define MaxMeshes KB(1)
#define MaxShaders KB(1)
#define MaxTextures KB(1)

KAPI extern f32 delta_time; // I dond't want where it should be

// Renderer
KAPI void r_init();
KAPI void r_shutdown();
KAPI void r_begin_draw_frame();
KAPI void r_end_draw_frame();

// Mesh
KAPI void mesh_init();
KAPI u32 mesh_create(String name);
KAPI void mesh_destroy(u32 id);
KAPI Mesh mesh_get(String name);

// Resource
KAPI void asset_init(String asset_path);
String asset_base_path();

// Shader
KAPI u32 shader_create(String shader, ShaderType type);
KAPI Shader& shader_get(String name);
KAPI void shader_init();

// Texture
KAPI void texture_init();
KAPI u32 texture_load(String name);
KAPI Texture& texture_get(String name);

////////////////////////////////////////////////////////////////////////
// Shaders

struct ShaderDefinition {
  String path;
  ShaderType type;
};

enum {
  Shader_Color,
  Shader_COUNT,
};

inline ShaderDefinition shaders_definition[Shader_COUNT] = {
  [Shader_Color] = "color_shader", ShaderType_Drawing,
};

KAPI extern u32 shaders[Shader_COUNT];

////////////////////////////////////////////////////////////////////////
// Meshes
enum {
  Mesh_Cube,
  Mesh_Room,
  Mesh_COUNT,
};

inline String meshs_path[Mesh_COUNT] = {
  [Mesh_Cube] = "cube.obj",
  // [Mesh_Room] = "room.obj",
};

KAPI extern u32 meshes[Mesh_COUNT];

////////////////////////////////////////////////////////////////////////
// Textures
enum {
  Texture_OrangeLines,
  Texture_Container,
  Texture_Room,
  Texture_COUNT,
};

inline String textures_path[Texture_COUNT] = {
  [Texture_OrangeLines] = "orange_lines_512.png",
  [Texture_Container] = "container.jpg",
  [Texture_Room] = "image.png",
};

KAPI extern u32 textures[Texture_COUNT];

