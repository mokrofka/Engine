#include "vendor/imgui/imgui_impl_vulkan.h"
#include "vk_types.h"
#include "vk_command_buffer.h"

internal VkDescriptorPool imgui_descriptor_pool;

typedef void* HINSTANCE;
typedef void* HWND;
typedef void* HANDLE;
typedef wchar_t* LPCWSTR;
typedef u32 DWORD;
typedef void* LPVOID;
typedef b32 BOOL;
typedef void* HMONITOR;
struct SECURITY_ATTRIBUTES {
  DWORD nLength;
  LPVOID lpSecurityDescriptor;
  BOOL bInheritHandle;
};
#include <vulkan/vulkan_win32.h>

i32 imgui_surface_create(void* vp, u64 vk_inst, const void* vk_allocators, u64* out_vk_surface) {
  VkInstance instance; Assign(instance, vk_inst);
  
  HINSTANCE hinstance = os_get_handle_info();
  HWND hwnd = os_get_window_handle();

  VkWin32SurfaceCreateInfoKHR surface_info = {};
  surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_info.hwnd = vp;
  surface_info.hinstance = hinstance;

  VkSurfaceKHR surface;
  vkCreateWin32SurfaceKHR(instance, &surface_info, nullptr, &surface);

  *out_vk_surface = (u64)surface; // Store the Vulkan surface in the out_vk_surface pointer
  return 0;                       // Success
}

void imgui_renderer_init() {
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
    {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
  };

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.poolSizeCount = ArrayCount(pool_sizes);
  pool_info.pPoolSizes = pool_sizes;
  pool_info.maxSets = 1000;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  VK_CHECK(vkCreateDescriptorPool(vkdevice, &pool_info, null, &imgui_descriptor_pool));

  VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
  ImGui_ImplVulkan_InitInfo init_info = {
    .Instance = vk.instance,
    .PhysicalDevice = vk.device.physical_device,
    .Device = vk.device.logical_device,
    .QueueFamily = vk.device.graphics_queue_index,
    .Queue = vk.device.graphics_queue,
    .DescriptorPool = imgui_descriptor_pool,
    .MinImageCount = vk.swapchain.max_frames_in_flight,
    .ImageCount = vk.swapchain.image_count,
    .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
    .UseDynamicRendering = true,
    .PipelineRenderingCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &format,
    },
  };

  ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
  Assign(platform_io.Platform_CreateVkSurface, imgui_surface_create);
  ImGui_ImplVulkan_Init(&init_info);
  
  VkCommandBuffer cmd = vk_cmd_alloc_and_begin_single_use();
  ImGui_ImplVulkan_CreateFontsTexture();
  vk_cmd_end_single_use(cmd);
  ImGui_ImplVulkan_DestroyFontsTexture(); // destroy staging/temp resources
}

void imgui_renderer_shutdown() {
  ImGui_ImplVulkan_Shutdown();
  vkDestroyDescriptorPool(vkdevice, imgui_descriptor_pool, 0);
}

void imgui_begin_frame() {
  ImGui_ImplVulkan_NewFrame();
}

void imgui_end_frame() {
  VkCommandBuffer cmd = vk_get_current_cmd();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}

ImTextureID imgui_add_texture(u32 id) {
  return (ImTextureID)ImGui_ImplVulkan_AddTexture(
      vk.texture_targets[id].sampler,
      vk.texture_targets[id].image.view,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void imgui_remove_texture(ImTextureID id) {
  ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)id);
}
