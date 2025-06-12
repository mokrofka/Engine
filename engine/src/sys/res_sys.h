#pragma once
#include "lib.h"

#include "res/res_types.h"

struct ResSysConfig {
  // The relative base path for assets
  String asset_base_path;
};

void res_sys_init(Arena* arena, ResSysConfig config);

String res_sys_base_path();

//////////////////////////////////////////////////////////
// Loaders

Binary res_binary_load(Arena* arena, String filepath);
MaterialConfig res_load_material_config(String filepath);

