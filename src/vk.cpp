#include "common.h"
#include "vulkan/vulkan_core.h"

const u32 MaxMaterials = KB(1);
const u32 MaxLights    = KB(1);
const u32 MaxLines     = KB(1);
const u32 MaxMeshes    = KB(1);
const u32 MaxShaders   = KB(1);
const u32 MaxTextures  = KB(1);
const u32 MaxDrawCalls = KB(1);
const u32 MaxDebugLines = KB(1);

struct GpuMaterial {
  u32 pipeline_idx;
  ShaderState shader_state;
  MaterialProps props;
  Handle<GpuTexture> texture;
};

///////////////////////////////////
// Gpu memory layout

struct EntityGPU {
  alignas(16) mat4 model;
  alignas(16) v4 color;
  u32 material;
};

struct MaterialGPU {
  alignas(16) v3 ambient;
  alignas(16) v3 diffuse;
  alignas(16) v3 specular;
  f32 shininess;
  Handle<GpuTexture> texture;
};

struct PointLightGPU {
  alignas(16) v3 color;
  alignas(16) v3 pos;
  f32 intensity;
  f32 rad;
};

struct DirLightGPU {
  alignas(16) v3 color;
  alignas(16) v3 dir;
  f32 intensity;
};

struct SpotLightGPU {
  alignas(16) v3 color;
  alignas(16) v3 pos;
  alignas(16) v3 dir;
  f32 intensity;
  f32 inner_cutoff;
  f32 outer_cutoff;
};

struct GlobalStateGPU {
  alignas(16) mat4 projection_view;
  alignas(16) mat4 projection;
  alignas(16) mat4 view;
  alignas(16) v4 ambient_color;
  f32 time;
  u32 point_light_count;
  u32 dir_light_count;
  u32 spot_light_count;
  u32 entity_indices[MaxEntities+MaxStaticEntities];
};

///////////////////////////////////
// Generic stuff

struct KeyToShaderPipeline {
  String name;
  ShaderState state;
};

intern u64 hash(KeyToShaderPipeline x) {
  return hash(x.name) + hash_memory(&x.state, sizeof(ShaderState));
}

intern b32 equal(KeyToShaderPipeline a, KeyToShaderPipeline b) {
  return equal(a.name, b.name) & MemMatchStruct(&a.state, &b.state);
}

enum RenderpassType {
  RenderpassType_World,
  RenderpassType_UI,
  RenderpassType_Screen,
};

struct PushConstant {
  u32 drawcall_offset;
};

template<typename T>
struct MeshBatch {
  Handle<GpuMesh> mesh_handle;
  Darray<Handle<T>> entities;
};

template<typename T>
struct ShaderBatch {
  Darray<MeshBatch<T>> mesh_batches;
  Map<u32, u32> mesh_to_batch;
};

struct RenderBatch {
  ShaderBatch<Entity> batch;
  ShaderBatch<Entity> batch_indexed;
  u32 static_entities_count;
  u32 static_entities_indexed_count;
  u32 static_entities_count_old;
  u32 static_entities_indexed_count_old;
  ShaderBatch<StaticEntity> static_batch;
  ShaderBatch<StaticEntity> static_batch_indexed;
};

struct RenderEntity {
  u32 entity_idx_in_array;
  u32 pipeline;
  Handle<GpuMesh> mesh;
#if BUILD_DEBUG
  b32 is_init;
#endif
};

///////////////////////////////////
// Vulkan

#if BUILD_DEBUG
  #define VK_CHECK(expr)                     \
    {                                        \
      if (expr != VK_SUCCESS) {              \
        Error("%s", vk_result_string(expr)); \
        InvalidPath;                         \
      }                                      \
    }
#else
  #define VK_CHECK(expr) expr
#endif

#define vkdevice vk.device.logical_device

struct VK_Buffer {
  VkBuffer handle;
  VkDeviceMemory memory;
  u8* mapped_memory;
  u64 size;
  u64 cap;
};

struct VK_ImageInfo {
  VkImageType image_type;
  u32 width;
  u32 height;
  u32 miplevels_count;
  VkImageCreateFlags flags;
  VkFormat format;
  u32 array_layers_count;
  VkSampleCountFlagBits samples;
  VkImageTiling tiling;
  VkImageUsageFlags usage;
  VkMemoryPropertyFlags memory_flags;
  VkImageAspectFlags aspect;
  VkImageViewType view_type;
};

struct VK_Image {
  VkImage handle;
  VkImageView view;
  VK_ImageInfo info;
  VkDeviceMemory memory;
};

struct VK_Mesh {
  u64 vert_count;
  VkDeviceSize vert_offset;
  GpuMemHandler vert_memory;
  u64 index_count;
  VkDeviceSize index_offset;
  GpuMemHandler index_memory;
};

struct VK_Swapchain {
  VkSwapchainKHR handle;
  VkSurfaceFormatKHR image_format;  
  VkPresentModeKHR present_mode;
  VK_Image images[4];
  VK_Image depth_attachment;
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
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memory;
  VkFormat depth_format;
};

struct VK_ShaderModule {
  VkPipelineShaderStageCreateInfo stages[2];
};

struct VK_SyncObj {
  VkSemaphore* image_available_semaphores;
  VkSemaphore* render_complete_semaphores;
  VkSemaphore* compute_complete_semaphores;
  VkFence* in_flight_fences;
};

struct VK_DrawCallInfo {
  union {
    VkDrawIndirectCommand draw_command;
    VkDrawIndexedIndirectCommand index_draw_command;
  };
  u32 entities_offset_id;
};

struct VK_State {
  Arena arena;
  
  VkAllocationCallbacks allocator_;
  VkAllocationCallbacks* allocator;
  VkInstance instance;
  VkSurfaceKHR surface;
  VkDebugUtilsMessengerEXT debug_messenger;
  
  VK_Device device;
  VK_SyncObj sync;
  VK_Swapchain swapchain;

  u32 images_in_flight;
  u32 frames_in_flight;
  u32 current_image_idx;
  u32 current_frame_idx;
  u32 width;
  u32 height;
  f32 scale;
  
  VK_Buffer vert_buffer;
  VK_Buffer index_buffer;
  VK_Buffer stage_buffer;
  VK_Buffer indirect_draw_buffer;
  VK_Buffer storage_buffer;
  VK_Buffer image_buffer;
  
  VkDescriptorPool descriptor_pool;
  VkDescriptorSetLayout descriptor_set_layout;
  VkDescriptorSet descriptor_sets;

  VkCommandBuffer* cmds;
  VkCommandBuffer upload_cmd;
  VkSampler sampler;
  VkPipelineLayout pipeline_layout;

  GlobalStateGPU* gpu_global_shader_st;
  EntityGPU* gpu_entities;
  MaterialGPU* gpu_materials;
  u32 gpu_materials_count;
  VK_DrawCallInfo* gpu_draw_call_infos;
  u32* gpu_entities_indexes;

  u32 static_draw_indexed_offset;
  u32 static_draw_offset;
  u32 static_indexed_draw_count;
  u32 static_draw_count;

  Array<RenderEntity, MaxEntities+MaxStaticEntities> entities;

  Array<VK_Mesh, MaxMeshes> meshes;
  Array<VK_Image, MaxTextures> textures;

  VK_Image msaa_texture_target;
  VK_Image offscreen_depth_buffer;
  VK_Image* texture_targets;

  VkPipeline screen_shader;
  VkPipeline cubemap_shader;
  VkPipeline debug_line_shader;
  VkPipeline ui_shader;

  mat4 view;
  mat4 projection;

  struct DebugDrawLine { Vertex vert[2]; };
  VkDeviceSize immediate_draw_lines_offset;
  Darray<DebugDrawLine> immediate_draw_lines;

  struct DebugDrawSquare { Vertex vert[6]; };
  VkDeviceSize immediate_draw_squares_offset;
  Darray<DebugDrawSquare> immediate_draw_squares;
  Darray<u32> immediate_draw_vert_offsets;

  Darray<VkPipeline> pipelines;
  struct ModuleEntry {
    VK_ShaderModule module;
    Darray<u32> track_pipelines;
    Darray<u32> track_shader_states;
  };
  Darray<ModuleEntry> modules;
  Darray<RenderBatch> batches;
  Darray<GpuMaterial> materials;
  
  Map<KeyToShaderPipeline, u32> shader_to_pipeline;
  Map<String, u32> shader_to_module;

  ///////////////////////////////////
  // Vulkan loader
  #define VK_GET_PROC_LIST \
    X(GetInstanceProcAddr) \
    X(EnumerateInstanceExtensionProperties) \
    X(EnumerateInstanceVersion) \
    X(EnumerateInstanceLayerProperties) \
    X(CreateInstance) \

  #define VK_INSTANCE_GET_PROC_LIST \
    X(DestroyInstance) \
    X(EnumeratePhysicalDevices) \
    X(GetDeviceProcAddr) \
    X(GetPhysicalDeviceProperties) \
    X(GetPhysicalDeviceFeatures) \
    X(GetPhysicalDeviceMemoryProperties) \
    X(GetPhysicalDeviceQueueFamilyProperties) \
    X(GetPhysicalDeviceFormatProperties) \
    X(GetPhysicalDeviceSurfaceFormatsKHR) \
    X(GetPhysicalDeviceSurfaceCapabilitiesKHR) \
    X(GetPhysicalDeviceSurfacePresentModesKHR) \
    X(GetPhysicalDeviceSurfaceSupportKHR) \
    X(EnumerateDeviceExtensionProperties) \
    X(CreateDevice) \
    X(DestroySurfaceKHR) \

  #define VK_DEVICE_GET_PROC_LIST \
    X(GetDeviceQueue) \
    X(DeviceWaitIdle) \
    X(CreateCommandPool) \
    X(DestroyCommandPool) \
    X(DestroyDevice) \
    X(CreateSwapchainKHR) \
    X(DestroySwapchainKHR) \
    X(GetSwapchainImagesKHR) \
    X(CreateImage) \
    X(CreateImageView) \
    X(DestroyImage) \
    X(DestroyImageView) \
    X(GetImageMemoryRequirements) \
    X(AllocateMemory) \
    X(FreeMemory) \
    X(AllocateCommandBuffers) \
    X(FreeCommandBuffers) \
    X(BeginCommandBuffer) \
    X(EndCommandBuffer) \
    X(BindImageMemory) \
    X(CreateSemaphore) \
    X(DestroySemaphore) \
    X(CreateFence) \
    X(DestroyFence) \
    X(WaitForFences) \
    X(AcquireNextImageKHR) \
    X(ResetFences) \
    X(CreateDescriptorSetLayout) \
    X(DestroyDescriptorSetLayout) \
    X(CreateDescriptorPool) \
    X(DestroyDescriptorPool) \
    X(CreateShaderModule) \
    X(DestroyShaderModule) \
    X(CreateSampler) \
    X(DestroySampler) \
    X(CreateBuffer) \
    X(DestroyBuffer) \
    X(GetBufferMemoryRequirements) \
    X(BindBufferMemory) \
    X(MapMemory) \
    X(UnmapMemory) \
    X(FlushMappedMemoryRanges) \
    X(CreatePipelineLayout) \
    X(DestroyPipelineLayout) \
    X(CreateGraphicsPipelines) \
    X(DestroyPipeline) \
    X(AllocateDescriptorSets) \
    X(FreeDescriptorSets) \
    X(UpdateDescriptorSets) \
    X(CmdBindPipeline) \
    X(CmdPipelineBarrier) \
    X(CmdBlitImage) \
    X(CmdCopyBuffer) \
    X(CmdCopyBufferToImage) \
    X(CmdCopyImageToBuffer) \
    X(CmdExecuteCommands) \
    X(CmdSetViewport) \
    X(CmdSetScissor) \
    X(CmdSetFrontFace) \
    X(CmdSetCullMode) \
    X(CmdSetStencilTestEnable) \
    X(CmdSetDepthTestEnable) \
    X(CmdSetDepthWriteEnable) \
    X(CmdSetStencilReference) \
    X(CmdSetStencilOp) \
    X(CmdBeginRendering) \
    X(CmdEndRendering) \
    X(CmdSetStencilCompareMask) \
    X(CmdSetStencilWriteMask) \
    X(CmdClearColorImage) \
    X(CmdClearDepthStencilImage) \
    X(CmdSetPrimitiveTopology) \
    X(CmdPushConstants) \
    X(CmdBindVertexBuffers) \
    X(CmdBindIndexBuffer) \
    X(CmdDraw) \
    X(CmdDrawIndexed) \
    X(CmdDrawIndirect) \
    X(CmdDrawIndexedIndirect) \
    X(CmdBindDescriptorSets) \
    X(QueueSubmit) \
    X(QueueWaitIdle) \
    X(QueuePresentKHR) \

  #define VK_DECL(name) Glue(PFN_, vk##name) name;
  #define VK_GET_PROC(name) vk.name = (Glue(PFN_, vk##name))os_lib_get_proc(vk.lib, Stringify(vk##name));
  #define VK_INSTANCE_GET_PROC(name) vk.name = (Glue(PFN_, vk##name))vk.GetInstanceProcAddr(vk.instance, Stringify(vk##name));
  #define VK_DEVICE_GET_PROC(name) vk.name = (Glue(PFN_, vk##name))vk.GetDeviceProcAddr(vkdevice, Stringify(vk##name));

  OS_Handle lib;

#define X(name) VK_DECL(name)
  VK_GET_PROC_LIST;
  VK_INSTANCE_GET_PROC_LIST;
  VK_DEVICE_GET_PROC_LIST;
#undef X
};

VK_State vk;

intern String vk_result_string(VkResult result) {
  switch (result) {
    case VK_SUCCESS:                                            return "VK_SUCCESS";
    case VK_NOT_READY:                                          return "VK_NOT_READY";
    case VK_TIMEOUT:                                            return "VK_TIMEOUT";
    case VK_EVENT_SET:                                          return "VK_EVENT_SET";
    case VK_EVENT_RESET:                                        return "VK_EVENT_RESET";
    case VK_INCOMPLETE:                                         return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY:                           return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:                         return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:                        return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:                                  return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:                            return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:                            return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:                        return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:                          return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:                          return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:                             return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:                         return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL:                              return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_UNKNOWN:                                      return "VK_ERROR_UNKNOWN";
    case VK_ERROR_OUT_OF_POOL_MEMORY:                           return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:                      return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_FRAGMENTATION:                                return "VK_ERROR_FRAGMENTATION";
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:               return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
    case VK_PIPELINE_COMPILE_REQUIRED:                          return "VK_PIPELINE_COMPILE_REQUIRED";
    case VK_ERROR_SURFACE_LOST_KHR:                             return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:                     return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR:                                     return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:                              return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:                     return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT:                        return "VK_ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_INVALID_SHADER_NV:                            return "VK_ERROR_INVALID_SHADER_NV";
    case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:                return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:       return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:    return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:       return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:        return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:          return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
    case VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT:                return "VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT";
    case VK_ERROR_NOT_PERMITTED_KHR:                            return "VK_ERROR_NOT_PERMITTED_KHR";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:          return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case VK_THREAD_IDLE_KHR:                                    return "VK_THREAD_IDLE_KHR";
    case VK_THREAD_DONE_KHR:                                    return "VK_THREAD_DONE_KHR";
    case VK_OPERATION_DEFERRED_KHR:                             return "VK_OPERATION_DEFERRED_KHR";
    case VK_OPERATION_NOT_DEFERRED_KHR:                         return "VK_OPERATION_NOT_DEFERRED_KHR";
    case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:                    return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
    case VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT:               return "VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT";
    case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:             return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
    case VK_PIPELINE_BINARY_MISSING_KHR:                        return "VK_PIPELINE_BINARY_MISSING_KHR";
    case VK_ERROR_NOT_ENOUGH_SPACE_KHR:                         return "VK_ERROR_NOT_ENOUGH_SPACE_KHR";
    case VK_RESULT_MAX_ENUM:                                    return "VK_RESULT_MAX_ENUM";
  }
}

#if OS_WINDOW
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
  intern void vk_surface_create() {
    VkWin32SurfaceCreateInfoKHR surface_create_info = {
      .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
      .hinstance = (HINSTANCE)os_get_handle_info(),
      .hwnd = (HWND)os_get_window_handle()
    };
    VK_CHECK(vkCreateWin32SurfaceKHR(vk.instance, &surface_create_info, vk.allocator, &vk.surface));
    Info("Vulkan win32 surface created");
  }
  #define VK_SURFACE_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
  
#elif OS_LINUX
  #if GFX_X11
    #include <xcb/xcb.h>
    #include <vulkan/vulkan_xcb.h>

    intern void vk_surface_create() {
      struct VK_Surface {
        xcb_connection_t* connection;
        xcb_window_t window;
      } vk_surface; 
      os_get_gfx_api_handlers(&vk_surface);
      VkXcbSurfaceCreateInfoKHR surfaceInfo = {
        .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        .connection = vk_surface.connection,
        .window = vk_surface.window,
      };
      PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)vk.GetInstanceProcAddr(vk.instance, "vkCreateXcbSurfaceKHR");
      VK_CHECK(vkCreateXcbSurfaceKHR(vk.instance, &surfaceInfo, vk.allocator, &vk.surface));
      Info("Vulkan XCB surface created");
    }
    #define VK_SURFACE_NAME VK_KHR_XCB_SURFACE_EXTENSION_NAME

  #else
    #include <vulkan/vulkan_wayland.h>
    intern void vk_surface_create() {
      struct VK_Surface{
        struct wl_display* wl_display;
        struct wl_surface* wl_surface;
      } vk_surface;
      os_get_gfx_api_handlers(&vk_surface);
      VkWaylandSurfaceCreateInfoKHR surfaceInfo = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = vk_surface.wl_display,
        .surface = vk_surface.wl_surface,
      };
      PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)vk.GetInstanceProcAddr(vk.instance, "vkCreateWaylandSurfaceKHR");
      VK_CHECK(vkCreateWaylandSurfaceKHR (vk.instance, &surfaceInfo, vk.allocator, &vk.surface));
      Info("Vulkan wayland surface created");
    }
    #define VK_SURFACE_NAME VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
  #endif
#endif

#define VulkanUseAllocator 0

