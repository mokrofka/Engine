#pragma once

#include "defines.h"

#include "res/res_types.h"

#include <math/math_types.h>

enum R_BackendType {
  RENDERER_BACKENED_TYPE_VULKAN,
  RENDERER_BACKENED_TYPE_OPENGL,
  RENDERER_BACKENED_TYPE_DIRECTX,
};

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
  struct Arena* arena;
  void* internal_context;
  u64 frame_number;
  

  b8 (*initialize)(struct R_Backend* backend);

  void (*shutdown)(struct R_Backend* backend);
  
  void (*resized)(struct R_Backend* backend, u16 width, u16 height);
  
  b8 (*begin_frame)(struct R_Backend* backend, f32 delta_time);
  void (*update_global_state)(mat4 projection, mat4 view, v3 view_position, v4 ambient_colour, i32 mode);
  b8 (*end_frame)(struct R_Backend* backend, f32 delta_time);
  
  b8 (*window_create)(struct R_Backend* backend, struct Window* window);

  void (*update_object)(GeometryRenderData data);

  void (*create_texture)(const u8* pixels, struct Texture* texture);
  void (*destroy_texture)(struct Texture* texture);
};

struct RenderPacket {
  f32 delta_time;
};
