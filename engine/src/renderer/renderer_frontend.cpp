#include "renderer_frontend.h"

#include "renderer_backend.h"
#include "resources/resources_types.h"

#include "logger.h"
#include "memory.h"
#include "math/maths.h"

struct RendererSystemState {
  Arena* arena;
  RendererBackend backend;
  mat4 projection;
  mat4 view;
  f32 near_clip;
  f32 far_clip;
  
  Texture default_texture;
  
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
  
  // TODO: make this configurable
  renderer_backend_create(RENDERER_BACKENED_TYPE_VULKAN, &state->backend);
  
  if (!state->backend.initialize(&state->backend)) {
    Fatal("Renderer backend failed to initialize. Shutting down.");
    return false;
  }

  state->near_clip = 0.1f;
  state->far_clip = 1000.0f;
  state->projection = mat4_perspective(deg_to_rad(45.0f), 1280 / 720.0f, state->near_clip, state->far_clip);
  state->view = mat4_translation(v3(0, 0, 30.0f));

  // NOTE: Create default texture, a 256x256 blue/white checkerboard pattern.
  // This is done in code to eliminate asset dependencies.
  Trace("Creating default texture...");
  const u32 tex_dimension = 256;
  const u32 channels = 4;
  const u32 pixel_count = tex_dimension * tex_dimension;
  u8 pixels[pixel_count * channels];
  // u8* pixels = kallocate(sizeof(u8) * pixel_count * bpp, MEMORY_TAG_TEXTURE);
  MemSet(pixels, 255, sizeof(u8) * pixel_count * channels);

  // Each pixel.
  for (u64 row = 0; row < tex_dimension; ++row) {
    for (u64 col = 0; col < tex_dimension; ++col) {
      u64 index = (row * tex_dimension) + col;
      u64 index_bpp = index * channels;
      if (row % 2) {
        if (col % 2) {
          pixels[index_bpp + 0] = 0;
          pixels[index_bpp + 1] = 0;
        }
      } else {
        if (!(col % 2)) {
          pixels[index_bpp + 0] = 0;
          pixels[index_bpp + 1] = 0;
        }
      }
    }
  }
  renderer_create_texture(
      "default",
      false,
      tex_dimension,
      tex_dimension,
      4,
      pixels,
      false,
      &state->default_texture);

  return true;
}

void renderer_shutdown() {
  if (state) {
    renderer_destroy_texture(&state->default_texture);
    
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
    // local_persist f32 angle = 0.01f;
    // angle += 0.01f;
    // quat roation = quat_from_axis_angle(v3_forward(), angle, false);
    // mat4 model = quat_to_rotation_matrix(roation, v3_zero());
    GeometryRenderData data = {};
    data.object_id = 0;
    data.model = model;
    data.textures[0] = &state->default_texture;
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
    const char* name, b8 auto_release, i32 width, i32 height, i32 channel_count,
    const u8* pixels, b8 has_transparency, struct Texture* texture) {
  state->backend.create_texture(name, auto_release, width, height, channel_count, pixels, has_transparency, texture);
}

void renderer_destroy_texture(struct Texture* texture) {
  state->backend.destroy_texture(texture);
}