#if VulkanUseAllocator
  VkAllocationCallbacks vk_allocator_create() {
    VkAllocationCallbacks callbacks = {
      .pUserData = null,
      .pfnAllocation = [](void* user_data, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope){
        void* result = mem_alloc(vk.alloc, size);
        return result;
      },
      .pfnReallocation = [](void* user_data, void* origin, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope) {
        ...
      },
      .pfnFree = [](void* user_data, void* memory){
        if (!memory) { // NOTE: it happens
          return;
        }
        mem_free(memory);
      },
      .pfnInternalAllocation = [](void* user_data, size_t size, VkInternalAllocationType allocation_type, VkSystemAllocationScope allocation_scope){},
      .pfnInternalFree = [](void* user_data, size_t size, VkInternalAllocationType allocation_type, VkSystemAllocationScope allocation_scope){},
    };
    return callbacks;
  }
#endif

////////////////////////////////////////////////////////////////////////
// @Utils

intern VkSemaphore vk_get_current_image_available_semaphore() { return vk.sync.image_available_semaphores[vk.current_frame_idx]; }
intern VkSemaphore vk_get_current_render_complete_semaphore() { return vk.sync.render_complete_semaphores[vk.current_image_idx]; }
intern VkCommandBuffer vk_get_current_cmd()                   { return vk.cmds[vk.current_frame_idx]; }

intern u32 vk_find_memory_idx(u32 type_filter, u32 property_flags) {
  VkPhysicalDeviceMemoryProperties memory_properties = vk.device.memory;
  u32 idx = -1;
  Loop (i, memory_properties.memoryTypeCount) {
    if (BitHas(type_filter, i) && FlagHas(memory_properties.memoryTypes[i].propertyFlags, property_flags)) {
      idx = i;
      break;
    }
  }
  AssertMsg(idx != -1, "Unable to find suitable memory type");
  // if (idx == -1) Assert(!"Unable to find sutable memory type");
  return idx;
}

////////////////////////////////////////////////////////////////////////
// @Cmd

intern VkCommandBuffer vk_cmd_alloc(VkCommandPool pool) {
  VkCommandBufferAllocateInfo allocate_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1,
  };
  VkCommandBuffer cmd;
  VK_CHECK(vk.AllocateCommandBuffers(vkdevice, &allocate_info, &cmd));
  return cmd;
}

intern void vk_cmd_free(VkCommandPool pool, VkCommandBuffer cmd) {
  vk.FreeCommandBuffers(vkdevice, pool, 1, &cmd);
}

intern void vk_cmd_begin(VkCommandBuffer cmd) {
  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  VK_CHECK(vk.BeginCommandBuffer(cmd, &begin_info));
}

intern void vk_cmd_end(VkCommandBuffer cmd) {
  VK_CHECK(vk.EndCommandBuffer(cmd));
}

intern void vk_cmd_submit(VkCommandBuffer cmd) {
  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
  };
  VK_CHECK(vk.QueueSubmit(vk.device.graphics_queue, 1, &submit_info, 0));
  VK_CHECK(vk.QueueWaitIdle(vk.device.graphics_queue));
}

intern void vk_cmd_end_submit(VkCommandBuffer cmd) {
  vk_cmd_end(cmd);
  vk_cmd_submit(cmd);
}

intern VkCommandBuffer vk_cmd_alloc_begin() {
  VkCommandBuffer result = vk_cmd_alloc(vk.device.cmd_pool);
  vk_cmd_begin(result);
  return result;
}

intern void vk_cmd_end_free(VkCommandBuffer cmd) {
  vk_cmd_end(cmd);
  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
  };
  VK_CHECK(vk.QueueSubmit(vk.device.graphics_queue, 1, &submit_info, 0));
  VK_CHECK(vk.QueueWaitIdle(vk.device.graphics_queue));
  vk_cmd_free(vk.device.cmd_pool, cmd);
}

////////////////////////////////////////////////////////////////////////
// @Buffer

intern VK_Buffer vk_buffer_create(u64 size, u32 usage, u32 memory_property_flags) {
  VkBufferCreateInfo buffer_create_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = size,
    .usage = usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  VK_Buffer buffer = {
    .cap = size,
  };
  VK_CHECK(vk.CreateBuffer(vkdevice, &buffer_create_info, vk.allocator, &buffer.handle));
  VkMemoryRequirements requirements;
  vk.GetBufferMemoryRequirements(vkdevice, buffer.handle, &requirements);
  u32 memory_index = vk_find_memory_idx(requirements.memoryTypeBits, memory_property_flags);
  VkMemoryAllocateInfo allocate_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = requirements.size,
    .memoryTypeIndex = memory_index,
  };
  VK_CHECK(vk.AllocateMemory(vkdevice, &allocate_info, vk.allocator, &buffer.memory));
  VK_CHECK(vk.BindBufferMemory(vkdevice, buffer.handle, buffer.memory, 0));
  return buffer;
}

intern void vk_buffer_destroy(VK_Buffer buffer) {
  vk.FreeMemory(vkdevice, buffer.memory, vk.allocator);
  vk.DestroyBuffer(vkdevice, buffer.handle, vk.allocator);
}

intern void vk_buffer_map_memory(VK_Buffer& buffer, u64 offset, u64 size) {
  VK_CHECK(vk.MapMemory(vkdevice, buffer.memory, offset, size, 0, (void**)&buffer.mapped_memory));
}

intern void vk_buffer_unmap_memory(VK_Buffer buffer) {
  vk.UnmapMemory(vkdevice, buffer.memory);
}

intern void vk_buffer_upload_to_gpu(VK_Buffer buffer, Range range, void* data) {
  MemCopy(vk.stage_buffer.mapped_memory, data, range.size);
  vk_cmd_begin(vk.upload_cmd);
  VkBufferCopy copy_region = {
    .srcOffset = 0,
    .dstOffset = range.offset,
    .size = range.size,
  };
  vk.CmdCopyBuffer(vk.upload_cmd, vk.stage_buffer.handle, buffer.handle, 1, &copy_region);
  vk_cmd_end_submit(vk.upload_cmd);
}

// intern void vk_buffer_descriptor_update(VK_SubBuffer buf, u32 binding) {
//   Range range = buf.range;
//   VkDescriptorBufferInfo buffer_info = {
//     .buffer = vk.storage_buffer.handle,
//     .offset = range.offset,
//     .range = range.size,
//   };
//   VkWriteDescriptorSet ubo_descriptor = {
//     .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//     .dstSet = vk.descriptor_sets,
//     .dstBinding = binding,
//     .dstArrayElement = 0,
//     .descriptorCount = 1,
//     .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//     .pBufferInfo = &buffer_info,
//   };
//   VkWriteDescriptorSet descriptors[] = {ubo_descriptor};
//   vk.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
// }

// intern void vk_buffer_alloc(VK_SubBuffer* buf, u32 binding, u64 size, u64 align) {
//   GpuMemHandler handler = vk.gpu_alloc.alloc(size, align);
//   buf->mem = handler;
//   buf->range = {vk.gpu_alloc.get(handler), size};
// }

////////////////////////////////////////////////////////////////////////
// @Pipeline

intern void vk_shader_pipeline_init() {
  VkPushConstantRange push_constant = {
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    .offset = 0,
    .size = sizeof(PushConstant),
  };
  VkPipelineLayoutCreateInfo pipeline_layout_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts = &vk.descriptor_set_layout,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges = &push_constant,
  };
  VK_CHECK(vk.CreatePipelineLayout(vkdevice, &pipeline_layout_info, vk.allocator, &vk.pipeline_layout));
}

intern VkPipeline vk_shader_pipeline_create(Shader shader) {

  ///////////////////////////////////
  // Shader modules
  VkPipelineShaderStageCreateInfo stages[2] = {};
  {
    Scratch scratch;
    Loop (i, 2) {
      String stage_type_strs[] = {"vert", "frag"};
      VkShaderStageFlagBits stage_types[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
      String filepath = push_strf(scratch, "%s/shaders/compiled/%s.%s.spv", asset_base_path(), shader.name, stage_type_strs[i]);
      Buffer binary = os_file_path_read_all(scratch, filepath);
      VkShaderModuleCreateInfo module_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = binary.size,
        .pCode = (u32*)binary.data,
      };
      VkShaderModule handle;
      VK_CHECK(vk.CreateShaderModule(vkdevice, &module_info, vk.allocator, &handle));
      stages[i] = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = stage_types[i],
        .module = handle,
        .pName = "main",
      };
    }
  }

  // Dynamic rendering
  // VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
  VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;
  VkPipelineRenderingCreateInfo rendering_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .colorAttachmentCount = 1,
    .pColorAttachmentFormats = &color_format,
    .depthAttachmentFormat = vk.device.depth_format,
  };
  
  // Vertex input
  VkVertexInputBindingDescription binding_description = {
    .binding = 0,
    .stride = sizeof(Vertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
  };

  // Attributes
  VkVertexInputAttributeDescription attribute_desriptions[] = {
    [0] = {
      .location = 0,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = (u32)OffsetOf(Vertex, pos),
    },
    [1] = {
      .location = 1,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = (u32)OffsetOf(Vertex, norm),
    },
    [2] = {
      .location = 2,
      .binding = 0,
      .format = VK_FORMAT_R32G32_SFLOAT,
      .offset = (u32)OffsetOf(Vertex, uv),
    },
    [3] = {
      .location = 3,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = (u32)OffsetOf(Vertex, color),
    },
  };
  VkPipelineVertexInputStateCreateInfo vertex_input_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &binding_description,
    .vertexAttributeDescriptionCount = ArrayCount(attribute_desriptions),
    .pVertexAttributeDescriptions = attribute_desriptions,
  };

  // Input assembly
  VkPrimitiveTopology topology;
  switch (shader.state.topology) {
    case ShaderTopology_Triangle: topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
    case ShaderTopology_Line:     topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
    case ShaderTopology_Point:    topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; break;
  }
  VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = topology,
    .primitiveRestartEnable = VK_FALSE,
  };
  
  // Viewport
  VkViewport viewport = {
    .x = 0.0f,
    .y = (f32)vk.height,
    .width = (f32)vk.width,
    .height = -(f32)vk.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  VkRect2D scissor = {
    .offset = { .x = 0, .y = 0 },
    .extent = {
      .width = vk.width,
      .height = vk.height,
    },
  };
  VkPipelineViewportStateCreateInfo viewport_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .pViewports = &viewport,
    .scissorCount = 1,
    .pScissors = &scissor,
  };
  
  // Rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_NONE,
    // .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = VK_FALSE,
    .depthBiasConstantFactor = 0.0f,
    .depthBiasClamp = 0.0f,
    .depthBiasSlopeFactor = 0.0f,
    .lineWidth = 1.0f,
  };
  
  // Multisampling
  VkPipelineMultisampleStateCreateInfo multisampling_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    // .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    .rasterizationSamples = (VkSampleCountFlagBits)shader.state.samples,
    .sampleShadingEnable = VK_FALSE,
    .minSampleShading = 1.0f,
    .pSampleMask = 0,
    .alphaToCoverageEnable = VK_FALSE,
    .alphaToOneEnable = VK_FALSE,
  };

  // Depth and stencil testing
  VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  if (shader.state.use_depth) {
    depth_stencil_state_info.depthTestEnable = VK_TRUE;
    if (shader.state.type == ShaderType_Cube) {
      depth_stencil_state_info.depthWriteEnable = VK_FALSE;
      depth_stencil_state_info.depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL;
    } else {
      depth_stencil_state_info.depthWriteEnable = VK_TRUE;
      depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS;
    }
    depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state_info.stencilTestEnable = VK_FALSE;
  }
  
  // Blending
  VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
  if (shader.state.is_transparent) {
    color_blend_attachment_state.blendEnable = VK_TRUE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
  }
  color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  VkPipelineColorBlendStateCreateInfo color_blend_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_COPY,
    .attachmentCount = 1,
    .pAttachments = &color_blend_attachment_state,
  };
   
  // Dynamic state
  VkDynamicState dynamic_states[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
  };
  VkPipelineDynamicStateCreateInfo dynamic_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = ArrayCount(dynamic_states),
    .pDynamicStates = dynamic_states,
  };
  
  // Pipeline
  VkGraphicsPipelineCreateInfo pipeline_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = &rendering_info,
    .stageCount = 2,
    .pStages = stages,
    .pVertexInputState = &vertex_input_state,
    .pInputAssemblyState = &input_assembly_state,
    .pTessellationState = null,
    .pViewportState = &viewport_state,
    .pRasterizationState = &rasterizer_state,
    .pMultisampleState = &multisampling_state_info,
    .pDepthStencilState = &depth_stencil_state_info,
    .pColorBlendState = &color_blend_state_info,
    .pDynamicState = &dynamic_state_info,
    .layout = vk.pipeline_layout,
    .renderPass = null,
    .basePipelineHandle = VK_NULL_HANDLE,
    .basePipelineIndex = -1,
  };
  VkPipeline result;
  VK_CHECK(vk.CreateGraphicsPipelines(vkdevice, VK_NULL_HANDLE, 1, &pipeline_info, vk.allocator, &result));
  return result;
}

VK_ShaderModule vk_shader_module_create(String name) {
  Scratch scratch;
  VK_ShaderModule module = {};
  Loop (i, 2) {
    String stage_type_strs[] = {"vert", "frag"};
    VkShaderStageFlagBits stage_types[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    String filepath = push_strf(scratch, "%s/shaders/compiled/%s.%s.spv", asset_base_path(), name, stage_type_strs[i]);
    Buffer binary = os_file_path_read_all(scratch, filepath);
    VkShaderModuleCreateInfo module_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = binary.size,
      .pCode = (u32*)binary.data,
    };
    VkShaderModule handle;
    VK_CHECK(vk.CreateShaderModule(vkdevice, &module_info, vk.allocator, &handle));
    module.stages[i] = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = stage_types[i],
      .module = handle,
      .pName = "main",
    };
  }
  return module;
}

intern VkPipeline vk_shader_pipeline_create_(VK_ShaderModule module, ShaderState state) {
  // Dynamic rendering
  // VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
  VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;
  VkPipelineRenderingCreateInfo rendering_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .colorAttachmentCount = 1,
    .pColorAttachmentFormats = &color_format,
    .depthAttachmentFormat = vk.device.depth_format,
  };
  
  // Vertex input
  VkVertexInputBindingDescription binding_description = {
    .binding = 0,
    .stride = sizeof(Vertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
  };

  // Attributes
  VkVertexInputAttributeDescription attribute_desriptions[] = {
    [0] = {
      .location = 0,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = (u32)OffsetOf(Vertex, pos),
    },
    [1] = {
      .location = 1,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = (u32)OffsetOf(Vertex, norm),
    },
    [2] = {
      .location = 2,
      .binding = 0,
      .format = VK_FORMAT_R32G32_SFLOAT,
      .offset = (u32)OffsetOf(Vertex, uv),
    },
    [3] = {
      .location = 3,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = (u32)OffsetOf(Vertex, color),
    },
  };
  VkPipelineVertexInputStateCreateInfo vertex_input_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &binding_description,
    .vertexAttributeDescriptionCount = ArrayCount(attribute_desriptions),
    .pVertexAttributeDescriptions = attribute_desriptions,
  };

  // Input assembly
  VkPrimitiveTopology topology;
  switch (state.topology) {
    case ShaderTopology_Triangle: topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
    case ShaderTopology_Line:     topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
    case ShaderTopology_Point:    topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; break;
  }
  VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = topology,
    .primitiveRestartEnable = VK_FALSE,
  };
  
  // Viewport
  VkViewport viewport = {
    .x = 0.0f,
    .y = (f32)vk.height,
    .width = (f32)vk.width,
    .height = -(f32)vk.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  VkRect2D scissor = {
    .offset = { .x = 0, .y = 0 },
    .extent = {
      .width = vk.width,
      .height = vk.height,
    },
  };
  VkPipelineViewportStateCreateInfo viewport_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .pViewports = &viewport,
    .scissorCount = 1,
    .pScissors = &scissor,
  };
  
  // Rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_NONE,
    // .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = VK_FALSE,
    .depthBiasConstantFactor = 0.0f,
    .depthBiasClamp = 0.0f,
    .depthBiasSlopeFactor = 0.0f,
    .lineWidth = 1.0f,
  };
  
  // Multisampling
  VkPipelineMultisampleStateCreateInfo multisampling_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    // .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    .rasterizationSamples = (VkSampleCountFlagBits)state.samples,
    .sampleShadingEnable = VK_FALSE,
    .minSampleShading = 1.0f,
    .pSampleMask = 0,
    .alphaToCoverageEnable = VK_FALSE,
    .alphaToOneEnable = VK_FALSE,
  };

  // Depth and stencil testing
  VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  if (state.use_depth) {
    depth_stencil_state_info.depthTestEnable = VK_TRUE;
    if (state.type == ShaderType_Cube) {
      depth_stencil_state_info.depthWriteEnable = VK_FALSE;
      depth_stencil_state_info.depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL;
    } else {
      depth_stencil_state_info.depthWriteEnable = VK_TRUE;
      depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS;
    }
    depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state_info.stencilTestEnable = VK_FALSE;
  }
  
  // Blending
  VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
  if (state.is_transparent) {
    color_blend_attachment_state.blendEnable = VK_TRUE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
  }
  color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  VkPipelineColorBlendStateCreateInfo color_blend_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_COPY,
    .attachmentCount = 1,
    .pAttachments = &color_blend_attachment_state,
  };
   
  // Dynamic state
  VkDynamicState dynamic_states[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
  };
  VkPipelineDynamicStateCreateInfo dynamic_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = ArrayCount(dynamic_states),
    .pDynamicStates = dynamic_states,
  };
  
  // Pipeline
  VkGraphicsPipelineCreateInfo pipeline_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = &rendering_info,
    .stageCount = 2,
    .pStages = module.stages,
    .pVertexInputState = &vertex_input_state,
    .pInputAssemblyState = &input_assembly_state,
    .pTessellationState = null,
    .pViewportState = &viewport_state,
    .pRasterizationState = &rasterizer_state,
    .pMultisampleState = &multisampling_state_info,
    .pDepthStencilState = &depth_stencil_state_info,
    .pColorBlendState = &color_blend_state_info,
    .pDynamicState = &dynamic_state_info,
    .layout = vk.pipeline_layout,
    .renderPass = null,
    .basePipelineHandle = VK_NULL_HANDLE,
    .basePipelineIndex = -1,
  };
  VkPipeline result;
  VK_CHECK(vk.CreateGraphicsPipelines(vkdevice, VK_NULL_HANDLE, 1, &pipeline_info, vk.allocator, &result));
  return result;
}

