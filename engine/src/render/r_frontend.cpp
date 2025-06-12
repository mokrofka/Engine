#include "r_frontend.h"

#include "vulkan/vk_backend.h"

#include "ui.h"

struct RendererSystemState {
  R_Config config;
  b8 is_render;
};

global RendererSystemState st;

void r_init(Arena* arena, R_Config config) {
  st.config = config;
  st.is_render = true;
  
  Arena* render_arena = arena_alloc(arena, config.mem_reserve);
  vk_r_backend_init(render_arena);
}

void r_shutdown() {
  vk_r_backend_shutdown();
}

void r_on_resized(u32 width, u32 height) {
  vk_r_backend_on_resize(width, height);
}

void r_begin_draw_frame() {
  vk_r_backend_begin_frame();

  // scene render
  if (vk_is_viewport_render()) {
    {
      vk_r_begin_renderpass(BuiltinRenderpass_World);
      // vk_compute_draw();

      vk_draw();
      vk_r_end_renderpass(BuiltinRenderpass_World);
    }
  }

  // begin UI render
  {
    vk_r_begin_renderpass(BuiltinRenderpass_UI);
    ui_begin_frame();
  }
}

void r_end_draw_frame() {
  // end UI ender
  {
    ui_end_frame();
    vk_r_end_renderpass(BuiltinRenderpass_UI);
  }

  vk_r_backend_end_frame();
}

v2 get_viewport_size() {
  return vk_get_viewport_size(); 
}
