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
#define ImagesInFlight 4

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
};

struct VK_SyncObj {
  VkSemaphore image_available_semaphores[ImagesInFlight];
  VkSemaphore render_complete_semaphores[ImagesInFlight];
  VkSemaphore compute_complete_semaphores[ImagesInFlight];
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
  SparseSetE sparse_set_entities;
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
  VkDeviceSize offset;
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
  u32 images_in_flight;
  
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
  VkDescriptorSet screen_descriptor_sets[FramesInFlight];

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
  u32 screen_shader_count;
  VK_Shader screen_shaders[10];
  u32 compute_shader_count;
  VK_Shader compute_shaders[10];
  VK_ComputeShader compute_shader;
  ShaderGlobalState* global_shader_state;
  VK_Shader screen_shader;

  // offscreen rendering
  VK_Texture texture_targets[ImagesInFlight];
  VK_Image offscreen_depth_buffer;
  b8 is_viewport_resized;
  b8 is_viewport_render; // TODO impelment
  b8 is_viewport_mode;

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

INLINE VkSemaphore vk_get_current_image_available_semaphore() { return vk.sync.image_available_semaphores[vk.frame.current_frame]; }
INLINE VkSemaphore vk_get_current_render_complete_semaphore() { return vk.sync.render_complete_semaphores[vk.frame.image_index]; }
INLINE VkCommandBuffer& vk_get_current_cmd() { return vk.cmds[vk.frame.current_frame]; }

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

////////////////////////////////////////////////////////////////////////
// Backend
void vk_instance_create();
void vk_r_backend_init();
void vk_r_backend_shutdown();
void vk_r_backend_begin_frame();
void vk_r_backend_end_frame();
void vk_r_begin_renderpass(u32 renderpass_id);
void vk_r_end_renderpass(u32 renderpass_id);

VkBool32 vk_debug_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
  VkDebugUtilsMessageTypeFlagsEXT message_types,
  const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
  void* user_data);
void vk_buffers_create();
void vk_resize_viewport();
void vk_resize_viewport();

////////////////////////////////////////////////////////////////////////
// Swapchain
void vk_swapchain_create(u32 width, u32 height);
void vk_swapchain_recreate(u32 width, u32 height);
void vk_swapchain_destroy();
u32 vk_swapchain_acquire_next_image_index(VkSemaphore image_available_semaphore);
void vk_swapchain_present(VkSemaphore render_complete_semaphore, u32 present_image_index);
void vk_surface_create();
const char* vk_surface_extension_name();

////////////////////////////////////////////////////////////////////////
// Shader
void vk_shader_init();

////////////////////////////////////////////////////////////////////////
// Image
VK_Image vk_image_create(
    VkImageType image_type,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memory_flags,
    b32 create_view,
    VkImageAspectFlags view_aspect_flags);
VkImageView vk_image_view_create(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags);
void vk_image_transition_layout(VkCommandBuffer cmd, VK_Image image, VkImageLayout old_layout, VkImageLayout new_layout);
void vk_upload_image_to_gpu(VkCommandBuffer cmd, VK_Image image);
void vk_image_destroy(VK_Image image);

////////////////////////////////////////////////////////////////////////
// Geometry
void vk_r_geometry_create(Geometry geom);

////////////////////////////////////////////////////////////////////////
// Drawing
void vk_draw_init();

////////////////////////////////////////////////////////////////////////
// Device
void vk_device_create();
void vk_device_destroy();
VK_SwapchainSupportInfo* vk_device_query_swapchain_support(VkPhysicalDevice physical_device, VK_SwapchainSupportInfo* support_info);
void vk_device_detect_depth_format(VK_Device* device);

////////////////////////////////////////////////////////////////////////
// Command Buffer
VkCommandBuffer vk_cmd_alloc(VkCommandPool pool);
void vk_cmd_free(VkCommandPool pool, VkCommandBuffer cmd);
void vk_cmd_begin(VkCommandBuffer cmd);
void vk_cmd_end(VkCommandBuffer cmd);
VkCommandBuffer vk_cmd_alloc_and_begin_single_use();
void vk_cmd_end_single_use(VkCommandBuffer cmd);

////////////////////////////////////////////////////////////////////////
// Buffer
VK_Buffer vk_buffer_create(u64 size, u32 usage, u32 memory_property_flags);
void vk_buffer_destroy(VK_Buffer& buffer);
void vk_buffer_map_memory(VK_Buffer& buffer, u64 offset, u64 size);
void vk_buffer_unmap_memory(VK_Buffer& buffer);
void vk_upload_to_gpu(VK_Buffer& buffer, Range range, void* data);


