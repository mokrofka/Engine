#pragma once
#include "lib.h"

#include "res/res_types.h"

#define DEFAULT_MATERIAL_NAME "default"_

struct MaterialSystemConfig {
  u32 max_material_count;
};

struct MaterialConfig {
  String64 name;
  b8 auto_release;
  v4 diffuse_color;
  String64 diffuse_map_name;
};

void material_system_init(Arena* arena, MaterialSystemConfig);
void material_system_shutdown();

Material* material_system_acquire(String name);
Material* material_system_acquire_from_config(MaterialConfig);
void material_sys_release(String name);

Material* material_sys_get_default();
