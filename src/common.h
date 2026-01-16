#pragma once
#include "lib.h"
#include "r_types.h"
#include "vk.h"

KAPI extern f32 delta_time; // I dond't know where it should be

KAPI void common_init();
KAPI void r_shutdown();
KAPI void r_begin_draw_frame();
KAPI void r_end_draw_frame();

// Assets
String asset_base_path();

// Mesh
KAPI u32 mesh_load(String name);

// Shader
KAPI u32 shader_load(String shader, ShaderType type);

// Texture
KAPI u32 texture_load(String name);

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

KAPI extern ShaderDefinition shaders_definition[Shader_COUNT];
KAPI extern u32 shaders[Shader_COUNT];

////////////////////////////////////////////////////////////////////////
// Meshes

enum {
  Mesh_Cube,
  Mesh_Room,
  Mesh_COUNT,
};

KAPI extern String meshs_path[Mesh_COUNT];
KAPI extern u32 meshes[ArrayCount(meshs_path)];

////////////////////////////////////////////////////////////////////////
// Textures
enum {
  Texture_OrangeLines,
  Texture_Container,
  Texture_Room,
  Texture_COUNT,
};

KAPI extern String textures_path[Texture_COUNT];
KAPI extern u32 textures[Texture_COUNT];

