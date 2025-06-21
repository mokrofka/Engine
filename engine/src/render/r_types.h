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
  alignas(16) v3 color;
  alignas(4)  f32 specular_strngth;
};

struct PointLight {
  alignas(16) v3 color;
  alignas(16) v3 pos;
  alignas(4)  f32 intensity;
  alignas(4)  f32 rad;
};

struct DirLight {
  alignas(16) v3 color;
  alignas(16) v3 direction;
  alignas(4)  f32 intensity;
};

struct SpotLight {
  alignas(16) v3 color;
  alignas(16) v3 pos;
  alignas(16) v3 dir;
  alignas(4)  f32 intensity;
  alignas(4)  f32 inner_cutoff;
  alignas(4)  f32 outer_cutoff;
};

struct ShaderGlobalState {
  alignas(16) mat4 projection_view;
  alignas(16) mat4 view;
  alignas(16) v4 ambient_color;
  alignas(4)  f32 time;
  alignas(4)  u32 point_light_count;
  alignas(4)  u32 dir_light_count;
  alignas(4)  u32 spot_light_count;
};
