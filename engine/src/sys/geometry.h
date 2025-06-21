#pragma once
#include "lib.h"

#include "render/r_types.h"

void geometry_init(Arena* arena);

KAPI u32 geometry_create(Geometry& geometry);
KAPI void geometry_destroy(u32 id);

KAPI u32 geometry_get(String name);
