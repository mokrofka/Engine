#pragma once
#include "lib.h"

KAPI void entity_init();

KAPI void entity_make_renderable(u32 id, u32 mesh_id, u32 shader_id);
KAPI void entity_remove_renderable(u32 id);
KAPI void entity_make_light(u32 id);
