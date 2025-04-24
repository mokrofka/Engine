#include "r_frontend.h"

#include "vulkan/vk_backend.h"

#include "sys/texture_sys.h"
#include "sys/material_sys.h"

// TODO temporary

#include <event.h>

// TODO end temporary

struct RendererSystemState {
  Arena* arena;
  R_Backend backend;
  mat4 projection;
  mat4 view;
  f32 near_clip;
  f32 far_clip;
};

// global R_Backend* backend;
global RendererSystemState* state;

void create_texture(Texture* t) {
  MemZeroStruct(t);
  t->generation = INVALID_ID;
}

void r_init(Arena* arena) {
  state = push_struct(arena, RendererSystemState);
  
  u64 mem_reserved = MB(1);
  state->arena = arena_alloc(arena, mem_reserved);
  state->backend.arena = state->arena;
  
  // TODO make this configurable
  state->backend.frame_number = 0;

  vk_r_backend_init(&state->backend);

  state->near_clip = 0.1f;
  state->far_clip = 1000.0f;
  state->projection = mat4_perspective(deg_to_rad(45.0f), 1280 / 720.0f, state->near_clip, state->far_clip);
  
  state->view = mat4_inverse(state->view);
}

void r_shutdown() {
  vk_r_backend_shutdown();
  state = 0;
}

b32 r_begin_frame(f32 delta_time) {
  return vk_r_backend_begin_frame(delta_time);
}

b32 r_end_frame(f32 delta_time) {
  b32 result = vk_r_backend_end_frame(delta_time);
  ++state->backend.frame_number;
  return result;
}

void r_on_resized(u32 width, u32 height) {
  state->projection = mat4_perspective(deg_to_rad(45.0f), width / (f32)height, state->near_clip, state->far_clip);
  vk_r_backend_on_resize(width, height);
}

void r_draw_frame(R_Packet* packet) {
  // If the begin frame returned successfully, mid-frame operations may continue.
  if (r_begin_frame(packet->delta_time)) {

    vk_r_update_global_state(state->projection, state->view, v3_zero(), v4_one(), 0);
    
    u32 count = packet->geometry_count;
    Loop (i, count) {
      vk_r_draw_geometry(packet->geometries[i]);
    }
    
    // End the fram. If this fails, it is likely unrecovarble.
    b32 result = r_end_frame(packet->delta_time);
    
    if (!result) {
      Error("r_end_frame failed. Application shutting down..."_);
    }
  }
}

void r_set_view(mat4 view) {
  state->view = view;
}

void r_create_texture(u8* pixels, Texture* texture) {
  vk_r_create_texture(pixels, texture);
}

void r_destroy_texture(Texture* texture) {
  vk_r_destroy_texture(texture);
}

void r_create_material(Material* material) {
  vk_r_create_material(material);
}

void r_destroy_material(Material* material) {
  vk_r_destroy_material(material);
}

void r_create_geometry(Geometry* geometry, u32 vertex_count, Vertex3D* vertices, u32 index_count, u32* indices) {
  vk_r_create_geometry(geometry, vertex_count, vertices, index_count, indices);
}

void r_destroy_geometry(Geometry* geometry) {
  vk_r_destroy_geometry(geometry);
}
