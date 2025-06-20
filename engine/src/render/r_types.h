#pragma once
#include "lib.h"

#include "res/res_types.h"

enum {
  Renderpass_World,
  Renderpass_UI,
};

struct PushConstant {
  mat4 model;
  u32 entity_index;
};

struct ShaderEntity {
  f32 intensity;
  alignas(16) v3 color;
  alignas(16) v4 padd;
};

struct DirectionalLight {
  alignas(16) v3 pos;
  alignas(16) v3 direction;
  alignas(16) v3 color;
};

struct ShaderGlobalState {
  mat4 g_projection_view;
  mat4 g_view;
  v4 ambient_color;
  f32 time;
  u32 light_count;
};
