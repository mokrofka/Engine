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

struct alignas(16) EntityShader {
  float intensity;
};

struct GlobalShaderState {
  mat4 g_projection_view;
  f32 time;
};
