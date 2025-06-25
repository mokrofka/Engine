#pragma once
#include "lib.h"

#include "res/res_types.h"

struct ResSysConfig {
  // The relative base path for assets
  String asset_base_path;
};

KAPI void res_sys_init(ResSysConfig config);

String res_sys_base_path();

//////////////////////////////////////////////////////////
// Loaders

Buffer res_binary_load(Arena* arena, String filename);

