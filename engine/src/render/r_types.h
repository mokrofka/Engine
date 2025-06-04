#pragma once
#include "lib.h"

#include "res/res_types.h"

enum BuiltinRenderpass {
  BuiltinRenderpass_World = 0x01,
  BuiltinRenderpass_UI = 0x02,
};

struct PushConstant {
  mat4 model;
  u32 entity_index;
};

struct ShaderEntity {
  f32 intensity;
  alignas(16) v4 padd;
};

struct DirectionalLight {
  alignas(16) v3 pos;
  alignas(16) v3 direction;
  alignas(16) v3 color;
};
// struct DirectionalLight {
//   v3 pos;
//   v3 direction;
//   v3 color;
// };

struct ShaderGlobalState {
  mat4 g_projection_view;
  v4 ambient_color;
  f32 time;
  u32 light_count;
};