void vk_shader_reload(String name) {
  u32* module_idx = vk.shader_to_module.get(name);
  VK_State::ModuleEntry& entry = vk.modules[*module_idx];
  entry.module = vk_shader_module_create(name);
  Loop (i, entry.track_pipelines.count) {
    u32 pipeline_idx = entry.track_pipelines[i];
    u32 material_idx = entry.track_shader_states[i];
    ShaderState state = vk.materials[material_idx].shader_state;
    vk.pipelines[pipeline_idx] = vk_shader_pipeline_create_(entry.module, state);
  }
}

intern void vk_descriptor_init() {
  // Pool
  #define MaxStorageBuffer 7
  #define MaxSets 1
  #define MaxSamplers 1
  #define MaxCubeTextures 1
  {
    VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MaxStorageBuffer},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MaxTextures},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MaxCubeTextures},
      {VK_DESCRIPTOR_TYPE_SAMPLER, MaxSamplers},
    };
    VkDescriptorPoolCreateInfo pool_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
      .maxSets = MaxSets,
      .poolSizeCount = ArrayCount(pool_sizes),
      .pPoolSizes = pool_sizes,
    };
    vk.CreateDescriptorPool(vkdevice, &pool_info, vk.allocator, &vk.descriptor_pool);
  }

  // Setlayout
  {
    // Bindless extension
    VkDescriptorBindingFlags flags[] = {
      0,
      VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
    };
    VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
      .pNext = null,
      .bindingCount = ArrayCount(flags),
      .pBindingFlags = flags,
    };
    VkDescriptorSetLayoutBinding layout_bindings[] = {
      // Global state
      {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // Textures
      {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = MaxTextures,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // Samplers
      {
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .descriptorCount = MaxSamplers,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // Cube textures
      {
        .binding = 3,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = MaxCubeTextures,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // DrawCall info
      {
        .binding = 4,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // Entities
      {
        .binding = 5,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // Materials
      {
        .binding = 6,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // Point lights
      {
        .binding = 7,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // Dir lights
      {
        .binding = 8,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // Spot lights
      {
        .binding = 9,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      },
    };
    VkDescriptorSetLayoutCreateInfo layout_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = &binding_flags,
      .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
      .bindingCount = ArrayCount(layout_bindings),
      .pBindings = layout_bindings,
    };
    VK_CHECK(vk.CreateDescriptorSetLayout(vkdevice, &layout_info, vk.allocator, &vk.descriptor_set_layout));
  }

  // Descriptor set
  {
    VkDescriptorSetAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = vk.descriptor_pool,
      .descriptorSetCount = 1,
      .pSetLayouts = &vk.descriptor_set_layout,
    };
    VK_CHECK(vk.AllocateDescriptorSets(vkdevice, &alloc_info, &vk.descriptor_sets));
  }

  // Descriptor
  {
    OffsetPusher pusher = {};
    offset_push_struct(pusher, GlobalStateGPU);
    u64 entities_offset = (u64)offset_push_array(pusher, EntityGPU, MaxEntities+MaxStaticEntities);
    u64 materials_offset = (u64)offset_push_array(pusher, MaterialGPU, MaxMaterials);
    // Global
    VkDescriptorBufferInfo global_state_buffer_info = {
      .buffer = vk.storage_buffer.handle,
      .offset = 0,
      .range = sizeof(GlobalStateGPU),
    };
    VkWriteDescriptorSet global_state_descriptor_write = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = vk.descriptor_sets,
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .pBufferInfo = &global_state_buffer_info,
    };
    // Entity
    VkDescriptorBufferInfo entity_buffer_info = {
      .buffer = vk.storage_buffer.handle,
      .offset = entities_offset,
      .range = sizeof(EntityGPU) * (MaxEntities + MaxStaticEntities),
    };
    VkWriteDescriptorSet entity_descriptor_write = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = vk.descriptor_sets,
      .dstBinding = 5,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .pBufferInfo = &entity_buffer_info,
    };
    // Material
    VkDescriptorBufferInfo material_buffer_info = {
      .buffer = vk.storage_buffer.handle,
      .offset = materials_offset,
      .range = sizeof(MaterialGPU) * MaxMaterials,
    };
    VkWriteDescriptorSet material_descriptor_write = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = vk.descriptor_sets,
      .dstBinding = 6,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .pBufferInfo = &material_buffer_info,
    };
    // Indirect draw
    VkDescriptorBufferInfo indirect_draw_buffer_info = {
      .buffer = vk.indirect_draw_buffer.handle,
      .offset = 0,
      .range = VK_WHOLE_SIZE,
    };
    VkWriteDescriptorSet indirect_draw_write_descriptor = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = vk.descriptor_sets,
      .dstBinding = 4,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .pBufferInfo = &indirect_draw_buffer_info,
    };
    VkWriteDescriptorSet descriptors[] = {global_state_descriptor_write, indirect_draw_write_descriptor, entity_descriptor_write, material_descriptor_write};
    vk.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  }

  // Set pointers to shader data
  {
    OffsetMemPusher pusher = {.offset = vk.storage_buffer.mapped_memory};
    vk.gpu_global_shader_st = offset_push_struct(pusher, GlobalStateGPU);
    vk.gpu_entities_indexes = vk.gpu_global_shader_st->entity_indices;
    vk.gpu_entities = offset_push_array(pusher, EntityGPU, MaxEntities+MaxStaticEntities);
    vk.gpu_materials = offset_push_array(pusher, MaterialGPU, MaxMaterials);

    vk.gpu_draw_call_infos = (VK_DrawCallInfo*)vk.indirect_draw_buffer.mapped_memory;
  }
  
}

////////////////////////////////////////////////////////////////////////
// @Image

intern void vk_image_init() {
  // vk_texture_load(Texture texture);

  // VkMemoryRequirements memory_requirements;
  // vk.GetImageMemoryRequirements(vkdevice, handle, &memory_requirements);
  // u32 memory_type = vk_find_memory_idx(memory_requirements.memoryTypeBits, info.memory_flags);
  // VkMemoryAllocateInfo memory_allocate_info = {
  //   .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
  //   .allocationSize = memory_requirements.size,
  //   .memoryTypeIndex = memory_type,
  // };
  // VkDeviceMemory memory;
  // VK_CHECK(vk.AllocateMemory(vkdevice, &allocate_info, vk.allocator, &buffer.memory));
  // VK_CHECK(vk.BindImageMemory(vkdevice, handle, memory, 0));
}

intern VK_ImageInfo vk_image_info_default(u32 width, u32 height) {
  VK_ImageInfo result = {
    .image_type = VK_IMAGE_TYPE_2D,
    .width = width,
    .height = height,
    .miplevels_count = 1,
    .flags = 0,
    .format = VK_FORMAT_R8G8B8A8_UNORM,
    .array_layers_count = 1,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
    .view_type = VK_IMAGE_VIEW_TYPE_2D,
  };
  return result;
};

intern VkImageView vk_image_view_create(VkImage image, VK_ImageInfo info) {
  VkImageViewCreateInfo view_create_info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .image = image,
    .viewType = info.view_type,
    .format = info.format,
    .subresourceRange = {
      .aspectMask = info.aspect,
      .baseMipLevel = 0,
      .levelCount = info.miplevels_count,
      .baseArrayLayer = 0,
      .layerCount = info.array_layers_count,
    },
  };
  VkImageView result;
  VK_CHECK(vk.CreateImageView(vkdevice, &view_create_info, vk.allocator, &result));
  return result;
}

intern VK_Image vk_image_create(VK_ImageInfo info) {
  VkImageCreateInfo image_create_info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .flags = info.flags,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = info.format,
    .extent = {info.width, info.height, 1},
    .mipLevels = info.miplevels_count,
    .arrayLayers = info.array_layers_count,
    .samples = info.samples,
    .tiling = info.tiling,
    .usage = info.usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };
  VkImage handle;
  VK_CHECK(vk.CreateImage(vkdevice, &image_create_info, vk.allocator, &handle));
  VkMemoryRequirements memory_requirements;
  vk.GetImageMemoryRequirements(vkdevice, handle, &memory_requirements);
  u32 memory_type = vk_find_memory_idx(memory_requirements.memoryTypeBits, info.memory_flags);
  VkMemoryAllocateInfo memory_allocate_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = memory_requirements.size,
    .memoryTypeIndex = memory_type,
  };
  VkDeviceMemory memory;
  VK_CHECK(vk.AllocateMemory(vkdevice, &memory_allocate_info, vk.allocator, &memory));
  VK_CHECK(vk.BindImageMemory(vkdevice, handle, memory, 0));
  VkImageView view = vk_image_view_create(handle, info);
  VK_Image result = {
    .handle = handle,
    .memory = memory,
    .view = view,
    .info = info,
  };
  return result;
}

intern void vk_image_destroy(VK_Image image) {
  if (image.view) {
    vk.DestroyImageView(vkdevice, image.view, vk.allocator);
  }
  if (image.memory) {
    vk.FreeMemory(vkdevice, image.memory, vk.allocator);
  }
  if (image.handle) {
    vk.DestroyImage(vkdevice, image.handle, vk.allocator);
  }
}

intern VkAccessFlags vk_image_layout_to_mem_access_flags(VkImageLayout layout) {
	switch (layout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:                return NoFlags;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:     return VK_ACCESS_TRANSFER_READ_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:     return VK_ACCESS_TRANSFER_WRITE_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:          return NoFlags;
    default: InvalidPath; return {};
	}
}

intern VkPipelineStageFlags vk_image_layout_to_pipeline_stage_flags(VkImageLayout layout) {
	switch (layout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:                return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:     return VK_PIPELINE_STAGE_TRANSFER_BIT; 
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:          return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    default: InvalidPath; return {};
	}
}

intern void vk_image_layout_transition(VkCommandBuffer cmd, VK_Image image, VkImageLayout old_layout, VkImageLayout new_layout, VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT) {
  VkImageMemoryBarrier barrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .srcAccessMask = vk_image_layout_to_mem_access_flags(old_layout),
    .dstAccessMask = vk_image_layout_to_mem_access_flags(new_layout),
    .oldLayout = old_layout,
    .newLayout = new_layout,
    .srcQueueFamilyIndex = vk.device.graphics_queue_index,
    .image = image.handle,
    .subresourceRange = {
      .aspectMask = aspect_mask,
      .baseMipLevel = 0,
      .levelCount = image.info.miplevels_count,
      .baseArrayLayer = 0,
      .layerCount = image.info.array_layers_count,
    },
  };
  VkPipelineStageFlags src_stage = vk_image_layout_to_pipeline_stage_flags(old_layout);
  VkPipelineStageFlags dst_stage = vk_image_layout_to_pipeline_stage_flags(new_layout);
  vk.CmdPipelineBarrier(cmd, src_stage, dst_stage, NoFlags, 0, null, 0, null, 1, &barrier);
}

intern void vk_image_upload_to_gpu(VkCommandBuffer cmd, VK_Image image) {
  VkBufferImageCopy region = {
    .bufferOffset = 0,
    .bufferRowLength = 0,
    .bufferImageHeight = 0,
    .imageSubresource = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .mipLevel = 0,
      .baseArrayLayer = 0,
      .layerCount = image.info.array_layers_count,
    },
    .imageExtent = { image.info.width, image.info.height, 1 },
  };
  vk.CmdCopyBufferToImage(cmd, vk.stage_buffer.handle, image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

intern void vk_texture_init() {
  // Common texture sampler
  VkSamplerCreateInfo sampler_info = {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .magFilter = VK_FILTER_LINEAR,
    .minFilter = VK_FILTER_LINEAR,
    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .mipLodBias = 0.0f,
    .anisotropyEnable = VK_TRUE,
    .maxAnisotropy = 8,
    .compareEnable = VK_FALSE,
    .compareOp = VK_COMPARE_OP_ALWAYS,
    .minLod = 0.0f,
    .maxLod = VK_LOD_CLAMP_NONE,
    .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
    .unnormalizedCoordinates = VK_FALSE,
  };
  VK_CHECK(vk.CreateSampler(vkdevice, &sampler_info, vk.allocator, &vk.sampler));
  VkDescriptorImageInfo descriptor_image_info = {
    .sampler = vk.sampler,
    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
  VkWriteDescriptorSet texture_descriptor = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = vk.descriptor_sets,
    .dstBinding = 2,
    .dstArrayElement = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
    .pImageInfo = &descriptor_image_info,
  };
  VkWriteDescriptorSet descriptors[] = {texture_descriptor};
  vk.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
};

intern void vk_texture_generate_mipmaps(VK_Image image) {
  VkCommandBuffer cmd = vk.upload_cmd;
  VkImageMemoryBarrier barrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .srcQueueFamilyIndex = vk.device.graphics_queue_index,
    .image = image.handle,
    .subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .levelCount = 1,
      .layerCount = 1,
    },
  };
  
  i32 width = image.info.width;
  i32 height = image.info.height;
  for (i32 i = 1; i < image.info.miplevels_count; ++i) {
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    vk.CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, null, 0, null, 1, &barrier);

    VkImageBlit blit = {
      .srcOffsets = { {0, 0, 0}, {width, height, 1} },
      .srcSubresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = (u32)i - 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
      },
      .dstOffsets = { {0, 0, 0}, {width > 1 ? width / 2 : 1, height > 1 ? height / 2 : 1, 1} },
      .dstSubresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = (u32)i,
        .baseArrayLayer = 0,
        .layerCount = 1,
      },
    };
    vk.CmdBlitImage(cmd, image.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vk.CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, null, 0, null, 1, &barrier);

    if (width > 1) width /= 2;
    if (height > 1) height /= 2;
  }

  barrier.subresourceRange.baseMipLevel = image.info.miplevels_count - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vk.CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, null, 0, null, 1, &barrier);
}

Handle<GpuTexture> vk_texture_load(Texture texture) {
  u64 size = texture.width * texture.height * 4;
  MemCopy(vk.stage_buffer.mapped_memory, texture.data, size);
  VK_ImageInfo image_info = vk_image_info_default(texture.width, texture.height);
  image_info.miplevels_count = Floor(Log2(Max(image_info.width, image_info.height))) + 1;
  image_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  VK_Image image = vk_image_create(image_info);
  {
    VkCommandBuffer cmd = vk.upload_cmd;
    vk_cmd_begin(cmd);
    vk_image_layout_transition(cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vk_image_upload_to_gpu(cmd, image);
    vk_texture_generate_mipmaps(image);
    vk_cmd_end_submit(cmd);
  }
  VkDescriptorImageInfo descriptor_image_info = {
    .sampler = vk.sampler,
    .imageView = image.view,
    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
  VkWriteDescriptorSet texture_write_descriptor = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = vk.descriptor_sets,
    .dstBinding = 1,
    .dstArrayElement = vk.textures.count,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    .pImageInfo = &descriptor_image_info,
  };
  VkWriteDescriptorSet descriptors[] = {texture_write_descriptor};
  vk.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  u32 id = vk.textures.count;
  vk.textures.add(image);
  Handle<GpuTexture> handle = {id};
  return handle;
}

intern void vk_texture_resize_target() {
  VK_CHECK(vk.DeviceWaitIdle(vkdevice));
  Debug("texture target resized: x = %u y = %u", vk.width, vk.height);
  u32 width = vk.width * vk.scale;
  u32 height = vk.height * vk.scale;
  if (width == 0) width = 1;
  if (height == 0) height = 1;

  // Msaa
  vk_image_destroy(vk.msaa_texture_target);
  VK_ImageInfo image_info = vk_image_info_default(width, height);
  image_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  image_info.format = VK_FORMAT_B8G8R8A8_UNORM;
  image_info.samples = VK_SAMPLE_COUNT_4_BIT;
  vk.msaa_texture_target = vk_image_create(image_info);

  // Texture target
  Loop (i, vk.images_in_flight) {
    vk_image_destroy(vk.texture_targets[i]);
    VK_ImageInfo image_info = vk_image_info_default(width, height);
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    vk.texture_targets[i] = vk_image_create(image_info);
    VkDescriptorImageInfo descriptor_image_info = {
      .sampler = vk.sampler,
      .imageView = vk.texture_targets[i].view,
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    VkWriteDescriptorSet texture_write_descriptor = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = vk.descriptor_sets,
      .dstBinding = 1,
      .dstArrayElement = (u32)i,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
      .pImageInfo = &descriptor_image_info,
    };
    VkWriteDescriptorSet descriptors[] = {texture_write_descriptor};
    vk.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  }

  // Offscreen depth buffer
  vk_image_destroy(vk.offscreen_depth_buffer);
  VK_ImageInfo depth_info = vk_image_info_default(width, height);
  depth_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  depth_info.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
  depth_info.format = vk.device.depth_format;
  depth_info.samples = VK_SAMPLE_COUNT_4_BIT;
  vk.offscreen_depth_buffer = vk_image_create(depth_info);
}

// Handle<MaterialGPU> vk_material_load(Material material) {
//   vk.gpu_materials[vk.gpu_materials_count] = {
//     .ambient = material.props.ambient,
//     .diffuse = material.props.diffuse,
//     .specular = material.props.specular,
//     .shininess = material.props.shininess, 
//     // .texture = material.texture, 
//     .texture = material.texture_handle, 
//   };
//   u32 id = vk.gpu_materials_count++;
//   Handle<MaterialGPU> handle = {id};
//   return handle;
// }

Handle<GpuMaterial> vk_material_load(Material material) {
  KeyToShaderPipeline key = {material.shader.name, material.shader.state};
  u32* pipeline_idx = vk.shader_to_pipeline.get(key);
  if (!pipeline_idx) {
    u32* module_idx = vk.shader_to_module.get(material.shader.name);
    if (!module_idx) {
      VK_ShaderModule module = vk_shader_module_create(material.shader.name);
      module_idx = vk.shader_to_module.add(material.shader.name, vk.modules.count);
      vk.modules.add({module});
    }
    VkPipeline pipeline = vk_shader_pipeline_create_(vk.modules[*module_idx].module, material.shader.state);
    pipeline_idx = vk.shader_to_pipeline.add(key, vk.pipelines.count);
    vk.pipelines.add(pipeline);
    vk.batches.add({{}});

    VK_State::ModuleEntry& entry = vk.modules[*module_idx];
    entry.track_pipelines.add(*pipeline_idx);
    entry.track_shader_states.add(vk.materials.count);
  }
  vk.gpu_materials[vk.materials.count] = {
    .ambient = material.props.ambient,
    .diffuse = material.props.diffuse,
    .specular = material.props.specular,
    .shininess = material.props.shininess, 
    .texture = material.texture_handle, 
  };
  GpuMaterial mat = {
    .pipeline_idx = *pipeline_idx,
    .shader_state = material.shader.state,
    .texture = material.texture_handle,
    .props = material.props,
  };
  Handle<GpuMaterial> result = {vk.materials.count};
  vk.materials.add(mat);
  return result;
}

Handle<GpuCubemap> vk_cubemap_load(Texture* textures) {
  u32 width = textures->width;
  u32 height = textures->height;
  u64 size = width * height * 4;
  Loop (i, 6) {
    MemCopy(Offset(vk.stage_buffer.mapped_memory, size * i), textures[i].data, size);
  }
  VK_ImageInfo image_info = vk_image_info_default(width, height);
  image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  image_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
  image_info.array_layers_count = 6;
  image_info.view_type = VK_IMAGE_VIEW_TYPE_CUBE;
  VK_Image image = vk_image_create(image_info);
  {
    VkCommandBuffer cmd = vk.upload_cmd;
    vk_cmd_begin(cmd);
    vk_image_layout_transition(cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vk_image_upload_to_gpu(cmd, image);
    vk_image_layout_transition(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vk_cmd_end_submit(cmd);
  }
  VkDescriptorImageInfo descriptor_image_info = {
    .sampler = vk.sampler,
    .imageView = image.view,
    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
  VkWriteDescriptorSet texture_descriptor = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = vk.descriptor_sets,
    .dstBinding = 3,
    .dstArrayElement = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    .pImageInfo = &descriptor_image_info,
  };
  VkWriteDescriptorSet descriptors[] = {texture_descriptor};
  vk.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  return {};
}

////////////////////////////////////////////////////////////////////////
// @Device

struct VK_DevicePhysicalRequirements {
  b8 graphics; 
  b8 present;
  b8 transfer;
  b8 compute;
  b8 depth_attachment;
  StringArray device_extension_names;
};

struct VK_DevicePhysicalQueueFamilyInfo {
  u32 graphics_family_index;
  u32 present_family_index;
  u32 compute_family_index;
  u32 transfer_family_index;
};

intern VK_SwapchainSupportInfo vk_device_query_swapchain_support(VkPhysicalDevice physical_device) {
  VK_SwapchainSupportInfo support_info = {};
  // Surface capabilities
  VK_CHECK(vk.GetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, vk.surface, &support_info.capabilities));
  // Surface formats
  VK_CHECK(vk.GetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk.surface, &support_info.format_count, null));
  support_info.formats = push_array(vk.arena, VkSurfaceFormatKHR, support_info.format_count);
  VK_CHECK(vk.GetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk.surface, &support_info.format_count, support_info.formats));
  // Present modes
  VK_CHECK(vk.GetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk.surface, &support_info.present_mode_count, null));
  support_info.present_modes = push_array(vk.arena, VkPresentModeKHR, support_info.present_mode_count);
  VK_CHECK(vk.GetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk.surface, &support_info.present_mode_count, support_info.present_modes));
  return support_info;
}

