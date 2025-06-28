#pragma once
#include "lib.h"

#include "render/r_types.h"

KAPI void geometry_init();

KAPI u32 geometry_create(Geometry& geometry);
KAPI void geometry_destroy(u32 id);

KAPI u32 geometry_get(String name);
