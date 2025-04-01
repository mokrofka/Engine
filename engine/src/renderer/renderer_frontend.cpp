#include "renderer_frontend.h"

#include "renderer_backend.h"
#include "resources/resources_types.h"

#include "systems/texture_system.h"

#include <logger.h>
#include <memory.h>
#include <maths.h>

// TODO temporary

#include <strings.h>
#include <event.h>


// TODO end temporary

struct RendererSystemState {
  Arena* arena;
  RendererBackend backend;
  mat4 projection;
  mat4 view;
  f32 near_clip;
  f32 far_clip;
  
  // TODO temporary
  Texture* test_diffuse;
  // TODO end temporary
  
  void* memory;
};

// global RendererBackend* backend;
global RendererSystemState* state;
global const u64 memory_reserved = MB(10);

void create_texture(Texture* t) {
  MemZeroStruct(t);
  t->generation = INVALID_ID;
}

// TODO temp
b8 event_on_debug_event(u16 code, void* sender, void* listener_inst, EventContext data) {
  const char* names[3] = {
    "cobblestone",
    "paving",
    "paving2"};
  local i8 choice = 2;
    
  // Save off the old name
  const char* old_name = names[choice];
  
  ++choice;
  choice %= 3;
  
  // Load up the new texture
  state->test_diffuse = texture_system_acquire(names[choice], true);
  
  // Release the old texture
  texture_system_release(old_name);
  return true;
}
// TODO end temp

b8 renderer_initialize(u64* memory_requirement, void* out_state) {
  *memory_requirement = memory_reserved+sizeof(RendererSystemState);
  if (out_state == 0) {
    return true;
  }
  state = (RendererSystemState*)out_state;
  state->arena = (Arena*)&state->memory;
  state->backend.arena = state->arena;
  state->arena->res = MB(10);
  
  // TODO temp
  event_register(EVENT_CODE_DEBUG0, state, event_on_debug_event);
  // TODO end temp
  
  // TODO make this configurable
  renderer_backend_create(RENDERER_BACKENED_TYPE_VULKAN, &state->backend);
  state->backend.frame_number = 0;
  
  if (!state->backend.initialize(&state->backend)) {
    Fatal("Renderer backend failed to initialize. Shutting down.");
    return false;
  }

  state->near_clip = 0.1f;
  state->far_clip = 1000.0f;
  state->projection = mat4_perspective(deg_to_rad(45.0f), 1280 / 720.0f, state->near_clip, state->far_clip);
  
  state->view = mat4_translation(v3(0, 0, -30.0f));
  state->view = mat4_inverse(state->view);

  return true;
}

void renderer_shutdown() {
  if (state) {
    // TODO temp
    event_unregister(EVENT_CODE_DEBUG0, state, event_on_debug_event);
    // TODO end temp
    state->backend.shutdown(&state->backend);
  }
  state = 0;
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
    state->projection = mat4_perspective(deg_to_rad(45.0f), width / (f32)height, state->near_clip, state->far_clip);
    state->backend.resized(&state->backend, width, height);
  } else {
    Warn("renderer backend does not exist to accept resize: %i %i", width, height);
  }
}

b8 renderer_draw_frame(RenderPacket* packet) {
  // If the begin frame returned successfully, mid-frame operations may continue.
  if (renderer_begin_frame(packet->delta_time)) {

    state->backend.update_global_state(state->projection, state->view, v3_zero(), v4_one(), 0);
    
    mat4 model = mat4_translation(v3(0,0,0));
    // local f32 angle = 0.01f;
    // angle += 0.01f;
    // quat roation = quat_from_axis_angle(v3_forward(), angle, false);
    // mat4 model = quat_to_rotation_matrix(roation, v3_zero());
    GeometryRenderData data = {};
    data.object_id = 0;
    data.model = model;
    
    // TODO temporary
    // Grab the default if does not exist
    if (!state->test_diffuse) {
      state->test_diffuse = texture_system_get_default_texture(); 
    }
    
    data.textures[0] = state->test_diffuse;
    state->backend.update_object(data);
    
    // End the fram. If this fails, it is likely unrecovarble.
    b8 result = renderer_end_frame(packet->delta_time);
    
    if (!result) {
      Error("renderer_end_frame failed. Application shutting down...");
      return false;
    }
  }
  
  return true;
}

void renderer_set_view(mat4 view) {
  state->view = view;
}

void renderer_create_texture(
    const char* name, i32 width, i32 height, i32 channel_count,
    const u8* pixels, b8 has_transparency, struct Texture* texture) {
  state->backend.create_texture(name, width, height, channel_count, pixels, has_transparency, texture);
}

void renderer_destroy_texture(struct Texture* texture) {
  state->backend.destroy_texture(texture);
}