intern VkFormat vk_device_detect_depth_format(VkPhysicalDevice physical_device) {
  VkFormat candidates[] = {
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D24_UNORM_S8_UINT
  };
  u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
  for (VkFormat x : candidates) {
    VkFormatProperties properties;
    vk.GetPhysicalDeviceFormatProperties(physical_device, x, &properties);
    if (FlagHas(properties.optimalTilingFeatures, flags)) {
      return x;
    }
  }
  Assert(!"Failed to find a supported format!");
  return VK_FORMAT_UNDEFINED;
}

intern VK_DevicePhysicalQueueFamilyInfo vk_device_physical_meets_requirements(
  VkPhysicalDevice device,
  VkPhysicalDeviceProperties properties,
  VkPhysicalDeviceFeatures features,
  VK_DevicePhysicalRequirements requirements,
  VK_SwapchainSupportInfo* out_swapchain_support)
{
  Scratch scratch;
  VK_DevicePhysicalQueueFamilyInfo queue_info = {
    .graphics_family_index = (u32)-1,
    .present_family_index = (u32)-1,
    .compute_family_index = (u32)-1,
    .transfer_family_index = (u32)-1,
  };
  u32 queue_family_count = 0;
  vk.GetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, null);
  VkQueueFamilyProperties* queue_families = push_array(scratch, VkQueueFamilyProperties, queue_family_count);
  vk.GetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);
  
  // Look at each queue and see what queues it supports
  Info("Graphics | Present | Computer | Transfer | Name");
  Loop (i, queue_family_count) {
    // Graphics queue?
    if ((queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queue_info.graphics_family_index == -1)) {
      queue_info.graphics_family_index = i;
    }
    // Transfer queue?
    if ((queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && (queue_info.transfer_family_index == -1) &&
       !(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
    {
      queue_info.transfer_family_index = i;
    }
    // Compute queue?
    if ((queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && (queue_info.compute_family_index == -1) &&
       !(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT))
    {
      queue_info.compute_family_index = i;
    }
    // Present queue?
    VkBool32 supports_present;
    VK_CHECK(vk.GetPhysicalDeviceSurfaceSupportKHR(device, i, vk.surface, &supports_present));
    if (supports_present && queue_info.present_family_index == -1) {
      queue_info.present_family_index = i;
    }
    if (queue_info.graphics_family_index != -1 &&
        queue_info.transfer_family_index != -1 &&
        queue_info.compute_family_index != -1 &&
        queue_info.present_family_index != -1) 
    {
      break;
    }
  }

  if (queue_info.transfer_family_index == -1) queue_info.transfer_family_index = queue_info.graphics_family_index;
  if (queue_info.compute_family_index == -1) queue_info.compute_family_index = queue_info.graphics_family_index;

  Info("       %i |       %i |        %i |        %i | %s",
    queue_info.graphics_family_index != -1,
    queue_info.present_family_index != -1,
    queue_info.compute_family_index != -1,
    queue_info.transfer_family_index != -1,
    String(properties.deviceName));

  var missing = [](b32 required, u32 index) {
    return required && index == -1;
  };
  if (missing(requirements.graphics, queue_info.graphics_family_index) ||
      missing(requirements.present, queue_info.present_family_index) ||
      missing(requirements.compute, queue_info.compute_family_index) ||
      missing(requirements.compute, queue_info.transfer_family_index)) 
  {
    AssertMsg(false, "Device doesn't have needed queues");
  }

  Info("Device meets queue requirements");
  Trace("Grahics Family Index: %i", queue_info.graphics_family_index);
  Trace("Present Family Index: %i", queue_info.present_family_index);
  Trace("Transfer Family Index: %i", queue_info.transfer_family_index);
  Trace("Compute Family Index: %i", queue_info.compute_family_index);

  *out_swapchain_support = vk_device_query_swapchain_support(device);
  Assert((out_swapchain_support->format_count >= 1) && (out_swapchain_support->present_mode_count >= 1) && "Required swapchain support not present");

  // Device extensions.
  if (requirements.device_extension_names.count) {
    u32 available_extension_count = 0;
    VkExtensionProperties* available_extensions = 0;
    VK_CHECK(vk.EnumerateDeviceExtensionProperties(device, null, &available_extension_count, null));
    if (available_extension_count != 0) {
      available_extensions = push_array(scratch, VkExtensionProperties, available_extension_count);
      VK_CHECK(vk.EnumerateDeviceExtensionProperties(device, null, &available_extension_count, available_extensions));
    }
    Loop(i, requirements.device_extension_names.count) {
      b32 found = false;
      Loop(j, available_extension_count) {
        if (str_match(requirements.device_extension_names.v[i], available_extensions[j].extensionName)) {
          found = true;
          break;
        }
      }
      Assert(found);
    }
  }

  return queue_info;
}

intern VK_Device vk_device_select_physical() {
  Scratch scratch;
  u32 physical_device_count;
  VK_CHECK(vk.EnumeratePhysicalDevices(vk.instance, &physical_device_count, null));
  VK_Device* devices = push_array(scratch, VK_Device, physical_device_count);
  VkPhysicalDevice* physical_devices = push_array(scratch, VkPhysicalDevice, physical_device_count);
  VK_CHECK(vk.EnumeratePhysicalDevices(vk.instance, &physical_device_count, physical_devices));

  i32 discrete_gpu_idx = -1;
  i32 fallback_gpu_idx = -1;

  Loop (i, physical_device_count) {
    VkPhysicalDeviceProperties properties;
    vk.GetPhysicalDeviceProperties(physical_devices[i], &properties);
    VkPhysicalDeviceFeatures features;
    vk.GetPhysicalDeviceFeatures(physical_devices[i], &features);
    VkPhysicalDeviceMemoryProperties memory;
    vk.GetPhysicalDeviceMemoryProperties(physical_devices[i], &memory);
    
    String extentions[] = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    VK_DevicePhysicalRequirements requirements = {
      .graphics = true,
      .present = true,
      .transfer = true,
      .compute = true,
      .depth_attachment = true,
      .device_extension_names = {extentions, ArrayCount(extentions)},
    };

    VK_DevicePhysicalQueueFamilyInfo queue_info = vk_device_physical_meets_requirements(
      physical_devices[i],
      properties,
      features,
      requirements,
      &devices[i].swapchain_support
    );

    Info("Available device: '%s'", String(properties.deviceName));
    // GPU type, etc.
    switch (properties.deviceType) {
      case VK_PHYSICAL_DEVICE_TYPE_OTHER: {
        Info("GPU type is Unkown");
        fallback_gpu_idx = i;
      } break;
      case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: {
        Info("GPU type is Integrated");
        fallback_gpu_idx = i;
      } break;
      case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: {
        discrete_gpu_idx = i;
      } Info("GPU type is Descrete");
        break;
      case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: {
        Info("GPU type is Virtual");
        fallback_gpu_idx = i;
      } break;
      case VK_PHYSICAL_DEVICE_TYPE_CPU: {
        Info("GPU type is CPU");
        fallback_gpu_idx = i;
      } break;
      default:;
    }

    Info("GPU Driver version: %i.%i.%i",
      VK_VERSION_MAJOR(properties.driverVersion),
      VK_VERSION_MINOR(properties.driverVersion),
      VK_VERSION_PATCH(properties.driverVersion));
    Info("GPU API version: %i.%i.%i",
      VK_VERSION_MAJOR(properties.apiVersion),
      VK_VERSION_MINOR(properties.apiVersion),
      VK_VERSION_PATCH(properties.apiVersion));

    Loop (j, memory.memoryHeapCount) {
      f32 memory_size_gib = (((f32)memory.memoryHeaps[j].size) / GB(1));
      if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        Info("Local GPU memory: %.2f GiB", memory_size_gib);
      else
        Info("Shared System memory: %.2f GiB", memory_size_gib);
    }

    devices[i].physical_device = physical_devices[i];
    devices[i].graphics_queue_index = queue_info.graphics_family_index;
    devices[i].present_queue_index = queue_info.present_family_index;
    devices[i].transfer_queue_index = queue_info.transfer_family_index;
    devices[i].compute_queue_index = queue_info.compute_family_index;

    devices[i].properties = properties;
    devices[i].features = features;
    devices[i].memory = memory;
    devices[i].depth_format = vk_device_detect_depth_format(physical_devices[i]);
  }
  i32 selected_index;
  if (discrete_gpu_idx != -1) {
    selected_index = discrete_gpu_idx; 
    Info("Discrete GPU was choosen");
  } else {
    selected_index = fallback_gpu_idx;
    Info("Integrated GPU was choosen");
  }
  Info("Physical device selected");
  return devices[selected_index];
}

intern void vk_device_create() {
  vk.device = vk_device_select_physical();

  // Features
  VkPhysicalDeviceShaderDrawParametersFeatures drawFeatures = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES,
    .shaderDrawParameters = VK_TRUE,
  };
  VkPhysicalDeviceDescriptorIndexingFeatures indexing_features = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
    .pNext = &drawFeatures,
    .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,     // allows runtime indexing
    .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,  // allows update image descriptors
    .descriptorBindingPartiallyBound = VK_TRUE,               // allows not updated descriptors
    .runtimeDescriptorArray = VK_TRUE,                        // allows not specified size of descriptor array in shader
  };
  VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
    .pNext = &indexing_features,
    .dynamicRendering = true
  };
  
  const u32 queue_count = 4;
  Array<u32, queue_count> indices = {};
  indices.add(vk.device.graphics_queue_index);
  if (!indices.exists(vk.device.present_queue_index)) {
    indices.add(vk.device.present_queue_index);
  }
  if (!indices.exists(vk.device.transfer_queue_index)) {
    indices.add(vk.device.transfer_queue_index);
  }
  if (!indices.exists(vk.device.compute_queue_index)) {
    indices.add(vk.device.compute_queue_index);
  }

  Array<VkDeviceQueueCreateInfo, queue_count> queue_create_infos = {};
  for (u32 i : indices) {
    f32 queue_priority = 1.0f;
    VkDeviceQueueCreateInfo device_queue_create_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = null,
      .flags = 0,
      .queueFamilyIndex = indices[i],
      .queueCount = 1,
      .pQueuePriorities = &queue_priority,
    };
    queue_create_infos.add(device_queue_create_info);
  }
  
  const char* extension_names[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };
  VkPhysicalDeviceFeatures device_features = {
    .multiDrawIndirect = true,
    .fillModeNonSolid = true,  // Request anistrophy
    .samplerAnisotropy = true, // Request wireframe
  };
  VkDeviceCreateInfo device_create_info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext = &dynamic_rendering_features,
    .queueCreateInfoCount = queue_create_infos.count,
    .pQueueCreateInfos = queue_create_infos.data,
    .enabledExtensionCount = ArrayCount(extension_names),
    .ppEnabledExtensionNames = extension_names,
    .pEnabledFeatures = &device_features,
  };
  VK_CHECK(vk.CreateDevice(vk.device.physical_device, &device_create_info, vk.allocator, &vk.device.logical_device));
  Info("Logical device created");
}

////////////////////////////////////////////////////////////////////////
// @Swapchain

intern void vk_swapchain_create(b32 reuse) {
  VK_CHECK(vk.GetPhysicalDeviceSurfaceCapabilitiesKHR(vk.device.physical_device, vk.surface, &vk.device.swapchain_support.capabilities));
  VkExtent2D swapchain_extent = {vk.width, vk.height};
  if (vk.device.swapchain_support.capabilities.currentExtent.width != U32_MAX) {
    swapchain_extent = vk.device.swapchain_support.capabilities.currentExtent;
  }
  VkExtent2D min = vk.device.swapchain_support.capabilities.minImageExtent;
  VkExtent2D max = vk.device.swapchain_support.capabilities.maxImageExtent;
  swapchain_extent.width = Clamp(min.width, swapchain_extent.width, max.width);
  swapchain_extent.height = Clamp(min.height, swapchain_extent.height, max.height);
  VkSwapchainCreateInfoKHR swapchain_create_info = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = vk.surface,
    .minImageCount = vk.images_in_flight,
    .imageFormat = vk.swapchain.image_format.format,
    .imageColorSpace = vk.swapchain.image_format.colorSpace,
    .imageExtent = swapchain_extent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = vk.device.swapchain_support.capabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = vk.swapchain.present_mode,
    .clipped = VK_TRUE,
    .oldSwapchain = reuse ? vk.swapchain.handle : null,
  };
  VK_CHECK(vk.CreateSwapchainKHR(vkdevice, &swapchain_create_info, vk.allocator, &vk.swapchain.handle));
  u32 image_count = vk.images_in_flight;

  VkImage images[4];
  VK_CHECK(vk.GetSwapchainImagesKHR(vkdevice, vk.swapchain.handle, &image_count, images));
  Loop (i, image_count) {
    vk.swapchain.images[i] = {
      .handle = images[i],
      .info = {
        .miplevels_count = 1,
        .array_layers_count = 1,
      }
    };
    VkImageViewCreateInfo view_create_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = vk.swapchain.images[i].handle,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = vk.swapchain.image_format.format,
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
      },
    };
    VK_CHECK(vk.CreateImageView(vkdevice, &view_create_info, vk.allocator, &vk.swapchain.images[i].view));
  }
  VK_ImageInfo depth_info = vk_image_info_default(swapchain_extent.width, swapchain_extent.height);
  depth_info.format = vk.device.depth_format;
  depth_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  depth_info.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
  depth_info.samples = VK_SAMPLE_COUNT_4_BIT;
  vk.swapchain.depth_attachment = vk_image_create(depth_info);
}

intern void vk_swapchain_query() {
  // vk.device.swapchain_support = vk_device_query_swapchain_support(vk.device.physical_device);
  // vk.device.depth_format = vk_device_detect_depth_format();
  vk.current_frame_idx = 0;
  // Choose a swap surface format
  b32 found = false;
  Loop (i, vk.device.swapchain_support.format_count) {
    VkSurfaceFormatKHR format = vk.device.swapchain_support.formats[i];
    // Preferred formats
    if (format.format == VK_FORMAT_B8G8R8A8_UNORM && // darker
    // if (format.format == VK_FORMAT_B8G8R8A8_SRGB && // brighter TODO: use
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
    {
      vk.swapchain.image_format = format;
      found = true;
      break;
    }
  }
  if (!found) {
    vk.swapchain.image_format = vk.device.swapchain_support.formats[0];
  }
  // Choose present mode
  VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
  Loop (i, vk.device.swapchain_support.present_mode_count) {
    VkPresentModeKHR mode = vk.device.swapchain_support.present_modes[i];
    if (mode == VK_PRESENT_MODE_FIFO_KHR) {
      present_mode = mode;
      break;
    }
  }
  vk.swapchain.present_mode = present_mode;
  // VkExtent2D swapchain_extent = {vk.width, vk.height};
  // if (vk.device.swapchain_support.capabilities.currentExtent.width != U32_MAX) {
    // swapchain_extent = vk.device.swapchain_support.capabilities.currentExtent;
  // }
  // VkExtent2D min = vk.device.swapchain_support.capabilities.minImageExtent;
  // VkExtent2D max = vk.device.swapchain_support.capabilities.maxImageExtent;
  // swapchain_extent.width = Clamp(min.width, swapchain_extent.width, max.width);
  // swapchain_extent.height = Clamp(min.height, swapchain_extent.height, max.height);
}

intern void vk_swapchain_destroy(VK_Swapchain swapchain) {
  VK_CHECK(vk.DeviceWaitIdle(vkdevice));
  vk_image_destroy(swapchain.depth_attachment);
  Loop (i, vk.images_in_flight) {
    vk.DestroyImageView(vkdevice, swapchain.images[i].view, vk.allocator);
  }
  vk.DestroySwapchainKHR(vkdevice, swapchain.handle, vk.allocator);
}

intern void vk_swapchain_recreate() {
  VK_Swapchain old = vk.swapchain;
  vk_swapchain_create(true);
  vk_swapchain_destroy(old);
  Info("Swapchain recreated x: %i y: %i", vk.width, vk.height);
}

intern u32 vk_swapchain_acquire_next_image_index(VkSemaphore image_available_semaphore) {
  u32 image_index;
#if GFX_X11 // NOTE: on x11 errors
  VkResult res = vk.AcquireNextImageKHR(vkdevice, vk.swapchain.handle, U64_MAX, image_available_semaphore, null, &image_index);
  if (res != VK_SUCCESS) {
    // Warn("%s", vk_result_string(res));
  }
#else
  VK_CHECK(vk.AcquireNextImageKHR(vkdevice, vk.swapchain.handle, U64_MAX, image_available_semaphore, null, &image_index));
#endif
  return image_index;
}

intern void vk_swapchain_present(VkSemaphore render_complete_semaphore, u32 present_image_index) {
  VkPresentInfoKHR present_info = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &render_complete_semaphore,
    .swapchainCount = 1,
    .pSwapchains = &vk.swapchain.handle,
    .pImageIndices = &present_image_index,
  };
#if GFX_X11 // NOTE: on x11 errors
  VkResult res = vk.QueuePresentKHR(vk.device.graphics_queue, &present_info);
  NotUsed(res);
  // if (res != VK_SUCCESS) {
  //   Error("%s", vk_result_string(res));
  // }
#else
  VK_CHECK(vk.QueuePresentKHR(vk.device.graphics_queue, &present_info));
#endif
}

