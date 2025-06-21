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
  alignas(4)  f32 intensity;
  alignas(16) v3 color;
  alignas(16) v4 padd;
};

struct DirectionalLight {
  alignas(16) v3 pos;
  alignas(16) v3 direction;
  alignas(16) v3 color;
};

struct ShaderGlobalState {
  alignas(16) mat4 projection_view;
  alignas(16) mat4 view;
  alignas(16) v4 ambient_color;
  alignas(4)  f32 time;
  alignas(4)  u32 light_count;
};
