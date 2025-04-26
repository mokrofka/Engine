#include "ui/imgui/imgui.h"
#include "ui.h"

#include "ui/imgui/imgui_impl_vulkan.h"
#include "render/vulkan/vk_utils.h"

void imgui_init(Arena* arena) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO io = ImGui::GetIO();
  ImGui::StyleColorsDark();
  
  Scratch scratch;
  ImGui_ImplVulkan_InitInfo init_info = {};
  vk_imgui_init(&init_info);
}

void ui_init(Arena* arena) {

}

#include <stdio.h>
void check(i32* val) {
  LoopC (i, *val) {
    *val += 10;
  }
}
