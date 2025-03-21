#pragma once

#include "defines.h"

enum RendererBackendType {
  RENDERER_BACKENED_TYPE_VULKAN,
  RENDERER_BACKENED_TYPE_OPENGL,
  RENDERER_BACKENED_TYPE_DIRECTX,
};

struct RendererBackend {
  struct Arena* arena;
  void* internal_context;
  u64 frame_number;

  b8 (*initialize)(struct RendererBackend* backend);

  void (*shutdown)(struct RendererBackend* backend);
  
  void (*resized)(struct RendererBackend* backend, u16 width, u16 height);
  
  b8 (*begin_frame)(struct RendererBackend* backend, f32 delta_time);
  b8 (*end_frame)(struct RendererBackend* backend, f32 delta_time);
  
  b8 (*window_create)(struct RendererBackend* backend, struct Window* window);
};

struct RenderPacket {
  f32 delta_time;
};
