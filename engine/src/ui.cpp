#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_win32.h"
#include "ui.h"
#include "render/vulkan/vk_types.h"

struct UI_State {
  ImTextureID texture_ids[10];
};

global UI_State st;

void imgui_begin_frame();
void imgui_end_frame();
void imgui_renderer_init();
void imgui_renderer_shutdown();
ImTextureID imgui_add_texture(u32 id);
void imgui_remove_texture(ImTextureID id);

void ui_init() {
  ImGui::CreateContext();
  ImGui_ImplWin32_Init(os_get_window_handle());
  imgui_renderer_init();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
}

void ui_shutdown() {
  imgui_renderer_shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
}

void ui_begin_frame() {
  ImGui_ImplWin32_NewFrame();
  imgui_begin_frame();
  ImGui::NewFrame();

  if (vk.is_viewport_resized) {
    vk.is_viewport_resized = false;
    Loop(i, ImagesInFlight) {
      imgui_remove_texture(st.texture_ids[i]);
      st.texture_ids[i] = imgui_add_texture(i);
    }
  }
}

void ui_end_frame() {
  ImGui::Render();
  imgui_end_frame();
  
  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();
}

void ui_texture_render() {
  UI_Window("viewport") {
    ImVec2 current_viewport_size = ImGui::GetContentRegionAvail();

    // if (vk.viewport_size != *(v2*)&current_viewport_size) {
    if (vk.viewport_size != Transmute(v2)current_viewport_size) {
      vk.viewport_size = *(v2*)&current_viewport_size;
      vk.is_viewport_resized = true;

      EventContext context;
      context.data.u32[0] = current_viewport_size.x;
      context.data.u32[1] = current_viewport_size.y;
      event_fire(EventCode_ViewportResized, 0, context);
    }

    ImGui::Image(st.texture_ids[vk.frame.image_index], current_viewport_size);
    
  }
}
