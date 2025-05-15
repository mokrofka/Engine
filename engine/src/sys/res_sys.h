#pragma once
#include "lib.h"

#include "res/res_types.h"

struct ResSysConfig {
  // The relative base path for assets
  String asset_base_path;
};

struct ResLoader {
  u32 id;
  ResType type;
  String64 custom_type64;
  String64 type_path64;
  b32 (*load)(Arena* arena, ResLoader* self, String name, Res* out_res);
  void (*unload)(ResLoader* self, Res* res);
};

void res_sys_init(Arena* arena, ResSysConfig config);
void res_sys_shutdown();

void res_sys_register_loader(ResLoader loader);

b32 res_sys_load(String name, ResType type, Res* out_res);
b32 res_sys_load(Arena* arena, String name, ResType type, Res* out_res);
b32 res_sys_load_custom(String name, String custom_type, Res* out_res);

void res_sys_unload(Res* res);

String res_sys_base_path();

//////////////////////////////////////////////////////////
// Loaders

Binary res_binary_load(Arena* arena, String filepath);
KAPI Texture res_texture_load(String filepath);
void res_texture_unload(void* data);
MaterialConfig res_load_material_config(String filepath);

