#pragma once
#include "lib.h"

#include "res/res_types.h"

#define DEFAULT_MATERIAL_NAME "default"

struct MaterialSystemConfig {
  u32 max_material_count;
};

struct MaterialConfig {
  char name[MATERIAL_NAME_MAX_LENGTH];
  b8 auto_release;
  v4 diffuse_color;
  char diffuse_map_name[TEXTURE_NAME_MAX_LENGTH];
};

void material_system_init(Arena* arena, MaterialSystemConfig);
void material_system_shutdown();

Material* material_system_acquire(char* name);
Material* material_system_acquire_from_config(MaterialConfig);
void material_sys_release(String name);

Material* material_sys_get_default();
