#pragma once

#include "defines.h"

#include "res/res_types.h"

// TODO make it gpu specific
struct GlobalUniformObject {
  mat4 projection;  // 64 bytes
  mat4 view;        // 64 bytes
  mat4 m_reserved0; // 64 bytes, reserved for future use
  mat4 m_reserved1; // 64 bytes, reserved for future use
};

struct ObjectUniformObject {
  v4 diffuse_color; // 16 bytes
  v4 v_reserved0;   // 16 bytes, reserved for future use
  v4 v_reserved1;   // 16 bytes, reserved for future use
  v4 v_reserved2;   // 16 bytes, reserved for future use
};

struct GeometryRenderData {
  u32 object_id;
  mat4 model;
  struct Texture* textures[16];
};

struct R_Backend {
  Arena* arena;
  void* internal_context;
  u64 frame_number;
};

struct R_Packet {
  f32 delta_time;
};
