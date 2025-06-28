#pragma once
#include "lib.h"

#include "render/r_types.h"
#include "entity.h"

#include <vulkan/vulkan.h>

#define VK_CHECK(expr)          \
  {                             \
    Assert(expr == VK_SUCCESS); \
  }
#define vkdevice vk.device.logical_device
#define FramesInFlight 2
#define ImagesInFlight 3

#define MaxLights KB(1)

struct VK_Buffer {
  VkBuffer handle;
  VkDeviceMemory memory;
  u8* maped_memory;
  FreelistGpu freelist;
  u64 size;
  u64 usage;
  u32 memory_index;
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
  u32 compute_queue_index;
  
  VkQueue graphics_queue;
  VkQueue present_queue;
  VkQueue transfer_queue;
  VkQueue compute_queue;
  
  VkCommandPool cmd_pool;
  VkCommandPool transient_cmd_pool;
  
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

struct VK_Swapchain  {
  VkSwapchainKHR handle;
  VkSurfaceFormatKHR image_format;  
  VkImage images[ImagesInFlight];
  VkImageView views[ImagesInFlight];
  
  VK_Image depth_attachment;
};

struct VK_Frame {
  u32 width;
  u32 height;
  u32 size_generation;
  u32 size_last_generation;

  u32 image_index;
  u32 current_frame;
  b8 recreating_swapchain;
};

struct VK_SyncObj {
  VkSemaphore image_available_semaphores[FramesInFlight];
  VkSemaphore queue_complete_semaphores[FramesInFlight];
  VkSemaphore compute_complete_semaphores[FramesInFlight];
  VkFence in_flight_fences[FramesInFlight];
};

struct VK_Pipeline {
  VkPipeline handle;
  VkPipelineLayout pipeline_layout;
};

struct VK_ShaderStage {
  VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info;
};

struct VK_Shader {
  String name;
  VK_Pipeline pipeline;
  VkPipelineShaderStageCreateInfo stages[2];
  SparseSetE sparse_set;
  u32 vert_stride;
  u32 attribute_count;
  VkVertexInputAttributeDescription attribute_desriptions[10];
  VkPrimitiveTopology topology;
  b8 is_transparent;
};

struct VK_ComputeShader {
  VK_Pipeline pipeline;
  VK_ShaderStage stage;
};

struct VK_Mesh {
  u64 offset;
  u64 vert_count;
};

struct VK_Texture {
  VK_Image image;
  VkSampler sampler;
};

struct VK {
  Arena* arena;
  
  VkAllocationCallbacks* allocator;
  VkAllocationCallbacks _allocator;
  VkInstance instance;
  VkSurfaceKHR surface;
  
  VK_Device device;
  VK_Frame frame;
  VK_SyncObj sync;
  VK_Swapchain swapchain;
  VK_Swapchain old_swapchain;
  
  VK_Buffer vert_buffer;
  VK_Buffer index_buffer;
  VK_Buffer stage_buffer;
  Range storage_buffer_range;
  VK_Buffer storage_buffer;
  VK_Buffer compute_storage_buffers[2];
  
  VkDescriptorPool descriptor_pool;
  VkDescriptorSetLayout descriptor_set_layout;
  VkDescriptorSet descriptor_sets[FramesInFlight];

  VkDescriptorSetLayout compute_descriptor_set_layout;
  VkDescriptorSet compute_descriptor_sets[FramesInFlight];
  
  VkCommandBuffer cmds[FramesInFlight];
  VkCommandBuffer compute_cmds[FramesInFlight];

  VK_Texture texture; // TODO bindless textures
  
  SparseSetIndex entity_to_mesh;
  VK_Mesh meshes[10];

  ShaderEntity* entities_data;
  // SparseSet entities_data;
  SparseSet point_light_data;
  SparseSet dir_light_data;
  SparseSet spot_light_data;
  SparseSet push_constants;
  
  // Shader
  u32 entity_to_shader[MaxEntities];
  u32 shader_count;
  VK_Shader shaders[10];
  VK_ComputeShader compute_shader;
  ShaderGlobalState* global_shader_state;

  // offscreen rendering
  VK_Texture texture_targets[ImagesInFlight];
  VK_Image offscreen_depth_buffer;
  b8 is_viewport_resized;
  b8 is_viewport_render; // TODO impelment

  v2 viewport_size;
  
  VkDebugUtilsMessengerEXT debug_messenger;
};

extern VK vk;

INLINE u32 vk_find_memory_index(u32 type_filter, u32 property_flags) {
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(vk.device.physical_device, &memory_properties);
  u32 index = INVALID_ID;

  Loop (i, memory_properties.memoryTypeCount) {
    // Check each memory type to see if its bit is set to 1.
    if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
      index = i;
    }
  }

  Assert(index != INVALID_ID && "Unable to find suitable memory type");
  return index;
}

INLINE VkSemaphore vk_get_current_image_available_semaphore() {
  return vk.sync.image_available_semaphores[vk.frame.current_frame];
}

INLINE VkSemaphore vk_get_current_queue_complete_semaphore() {
  return vk.sync.queue_complete_semaphores[vk.frame.current_frame];
}

INLINE VkCommandBuffer& vk_get_current_cmd() {
  return vk.cmds[vk.frame.current_frame];
}

#define ParticleCount KB(40)

struct alignas(16) Particle {
  v3 pos;
  v3 velocity;
  v4 color;
};

struct UniformBufferObject {
  mat4 projection_view;
  f32 delta_time = 1.0f;
};