////////////////////////////////////////////////////////////////////////
// @Mesh

Handle<GpuMesh> vk_mesh_load(Mesh mesh) {
  VK_Buffer& vert_buff = vk.vert_buffer;
  u64 vert_size = mesh.vert_count*sizeof(Vertex);
  u64 vert_offset = vert_buff.size;
  vert_buff.size += vert_size;
  Range vert_range = { .offset = vert_offset, .size = vert_size };
  vk_buffer_upload_to_gpu(vk.vert_buffer, vert_range, mesh.vertices);

  VK_Buffer& index_buff = vk.index_buffer;
  u64 index_size = mesh.index_count*sizeof(u32);
  u64 index_offset = index_buff.size;
  index_buff.size += index_size;
  Range index_range = { .offset = index_offset, .size = index_size };
  if (mesh.indices) {
    vk_buffer_upload_to_gpu(vk.index_buffer, index_range, mesh.indices);
  }

  VK_Mesh vk_mesh = {
    .vert_count = mesh.vert_count,
    .vert_offset = vert_range.offset,
    .index_count = mesh.index_count,
    .index_offset = index_range.offset,
  };
  u32 id = vk.meshes.count;
  vk.meshes.add(vk_mesh);
  Handle<GpuMesh> handle = {id};
  return handle;
}

////////////////////////////////////////////////////////////////////////
// @Drawing

void vk_draw() {
  GlobalStateGPU& shader_st = *vk.gpu_global_shader_st;
  shader_st.projection_view = vk.projection * vk.view;
  shader_st.projection = vk.projection;
  shader_st.view = vk.view;

  VkCommandBuffer cmd = vk_get_current_cmd();
  vk.CmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.pipeline_layout, 0, 1, &vk.descriptor_sets, 0, null);

  VkDeviceSize size = 0;
  vk.CmdBindVertexBuffers(cmd, 0, 1, &vk.vert_buffer.handle, &size);
  vk.CmdBindIndexBuffer(cmd, vk.index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);
  u32 draw_call_count = 0;
  u32 draw_call_mem_offset = 0;
  u32 entities_draw_count = 0;
  u32 entities_draw_id_offset = 0;

  // for (VK_Shader& shader : vk.shaders) {
  //   vk.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.pipeline);

  //   // Indexed Entities per shader
  //   u32 per_shader_indexed_draw_count = 0;
  //   for (MeshBatch mesh_batch : shader.batch_indexed.mesh_batches) {
  //     if (mesh_batch.entities.count == 0) continue;
  //     for (Handle<Entity> entity_handle : mesh_batch.entities) {
  //       u32 entity_idx = entity_handle.idx();
  //       vk.gpu_entities[entity_idx].model = mat4_transform(entities_transforms(entity_handle));
  //       vk.gpu_entities_indexes[entities_draw_count++] = entity_idx;
  //     }
  //     u32 mesh_idx = mesh_batch.mesh_handle.handle;
  //     VK_Mesh mesh = vk.meshes[mesh_idx];
  //     VK_DrawCallInfo info = {
  //       .index_draw_command = {
  //         .indexCount = (u32)mesh.index_count,
  //         .instanceCount = mesh_batch.entities.count,
  //         .firstIndex = (u32)(mesh.index_offset/sizeof(u32)),
  //         .vertexOffset = (i32)(mesh.vert_offset/sizeof(Vertex)),
  //         .firstInstance = 0,
  //       },
  //       .entities_offset_id = entities_draw_id_offset,
  //     };
  //     vk.gpu_draw_call_infos[draw_call_count++] = info;
  //     entities_draw_id_offset += mesh_batch.entities.count;
  //     ++per_shader_indexed_draw_count;
  //   }
  //   if (per_shader_indexed_draw_count > 0) {
  //     PushConstant push = {.drawcall_offset = draw_call_mem_offset / (u32)sizeof(VK_DrawCallInfo)};
  //     vk.CmdPushConstants(cmd, vk.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);
  //     vk.CmdDrawIndexedIndirect(cmd, vk.indirect_draw_buffer.handle, draw_call_mem_offset, per_shader_indexed_draw_count, sizeof(VK_DrawCallInfo));
  //   }

  //   // Entities per shader
  //   u32 per_shader_draw_count = 0;
  //   for (MeshBatch mesh_batch : shader.batch.mesh_batches) {
  //     if (mesh_batch.entities.count == 0) continue;
  //     for (Handle<Entity> entity_handle : mesh_batch.entities) {
  //       u32 entity_idx = entity_handle.idx();
  //       vk.gpu_entities[entity_idx].model = mat4_transform(entities_transforms(entity_handle));
  //       vk.gpu_entities_indexes[entities_draw_count++] = entity_idx;
  //     }
  //     u32 mesh_idx = mesh_batch.mesh_handle.handle;
  //     VK_Mesh mesh = vk.meshes[mesh_idx];
  //     VK_DrawCallInfo info = {
  //       .draw_command = {
  //         .vertexCount = (u32)mesh.vert_count,
  //         .instanceCount = mesh_batch.entities.count,
  //         .firstVertex = (u32)(mesh.vert_offset/sizeof(Vertex)),
  //         .firstInstance = 0,
  //       },
  //       .entities_offset_id = entities_draw_id_offset,
  //     };
  //     vk.gpu_draw_call_infos[draw_call_count++] = info;
  //     entities_draw_id_offset += mesh_batch.entities.count;
  //     ++per_shader_draw_count;
  //   }
  //   if (per_shader_draw_count > 0) {
  //     u32 draw_call_indexed_mem_offset = draw_call_mem_offset + (u32)sizeof(VK_DrawCallInfo)*per_shader_indexed_draw_count;
  //     PushConstant push = {.drawcall_offset = draw_call_indexed_mem_offset / (u32)sizeof(VK_DrawCallInfo)};
  //     vk.CmdPushConstants(cmd, vk.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);
  //     vk.CmdDrawIndirect(cmd, vk.indirect_draw_buffer.handle, draw_call_indexed_mem_offset , per_shader_draw_count, sizeof(VK_DrawCallInfo));
  //   }
  //   draw_call_mem_offset = draw_call_count * sizeof(VK_DrawCallInfo);

  //   /////////////////////////////////
  //   // Rebuilding static buffer
  //   if ((shader.static_entities_count_old != shader.static_entities_count) || (shader.static_entities_indexed_count_old != shader.static_entities_indexed_count)) {
  //     shader.static_entities_indexed_count_old = shader.static_entities_indexed_count;
  //     shader.static_entities_count_old = shader.static_entities_count;

  //     u32 static_draw_call_count = 0;
  //     u32 static_entities_draw_count = 0;
  //     u32 static_entities_draw_id_offset = 0;

  //     // Indexed Entities per shader
  //     u32 per_shader_static_indexed_draw_count = 0;
  //     for (MeshBatch mesh_batch : shader.static_batch_indexed.mesh_batches) {
  //       if (mesh_batch.entities.count == 0) continue;
  //       for (Handle<StaticEntity> entity_handle : mesh_batch.entities) {
  //         u32 entity_idx = entity_handle.idx();
  //         vk.gpu_entities[MaxEntities+entity_idx].model = mat4_transform(static_entities_transforms(entity_handle));
  //         vk.gpu_entities_indexes[MaxEntities+static_entities_draw_count++] = MaxEntities+entity_idx;
  //       }
  //       u32 mesh_idx = mesh_batch.mesh_handle.handle;
  //       VK_Mesh mesh = vk.meshes[mesh_idx];
  //       VK_DrawCallInfo info = {
  //         .index_draw_command = {
  //           .indexCount = (u32)mesh.index_count,
  //           .instanceCount = mesh_batch.entities.count,
  //           .firstIndex = (u32)(mesh.index_offset/sizeof(u32)),
  //           .vertexOffset = (i32)(mesh.vert_offset/sizeof(Vertex)),
  //           .firstInstance = 0,
  //         },
  //         .entities_offset_id = MaxEntities+static_entities_draw_id_offset,
  //       };
  //       vk.gpu_draw_call_infos[MaxDrawCalls+static_draw_call_count++] = info;
  //       static_entities_draw_id_offset += mesh_batch.entities.count;
  //       ++per_shader_static_indexed_draw_count;
  //     }
  //     vk.static_draw_indexed_offset = MaxDrawCalls*sizeof(VK_DrawCallInfo);
  //     vk.static_draw_offset = vk.static_draw_indexed_offset + per_shader_static_indexed_draw_count*sizeof(VK_DrawCallInfo);

  //     // Entities per shader
  //     u32 per_shader_static_draw_count = 0;
  //     for (MeshBatch mesh_batch : shader.static_batch.mesh_batches) {
  //       if (mesh_batch.entities.count == 0) continue;
  //       for (Handle<StaticEntity> entity_handle : mesh_batch.entities) {
  //         u32 entity_idx = entity_handle.idx();
  //         vk.gpu_entities[MaxEntities+entity_idx].model = mat4_transform(static_entities_transforms(entity_handle));
  //         vk.gpu_entities_indexes[MaxEntities+static_draw_call_count++] = MaxEntities+entity_idx;
  //       }
  //       u32 mesh_idx = mesh_batch.mesh_handle.handle;
  //       VK_Mesh mesh = vk.meshes[mesh_idx];
  //       VK_DrawCallInfo info = {
  //         .draw_command = {
  //           .vertexCount = (u32)mesh.vert_count,
  //           .instanceCount = mesh_batch.entities.count,
  //           .firstVertex = (u32)(mesh.vert_offset/sizeof(Vertex)),
  //           .firstInstance = 0,
  //         },
  //         .entities_offset_id = MaxEntities+static_entities_draw_id_offset,
  //       };
  //       vk.gpu_draw_call_infos[MaxDrawCalls+static_draw_call_count++] = info;
  //       static_entities_draw_id_offset += mesh_batch.entities.count;
  //       ++per_shader_static_draw_count;
  //     }

  //     vk.static_indexed_draw_count = per_shader_static_indexed_draw_count;
  //     vk.static_draw_count = per_shader_static_draw_count;
  //   }
  //   if (shader.static_entities_indexed_count) {
  //     PushConstant push = {MaxDrawCalls};
  //     vk.CmdPushConstants(cmd, vk.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);
  //     vk.CmdDrawIndexedIndirect(cmd, vk.indirect_draw_buffer.handle, vk.static_draw_indexed_offset, vk.static_indexed_draw_count, sizeof(VK_DrawCallInfo));
  //   }
  //   if (shader.static_entities_count) {
  //     PushConstant push = {MaxDrawCalls+vk.static_indexed_draw_count};
  //     vk.CmdPushConstants(cmd, vk.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);
  //     vk.CmdDrawIndirect(cmd, vk.indirect_draw_buffer.handle, vk.static_draw_offset, vk.static_draw_count, sizeof(VK_DrawCallInfo));
  //   }
  // }

  Loop (i, vk.pipelines.count) {
    vk.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.pipelines[i]);
    RenderBatch& batch = vk.batches[i];

    // Indexed Entities per shader
    u32 per_shader_indexed_draw_count = 0;
    for (MeshBatch mesh_batch : batch.batch_indexed.mesh_batches) {
      if (mesh_batch.entities.count == 0) continue;
      for (Handle<Entity> entity_handle : mesh_batch.entities) {
        u32 entity_idx = entity_handle.idx();
        vk.gpu_entities[entity_idx].model = mat4_transform(entities_transforms(entity_handle));
        vk.gpu_entities_indexes[entities_draw_count++] = entity_idx;
      }
      u32 mesh_idx = mesh_batch.mesh_handle.handle;
      VK_Mesh mesh = vk.meshes[mesh_idx];
      VK_DrawCallInfo info = {
        .index_draw_command = {
          .indexCount = (u32)mesh.index_count,
          .instanceCount = mesh_batch.entities.count,
          .firstIndex = (u32)(mesh.index_offset/sizeof(u32)),
          .vertexOffset = (i32)(mesh.vert_offset/sizeof(Vertex)),
          .firstInstance = 0,
        },
        .entities_offset_id = entities_draw_id_offset,
      };
      vk.gpu_draw_call_infos[draw_call_count++] = info;
      entities_draw_id_offset += mesh_batch.entities.count;
      ++per_shader_indexed_draw_count;
    }
    if (per_shader_indexed_draw_count > 0) {
      PushConstant push = {.drawcall_offset = draw_call_mem_offset / (u32)sizeof(VK_DrawCallInfo)};
      vk.CmdPushConstants(cmd, vk.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);
      vk.CmdDrawIndexedIndirect(cmd, vk.indirect_draw_buffer.handle, draw_call_mem_offset, per_shader_indexed_draw_count, sizeof(VK_DrawCallInfo));
    }

    // Entities per shader
    u32 per_shader_draw_count = 0;
    for (MeshBatch mesh_batch : batch.batch.mesh_batches) {
      if (mesh_batch.entities.count == 0) continue;
      for (Handle<Entity> entity_handle : mesh_batch.entities) {
        u32 entity_idx = entity_handle.idx();
        vk.gpu_entities[entity_idx].model = mat4_transform(entities_transforms(entity_handle));
        vk.gpu_entities_indexes[entities_draw_count++] = entity_idx;
      }
      u32 mesh_idx = mesh_batch.mesh_handle.handle;
      VK_Mesh mesh = vk.meshes[mesh_idx];
      VK_DrawCallInfo info = {
        .draw_command = {
          .vertexCount = (u32)mesh.vert_count,
          .instanceCount = mesh_batch.entities.count,
          .firstVertex = (u32)(mesh.vert_offset/sizeof(Vertex)),
          .firstInstance = 0,
        },
        .entities_offset_id = entities_draw_id_offset,
      };
      vk.gpu_draw_call_infos[draw_call_count++] = info;
      entities_draw_id_offset += mesh_batch.entities.count;
      ++per_shader_draw_count;
    }
    if (per_shader_draw_count > 0) {
      u32 draw_call_indexed_mem_offset = draw_call_mem_offset + (u32)sizeof(VK_DrawCallInfo)*per_shader_indexed_draw_count;
      PushConstant push = {.drawcall_offset = draw_call_indexed_mem_offset / (u32)sizeof(VK_DrawCallInfo)};
      vk.CmdPushConstants(cmd, vk.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);
      vk.CmdDrawIndirect(cmd, vk.indirect_draw_buffer.handle, draw_call_indexed_mem_offset , per_shader_draw_count, sizeof(VK_DrawCallInfo));
    }
    draw_call_mem_offset = draw_call_count * sizeof(VK_DrawCallInfo);

    /////////////////////////////////
    // Rebuilding static buffer
    if ((batch.static_entities_count_old != batch.static_entities_count) || (batch.static_entities_indexed_count_old != batch.static_entities_indexed_count)) {
      batch.static_entities_indexed_count_old = batch.static_entities_indexed_count;
      batch.static_entities_count_old = batch.static_entities_count;

      u32 static_draw_call_count = 0;
      u32 static_entities_draw_count = 0;
      u32 static_entities_draw_id_offset = 0;

      // Indexed Entities per shader
      u32 per_shader_static_indexed_draw_count = 0;
      for (MeshBatch mesh_batch : batch.static_batch_indexed.mesh_batches) {
        if (mesh_batch.entities.count == 0) continue;
        for (Handle<StaticEntity> entity_handle : mesh_batch.entities) {
          u32 entity_idx = entity_handle.idx();
          vk.gpu_entities[MaxEntities+entity_idx].model = mat4_transform(static_entities_transforms(entity_handle));
          vk.gpu_entities_indexes[MaxEntities+static_entities_draw_count++] = MaxEntities+entity_idx;
        }
        u32 mesh_idx = mesh_batch.mesh_handle.handle;
        VK_Mesh mesh = vk.meshes[mesh_idx];
        VK_DrawCallInfo info = {
          .index_draw_command = {
            .indexCount = (u32)mesh.index_count,
            .instanceCount = mesh_batch.entities.count,
            .firstIndex = (u32)(mesh.index_offset/sizeof(u32)),
            .vertexOffset = (i32)(mesh.vert_offset/sizeof(Vertex)),
            .firstInstance = 0,
          },
          .entities_offset_id = MaxEntities+static_entities_draw_id_offset,
        };
        vk.gpu_draw_call_infos[MaxDrawCalls+static_draw_call_count++] = info;
        static_entities_draw_id_offset += mesh_batch.entities.count;
        ++per_shader_static_indexed_draw_count;
      }
      vk.static_draw_indexed_offset = MaxDrawCalls*sizeof(VK_DrawCallInfo);
      vk.static_draw_offset = vk.static_draw_indexed_offset + per_shader_static_indexed_draw_count*sizeof(VK_DrawCallInfo);

      // Entities per shader
      u32 per_shader_static_draw_count = 0;
      for (MeshBatch mesh_batch : batch.static_batch.mesh_batches) {
        if (mesh_batch.entities.count == 0) continue;
        for (Handle<StaticEntity> entity_handle : mesh_batch.entities) {
          u32 entity_idx = entity_handle.idx();
          vk.gpu_entities[MaxEntities+entity_idx].model = mat4_transform(static_entities_transforms(entity_handle));
          vk.gpu_entities_indexes[MaxEntities+static_draw_call_count++] = MaxEntities+entity_idx;
        }
        u32 mesh_idx = mesh_batch.mesh_handle.handle;
        VK_Mesh mesh = vk.meshes[mesh_idx];
        VK_DrawCallInfo info = {
          .draw_command = {
            .vertexCount = (u32)mesh.vert_count,
            .instanceCount = mesh_batch.entities.count,
            .firstVertex = (u32)(mesh.vert_offset/sizeof(Vertex)),
            .firstInstance = 0,
          },
          .entities_offset_id = MaxEntities+static_entities_draw_id_offset,
        };
        vk.gpu_draw_call_infos[MaxDrawCalls+static_draw_call_count++] = info;
        static_entities_draw_id_offset += mesh_batch.entities.count;
        ++per_shader_static_draw_count;
      }

      vk.static_indexed_draw_count = per_shader_static_indexed_draw_count;
      vk.static_draw_count = per_shader_static_draw_count;
    }
    if (batch.static_entities_indexed_count) {
      PushConstant push = {MaxDrawCalls};
      vk.CmdPushConstants(cmd, vk.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);
      vk.CmdDrawIndexedIndirect(cmd, vk.indirect_draw_buffer.handle, vk.static_draw_indexed_offset, vk.static_indexed_draw_count, sizeof(VK_DrawCallInfo));
    }
    if (batch.static_entities_count) {
      PushConstant push = {MaxDrawCalls+vk.static_indexed_draw_count};
      vk.CmdPushConstants(cmd, vk.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);
      vk.CmdDrawIndirect(cmd, vk.indirect_draw_buffer.handle, vk.static_draw_offset, vk.static_draw_count, sizeof(VK_DrawCallInfo));
    }
  }

  // Debug drawing
  if (vk.immediate_draw_lines.count > 0) {
    vk.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.debug_line_shader);
    vk.CmdBindVertexBuffers(cmd, 0, 1, &vk.vert_buffer.handle, (VkDeviceSize*)&vk.immediate_draw_lines_offset);
    vk.CmdDraw(cmd, vk.immediate_draw_lines.count*2, 1, 0, 0);
    vk.immediate_draw_lines.clear();
  }

  // Cube map
  vk.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.cubemap_shader);
  Handle<GpuMesh> h = mesh_get(Mesh_Cube);
  VK_Mesh mesh = vk.meshes[h.handle];
  vk.CmdBindVertexBuffers(cmd, 0, 1, &vk.vert_buffer.handle, &mesh.vert_offset);
  if (mesh.index_count) {
    vk.CmdBindIndexBuffer(cmd, vk.index_buffer.handle, mesh.index_offset, VK_INDEX_TYPE_UINT32);
    vk.CmdDrawIndexed(cmd, mesh.index_count, 1, 0, 0, 0);
  } else {
    vk.CmdDraw(cmd, mesh.vert_count, 1, 0, 0);
  }

  vk.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.ui_shader);
  if (vk.immediate_draw_squares.count > 0) {
    vk.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.ui_shader);
    vk.CmdBindVertexBuffers(cmd, 0, 1, &vk.vert_buffer.handle, (VkDeviceSize*)&vk.immediate_draw_squares_offset);
    vk.CmdDraw(cmd, vk.immediate_draw_squares.count*6, 1, 0, 0);
    vk.immediate_draw_squares.clear();
  }

}

