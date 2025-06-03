#pragma once
#include "lib.h"

#include "render/r_types.h"
#include "ecs.h"

#include <vulkan/vulkan.h>

#define VK_CHECK(expr)          \
  {                             \
    Assert(expr == VK_SUCCESS); \
  }
#define vkdevice vk.device.logical_device
#define FramesInFlight 2
#define ImagesInFlight 3

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
  VkImage images[ImagesInFlight];
  VkImageView views[ImagesInFlight];
  
  VK_Image depth_attachment;
  
  VkFramebuffer framebuffers[ImagesInFlight];
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

struct VK_Render {
  VK_Buffer obj_vertex_buffer;
  VK_Buffer obj_index_buffer;
  u64 geometry_vertex_offset;
  u64 geometry_index_offset;

  // VkCommandBuffer cmds[2];
};

struct vk_Shader {
  String name;
  VK_Pipeline pipeline;
  VK_ShaderStage stages[3];
  // SparseSetKeep push_constants;
  SparseSetEntity sparse_set;
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
  
  VkInstance instance;
  VkAllocationCallbacks* allocator;
  VkAllocationCallbacks _allocator;
  VkSurfaceKHR surface;
  
  VK_Frame frame;
  VK_SyncObj sync;

  VK_Device device;
  
  VK_Swapchain swapchain;
  VK_Swapchain old_swapchain;
  
  b8 recreating_swapchain;
  
  // new stuff
  VK_Buffer vert_buffer;
  VK_Buffer index_buffer;
  VK_Buffer stage_buffer;
  VK_Buffer storage_buffer;
  VK_Buffer storage_buffers[2];
  VK_Buffer compute_uniform_buffer;
  
  VkDescriptorPool descriptor_pool;
  VkDescriptorSetLayout descriptor_set_layout;
  VkDescriptorSetLayout compute_descriptor_set_layout;
  VkDescriptorSet descriptor_sets[FramesInFlight];
  VkDescriptorSet compute_descriptor_sets[FramesInFlight];
  
  vk_Shader shader;
  u32 entity_count;
  u32 entities[MaxEntities];
  MemRange uniform_buffer_mem_range;
  u64 vulkan_driver_memory_allocated;
  VK_Mesh meshes[10];
  SparseSetKeep sparse_push_constants;

  VK_Texture texture; // TODO bindless textures
  
  VkCommandBuffer cmds[FramesInFlight];
  VkCommandBuffer compute_cmds[FramesInFlight];

  SparseSetIndex entity_to_mesh;
  u32 entity_to_shader[MaxEntities];
  
  // Shader
  u32 shader_count;
  vk_Shader shaders[10];
  VK_ComputeShader compute_shader;
  vk_Shader graphics_shader_compute;
  mat4* projection_view;
  SparseSetKeep push_constants;
  GlobalShaderState* global_shader_state;
  EntityShader* entities_shader_data;

  // offscreen rendering
  VK_Texture texture_targets[ImagesInFlight];
  VkFramebuffer texture_framebuffers[ImagesInFlight];
  VK_Image depth;
  b8 is_viewport_sezied;

  v2 viewport_size;
  
#ifdef _DEBUG
  VkDebugUtilsMessengerEXT debug_messenger;
#endif
};

extern VK vk;

INLINE i32 vk_find_memory_index(u32 type_filter, u32 property_flags) {
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(vk.device.physical_device, &memory_properties);
  u32 index = -1;

  Loop (i, memory_properties.memoryTypeCount) {
    // Check each memory type to see if its bit is set to 1.
    if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
      index = i;
    }
  }

  Assert(index != -1 && "Unable to find suitable memory type");
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

void vk_resize_viewport();
