#pragma once

#include "asserts.h"

#include <vulkan/vulkan.h>

#define VK_CHECK(expr)           \
  {                              \
    Assert(expr == VK_SUCCESS); \
  }

struct VulkanSwapchainSupportInfo {
  VkSurfaceCapabilitiesKHR capabilities;
  u32 format_count;
  VkSurfaceFormatKHR* formats;
  u32 present_mode_count;
  VkPresentModeKHR* present_modes;
};

struct VulkanDevice {
  VkPhysicalDevice physical_device;
  VkDevice logical_device;
  VulkanSwapchainSupportInfo swapchain_support;
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

struct VulkanImage {
  VkImage handle;
  VkDeviceMemory memory;
  VkImageView view;
  u32 width;
  u32 height;
};

enum VulkanRenderPassState {
  READY,
  RECORDING,
  IN_RENDER_PASS,
  RECORDING_ENDED,
  SUBMITTED,
  NOT_ALLOCATED
};

struct VulkanRenderPass {
  VkRenderPass handle;
  f32 x,y,w,h;
  f32 r,g,b,a;
  
  f32 depth;
  u32 stencil;
  
  VulkanRenderPassState state;
};

struct VulkanFramebuffer {
  VkFramebuffer handle;
  u32 attachment_count;
  VkImageView* attachments;
  VulkanRenderPass* renderpass;
};

struct VulkanSwapchain {
  VkSurfaceFormatKHR image_format;  
  u8 max_frames_in_flight;
  VkSwapchainKHR handle;
  u32 image_count;
  VkImage* images;
  VkImageView* views;
  
  VulkanImage depth_attachment;
  
  // framebuffers used for on-screen rendering.
  VulkanFramebuffer* framebuffers;
};

enum VulkanCommandBufferState {
  COMMAND_BUFFER_STATE_READY,
  COMMAND_BUFFER_STATE_RECORDING,
  COMMAND_BUFFER_STATE_IN_RENDER_PASS,
  COMMAND_BUFFER_STATE_RECORDING_ENDED,
  COMMAND_BUFFER_STATE_SUBMITTED,
  COMMAND_BUFFER_STATE_NOT_ALLOCATED,
};

struct VulkanCommandBuffer { 
  VkCommandBuffer handle;
  
  // Command buffer state.
  VulkanCommandBufferState state;
};

struct VulkanFence {
  VkFence handle;
  b8 is_signaled;
};

struct VulkanShaderStage {
  VkShaderModuleCreateInfo create_info; 
  VkShaderModule handle;
  VkPipelineShaderStageCreateInfo shader_state_create_info;
};

struct VulkanPipeline {
  VkPipeline handle;
  VkPipelineLayout pipeline_layout;
};

#define OBJECT_SHADER_STAGE_COUNT 2
struct VulkanObjectShader {
  VulkanShaderStage stages[OBJECT_SHADER_STAGE_COUNT];
  
  VulkanPipeline pipeline;
};

struct VulkanContext {
  struct Arena* arena;
   
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
  
  VkInstance instance;
  VkAllocationCallbacks* allocator;
  VkSurfaceKHR surface;

#if defined(_DEBUG)
  VkDebugUtilsMessengerEXT debug_messenger;
#endif

  VulkanDevice device;
  
  VulkanSwapchain swapchain;
  VulkanRenderPass main_renderpass;
  
  // array
  VulkanCommandBuffer* graphics_command_buffers;
  
  // array
  VkSemaphore* image_available_semaphores;
  
  // array
  VkSemaphore* queue_complete_semaphores;
  
  u32 in_flight_fence_count;
  VulkanFence* in_flight_fences;
  
  // Holds pointers to fences which exist and are owned elsewhere.
  VulkanFence** images_in_flight;
  
  u32 image_index;
  u32 current_frame;
  
  b8 recreating_swapchain;
  
  VulkanObjectShader object_shader;
  
  i32 (*find_memory_index)(u32 type_filter, u32 property_flags);
};
