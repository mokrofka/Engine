#pragma once
#include "lib.h"

#include "res/res_types.h"

struct GeometryRenderData {
  mat4 model;
  Geometry* geometry;
};

enum BuiltinRenderpass {
  BuiltinRenderpass_World = 0x01,
  BuiltinRenderpass_UI = 0x02,
};

struct R_Backend {
  Arena* arena;
  u64 frame_number;
};

struct R_Packet {
  f32 delta_time;
  
  u32 geometry_count;
  GeometryRenderData* geometries;

  u32 ui_geometry_count;
  GeometryRenderData* ui_geometries;
};
