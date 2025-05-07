#pragma once
#include "lib.h"

#include "render/r_types.h"

#include <vulkan/vulkan.h>

#define VK_CHECK(expr)          \
  {                             \
    Assert(expr == VK_SUCCESS); \
  }
#define vkdevice vk->device.logical_device

struct VK_Buffer {
  VkBuffer handle;
  VkDeviceMemory memory;
  FreeList freelist;
  u64 size;
  u64 usage;
  u32 memory_index;
  u32 memory_property_flags;
  b8 is_locked;
  b8 has_freelist;
};

struct VK_SwapchainSupportInfo {
  VkSurfaceCapabilitiesKHR capabilities;
  u32 format_count;
  VkSurfaceFormatKHR* formats;
  u32 present_mode_count;
  VkPresentModeKHR* present_modes;
};

struct VK_Device {
  VkPhysicalDevice physical_device;
  VkDevice logical_device;
  VK_SwapchainSupportInfo swapchain_support;
  u32 graphics_queue_index;
  u32 present_queue_index;
  u32 transfer_queue_index;
  
  VkQueue graphics_queue;
  VkQueue present_queue;
  VkQueue transfer_queue;
  
  VkCommandPool graphics_cmd_pool;
  
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memory;
  
  VkFormat depth_format;
};

struct VK_Image  {
  VkImage handle;
  VkDeviceMemory memory;
  VkImageView view;
  u16 height;
  u16 width;
};

enum VK_RenderPassState {
  VK_RenderPassState_Ready,
  VK_RenderPassState_Recording,
  VK_RenderPassState_InRenderPass,
  VK_RenderPassState_RecordingEnded,
  VK_RenderPassState_Submitted,
  VK_RenderPassState_NotAllocated
};

struct VK_Renderpass  {
  VkRenderPass handle;
  Rect render_area;
  v4 clear_color;
  f32 depth;
  u32 stencil;
  
  u8 clear_flags;
  b8 has_prev_pass;
  b8 has_next_pass;

  VK_RenderPassState state;
  u32 id;
};

struct VK_Swapchain  {
  VkSwapchainKHR handle;
  VkSurfaceFormatKHR image_format;  
  u32 max_frames_in_flight;
  u32 image_count;
  VkImage images[3];
  VkImageView views[3];
  
  VK_Image depth_attachment;
  
  // framebuffers used for on-screen rendering.
  VkFramebuffer framebuffers[3];
};

enum VK_CmdState {
  VK_CmdState_Ready,
  VK_CmdState_Recording,
  VK_CmdState_InRenderPass,
  VK_CmdState_RecordingEnded,
  VK_CmdState_Submitted,
  VK_CmdState_NotAllocated,
};

struct VK_CommandBuffer { 
  VkCommandBuffer handle;
  
  VK_CmdState state;
};

struct VK_ShaderStage {
  VkShaderModuleCreateInfo create_info; 
  VkShaderModule handle;
  VkPipelineShaderStageCreateInfo shader_state_create_info;
};

struct VK_Pipeline {
  VkPipeline handle;
  VkPipelineLayout pipeline_layout;
};

#define MaterialShaderStageCount 2

struct VK_DescriptorState {
  // One per frame
  u32 generations[3];
  u32 ids[3];
};

#define MaterialShaderDescriptorCount 2
#define MaterialShaderSamplerCount 1

struct VK_MaterialShaderInstState {
  // Per frame
  VkDescriptorSet descriptor_sets[3];
  
  // Per descriptor
  VK_DescriptorState descriptor_states[MaterialShaderDescriptorCount];
};

// Max number of material instances
#define MaxMaterialCount 1024

#define MaxGeometryCount 4096

struct VK_GeometryData {
  u32 id;
  u32 generation;
  u32 vertex_count;
  u32 vertex_element_size;
  u32 vertex_buffer_offset;
  u32 index_count;
  u32 index_element_size;
  u32 index_buffer_offset;
};

struct VK_MaterialShaderGlobalUbo {
  mat4 projection;  // 64 bytes
  mat4 view;        // 64 bytes
  mat4 m_reserved0; // 64 bytes, reserved for future use
  mat4 m_reserved1; // 64 bytes, reserved for future use
};

struct VK_MaterialShaderInstUbo  {
  v4 diffuse_color; // 16 bytes
  v4 v_reserved0;   // 16 bytes, reserved for future use
  v4 v_reserved1;   // 16 bytes, reserved for future use
  v4 v_reserved2;   // 16 bytes, reserved for future use
};

struct VK_MaterialShader {
  // vertex, fragment
  VK_ShaderStage stages[MaterialShaderStageCount];
  
  VkDescriptorPool global_descriptor_pool;
  VkDescriptorSetLayout global_descriptor_set_layout;
  
  // One descriptor set per frame - max 3 for triple-buffering
  VkDescriptorSet global_descriptor_sets[3];
  
