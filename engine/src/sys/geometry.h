#pragma once
#include "lib.h"

#include "render/r_types.h"

struct GeometrySysConfig {
  u32 max_geometry_count;
};

void geometry_sys_init(Arena* arena, GeometrySysConfig config);

KAPI u32 geometry_create(Geometry geometry);
KAPI void geometry_destroy(u32 id);
