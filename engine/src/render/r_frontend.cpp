#include "r_frontend.h"

#include "vulkan/vk_backend.h"

#include "sys/texture_sys.h"
#include "sys/material_sys.h"

// TODO temporary

#include "event.h"

// TODO end temporary

struct RendererSystemState {
  Arena* arena;
  R_Backend backend;
  mat4 projection;
  mat4 view;
  mat4 ui_projection;
  mat4 ui_view;
  f32 near_clip;
  f32 far_clip;
};

global RendererSystemState* state;

void create_texture(Texture* t) {
  MemZeroStruct(t);
  t->generation = INVALID_ID;
}

void r_init(Arena* arena) {
  state = push_struct(arena, RendererSystemState);
  
  state->arena = arena_alloc(arena, MB(1));
  state->backend.arena = state->arena;
  
  // TODO make this configurable
  state->backend.frame_number = 0;

  vk_r_backend_init(&state->backend);

  // World projection/view
  state->near_clip = 0.1f;
  state->far_clip = 1000.0f;
  state->projection = mat4_perspective(deg_to_rad(45.0f), 1280 / 720.0f, state->near_clip, state->far_clip);
  
  // TODO configutable camera starting position
  state->view = mat4_translation(v3(0, 0, -30));
  state->view = mat4_inverse(state->view);
  
  // UI projection/view
  state->ui_projection = mat4_orthographic(0.0f, 1280.0f, 720.0f, 0.0f, -100.0f, 100.0f); // Intentionally flipped on y axis
  state->ui_view = mat4_inverse(mat4_identity());
}

void r_shutdown() {
  vk_r_backend_shutdown();
  state = 0;
}

b32 r_begin_frame(f32 delta_time) { // TODO
  return vk_r_backend_begin_frame(delta_time);
}

b32 r_end_frame(f32 delta_time) {
  b32 result = vk_r_backend_end_frame(delta_time);
  ++state->backend.frame_number;
  return result;
}

void r_on_resized(u32 width, u32 height) {
  state->projection = mat4_perspective(deg_to_rad(45.0f), width / (f32)height, state->near_clip, state->far_clip);
  state->ui_projection = mat4_orthographic(0, (f32)width, (f32)height, 0, -100.0f, 100.0f);
  vk_r_backend_on_resize(width, height);
}

void r_draw_frame(R_Packet* packet) {
  // If the begin frame returned successfully, mid-frame operations may continue.
  if (vk_r_backend_begin_frame(packet->delta_time)) {
    
    // World renderpass
    {
      vk_r_begin_renderpass(BuiltinRenderpass_World);

      vk_r_update_global_world_state(state->projection, state->view, v3_zero(), v4_one(), 0);
      
      // Draw geometries
      u32 count = packet->geometry_count;
      Loop (i, count) {
        vk_r_draw_geometry(packet->geometries[i]);
      }
      vk_r_end_renderpass(BuiltinRenderpass_World);
    }
    
    // UI renderpass
    {
      vk_r_begin_renderpass(BuiltinRenderpass_UI);
      
      vk_r_update_global_ui_state(state->ui_projection, state->ui_view, 0);
      
      // Draw geometries
      u32 count = packet->ui_geometry_count;
      Loop (i, count) {
        vk_r_draw_geometry(packet->ui_geometries[i]);
      }
      vk_r_end_renderpass(BuiltinRenderpass_UI);
    }
    
    // End the fram. If this fails, it is likely unrecovarble.
    b32 result = vk_r_backend_end_frame(packet->delta_time);
    
    if (!result) {
      Error("r_end_frame failed. Application shutting down..."_);
    }
  } else {
    Info("skip frame");
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
