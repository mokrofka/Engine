#pragma once
#include "lib.h"

#include "res/res_types.h"

struct ResSysConfig {
  u32 max_loader_count;
  // The relative base path for assets
  String asset_base_path;
};

struct ResLoader {
  u32 id;
  ResType type;
  String64 custom_type64;
  String64 type_path64;
  b32 (*load)(ResLoader* self, String name, Res* out_res);
  void (*unload)(ResLoader* self, Res* res);
};

void res_sys_init(Arena* arena, ResSysConfig config);
void res_sys_shutdown();

KAPI b32 res_sys_register_loader(ResLoader loader);

KAPI b32 res_sys_load(String name, ResType type, Res* out_res);
KAPI b32 res_sys_load_custom(String name, String custom_type, Res* out_res);

KAPI void res_sys_unload(Res* res);

KAPI String res_sys_base_path();
