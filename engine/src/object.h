#pragma once
#include "lib.h"

struct Object {
  u32 id;
  u32 mesh_id;
  u32 shader_id;
};

KAPI void object_sys_init();
KAPI Object object_create(u32 mesh_id, u32 shader_id);
KAPI void object_make_renderable(Object obj);
