#pragma once
#include "lib.h"

#include "render/r_types.h"

KAPI void geometry_init();

KAPI void geometry_create(Geometry geometry);
KAPI void geometry_destroy(u32 id);

KAPI Geometry& geometry_get(String name);
