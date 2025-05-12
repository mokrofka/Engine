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

#define DefaultGeometryName "default"_

void geometry_sys_init(Arena* arena, GeometrySysConfig config);
void geometry_sys_shutdown();

KAPI Geometry* geometry_sys_acquire_by_id(u32 id);

KAPI Geometry* geometry_sys_acquire_from_config(GeometryConfig config, b8 auto_release);

void geometry_sys_release(Geometry* geometry);

Geometry* geometry_sys_get_default();
Geometry* geometry_sys_get_default_2D();

GeometryConfig geometry_sys_generate_plane_config(f32 width, f32 height, u32 x_segment_count, u32 y_segment, f32 tile_x, f32 tile_y, String name, String material_name);
