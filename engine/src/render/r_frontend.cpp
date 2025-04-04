#include "r_frontend.h"

#include "vulkan/vk_backend.h"

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
  R_Backend backend;
  mat4 projection;
  mat4 view;
  f32 near_clip;
  f32 far_clip;
  
  // TODO temporary
  Texture* test_diffuse;
  // TODO end temporary
  
  void* memory;
};

// global R_Backend* backend;
global RendererSystemState* state;

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

b8 r_init(Arena* arena) {
  u64 memory_reserved = MB(10);
  u64 memory_requirement = memory_reserved+sizeof(RendererSystemState);
  
  state = push_buffer(arena, RendererSystemState, memory_requirement);
  state->arena = (Arena*)&state->memory;
  state->backend.arena = state->arena;
  state->arena->res = memory_reserved;
  
  // TODO temp
  event_register(EVENT_CODE_DEBUG0, state, event_on_debug_event);
  // TODO end temp
  
  // TODO make this configurable
  state->backend.frame_number = 0;
  
  if (!vk_r_backend_init(&state->backend)) {
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

void r_shutdown() {
  if (state) {
    // TODO temp
    event_unregister(EVENT_CODE_DEBUG0, state, event_on_debug_event);
    // TODO end temp
    vk_r_backend_shutdown();
  }
  state = 0;
}

b8 r_begin_frame(f32 delta_time) {
  return vk_r_backend_begin_frame(delta_time);
}

b8 r_end_frame(f32 delta_time) {
  b8 result = vk_r_backend_end_frame(delta_time);
  ++state->backend.frame_number;
  return result;
}

void r_on_resized(u16 width, u16 height) {
  if (state) {
    state->projection = mat4_perspective(deg_to_rad(45.0f), width / (f32)height, state->near_clip, state->far_clip);
    vk_r_backend_on_resize(width, height);
  } else {
    Warn("renderer backend does not exist to accept resize: %i %i", width, height);
  }
}

b8 r_draw_frame(R_Packet* packet) {
  // If the begin frame returned successfully, mid-frame operations may continue.
  if (r_begin_frame(packet->delta_time)) {

    vk_r_update_global_state(state->projection, state->view, v3_zero(), v4_one(), 0);
    
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
    vk_r_update_object(data);

    
    // End the fram. If this fails, it is likely unrecovarble.
    b8 result = r_end_frame(packet->delta_time);
    
    if (!result) {
      Error("r_end_frame failed. Application shutting down...");
      return false;
    }
  }
  
  return true;
}

void r_set_view(mat4 view) {
  state->view = view;
}

void r_create_texture(const u8* pixels, Texture* texture) {
  vk_r_create_texture(pixels, texture);
}

void r_destroy_texture(struct Texture* texture) {
  vk_r_destroy_texture(texture);
}
