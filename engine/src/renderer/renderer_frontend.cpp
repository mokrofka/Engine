#include "renderer_frontend.h"

#include "renderer_backend.h"
#include "vulkan/vulkan_platform.h"

#include "logger.h"
#include "memory.h"

struct RendererSystemState {
  Arena* arena;
  RendererBackend backend;
  void* memory;
};

// global RendererBackend* backend;
global RendererSystemState* state;
global const u64 memory_reserved = MB(10);

b8 renderer_initialize(u64* memory_requirement, void* out_state) {
  *memory_requirement = memory_reserved+sizeof(RendererSystemState);
  if (out_state == 0) {
    return true;
  }
  state = (RendererSystemState*)out_state;
  state->arena = (Arena*)&state->memory;
  state->backend.arena = state->arena;
  state->arena->res = MB(10);

  // backend = (RendererBackend*)kallocate(sizeof(RendererBackend), MEMORY_TAG_RENDERER);
  
  // TODO: make this configurable
  renderer_backend_create(RENDERER_BACKENED_TYPE_VULKAN, &state->backend);
  
  if (!state->backend.initialize(&state->backend)) {
    Fatal("Renderer backend failed to initialize. Shutting down.");
    return false;
  }

  return true;
}

void renderer_shutdown() {
  state->backend.shutdown(&state->backend);
}

b8 renderer_begin_frame(f32 delta_time) {
  // return backend->begin_frame(backend, delta_time);
}

b8 renderer_end_frame(f32 delta_time) {
  // b8 result = backend->end_frame(backend, delta_time);
  // ++backend->frame_number;
  // return result;
  return true;
}

b8 renderer_draw_frame(RenderPacket* packet) {
  // If the begin frame returned successfully, mid-frame operations may continue.
  if (renderer_begin_frame(packet->delta_time)) {
    
    // End the fram. If this fails, it is likely unrecovarble.
    b8 result = renderer_end_frame(packet->delta_time);
    
    if (!result) {
      Error("renderer_end_frame failed. Application shutting down...");
      return false;
    }
  }
  
  return true;
}
