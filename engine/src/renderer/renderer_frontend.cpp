#include "renderer_frontend.h"

#include "renderer_backend.h"
#include "vulkan/vulkan_platform.h"

#include "logger.h"
#include "memory.h"
#include "math/maths.h"

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
  return state->backend.begin_frame(&state->backend, delta_time);
}

b8 renderer_end_frame(f32 delta_time) {
  b8 result = state->backend.end_frame(&state->backend, delta_time);
  ++state->backend.frame_number;
  return result;
}

void renderer_on_resized(u16 width, u16 height) {
  if (state) {
    state->backend.resized(&state->backend, width, height);
  } else {
    Warn("renderer backend does not exist to accept resize: %i %i", width, height);
  }
}

b8 renderer_draw_frame(RenderPacket* packet) {
  // If the begin frame returned successfully, mid-frame operations may continue.
  if (renderer_begin_frame(packet->delta_time)) {
    
    mat4 projection = mat4_perspective(deg_to_rad(45.0f), 1280/720.0f, 0.1f, 1000.0f);
    local_persist f32 z = -1.0f;
    z -= 0.01f;
    mat4 view = mat4_translation(v3(0,0,z));

    state->backend.update_global_state(projection, view, v3_zero(), v4_one(), 0);
    // state->backend.update_global_state(mat4_identity(), view, v3_zero(), v4_one(), 0);
    // state->backend.update_global_state(mat4_identity(), mat4_identity(), v3_zero(), v4_one(), 0);
    
    // End the fram. If this fails, it is likely unrecovarble.
    b8 result = renderer_end_frame(packet->delta_time);
    
    if (!result) {
      Error("renderer_end_frame failed. Application shutting down...");
      return false;
    }
  }
  
  return true;
}
