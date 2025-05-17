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
  u8* maped_memory;
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
  INLINE operator VkCommandBuffer() { return handle; };
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
#define VK_MaxMaterialCount 1024

#define VK_MaxGeometryCount 4096

struct VK_GeometryData {
  u32 id;
  u32 vertex_size;
  u32 vertex_count;
  u32 vertex_buffer_offset;
  u32 index_size;
  u32 index_count;
  u32 index_buffer_offset;
  MemRange range;
};

#define VK_MaxUICount 1024

// vertex, fragment, geometry, compute
#define VK_ShaderMaxStages 4

#define VK_ShaderMaxGlobalTextures 31
#define VK_ShaderMaxInstTextures 31
#define VK_ShaderMaxAttributes 16
/**
 * @brief The maximum number of uniforms and samplers allowed at the
 * global, instance and local levels combined. It's probably more than
 * will ever be needed.
 */
#define VK_ShaderMaxUniforms 128

/** @brief The maximum number of bindings per descriptor set. */
#define VK_ShaderMaxBindings 32
#define VK_ShaderMaxPushConstRanges 32

/**
 * @brief Configuration for a shader stage, such as vertex or fragment.
 */
struct VK_ShaderStageConfig {
  /** @brief The shader stage bit flag. */
  VkShaderStageFlagBits stage;
  /** @brief The shader file name. */
  String64 file_name64;
};

/**
 * @brief The configuration for a descriptor set.
 */
struct VK_DescriptorSetConfig {
  /** @brief The number of bindings in this set. */
  u8 binding_count;
  /** @brief An array of binding layouts for this set. */
  VkDescriptorSetLayoutBinding bindings[VK_ShaderMaxBindings];
};

struct VK_ShaderConfig {
  /** @brief The number of shader stages in this shader. */
  u8 stage_count;
  /** @brief  The configuration for every stage of this shader. */
  VK_ShaderStageConfig stages[VK_ShaderMaxStages];
  /** @brief An array of descriptor pool sizes. */
  VkDescriptorPoolSize pool_sizes[2];
  /**
   * @brief The max number of descriptor sets that can be allocated from this shader.
   * Should typically be a decently high number.
   */
  u16 max_descriptor_set_count;

  /**
   * @brief The total number of descriptor sets configured for this shader.
   * Is 1 if only using global uniforms/samplers; otherwise 2.
   */
  u8 descriptor_set_count;
  /** @brief Descriptor sets, max of 2. Index 0=global, 1=instance */
  VK_DescriptorSetConfig descriptor_sets[2];

  /** @brief An array of attribute descriptions for this shader. */
  VkVertexInputAttributeDescription attributes[VK_ShaderMaxAttributes];
};

struct VK_ShaderDescriptorSetState {
  /** @brief The descriptor sets for this instance, one per frame. */
  VkDescriptorSet descriptor_sets[3];

  /** @brief A descriptor state per descriptor, which in turn handles frames. Count is managed in shader config. */
  VK_DescriptorState descriptor_states[VK_ShaderMaxBindings];
};

struct VK_ShaderInstState {
  /** @brief The instance id. INVALID_ID if not used. */
  u32 id;
  /** @brief The offset in bytes in the instance uniform buffer. */
  u64 offset;

  /** @brief  A state for the descriptor set. */
  VK_ShaderDescriptorSetState descriptor_set_state;

  /**
   * @brief Instance texture pointers, which are used during rendering. These
   * are set by calls to set_sampler.
   */
  struct texture** instance_textures;
};

struct VK_Shader {
  /** @brief The block of memory mapped to the uniform buffer. */
  void* mapped_uniform_buffer_block;

  /** @brief The shader identifier. */
  u32 id;

  /** @brief The configuration of the shader generated by vulkan_create_shader(). */
  VK_ShaderConfig config;

  /** @brief A pointer to the renderpass to be used with this shader. */
  VK_Renderpass* renderpass;

  /** @brief An array of stages (such as vertex and fragment) for this shader. Count is located in config.*/
  VK_ShaderStage stages[VK_ShaderMaxStages];

  /** @brief The descriptor pool used for this shader. */
  VkDescriptorPool descriptor_pool;

  /** @brief Descriptor set layouts, max of 2. Index 0=global, 1=instance. */
  VkDescriptorSetLayout descriptor_set_layouts[2];
  /** @brief Global descriptor sets, one per frame. */
  VkDescriptorSet global_descriptor_sets[3];
  /** @brief The uniform buffer used by this shader. */
  VK_Buffer uniform_buffer;

  /** @brief The pipeline associated with this shader. */
  VK_Pipeline pipeline;

  /** @brief The instance states for all instances. @todo TODO: make dynamic */
  u32 instance_count;
  VK_ShaderInstState instance_states[VK_MaxMaterialCount];
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
  VK_MaterialShaderInstState instance_states[VK_MaxMaterialCount];
  
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
  
  VK_GeometryData geometries[VK_MaxGeometryCount];
};

struct vk_Shader {
  VK_Pipeline pipeline;
  VK_ShaderStage stages[3];
  u32 entities[1024];
  u32 entity_count;
  SparseSetKeep push_constants;
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
  
  // new stuff
  VK_Buffer vert_buffer;
  VK_Buffer index_buffer;
  VK_Buffer stage_buffer;
  VK_Buffer uniform_buffer;
  
  VkDescriptorPool descriptor_pool;
  VkDescriptorSetLayout descriptor_set_layout;
  VkDescriptorSet descriptor_sets[3];
  
  vk_Shader shader;
  u32 entity_count;
  u32 entities[MaxEntities];
  MemRange uniform_buffer_mem_range;
  u64 vulkan_driver_memory_allocated;
  VK_Mesh meshes[10];
  SparseSetKeep sparse_push_constants;

  VK_Texture texture; // TODO bindless textures
  
  // Shader
  u32 shader_count;
  vk_Shader shaders[10];
  
#if _DEBUG
  VkDebugUtilsMessengerEXT debug_messenger;
#endif
};

struct Te {
  VK_Texture texture;
  VkDescriptorSetLayout layout;
  VkDescriptorSet descriptors[3];
  VkDescriptorPool descriptor_pool;
};
extern Te te;

extern VK* vk;

INLINE i32 vk_find_memory_index(u32 type_filter, u32 property_flags) {
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(vk->device.physical_device, &memory_properties);
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

INLINE VK_Renderpass* vk_get_renderpass(u32 id) {
  return &vk->renderpasses[id];
}

INLINE VkSemaphore vk_get_current_image_available_semaphore() {
  return vk->sync.image_available_semaphores[vk->frame.current_frame];
}

INLINE VkSemaphore vk_get_current_queue_complete_semaphore() {
  return vk->sync.queue_complete_semaphores[vk->frame.current_frame];
}

INLINE VK_CommandBuffer& vk_get_current_cmd() {
  return vk->render.cmds[vk->frame.image_index];
}