void vk_draw_screen() {
  VkCommandBuffer cmd = vk_get_current_cmd();
  PushConstant push = {vk.current_image_idx};
  vk.CmdPushConstants(cmd, vk.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);
  vk.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.screen_shader);
  vk.CmdDraw(cmd, 3, 1, 0, 0);
}

void vk_loader_load_core() {
  vk.lib = os_lib_open("libvulkan.so");
#define X(name) VK_GET_PROC(name)
  VK_GET_PROC_LIST
#undef X
}

void vk_loader_load_instance() {
#define X(name) VK_INSTANCE_GET_PROC(name)
  VK_INSTANCE_GET_PROC_LIST
#undef X
}

void vk_loader_load_device() {
#define X(name) VK_DEVICE_GET_PROC(name)
  VK_DEVICE_GET_PROC_LIST
#undef X
}

intern void vk_instance_create() {
  Scratch scratch;
  VkApplicationInfo app_info = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .apiVersion = VK_API_VERSION_1_4
  };
  
  Array<const char*, 1> required_validation_layer_names = {};
  Array<const char*, 3> required_extensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_SURFACE_NAME };

#if BUILD_DEBUG

  // Validation layer
  required_validation_layer_names.add("VK_LAYER_KHRONOS_validation");
  Debug("%Required layers:");
  for (const char* x : required_validation_layer_names) {
    Debug(x);
  }
  u32 available_layer_count = 0;
  VK_CHECK(vk.EnumerateInstanceLayerProperties(&available_layer_count, null));
  VkLayerProperties* available_layers = push_array(scratch, VkLayerProperties, available_layer_count);
  VK_CHECK(vk.EnumerateInstanceLayerProperties(&available_layer_count, available_layers));
  Loop (i, required_validation_layer_names.count) {
    b32 found = false;
    Loop (j, available_layer_count) {
      if (str_match(required_validation_layer_names[i], available_layers[j].layerName)) {
        found = true;
        Info("Validation layer %s found", String(required_validation_layer_names[i]));
        break;
      }
    }
    AssertMsg(found, "Required validation layer is missing: %s", String(required_validation_layer_names[i]));
  }

  // Extensions
  required_extensions.add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  Debug("Required extensions:");
  for (const char* x : required_extensions) {
    Debug(x);
  }
  u32 extension_count = 0;
  vk.EnumerateInstanceExtensionProperties(null, &extension_count, null);
  VkExtensionProperties* props = push_array(scratch, VkExtensionProperties, extension_count);
  vk.EnumerateInstanceExtensionProperties(null, &extension_count, props);
  Loop (i, required_extensions.count) {
    b32 found = false;
    Loop (j, extension_count) {
      if (str_match(required_extensions[i], props[j].extensionName)) {
        found = true;
        Info("extension %s found", String(required_extensions[i]));
        break;
      }
    }
    AssertMsg(found, "Required extension is missing: %s", String(required_extensions[i]));
  }
#endif

  VkInstanceCreateInfo instance_create_info = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &app_info,
    .enabledLayerCount = (u32)required_validation_layer_names.count,
    .ppEnabledLayerNames = required_validation_layer_names.data,
    .enabledExtensionCount = (u32)required_extensions.count,
    .ppEnabledExtensionNames = required_extensions.data,
  };
  
  VK_CHECK(vk.CreateInstance(&instance_create_info, vk.allocator, &vk.instance));
  Info("Vulkan insance created");

#if BUILD_DEBUG
  VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
    .pfnUserCallback = [](
      VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      VkDebugUtilsMessageTypeFlagsEXT message_types,
      const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
      void* user_data) -> VkBool32 
    {
      switch (message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
          // Error(String(callback_data->pMessage))
          // NOTE: For some reason Scratch arena is invalid here?
          ErrorArena({}, "%s", String(callback_data->pMessage));
        } break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
          Warn(String(callback_data->pMessage));
        } break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
          Info(String(callback_data->pMessage));
        } break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
          Trace(String(callback_data->pMessage));
        } break;
        default: break;
      }
      return false;
    },
  };

  PFN_vkCreateDebugUtilsMessengerEXT func = null;
  Assign(func, vk.GetInstanceProcAddr(vk.instance, "vkCreateDebugUtilsMessengerEXT"));
  AssertMsg(func, "Failed to create debug messenger");

  VK_CHECK(func(vk.instance, &debug_create_info, vk.allocator, &vk.debug_messenger));
  Debug("Vulkan debugger created");
#endif
}

