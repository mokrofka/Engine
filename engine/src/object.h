#pragma once
#include "lib.h"

KAPI void entity_init();
KAPI u32 entity_create();
KAPI void entity_destroy(u32 id);
KAPI void object_make_renderable(u32 id, u32 mesh_id);
