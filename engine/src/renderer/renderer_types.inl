#pragma once

#include "defines.h"

#include "math/math_types.h"

enum RendererBackendType {
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

struct RendererBackend {
  struct Arena* arena;
  void* internal_context;
  u64 frame_number;

  b8 (*initialize)(struct RendererBackend* backend);

  void (*shutdown)(struct RendererBackend* backend);
  
  void (*resized)(struct RendererBackend* backend, u16 width, u16 height);
  
  b8 (*begin_frame)(struct RendererBackend* backend, f32 delta_time);
  void (*update_global_state)(mat4 projection, mat4 view, v3 view_position, v4 ambient_colour, i32 mode);
  b8 (*end_frame)(struct RendererBackend* backend, f32 delta_time);
  
  b8 (*window_create)(struct RendererBackend* backend, struct Window* window);

  void (*update_object)(mat4 model);

  void (*create_texture)(
      const char* name, b8 auto_release, i32 width, i32 height, i32 channel_count,
      const b8* pixels, b8 has_transparency, struct Texture* texture);
  void (*destroy_texture)(struct Texture* texture);
};

struct RenderPacket {
  f32 delta_time;
};