  // Global uniform object
  VK_MaterialShaderGlobalUbo global_ubo;
  // Global uniform buffer
  VK_Buffer global_uniform_buffer;
  
  VkDescriptorPool obj_descriptor_pool;
  VkDescriptorSetLayout obj_descriptor_set_layout;
  // Object uniform buffers
  VK_Buffer obj_uniform_buffer;
  // TODO manage a free list of some kind here instead
  u32 obj_uniform_buffer_index;
  
  TextureUse sampler_uses[MaterialShaderSamplerCount];
  
  // TODO make dynamic
  VK_MaterialShaderInstState instance_states[MaxMaterialCount];
  
  VK_Pipeline pipeline;
};

struct VK_Frame {
  f32 delta_time;
  u32 width;
  u32 height;
  u32 size_generation;
  u32 size_last_generation;

  u32 image_index;
  u32 current_frame;
  b8 recreating_swapchain;
};

struct VK_SyncObj {
  VkSemaphore* image_available_semaphores;
  VkSemaphore* queue_complete_semaphores;
  
  u32 in_flight_fence_count;
  VkFence in_flight_fences[2];
};

#define UI_ShaderStageCount 2
#define VK_UIShaderDescriptorCount 2
#define VK_UIShaderSamplerCount 1

// Max number of ui control instances
// TODO: make configurable
#define VK_MaxUICount 1024

struct VK_UIShaderInstState {
  // Per frame
  VkDescriptorSet descriptor_sets[3];

  // Per descriptor
  VK_DescriptorState descriptor_states[VK_UIShaderDescriptorCount];
};

struct VK_UIShaderGlobalUbo {
  mat4 projection;  // 64 bytes
  mat4 view;        // 64 bytes
  mat4 m_reserved0; // 64 bytes, reserved for future use
  mat4 m_reserved1; // 64 bytes, reserved for future use
};

struct VK_UIShaderInstUbo {
  v4 diffuse_color; // 16 bytes
  v4 v_reserved0;   // 16 bytes, reserved for future use
  v4 v_reserved1;   // 16 bytes, reserved for future use
  v4 v_reserved2;   // 16 bytes, reserved for future use
};

struct VK_UIShader {
  // vertex, fragment
  VK_ShaderStage stages[UI_ShaderStageCount];

  VkDescriptorPool global_descriptor_pool;
  VkDescriptorSetLayout global_descriptor_set_layout;

  // One descriptor set per frame - max 3 for triple-buffering.
  VkDescriptorSet global_descriptor_sets[3];

  // Global uniform object.
  VK_MaterialShaderGlobalUbo global_ubo;

  // Global uniform buffer.
  VK_Buffer global_uniform_buffer;

  VkDescriptorPool obj_descriptor_pool;
  VkDescriptorSetLayout obj_descriptor_set_layout;
  // Object uniform buffers.
  VK_Buffer obj_uniform_buffer;
  // TODO: manage a free list of some kind here instead.
  u32 obj_uniform_buffer_index;

  TextureUse sampler_uses[VK_UIShaderSamplerCount];

  // TODO: make dynamic
  VK_UIShaderInstState instance_states[VK_MaxUICount];

  VK_Pipeline pipeline;
};

struct VK_Render {
  VK_Buffer obj_vertex_buffer;
  VK_Buffer obj_index_buffer;
  u64 geometry_vertex_offset;
  u64 geometry_index_offset;

  VK_CommandBuffer cmds[3];
  VK_MaterialShader material_shader;
  VK_UIShader ui_shader;
  
  VK_GeometryData geometries[MaxGeometryCount];
};

struct VK {
  Arena* arena;
  
  VkInstance instance;
  VkAllocationCallbacks* allocator;
  VkSurfaceKHR surface;
  
  VK_Frame frame;
  VK_SyncObj sync;
  VK_Render render;

  VK_Device device;
  
  VK_Swapchain swapchain;
  VK_Swapchain old_swapchain;
  
  #define RenderpassCount 2
  u32 main_renderpass_id;
  u32 ui_renderpass_id;
  VK_Renderpass renderpasses[RenderpassCount];
  
  b8 recreating_swapchain;
  
  VkFramebuffer world_framebuffers[3];
  
#if _DEBUG
  VkDebugUtilsMessengerEXT debug_messenger;
#endif
};

struct VK_TextureData {
  VK_Image image;
  VkSampler sampler;
};

extern VK* vk;

INLINE i32 vk_find_memory_index(u32 type_filter, u32 property_flags) {
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(vk->device.physical_device, &memory_properties);

  Loop (i, memory_properties.memoryTypeCount) {
    // Check each memory type to see if its bit is set to 1.
    if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
      return i;
    }
  }

  Assert(!"Unable to find suitable memory type");
  return -1;
}

VK_Renderpass* vk_get_renderpass(u32 id);
VkSemaphore vk_get_current_image_available_semaphore();
VkSemaphore vk_get_current_queue_complete_semaphore();
