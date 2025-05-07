#include "r_frontend.h"

#include "vulkan/vk_backend.h"

#include "ui.h"

struct RendererSystemState {
  Arena* arena;
  R_Config config;
  R_Backend backend;
  mat4 projection;
  mat4 view;
  mat4 ui_projection;
  mat4 ui_view;
  f32 near_clip;
  f32 far_clip;
  b8 do_render_frame;
};

global RendererSystemState* state;

void create_texture(Texture* t) {
  MemZeroStruct(t);
  t->generation = INVALID_ID;
}

void r_init(Arena* arena, R_Config config) {
  state = push_struct(arena, RendererSystemState);
  state->config = config;
  
  state->arena = arena_alloc(arena, config.mem_reserve);
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
  // state->ui_projection = mat4_orthographic(0.0f, 1280.0f, 720.0f, 0.0f, -100.0f, 100.0f); // Intentionally flipped on y axis
  state->ui_projection = mat4_orthographic(0.0f, 1280.0f, 0.0f, 720.0f, -100.0f, 100.0f); // Intentionally flipped on y axis
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
      // Loop (i, count) {
      //   vk_r_draw_geometry(packet->ui_geometries[i]);
      // }
      
      ui_begin_frame();
      ui_end_frame();
      
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

void r_begin_draw_frame(R_Packet* packet) {
  // If the begin frame returned successfully, mid-frame operations may continue.
  vk_r_backend_begin_frame(packet->delta_time);

  // World renderpass
  {
    vk_r_begin_renderpass(BuiltinRenderpass_World);

    vk_r_update_global_world_state(state->projection, state->view, v3_zero(), v4_one(), 0);

    // ui_begin_frame();
    // ui_test();
    // ui_end_frame();
    // Draw geometries
    u32 count = packet->geometry_count;
    Loop(i, count) {
      vk_r_draw_geometry(packet->geometries[i]);
    }
    vk_r_end_renderpass(BuiltinRenderpass_World);
  }

  ui_begin_frame();
  vk_r_begin_renderpass(BuiltinRenderpass_UI);
}

void r_end_draw_frame(R_Packet* packet) {
  ui_end_frame();
  vk_r_end_renderpass(BuiltinRenderpass_UI);

  // End the fram. If this fails, it is likely unrecovarble.
  vk_r_backend_end_frame(packet->delta_time);
}

void r_set_view(mat4 view) {
  state->view = view;
}

void* r_create_texture(u8* pixels, u32 width, u32 height, u32 channel_count) {
  return vk_r_create_texture(pixels, width, height, channel_count);
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

void r_create_geometry(Geometry* geometry, u32 vertex_size, u32 vertex_count, void* vertices, u32 index_size, u32 index_count, void* indices) {
  vk_r_create_geometry(geometry, vertex_size, vertex_count, vertices, index_size, index_count, indices);
}

void r_destroy_geometry(Geometry* geometry) {
  vk_r_destroy_geometry(geometry);
}

u32 r_renderpass_id(String name) {
  // TODO: HACK: Need dynamic renderpasses instead of hardcoding them.
  if (str_matchi("Renderpass.Builtin.World"_, name)) {
    return BuiltinRenderpass_World;
  } else if (str_matchi("Renderpass.Builtin.UI"_, name)) {
    return BuiltinRenderpass_UI;
  }

  Error("No renderpass named '%s'", name);
  Assert(false);
  return 0;
}

void r_shader_create(struct Shader* s, u32 renderpass_id, u32 stage_count, String* stage_filenames, ShaderStage* stages) {
  vk_r_shader_create(s, renderpass_id, stage_count, stage_filenames, stages);
}

void r_shader_destroy(Shader* s) {
  vk_r_shader_destroy(s);
}

void r_shader_initialize(Shader* s) {
  vk_r_shader_initialize(s);
}

void r_shader_use(Shader* s) {
  vk_r_shader_use(s);
}

void r_shader_bind_globals(Shader* s) {
  vk_r_shader_bind_globals(s);
}

void r_shader_bind_instance(Shader* s, u32 instance_id) {
  vk_r_shader_bind_instance(s, instance_id);
}

void r_shader_apply_globals(Shader* s) {
  vk_r_shader_apply_globals(s);
}

void r_shader_apply_instance(Shader* s) {
  vk_r_shader_apply_instance(s);
}

void r_shader_acquire_instance_resources(Shader* s, u32* out_instance_id) {
  vk_r_shader_acquire_instance_resources(s, out_instance_id);
}

void r_shader_release_instance_resources(Shader* s, u32 instance_id) {
  vk_r_shader_release_instance_resources(s, instance_id);
}

void r_set_uniform(Shader* s, shader_uniform* uniform, const void* value) {
  // vk_r_set_uniform(s, uniform, value);
}