void vk_init() {
  Scratch scratch;
  vk.arena = arena_init();
  
#if VulkanUseAllocator
  vk._allocator = vk_allocator_create();
  vk.allocator = &vk._allocator;
#endif

  vk_loader_load_core();
  vk_instance_create();
  vk_loader_load_instance();
  vk_surface_create();
  vk_device_create();
  vk_loader_load_device();

  {
    vk.GetDeviceQueue(vkdevice, vk.device.graphics_queue_index, 0, &vk.device.graphics_queue);
    vk.GetDeviceQueue(vkdevice, vk.device.present_queue_index, 0, &vk.device.present_queue);
    vk.GetDeviceQueue(vkdevice, vk.device.transfer_queue_index, 0, &vk.device.transfer_queue);
    vk.GetDeviceQueue(vkdevice, vk.device.compute_queue_index, 0, &vk.device.compute_queue);
    Info("Queues obtained");
    
    VkCommandPoolCreateInfo graphics_pool_create_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = vk.device.graphics_queue_index,
    };
    VK_CHECK(vk.CreateCommandPool(vkdevice, &graphics_pool_create_info, vk.allocator, &vk.device.cmd_pool));
    Info("Graphics command pool created");

    v2u win_size = os_get_window_size();
    vk.width = win_size.x;
    vk.height = win_size.y;

    vk.images_in_flight = vk.device.swapchain_support.capabilities.minImageCount;
    vk.frames_in_flight = vk.images_in_flight - 1;
    vk_swapchain_query();
    vk_swapchain_create(false);
    Info("Swapchain created");
  
    vk.cmds = push_array(vk.arena, VkCommandBuffer, vk.frames_in_flight);
    // vk.compute_cmds = push_array(vk.arena, VkCommandBuffer, vk.frames_in_flight);
    Loop (i, vk.frames_in_flight) {
      vk.cmds[i] = vk_cmd_alloc(vk.device.cmd_pool);
      // vk.compute_cmds[i] = vk_cmd_alloc(vk.device.cmd_pool);
    }
    vk.upload_cmd = vk_cmd_alloc(vk.device.cmd_pool);
    Info("Command buffers created");
  }

  // Sync
  {
    // NOTE: for some reasons validation layer complains about render_complete_sempahores when their number is frames_in_flight but doesn't when is images_in_flight
    vk.sync.render_complete_semaphores = push_array(vk.arena, VkSemaphore, vk.images_in_flight);
    // vk.sync.compute_complete_semaphores = push_array(vk.arena, VkSemaphore, vk.images_in_flight);
    Loop (i, vk.images_in_flight) {
      VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
      vk.CreateSemaphore(vkdevice, &semaphore_create_info, vk.allocator, &vk.sync.render_complete_semaphores[i]);
      // vk.CreateSemaphore(vkdevice, &semaphore_create_info, vk.allocator, &vk.sync.compute_complete_semaphores[i]);
    }
    vk.sync.image_available_semaphores = push_array(vk.arena, VkSemaphore, vk.images_in_flight);
    vk.sync.in_flight_fences = push_array(vk.arena, VkFence, vk.frames_in_flight);
    Loop (i, vk.frames_in_flight) {
      VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
      vk.CreateSemaphore(vkdevice, &semaphore_create_info, vk.allocator, &vk.sync.image_available_semaphores[i]);
      VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
      };
      VK_CHECK(vk.CreateFence(vkdevice, &fence_create_info, vk.allocator, &vk.sync.in_flight_fences[i]));
    }
  }
  
  // Buffers
  {
    vk.vert_buffer = vk_buffer_create(MB(1), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vk.index_buffer = vk_buffer_create(MB(1), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vk.stage_buffer = vk_buffer_create(MB(10), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vk_buffer_map_memory(vk.stage_buffer, 0, vk.stage_buffer.cap);
    vk.storage_buffer = vk_buffer_create(MB(200), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vk_buffer_map_memory(vk.storage_buffer, 0, vk.storage_buffer.cap);
    vk.indirect_draw_buffer = vk_buffer_create(MB(100), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vk_buffer_map_memory(vk.indirect_draw_buffer, 0, vk.indirect_draw_buffer.cap);
  }

  vk_descriptor_init();
  vk_texture_init();
  vk_shader_pipeline_init();

  // Load basic shaders
  ///////////////////////////////////
  // Debug lines
  {
    Shader shader = {
      .name = "line",
      .state = {
        .topology = ShaderTopology_Line,
        .is_transparent = true,
        .use_depth = true,
      },
    };
    vk.debug_line_shader = vk_shader_pipeline_create(shader);
  }
  // Screen drawing
  {
    Shader shader = {
      .name = "screen",
      .state = {
        .topology = ShaderTopology_Triangle,
        .samples = 1,
      },
    };
    vk.screen_shader = vk_shader_pipeline_create(shader);
  }
  {
    Shader shader = {
      .name = "cubemap",
      .state = {
        .type = ShaderType_Cube,
        .topology = ShaderTopology_Triangle,
        .is_transparent = false,
        .use_depth = true,
      },
    };
    vk.cubemap_shader = vk_shader_pipeline_create(shader);
  }
  {
    Shader shader = {
      .name = "ui",
      .state = {
        .type = ShaderType_Drawing,
        .topology = ShaderTopology_Triangle,
        .is_transparent = false,
        .use_depth = false,
      },
    };
    vk.ui_shader = vk_shader_pipeline_create(shader);
  }

  vk.scale = 1;
  // Texture target
  {
    vk.texture_targets = push_array_zero(vk.arena, VK_Image, vk.images_in_flight);
    vk.textures.count += vk.images_in_flight;
    vk_texture_resize_target();
  }

  vk.immediate_draw_lines_offset = vk.vert_buffer.size;
  vk.vert_buffer.size += sizeof(Vertex)*KB(1);

  vk.immediate_draw_squares_offset = vk.vert_buffer.size;
  vk.vert_buffer.size += sizeof(Vertex)*KB(1);

  Info("Vulkan renderer initialized");
}

void vk_shutdown() {
  VK_CHECK(vk.DeviceWaitIdle(vkdevice));
  
  Loop (i, vk.frames_in_flight) {
    vk.DestroySemaphore(vkdevice, vk.sync.image_available_semaphores[i], vk.allocator);
    vk.DestroySemaphore(vkdevice, vk.sync.render_complete_semaphores[i], vk.allocator);
    vk.DestroySemaphore(vkdevice, vk.sync.compute_complete_semaphores[i], vk.allocator);
    vk.DestroyFence(vkdevice, vk.sync.in_flight_fences[i], vk.allocator);
    vk_cmd_free(vk.device.cmd_pool, vk.cmds[i]);
  }

  {
    vk.DestroyBuffer(vkdevice, vk.vert_buffer.handle, vk.allocator);
    vk.DestroyBuffer(vkdevice, vk.index_buffer.handle, vk.allocator);
    vk.DestroyBuffer(vkdevice, vk.stage_buffer.handle, vk.allocator);
    vk.DestroyBuffer(vkdevice, vk.storage_buffer.handle, vk.allocator);
  }
  
  vk_swapchain_destroy(vk.swapchain);
  
  // Device
  {
    Debug("Destroying Vulkan device...");
    vk.DestroyCommandPool(vkdevice, vk.device.cmd_pool, vk.allocator);
    vk.DestroyCommandPool(vkdevice, vk.device.cmd_pool, vk.allocator);
    vk.DestroyDevice(vkdevice, vk.allocator);
  }
  
  Info("Releasing physical device resources...");
  
  Debug("Destroying Vulkan surface...");
  vk.DestroySurfaceKHR(vk.instance, vk.surface, vk.allocator);
  
#if DUILD_DEBUG
  Debug("Destroying Vulkan debugger...");
  PFN_vkDestroyDebugUtilsMessengerEXT func; Assign(func, vkGetInstanceProcAddr(vk.instance, "vkDestroyDebugUtilsMessengerEXT"));
  func(vk.instance, vk.debug_messenger, vk.allocator);
#endif

  Debug("Destroying Vulkan instance...");
  vk.DestroyInstance(vk.instance, vk.allocator);
}

void vk_begin_frame() {
  // for (i32 i = 0; i < vk.debug_lines_remain.count; ) {
  //   vk.debug_line_times[i] -= g_dt;
  //   if (vk.debug_line_times[i] <= 0) {
  //     vk.debug_lines_remain.swap_remove(i);
  //   } else {
  //     ++i;
  //   }
  // }

  if (vk.immediate_draw_lines.count > 0) {
    u32 size = vk.immediate_draw_lines.count * sizeof(VK_State::DebugDrawLine);
    void* data = vk.immediate_draw_lines.data;
    vk_buffer_upload_to_gpu(vk.vert_buffer, {vk.immediate_draw_lines_offset, size}, data);
  }

  if (vk.immediate_draw_squares.count > 0) {
    u32 size = vk.immediate_draw_squares.count * sizeof(VK_State::DebugDrawSquare);
    void* data = vk.immediate_draw_squares.data;
    vk_buffer_upload_to_gpu(vk.vert_buffer, {vk.immediate_draw_squares_offset, size}, data);
  }

  VK_CHECK(vk.WaitForFences(vkdevice, 1, &vk.sync.in_flight_fences[vk.current_frame_idx], true, U64_MAX));
  VK_CHECK(vk.ResetFences(vkdevice, 1, &vk.sync.in_flight_fences[vk.current_frame_idx]));

  v2u win_size = os_get_window_size();
  if (vk.width != win_size.x || vk.height != win_size.y) {
    vk.width = win_size.x;
    vk.height = win_size.y;
    VK_CHECK(vk.DeviceWaitIdle(vkdevice));
    vk_swapchain_recreate();
    vk_texture_resize_target();
  }

  vk.current_image_idx = vk_swapchain_acquire_next_image_index(vk_get_current_image_available_semaphore());

  VkCommandBuffer cmd = vk_get_current_cmd();
  vk_cmd_begin(cmd);
  
  // NOTE: we flip Y coordinate so Y:0 is on bottom of screen
  VkViewport viewport = {
    .x = 0.0f,
    .y = (f32)vk.height*vk.scale,
    .width = (f32)vk.width*vk.scale,
    .height = -(f32)vk.height*vk.scale,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  vk.CmdSetViewport(cmd, 0, 1, &viewport);
  VkRect2D scissor = {
    .offset = {.x = 0, .y = 0},
    .extent = {
      .width = (u32)(vk.width*vk.scale), 
      .height = (u32)(vk.height*vk.scale)
    },
  };
  vk.CmdSetScissor(cmd, 0, 1, &scissor);
}

void vk_end_frame() {
  VkCommandBuffer cmd = vk_get_current_cmd();
  vk_cmd_end(cmd);
  VkSemaphore semaphores_wait[] = {
    vk_get_current_image_available_semaphore(),
  };
  VkPipelineStageFlags sync_flags[] = {
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
  };
  VkSemaphore semaphore_render_complete = vk_get_current_render_complete_semaphore();
  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = ArrayCount(semaphores_wait),
    .pWaitSemaphores = semaphores_wait,
    .pWaitDstStageMask = sync_flags,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &semaphore_render_complete,
  };
  VK_CHECK(vk.QueueSubmit(vk.device.graphics_queue, 1, &submit_info, vk.sync.in_flight_fences[vk.current_frame_idx]));
  vk_swapchain_present(vk_get_current_render_complete_semaphore(), vk.current_image_idx);
  vk.current_frame_idx = (vk.current_frame_idx + 1) % vk.frames_in_flight;
}

intern VkRenderingAttachmentInfo vk_default_color_attachment_info(VkImageView view) {
  VkRenderingAttachmentInfo result = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView = view,
    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .clearValue = {
      .color = {0.1f, 0.1f, 0.1f, 1.0f},
    }
  };
  return result;
}

intern VkRenderingAttachmentInfo vk_default_depth_attachment_info(VkImageView view) {
  VkRenderingAttachmentInfo result = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView = view,
    .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .clearValue = { 
      .depthStencil = {1, 0} 
    }
  };
  return result;
}

intern VkRenderingInfo vk_default_rendering_info(VkRenderingAttachmentInfo* color_attachment, VkRenderingAttachmentInfo* depth_attachment = null) {
  VkRenderingInfo result = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
    .renderArea = {
      .offset = {.x = 0, .y = 0}, 
      .extent = {.width = vk.width, .height = vk.height}
    },
    .layerCount = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments = color_attachment,
    .pDepthAttachment = depth_attachment
  };
  return result;
}

void vk_begin_renderpass(RenderpassType renderpass_id) {
  VkCommandBuffer cmd = vk_get_current_cmd();
  switch (renderpass_id) {
    case RenderpassType_World: {
      vk_image_layout_transition(cmd, vk.msaa_texture_target, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      vk_image_layout_transition(cmd, vk.offscreen_depth_buffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
      vk_image_layout_transition(cmd, vk.texture_targets[vk.current_image_idx], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

      VkRenderingAttachmentInfo color_attachment = vk_default_color_attachment_info(vk.msaa_texture_target.view);
      color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
      color_attachment.resolveImageView = vk.texture_targets[vk.current_image_idx].view;
      color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      VkRenderingAttachmentInfo depth_attachment = vk_default_depth_attachment_info(vk.offscreen_depth_buffer.view);
      
      VkRenderingInfo render_info = vk_default_rendering_info(&color_attachment, &depth_attachment);
      render_info.renderArea.extent = {vk.texture_targets->info.width, vk.texture_targets->info.height};
      vk.CmdBeginRendering(cmd, &render_info);

      ///////////////////////////////////
      // vk_image_layout_transition(cmd, vk.msaa_texture_target, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      // vk_image_layout_transition(cmd, vk.swapchain.depth_attachment, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
      // vk_image_layout_transition(cmd, vk.swapchain.images[vk.current_image_idx], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

      // VkRenderingAttachmentInfo color_attachment = vk_default_color_attachment_info(vk.msaa_texture_target.view);
      // color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
      // color_attachment.resolveImageView = vk.swapchain.views[vk.current_image_idx];
      // color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

      // VkRenderingAttachmentInfo depth_attachment = vk_default_depth_attachment_info(vk.swapchain.depth_attachment.view);
      // // depth_attachment
      
      // VkRenderingInfo render_info = vk_default_rendering_info(&color_attachment, &depth_attachment);
      // vk.CmdBeginRendering(cmd, &render_info);

      ///////////////////////////////////
      // // OLD
      // // Color
      // vk_image_layout_transition(cmd, vk.swapchain.images[vk.current_image_idx], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      // // Depth
      // vk_image_layout_transition(cmd, vk.swapchain.depth_attachment, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
      // // Render
      // VkRenderingAttachmentInfo color_attachment = vk_default_color_attachment_info(vk.swapchain.views[vk.current_image_idx]);
      // VkRenderingAttachmentInfo depth_attachment = vk_default_depth_attachment_info(vk.swapchain.depth_attachment.view);
      // VkRenderingInfo render_info = vk_default_rendering_info(&color_attachment, &depth_attachment);
      // vk.CmdBeginRendering(cmd, &render_info);
    } break;
    case RenderpassType_UI: {
    } break;
    case RenderpassType_Screen: {
      vk_image_layout_transition(cmd, vk.swapchain.images[vk.current_image_idx], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      VkRenderingAttachmentInfo color_attachment = vk_default_color_attachment_info(vk.swapchain.images[vk.current_image_idx].view);
      VkRenderingInfo render_info = vk_default_rendering_info(&color_attachment);
      vk.CmdBeginRendering(cmd, &render_info);
    } break;
  }
  return;
}

void vk_end_renderpass(RenderpassType renderpass) {
  VkCommandBuffer cmd = vk_get_current_cmd();
  switch (renderpass) {
    case RenderpassType_World: {
      vk.CmdEndRendering(cmd);
      vk_image_layout_transition(cmd, vk.texture_targets[vk.current_image_idx], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      ///////////////////////////////////
      // vk_image_layout_transition(cmd, vk.swapchain.images[vk.current_image_idx], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    } break;
    case RenderpassType_UI: {
    } break;
    case RenderpassType_Screen: {
      vk.CmdEndRendering(cmd);
      vk_image_layout_transition(cmd, vk.swapchain.images[vk.current_image_idx], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    } break;
  }
  return;
}

void vk_begin_draw_frame() {
  imgui_begin_frame();
}

void vk_end_draw_frame() {
  vk_begin_frame();
  // World
  {
    vk_begin_renderpass(RenderpassType_World);
    vk_draw();
    vk_end_renderpass(RenderpassType_World);
  }
  // {
  //   vk_begin_renderpass(Renderpass_UI);
  //   ui_begin_frame();
  //   ui_end_frame();
  //   vk_end_renderpass(Renderpass_UI);
  // }
  // Screen
  {
    VkCommandBuffer cmd = vk_get_current_cmd();
    VkViewport viewport = {
      .x = 0.0f,
      .y = (f32)vk.height,
      .width = (f32)vk.width,
      .height = -(f32)vk.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
    };
    vk.CmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor = {
      .offset = {.x = 0, .y = 0},
      .extent = {
        .width = (u32)vk.width, 
        .height = (u32)vk.height
      },
    };
    vk.CmdSetScissor(cmd, 0, 1, &scissor);
    vk_begin_renderpass(RenderpassType_Screen);
    vk_draw_screen();
    imgui_end_frame();
    vk_end_renderpass(RenderpassType_Screen);
  }
  vk_end_frame();
}

////////////////////////////////////////////////////////////////////////
// Entity

// void vk_make_renderable(Handle<Entity> entity_handle, Handle<GpuMesh> mesh_handle, Handle<GpuShader> shader_handle, Handle<MaterialGPU> material_handle) {
//   u32 entity_idx = entity_handle.idx();
//   Assert(!vk.entities[entity_idx].is_init);
//   DebugDo(vk.entities[entity_idx].is_init = true);
//   u32 shader_idx = shader_handle.idx();
//   vk.entities[entity_idx].shader = shader_handle;
//   u32 mesh_idx = mesh_handle.idx();
//   VK_Mesh mesh = vk.meshes[mesh_idx];
//   // Indexed
//   if (mesh.index_count) {
//     ShaderBatch<Entity>& shader_batch = vk.shaders[shader_idx].batch_indexed;
//     u32* p_mesh_idx_in_array = shader_batch.mesh_to_batch.get(mesh_idx);
//     u32 mesh_idx_in_array;
//     if (!p_mesh_idx_in_array) {
//       mesh_idx_in_array = shader_batch.mesh_batches.count;
//       p_mesh_idx_in_array = &mesh_idx_in_array;
//       shader_batch.mesh_to_batch.add(mesh_idx, mesh_idx_in_array);
//       MeshBatch<Entity> mesh_batch = {.mesh_handle = mesh_handle};
//       shader_batch.mesh_batches.add(mesh_batch);
//     }
//     MeshBatch<Entity>& mesh_batch = shader_batch.mesh_batches[*p_mesh_idx_in_array];
//     u32 entity_idx_in_array = mesh_batch.entities.count;
//     mesh_batch.entities.add(entity_handle);
//     vk.entities[entity_idx].entity_idx_in_array = entity_idx_in_array;
//   }
//   // Not Indexed
//   else {
//     ShaderBatch<Entity>& shader_batch = vk.shaders[shader_idx].batch;
//     u32* p_mesh_idx_in_array = shader_batch.mesh_to_batch.get(mesh_idx);
//     u32 mesh_idx_in_array;
//     if (!p_mesh_idx_in_array) {
//       mesh_idx_in_array = shader_batch.mesh_batches.count;
//       p_mesh_idx_in_array = &mesh_idx_in_array;
//       shader_batch.mesh_to_batch.add(mesh_idx, mesh_idx_in_array);
//       MeshBatch<Entity> mesh_batch = {.mesh_handle = mesh_handle};
//       shader_batch.mesh_batches.add(mesh_batch);
//     }
//     MeshBatch<Entity>& mesh_batch = shader_batch.mesh_batches[*p_mesh_idx_in_array];
//     u32 entity_idx_in_array = mesh_batch.entities.count;
//     mesh_batch.entities.add(entity_handle);
//     vk.entities[entity_idx].entity_idx_in_array = entity_idx_in_array;
//   }
//   vk.entities[entity_idx].mesh = mesh_handle;
//   vk.gpu_entities[entity_idx].material = material_handle;
// }

void vk_make_renderable_(Handle<Entity> entity_handle, Handle<GpuMesh> mesh_handle, Handle<GpuMaterial> material_handle) {
  u32 entity_idx = entity_handle.idx();
  Assert(!vk.entities[entity_idx].is_init);
  DebugDo(vk.entities[entity_idx].is_init = true);
  u32 material_idx = material_handle.idx();
  u32 pipeline_idx = vk.materials[material_idx].pipeline_idx;
  u32 mesh_idx = mesh_handle.idx();
  vk.entities[entity_idx].pipeline = pipeline_idx;
  VK_Mesh mesh = vk.meshes[mesh_idx];
  // Indexed
  if (mesh.index_count) {
    ShaderBatch<Entity>& shader_batch = vk.batches[pipeline_idx].batch_indexed;
    u32* mesh_idx_in_array = shader_batch.mesh_to_batch.get(mesh_idx);
    if (!mesh_idx_in_array) {
      mesh_idx_in_array = shader_batch.mesh_to_batch.add(mesh_idx, shader_batch.mesh_batches.count);
      MeshBatch<Entity> mesh_batch = {.mesh_handle = mesh_handle};
      shader_batch.mesh_batches.add(mesh_batch);
    }
    MeshBatch<Entity>& mesh_batch = shader_batch.mesh_batches[*mesh_idx_in_array];
    u32 entity_idx_in_array = mesh_batch.entities.count;
    mesh_batch.entities.add(entity_handle);
    vk.entities[entity_idx].entity_idx_in_array = entity_idx_in_array;
  }
  // Not Indexed
  else {
    ShaderBatch<Entity>& shader_batch = vk.batches[pipeline_idx].batch;
    u32* mesh_idx_in_array = shader_batch.mesh_to_batch.get(mesh_idx);
    if (!mesh_idx_in_array) {
      mesh_idx_in_array = shader_batch.mesh_to_batch.add(mesh_idx, shader_batch.mesh_batches.count);
      MeshBatch<Entity> mesh_batch = {.mesh_handle = mesh_handle};
      shader_batch.mesh_batches.add(mesh_batch);
    }
    MeshBatch<Entity>& mesh_batch = shader_batch.mesh_batches[*mesh_idx_in_array];
    u32 entity_idx_in_array = mesh_batch.entities.count;
    mesh_batch.entities.add(entity_handle);
    vk.entities[entity_idx].entity_idx_in_array = entity_idx_in_array;
  }
  vk.entities[entity_idx].mesh = mesh_handle;
  vk.gpu_entities[entity_idx].material = material_handle.idx();
}

// void vk_make_renderable_static(Handle<StaticEntity> entity_handle, Handle<GpuMesh> mesh_handle, Handle<GpuMaterial> material_handle) {
  // u32 entity_idx = entity_handle.idx() + MaxEntities;
  // u32 shader_idx = shader_handle.handle;
  // u32 mesh_idx = mesh_handle.handle;
  // VK_Mesh mesh = vk.meshes[mesh_idx];
  // // Indexed
  // if (mesh.index_count) {
  //   ++vk.shaders[shader_idx].static_entities_indexed_count;
  //   ShaderBatch<StaticEntity>& shader_batch = vk.shaders[shader_idx].static_batch_indexed;
  //   u32* p_mesh_idx_in_array = shader_batch.mesh_to_batch.get(mesh_idx);
  //   u32 mesh_idx_in_array;
  //   if (!p_mesh_idx_in_array) {
  //     mesh_idx_in_array = shader_batch.mesh_batches.count;
  //     p_mesh_idx_in_array = &mesh_idx_in_array;
  //     shader_batch.mesh_to_batch.add(mesh_idx, mesh_idx_in_array);
  //     MeshBatch<StaticEntity> mesh_batch = {.mesh_handle = mesh_handle};
  //     shader_batch.mesh_batches.add(mesh_batch);
  //   }
  //   MeshBatch<StaticEntity>& mesh_batch = shader_batch.mesh_batches[*p_mesh_idx_in_array];
  //   mesh_batch.entities.add(entity_handle);
  //   u32 entity_idx_in_array = mesh_batch.entities.count;
  // }
  // // Not Indexed
  // else {
  //   ++vk.shaders[shader_idx].static_entities_count;
  //   ShaderBatch<StaticEntity>& shader_batch = vk.shaders[shader_idx].static_batch;
  //   u32* p_mesh_idx_in_array = shader_batch.mesh_to_batch.get(mesh_idx);
  //   u32 mesh_idx_in_array;
  //   if (!p_mesh_idx_in_array) {
  //     mesh_idx_in_array = shader_batch.mesh_batches.count;
  //     p_mesh_idx_in_array = &mesh_idx_in_array;
  //     shader_batch.mesh_to_batch.add(mesh_idx, mesh_idx_in_array);
  //     MeshBatch<StaticEntity> mesh_batch = {.mesh_handle = mesh_handle};
  //     shader_batch.mesh_batches.add(mesh_batch);
  //   }
  //   MeshBatch<StaticEntity>& mesh_batch = shader_batch.mesh_batches[*p_mesh_idx_in_array];
  //   mesh_batch.entities.add(entity_handle);
  //   u32 entity_idx_in_array = mesh_batch.entities.count;
  // }
  // // vk.entities[entity_idx].shader_handle = vk.shaders[shader_idx].entities.add(entity_handle);
  // vk.gpu_entities[entity_idx].material = material_handle.idx();
// }

void vk_make_renderable_static(Handle<StaticEntity> entity_handle, Handle<GpuMesh> mesh_handle, Handle<GpuMaterial> material_handle) {
  u32 entity_idx = entity_handle.idx() + MaxEntities;
  Assert(!vk.entities[entity_idx].is_init);
  DebugDo(vk.entities[entity_idx].is_init = true);
  u32 material_idx = material_handle.idx();
  u32 pipeline_idx = vk.materials[material_idx].pipeline_idx;
  u32 mesh_idx = mesh_handle.handle;
  VK_Mesh mesh = vk.meshes[mesh_idx];
  // Indexed
  if (mesh.index_count) {
    ++vk.batches[pipeline_idx].static_entities_indexed_count;
    ShaderBatch<StaticEntity>& shader_batch = vk.batches[pipeline_idx].static_batch_indexed;
    u32* mesh_idx_in_array = shader_batch.mesh_to_batch.get(mesh_idx);
    if (!mesh_idx_in_array) {
      mesh_idx_in_array = shader_batch.mesh_to_batch.add(mesh_idx, shader_batch.mesh_batches.count);
      MeshBatch<StaticEntity> mesh_batch = {.mesh_handle = mesh_handle};
      shader_batch.mesh_batches.add(mesh_batch);
    }
    MeshBatch<StaticEntity>& mesh_batch = shader_batch.mesh_batches[*mesh_idx_in_array];
    mesh_batch.entities.add(entity_handle);
    u32 entity_idx_in_array = mesh_batch.entities.count;
    NotUsed(entity_idx_in_array); // TODO: remove static entity
  }
  // Not Indexed
  else {
    ++vk.batches[pipeline_idx].static_entities_count;
    ShaderBatch<StaticEntity>& shader_batch = vk.batches[pipeline_idx].static_batch;
    u32* mesh_idx_in_array = shader_batch.mesh_to_batch.get(mesh_idx);
    if (!mesh_idx_in_array) {
      mesh_idx_in_array = shader_batch.mesh_to_batch.add(mesh_idx, shader_batch.mesh_batches.count);
      MeshBatch<StaticEntity> mesh_batch = {.mesh_handle = mesh_handle};
      shader_batch.mesh_batches.add(mesh_batch);
    }
    MeshBatch<StaticEntity>& mesh_batch = shader_batch.mesh_batches[*mesh_idx_in_array];
    mesh_batch.entities.add(entity_handle);
    u32 entity_idx_in_array = mesh_batch.entities.count;
    NotUsed(entity_idx_in_array); // TODO: remove static entity
  }
  // vk.entities[entity_idx].shader_handle = vk.shaders[shader_idx].entities.add(entity_handle);
  vk.gpu_entities[entity_idx].material = material_handle.idx();
}

void vk_remove_renderable(Handle<Entity> entity_handle) {
  u32 entity_idx = entity_handle.idx();
  Assert(vk.entities[entity_idx].is_init == true);
  DebugDo(vk.entities[entity_idx].is_init = false);
  u32 pipeline_idx = vk.entities[entity_idx].pipeline;
  u32 mesh_idx = vk.entities[entity_idx].mesh.handle;
  VK_Mesh mesh = vk.meshes[mesh_idx];
  if (mesh.index_count) {
    ShaderBatch<Entity>& shader_batch = vk.batches[pipeline_idx].batch_indexed;
    u32* mesh_batch_idx = shader_batch.mesh_to_batch.get(mesh_idx);
    MeshBatch<Entity>& mesh_batch = shader_batch.mesh_batches[*mesh_batch_idx];

    u32 idx = vk.entities[entity_idx].entity_idx_in_array;
    u32 last_idx = mesh_batch.entities.count-1;
    Handle<Entity> swapped = mesh_batch.entities[last_idx];
    u32 swapped_idx = swapped.idx();

    mesh_batch.entities[idx] = swapped;
    mesh_batch.entities.pop();
    vk.entities[swapped_idx].entity_idx_in_array = idx;
  } else {
    ShaderBatch<Entity>& shader_batch = vk.batches[pipeline_idx].batch;
    u32* mesh_batch_idx = shader_batch.mesh_to_batch.get(mesh_idx);
    MeshBatch<Entity>& mesh_batch = shader_batch.mesh_batches[*mesh_batch_idx];

    u32 idx = vk.entities[entity_idx].entity_idx_in_array;
    u32 last_idx = mesh_batch.entities.count-1;
    Handle<Entity> swapped = mesh_batch.entities[last_idx];
    u32 swapped_idx = swapped.idx();

    mesh_batch.entities[idx] = swapped;
    mesh_batch.entities.pop();
    vk.entities[swapped_idx].entity_idx_in_array = idx;
  }
}

void vk_set_entity_color(Handle<Entity> entity_handle, v4 color) {
  vk.gpu_entities[entity_handle.idx()].color = color;
}

////////////////////////////////////////////////////////////////////////
// Point light
// KAPI void vk_point_light_create(u32 entity_id) {
//   // vk.point_light_data.add(entity_id);
//   // ++vk.global_shader_state->point_light_count;
// }

// KAPI void vk_point_light_destroy(u32 entity_id) {
//   // vk.point_light_data.remove_data(entity_id);;
//   // --vk.global_shader_state->point_light_count;
// }

// KAPI PointLight& vk_point_light_get(u32 entity_id) {
//   // return *(PointLight*)vk.point_light_data.get_data(entity_id);
// }

// ////////////////////////////////////////////////////////////////////////
// // Directional light
// KAPI void vk_dir_light_create(u32 entity_id) {
//   // vk.dir_light_data.add(entity_id);
//   // ++vk.global_shader_state->dir_light_count;
// }

// KAPI void vk_dir_light_destroy(u32 entity_id) {
//   // vk.dir_light_data.remove_data(entity_id);
//   // --vk.global_shader_state->dir_light_count;
// }

// KAPI DirLight& vk_dir_light_get(u32 entity_id) {
//   // return *(DirLight*)vk.dir_light_data.get_data(entity_id);
// }

// ////////////////////////////////////////////////////////////////////////
// // Spot light
// KAPI void vk_spot_light_create(u32 entity_id) {
//   // vk.spot_light_data.add(entity_id);
//   // ++vk.global_shader_state->spot_light_count;
// }

// KAPI void vk_spot_light_destroy(u32 entity_id) {
//   // vk.spot_light_data.remove_data(entity_id);
//   // --vk.global_shader_state->spot_light_count;
// }

// KAPI SpotLight& vk_spot_light_get(u32 entity_id) {
//   // return *(SpotLight*)vk.spot_light_data.get_data(entity_id);
// }

////////////////////////////////////////////////////////////////////////
// Util

GlobalStateGPU* vk_get_shader_state() {
  return (GlobalStateGPU*)vk.storage_buffer.mapped_memory;
}

mat4& vk_get_view() { return vk.view; }
mat4& vk_get_projection() { return vk.projection; };

void debug_draw_line(v3 a, v3 b, v3 color) {
  Vertex vert[] = {
    {.pos = a, .color = color},
    {.pos = b, .color = color},
  };
  vk.immediate_draw_lines.add({vert[0], vert[1]});
}

void debug_draw_aabb(v3 min, v3 max, v3 color) {
  v3 p000 = {min.x, min.y, min.z};
  v3 p001 = {min.x, min.y, max.z};
  v3 p010 = {min.x, max.y, min.z};
  v3 p011 = {min.x, max.y, max.z};

  v3 p100 = {max.x, min.y, min.z};
  v3 p101 = {max.x, min.y, max.z};
  v3 p110 = {max.x, max.y, min.z};
  v3 p111 = {max.x, max.y, max.z};

  debug_draw_line(p000, p001, color);
  debug_draw_line(p000, p010, color);
  debug_draw_line(p000, p100, color);

  debug_draw_line(p111, p110, color);
  debug_draw_line(p111, p101, color);
  debug_draw_line(p111, p011, color);

  debug_draw_line(p001, p011, color);
  debug_draw_line(p001, p101, color);

  debug_draw_line(p010, p011, color);
  debug_draw_line(p010, p110, color);

  debug_draw_line(p100, p101, color);
  debug_draw_line(p100, p110, color);
}

void draw_quad(v2 min, v2 max, v3 color) {
  v2 size = v2_of_v2u(os_get_window_size());
  min = v2_map_to_v2_11(min, size);
  max = v2_map_to_v2_11(max, size);
  VK_State::DebugDrawSquare square = {
    .vert = {
      {.pos = v2_to_v3(min,0),              .color = color},
      {.pos = v2_to_v3(v2(min.x, max.y),0), .color = color},
      {.pos = v2_to_v3(max,0),              .color = color},
      {.pos = v2_to_v3(max,0),              .color = color},
      {.pos = v2_to_v3(v2(max.x, min.y),0), .color = color},
      {.pos = v2_to_v3(min,0),              .color = color},
    }
  };
  vk.immediate_draw_squares.add(square);
}

struct UI_Window {
  v2 pos;
  v2 size;
};

struct UI_Box {
  v2 pos;
  v2 size;
  u64 hash;
};

struct UI_State {
  u32 hot;
  u32 last_hot;
  u32 active;
  u32 active_window;
  v2 drag_offset;
  UI_Window windows[10];
  UI_Box boxes[10];
  u32 boxes_count;
  HashedStrMap<u32> hashes;
};

UI_State ui_state;

void ui_begin() {
  UI_State& ui = ui_state;
  ui.last_hot = ui.hot;
  ui_state.hot = 0;
}

void ui_end() {
  if (os_is_key_released(MouseKey_Left)) {
    ui_state.active = 0;
  }
}

void ui_push_box(String str) {
  UI_State& ui = ui_state;

  UI_Box& parent = ui.boxes[ui.boxes_count];
  ++ui.boxes_count;

  u64 hash_idx = hash(str);
  UI_Box box = {
    .pos = v2(parent.pos + parent.size),
    .size = {100 + parent.size.x, 100 + parent.pos.y},
    .hash = (hash(hash_idx, parent.hash))
  };
  ui.boxes[ui.boxes_count] = box;

  ui_button(box.hash, box.pos, box.pos+box.size);
}

void ui_pop_box() {
  UI_State& ui = ui_state;
  if (ui.boxes_count > 0) {
    --ui.boxes_count;
  }
}

b32 ui_begin_window(u32 id, v2 size) {
  UI_State& ui = ui_state;
  v2& pos = ui.windows[id].pos;
  v2 mouse = os_get_mouse_pos();
  Rect title_rect(pos, v2(pos.x + size.x, pos.y + 20));

  b32 hovered = v2_in_rect(title_rect, mouse);
  if (hovered) {
    ui.hot = id;
  }

  // PRESS → start dragging
  if (ui.last_hot == id && os_is_key_pressed(MouseKey_Left)) {
    ui.active = id;
    ui.active_window = id;

    // store offset
    ui.drag_offset = v2(mouse.x - pos.x, mouse.y - pos.y);
  }

  // DRAG
  if (ui.active == id && os_is_key_down(MouseKey_Left)) {
    pos.x = mouse.x - ui.drag_offset.x;
    pos.y = mouse.y - ui.drag_offset.y;
  }

  // RELEASE
  if (ui.active == id && os_is_key_released(MouseKey_Left)) {
    ui.active = 0;
  }

  // 🎨 Draw window body
  draw_quad(pos, v2(pos.x + size.x, pos.y + size.y), v3(0.2f,0.2f,0.2f));

  // 🎨 Draw title bar
  v3 title_color = v3(0.3f,0.3f,0.3f);
  if (ui.hot == id) title_color = v3(0.4f,0.4f,0.4f);
  if (ui.active == id) title_color = v3(0.2f,0.2f,0.2f);

  title_rect = {pos, v2(pos.x + size.x, pos.y + 20)};
  draw_quad(title_rect.min, title_rect.max, title_color);

  return true;
}

b32 ui_button(u32 id, v2 min, v2 max) {
  UI_State* ui = &ui_state;
  b32 hovered = v2_in_rect({min, max}, os_get_mouse_pos());
  if (hovered) {
    ui->hot = id;
  }

  if (ui->last_hot == id && os_is_key_pressed(MouseKey_Left)) {
    ui->active = id;
  }

  b32 clicked = 0;
  if (ui->active == id && os_is_key_released(MouseKey_Left)) {
    if (ui->hot == id) {
      clicked = true;
    }
    ui->active = 0;
  }

  v3 color = {0.6f, 0.6f, 0.6f};
  if (ui->hot == id) color = v3(0.8f, 0.8f, 0.8f);
  if (ui->active == id) color = v3(0.4f, 0.4f, 0.4f);

  draw_quad(min, max, color);
  return clicked;
}

#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"

void imgui_impl_x11_init() {
  ImGuiIO& io = ImGui::GetIO();
  io.BackendPlatformName = "imgui_impl_x11";
  ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
  platform_io.Platform_GetClipboardTextFn = [](ImGuiContext*)->const char* {
    Scratch scratch;
    String str = push_str_copy(scratch, os_clipboard_read());
    return (const char*)str.str;
  };
  platform_io.Platform_SetClipboardTextFn = [](ImGuiContext*, const char* text) { 
    os_clipboard_write(text);
  };
}

ImGuiKey imgui_keycode_translate(Key key) {
  switch (key) {
    case Key_Tab:         return ImGuiKey_Tab;
    case Key_Left:        return ImGuiKey_LeftArrow;
    case Key_Right:       return ImGuiKey_RightArrow;
    case Key_Up:          return ImGuiKey_UpArrow;
    case Key_Down:        return ImGuiKey_DownArrow;

    case Key_Pageup:      return ImGuiKey_PageUp;
    case Key_Pagedown:    return ImGuiKey_PageDown;
    case Key_Home:        return ImGuiKey_Home;
    case Key_End:         return ImGuiKey_End;
    case Key_Delete:      return ImGuiKey_Delete;

    case Key_Backspace:   return ImGuiKey_Backspace;
    case Key_Space:       return ImGuiKey_Space;
    case Key_Enter:       return ImGuiKey_Enter;
    case Key_Escape:      return ImGuiKey_Escape;

    case Key_Capslock:    return ImGuiKey_CapsLock;
    case Key_Numlock:     return ImGuiKey_NumLock;
    case Key_Printscreen: return ImGuiKey_PrintScreen;
    case Key_Pause:       return ImGuiKey_Pause;

    case Key_LShift:      return ImGuiKey_LeftShift;
    case Key_RShift:      return ImGuiKey_RightShift;
    case Key_LControl:    return ImGuiKey_LeftCtrl;
    case Key_RControl:    return ImGuiKey_RightCtrl;
    case Key_LAlt:        return ImGuiKey_LeftAlt;
    case Key_RAlt:        return ImGuiKey_RightAlt;
    case Key_Lsuper:      return ImGuiKey_LeftSuper;
    case Key_Rsuper:      return ImGuiKey_RightSuper;

    // Digits
    case Key_0: return ImGuiKey_0;
    case Key_1: return ImGuiKey_1;
    case Key_2: return ImGuiKey_2;
    case Key_3: return ImGuiKey_3;
    case Key_4: return ImGuiKey_4;
    case Key_5: return ImGuiKey_5;
    case Key_6: return ImGuiKey_6;
    case Key_7: return ImGuiKey_7;
    case Key_8: return ImGuiKey_8;
    case Key_9: return ImGuiKey_9;

    // Letters
    case Key_A: return ImGuiKey_A;
    case Key_B: return ImGuiKey_B;
    case Key_C: return ImGuiKey_C;
    case Key_D: return ImGuiKey_D;
    case Key_E: return ImGuiKey_E;
    case Key_F: return ImGuiKey_F;
    case Key_G: return ImGuiKey_G;
    case Key_H: return ImGuiKey_H;
    case Key_I: return ImGuiKey_I;
    case Key_J: return ImGuiKey_J;
    case Key_K: return ImGuiKey_K;
    case Key_L: return ImGuiKey_L;
    case Key_M: return ImGuiKey_M;
    case Key_N: return ImGuiKey_N;
    case Key_O: return ImGuiKey_O;
    case Key_P: return ImGuiKey_P;
    case Key_Q: return ImGuiKey_Q;
    case Key_R: return ImGuiKey_R;
    case Key_S: return ImGuiKey_S;
    case Key_T: return ImGuiKey_T;
    case Key_U: return ImGuiKey_U;
    case Key_V: return ImGuiKey_V;
    case Key_W: return ImGuiKey_W;
    case Key_X: return ImGuiKey_X;
    case Key_Y: return ImGuiKey_Y;
    case Key_Z: return ImGuiKey_Z;

    // Function keys
    case Key_F1:  return ImGuiKey_F1;
    case Key_F2:  return ImGuiKey_F2;
    case Key_F3:  return ImGuiKey_F3;
    case Key_F4:  return ImGuiKey_F4;
    case Key_F5:  return ImGuiKey_F5;
    case Key_F6:  return ImGuiKey_F6;
    case Key_F7:  return ImGuiKey_F7;
    case Key_F8:  return ImGuiKey_F8;
    case Key_F9:  return ImGuiKey_F9;
    case Key_F10: return ImGuiKey_F10;
    case Key_F11: return ImGuiKey_F11;
    case Key_F12: return ImGuiKey_F12;

    // Symbols
    case Key_Semicolon:   return ImGuiKey_Semicolon;
    case Key_Equal:       return ImGuiKey_Equal;
    case Key_Comma:       return ImGuiKey_Comma;
    case Key_Minus:       return ImGuiKey_Minus;
    case Key_Dot:         return ImGuiKey_Period;
    case Key_Slash:       return ImGuiKey_Slash;
    case Key_Grave:       return ImGuiKey_GraveAccent;
    case Key_LBracket:    return ImGuiKey_LeftBracket;
    case Key_Backslash:   return ImGuiKey_Backslash;
    case Key_RBracket:    return ImGuiKey_RightBracket;
    case Key_Apostrophe:  return ImGuiKey_Apostrophe;

    // Mouse
    case MouseKey_Left:   return (ImGuiKey)ImGuiMouseButton_Left;
    case MouseKey_Right:  return (ImGuiKey)ImGuiMouseButton_Right;
    case MouseKey_Middle: return (ImGuiKey)ImGuiMouseButton_Middle;

    default:
    return ImGuiKey_None;
  }
}

void imgui_impl_x11_new_frame() {
  ImGuiIO& io = ImGui::GetIO();
  io.DeltaTime = g_dt;
  v2u win_size = os_get_window_size();
  io.DisplaySize = ImVec2(win_size.x, win_size.y);
  Slice<OS_InputEvent> events = os_get_events();
  for (OS_InputEvent event : events) {
    switch (event.type) {
      case OS_EventKind_Key: {
        ImGuiKey key = imgui_keycode_translate(event.key);
        io.AddKeyEvent(key, event.is_pressed);
          if (event.is_pressed) {
            // io.AddInputCharacter(os_key_to_str(event.key, event.modifier));
            io.AddInputCharacter(os_key_to_str(event.key, event.modifier));
          }
      } break;
      case OS_EventKind_MouseButton: {
        ImGuiKey key = imgui_keycode_translate(event.key);
        io.AddMouseButtonEvent(key, event.is_pressed);
      } break;
      case OS_EventKind_MouseMove: {
        io.AddMousePosEvent(event.x, event.y);
      } break;
      case OS_EventKind_Scroll: {
        io.AddMouseWheelEvent(event.scroll_x, event.scroll_y);
      } break;
      case OS_EventKind_Modifier:
        if (FlagHas(event.modifier, OS_Modifier_Shift)) {
          io.AddKeyEvent(ImGuiMod_Shift, true);
        } else {
          io.AddKeyEvent(ImGuiMod_Shift, false);
        }
        if (FlagHas(event.modifier, OS_Modifier_Alt)) {
          io.AddKeyEvent(ImGuiMod_Alt, true);
        } else {
          io.AddKeyEvent(ImGuiMod_Alt, false);
        }
        if (FlagHas(event.modifier, OS_Modifier_Ctrl)) {
          io.AddKeyEvent(ImGuiMod_Ctrl, true);
        } else {
          io.AddKeyEvent(ImGuiMod_Ctrl, false);
        } break;
      }
  }
}

struct ImguiState {
  VkDescriptorPool descriptor_pool;
};

global ImguiState imgui_st;

PFN_vkVoidFunction imgui_load_fn(const char* function_name, void* user_data) {
  return (PFN_vkVoidFunction)os_lib_get_proc(vk.lib, function_name);
}

void imgui_init() {
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  imgui_impl_x11_init();

  VkDescriptorPoolSize pool_sizes[] = {
    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
  };
  VkDescriptorPoolCreateInfo pool_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .poolSizeCount = ArrayCount(pool_sizes),
    .pPoolSizes = pool_sizes,
    .maxSets = 1000,
    .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
  };
  VK_CHECK(vk.CreateDescriptorPool(vkdevice, &pool_info, null, &imgui_st.descriptor_pool));
  VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
  ImGui_ImplVulkan_InitInfo init_info = {
    .Instance = vk.instance,
    .PhysicalDevice = vk.device.physical_device,
    .Device = vk.device.logical_device,
    .QueueFamily = vk.device.graphics_queue_index,
    .Queue = vk.device.graphics_queue,
    .DescriptorPool = imgui_st.descriptor_pool,
    .MinImageCount = vk.frames_in_flight,
    .ImageCount = vk.images_in_flight,
    .PipelineInfoMain = {
      .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
      .PipelineRenderingCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &format,
      },
    },
    .UseDynamicRendering = true,
  };
  ImGui_ImplVulkan_LoadFunctions(VK_API_VERSION_1_4, imgui_load_fn, null);
  ImGui_ImplVulkan_Init(&init_info);
}

void imgui_begin_frame() {
  ImGui_ImplVulkan_NewFrame();
  imgui_impl_x11_new_frame();
  ImGui::NewFrame();
}

void imgui_end_frame() {
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vk.cmds[vk.current_frame_idx]);
  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();
}

