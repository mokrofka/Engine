#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_win32.h"
#include "vendor/imgui/imgui_impl_vulkan.h"
#include "ui.h"

#include "render/vulkan/vk_types.h"
#include "render/vulkan/vk_command_buffer.h"

struct UIState {
  VkDescriptorPool imgui_pool;
  ImFont* font;
};

global UIState state;

void alloc_resource() {
  VkDescriptorPoolSize pool_sizes[] = {
    {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
    {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.poolSizeCount = ArrayCount(pool_sizes);
  pool_info.pPoolSizes = pool_sizes;
  pool_info.maxSets = 1000;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

  VkResult result = vkCreateDescriptorPool(vkdevice, &pool_info, null, &state.imgui_pool);
}

i32 imgui_surface_create(void* vp, u64 vk_inst, const void* vk_allocators, u64* out_vk_surface);

void imgui_init() {
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  
  ImGui_ImplWin32_Init(os_get_window_handle());

  alloc_resource();
  ImGui_ImplVulkan_InitInfo init_info = {
    .Instance = vk.instance,
    .PhysicalDevice = vk.device.physical_device,
    .Device = vk.device.logical_device,
    .QueueFamily = vk.device.graphics_queue_index,
    .Queue = vk.device.graphics_queue,
    .DescriptorPool = state.imgui_pool,
    .RenderPass = vk_get_renderpass(vk.ui_renderpass_id)->handle,
    .MinImageCount = vk.swapchain.max_frames_in_flight,
    .ImageCount = vk.swapchain.image_count,
    .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
  };

  ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
  Assign(platform_io.Platform_CreateVkSurface, imgui_surface_create);
  ImGui_ImplVulkan_Init(&init_info);
  
  VK_CommandBuffer cmd = vk_cmd_alloc_and_begin_single_use();
  ImGui_ImplVulkan_CreateFontsTexture();
  vk_cmd_end_single_use(&cmd);
  ImGui_ImplVulkan_DestroyFontsTexture(); // destroy staging/temp resources
}

void ui_init() {
  imgui_init();
}

void ui_shutdown() {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
  vkDestroyDescriptorPool(vkdevice, state.imgui_pool, 0);
}

void ui_begin_frame() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
  ImGui::PushFont(state.font);
}

void ui_end_frame() {
  ImGui::PopFont();
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vk.render.cmds[vk.frame.image_index].handle);
  
  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();
}
