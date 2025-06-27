#include "r_frontend.h"

#include "vulkan/vk_backend.h"

#include "ui.h"

struct RendererSystemState {
  b8 is_render;
};

global RendererSystemState st;

void r_init() {
  st.is_render = true;
  
  Arena* render_arena = mem_arena_alloc(MB(1));
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

  // World
  if (vk_is_viewport_render()) {
    {
      vk_r_begin_renderpass(Renderpass_World);
      // vk_compute_draw();

      vk_draw();
      vk_r_end_renderpass(Renderpass_World);
    }
  }

  // Begin UI
  {
    vk_r_begin_renderpass(Renderpass_UI);
    ui_begin_frame();
  }
}

void r_end_draw_frame() {
  // End UI
  {
    ui_end_frame();
    vk_r_end_renderpass(Renderpass_UI);
  }

  vk_r_backend_end_frame();
}

v2 get_viewport_size() {
  return vk_get_viewport_size(); 
}
