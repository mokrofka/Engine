#pragma once
#include "lib.h"

#include "render/r_types.h"

struct GeometrySysConfig {
  u32 max_geometry_count;
};

struct GeometryConfig {
  String name;
  u32 vertex_size;
  u32 vertex_count;
  void* vertices;
  u32 index_size;
  u32 index_count;
  void* indices;
  String64 material_name64;
};

void geometry_sys_init(Arena* arena, GeometrySysConfig config);

KAPI Geometry* geometry_create(GeometryConfig config);
