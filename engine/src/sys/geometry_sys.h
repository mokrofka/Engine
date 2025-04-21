#pragma once
#include "lib.h"

#include "render/r_types.h"

struct GeometrySysConfig {
  u32 max_geometry_count;
};

struct GeometryConfig {
  u32 vertex_count;
  Vertex3D* vertices;
  u32 index_count;
  u32* indices;
  String64 name;
  String64 material_name;
};

#define DEFAULT_GEOMETRY_NAME "default"_

void geometry_sys_init(Arena* arena, GeometrySysConfig config);
void geometry_sys_shutdown();

Geometry* geometry_sys_acquire_by_id(u32 id);

Geometry* geometry_sys_acquire_from_config(GeometryConfig config, b8 auto_release);

void geometry_sys_release(Geometry* geometry);

Geometry* geometry_sys_get_default();

GeometryConfig geometry_sys_generate_plane_config(f32 width, f32 height, u32 x_segment_count, u32 y_segment, f32 tile_x, f32 tile_y, String name, String material_name);
