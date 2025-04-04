#pragma once

#include "render/r_types.h"

#include <logger.h>
#include <memory.h>

#include <vulkan/vulkan.h>

#define VK_CHECK(expr)          \
  {                             \
    Assert(expr == VK_SUCCESS); \
  }
#define vkdevice context->device.logical_device

struct VK_Buffer {
  u64 size;
  VkBuffer handle;
  VkBufferUsageFlagBits usage;
  b8 is_locked;
  VkDeviceMemory memory;
  i32 memory_index;
  u32 memory_property_flags;
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
  
  VkCommandPool graphics_command_pool;
  
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memory;
  
  VkFormat depth_format;
};

struct VK_Image  {
  VkImage handle;
  VkDeviceMemory memory;
  VkImageView view;
  u32 width;
  u32 height;
};

enum VK_RenderPassState {
  READY,
  RECORDING,
  IN_RENDER_PASS,
  RECORDING_ENDED,
  SUBMITTED,
  NOT_ALLOCATED
};

struct VK_RenderPass  {
  VkRenderPass handle;
  f32 x,y,w,h;
  f32 r,g,b,a;
  
  f32 depth;
  u32 stencil;
  
  VK_RenderPassState state;
};

struct VK_Framebuffer {
  VkFramebuffer handle;
  u32 attachment_count;
  VkImageView* attachments;
  VK_RenderPass * renderpass;
};

struct VK_Swapchain  {
  VkSurfaceFormatKHR image_format;  
  u8 max_frames_in_flight;
  VkSwapchainKHR handle;
  u32 image_count;
  VkImage* images;
  VkImageView* views;
  
  VK_Image depth_attachment;
  
  // framebuffers used for on-screen rendering.
  VK_Framebuffer* framebuffers;
};

enum VK_CommandBufferState {
  COMMAND_BUFFER_STATE_READY,
  COMMAND_BUFFER_STATE_RECORDING,
  COMMAND_BUFFER_STATE_IN_RENDER_PASS,
  COMMAND_BUFFER_STATE_RECORDING_ENDED,
  COMMAND_BUFFER_STATE_SUBMITTED,
  COMMAND_BUFFER_STATE_NOT_ALLOCATED,
};

struct VK_CommandBuffer { 
  VkCommandBuffer handle;
  
  // Command buffer state.
  VK_CommandBufferState state;
};

struct VK_Fence {
  VkFence handle;
  b8 is_signaled;
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

#define MATERIAL_SHADER_STAGE_COUNT 2

struct VK_DescriptorState {
  // One per frame
  u32 generations[3];
  u32 ids[3];
};

#define VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT 2
#define VULKAN_MATERIAL_SHADER_SAMPLER_COUNT 1

struct VK_MaterialShaderInstState {
  // Per frame
  VkDescriptorSet descriptor_sets[3];
  
  // Per descriptor
  VK_DescriptorState descriptor_states[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
};

// Max number of objects
#define VULKAN_MAX_MATERIAL_COUNT 1024

struct VK_MaterialShader {
  // vertex, fragment
  VK_ShaderStage stages[MATERIAL_SHADER_STAGE_COUNT];
  
  VkDescriptorPool global_descriptor_pool;
  VkDescriptorSetLayout global_descriptor_set_layout;
  
  // One descriptor set per frame - max 3 for triple-buffering
  VkDescriptorSet global_descriptor_sets[3];
  
  // Global uniform object
  GlobalUniformObject global_ubo;
  
  // Global uniform buffer
  VK_Buffer global_uniform_buffer;
  
  VkDescriptorPool object_descriptor_pool;
  VkDescriptorSetLayout object_descriptor_set_layout;
  // Object uniform buffers
  VK_Buffer object_uniform_buffer;
  // TODO manage a free list of somd kind here instead
  u32 object_uniform_buffer_index;
  
  TextureUse sampler_uses[VULKAN_MATERIAL_SHADER_SAMPLER_COUNT];
  
  // TODO make dynamic
  VK_MaterialShaderInstState instance_states[VULKAN_MAX_MATERIAL_COUNT];
  
  VK_Pipeline pipeline;
};

struct VK_Context {
  struct Arena* arena;
  
  VkInstance instance;
  VkAllocationCallbacks* allocator;
  VkSurfaceKHR surface;
  
  f32 frame_delta_time;
   
  // The framebuffer's current width.
  u32 framebuffer_width;
  
  // The framebuffer's current width.
  u32 framebuffer_height;
  
  // Current generation of framebuffer size. If it does not match framebuffer_size_last_generation,
  // a new one should be generated.
  u64 framebuffer_size_generation;
  
  // The generation of the framebuffer when it was last created. Set to framebuffer_size_generation
  // when updated.
  u64 framebuffer_size_last_generation;

#if defined(_DEBUG)
  VkDebugUtilsMessengerEXT debug_messenger;
#endif

  VK_Device device;
  
  VK_Swapchain  swapchain;
  VK_RenderPass  main_renderpass;
  
  VK_Buffer object_vertex_buffer;
  VK_Buffer object_index_buffer;
  
  // array
  VK_CommandBuffer* graphics_command_buffers;
  
  // array
  VkSemaphore* image_available_semaphores;
  
  // array
  VkSemaphore* queue_complete_semaphores;
  
  u32 in_flight_fence_count;
  VK_Fence* in_flight_fences;
  
  // Holds pointers to fences which exist and are owned elsewhere.
  VK_Fence** images_in_flight;
  
  u32 image_index;
  u32 current_frame;
  
  b8 recreating_swapchain;
  
  VK_MaterialShader material_shader;
  
  u64 geometry_vertex_offset;
  u64 geometry_index_offset;
  
  i32 (*find_memory_index)(u32 type_filter, u32 property_flags);
};

struct VK_TextureData {
  VK_Image  image;
  VkSampler sampler;
};
