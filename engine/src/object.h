#pragma once
#include "lib.h"

KAPI void entity_init();
// KAPI u32 entity_create();
// KAPI void entity_destroy(u32 id);
KAPI void entity_make_renderable(u32 id, u32 mesh_id, u32 shader_id);
KAPI void entity_remove_renderable(u32 id);
