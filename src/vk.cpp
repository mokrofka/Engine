#include "vk.h"
#include "common.h"
#include "vulkan/vulkan_core.h"

#if BUILD_DEBUG
  #define VK_CHECK(expr)          \
    {                             \
      Assert(expr == VK_SUCCESS); \
    }
  #else
    #define VK_CHECK(expr) expr
#endif

#define vkdevice vk.device.logical_device

struct VK_Buffer {
  VkBuffer handle;
  VkDeviceMemory memory;
  u8* maped_memory;
  u64 size;
  u64 cap;
};

struct VK_Image {
  VkImage handle;
  VkDeviceMemory memory;
  VkImageView view;
  u32 width;
  u32 height;
  u32 array_layers;
};

struct VK_Mesh {
  u64 vert_count;
  VkDeviceSize vert_offset;
  u64 index_count;
  VkDeviceSize index_offset;
};

struct VK_Swapchain {
  VkSwapchainKHR handle;
  VkSurfaceFormatKHR image_format;  
  VkPresentModeKHR present_mode;
  VK_Image images[4];
  VkImageView views[4];
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

struct VK_Shader {
  String name;
  ShaderType type;
  VkPipeline pipeline;
  Array<VkPipelineShaderStageCreateInfo, 2> stages;
  HandlerDarray<u32> entities;
};

struct VK_ShaderCompute {
  VkPipeline pipeline;
  VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info;
};

struct VK_SyncObj {
  VkSemaphore* image_available_semaphores;
  VkSemaphore* render_complete_semaphores;
  VkSemaphore* compute_complete_semaphores;
  VkFence* in_flight_fences;
};

struct RenderEntity {
  u32 shader_handle;
};

struct VK_State {
  Arena arena;
  
  OS_Handle lib;
  AllocSegList alloc;
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
  
  VK_Buffer vert_buffer;
  VK_Buffer index_buffer;
  VK_Buffer stage_buffer;
  VK_Buffer storage_buffer;
  VK_Buffer compute_storage_buffers[2];
  
  VkDescriptorPool descriptor_pool;
  VkDescriptorSetLayout descriptor_set_layout;
  VkDescriptorSet descriptor_sets;

  VkSampler sampler;
  VkPipelineLayout pipeline_layout;

  VkDescriptorSetLayout compute_descriptor_set_layout;
  // VkDescriptorSet compute_descriptor_sets[FramesInFlight];
  
  VkCommandBuffer* cmds;
  VkCommandBuffer* compute_cmds;

  ShaderEntity* entities_data;

  Array<RenderEntity, MaxEntities> entities;
  Array<u32, MaxEntities> entities_to_shader;
  Array<u32, MaxEntities> entities_to_mesh;

  Array<PushConstant, MaxEntities> push_constants;
  Array<VK_Shader, MaxShaders> shaders;
  Array<VK_Mesh, MaxMeshes> meshes;
  Array<VK_Image, MaxTextures> texture;

  SparseSet<PointLight> point_light_data;
  SparseSet<DirLight> dir_light_data;
  SparseSet<SpotLight> spot_light_data;

  // VK_ShaderCompute compute_shader;
  // ShaderGlobalState* global_shader_state;
  VK_Shader screen_shader;
  VK_Shader cubemap_shader;

  // offscreen rendering
  // VK_Texture* texture_targets;
  VK_Image offscreen_depth_buffer;

  #define VK_DECL(name) Glue(PFN_, vk##name) name

  #define VK_FUNCTION(name) vk.name = (Glue(PFN_, vk##name))os_lib_get_proc(vk.lib, Stringify(vk##name))
  #define VK_INSTANCE_FUNCTION(name) vk.name = (Glue(PFN_, vk##name))vk.GetInstanceProcAddr(vk.instance, Stringify(vk##name))
  #define VK_DEVICE_FUNCTION(name) vk.name = (Glue(PFN_, vk##name))vk.GetDeviceProcAddr(vkdevice, Stringify(vk##name))

  // Core functions
  VK_DECL(GetInstanceProcAddr);

  // Instance functions
  VK_DECL(EnumerateInstanceExtensionProperties);
  VK_DECL(EnumerateInstanceVersion);
  VK_DECL(EnumerateInstanceLayerProperties);
  VK_DECL(CreateInstance);
  VK_DECL(DestroyInstance);
  VK_DECL(EnumeratePhysicalDevices);
  VK_DECL(GetDeviceProcAddr);
  VK_DECL(GetPhysicalDeviceProperties);
  VK_DECL(GetPhysicalDeviceFeatures);
  VK_DECL(GetPhysicalDeviceMemoryProperties);
  VK_DECL(GetPhysicalDeviceQueueFamilyProperties);
  VK_DECL(GetPhysicalDeviceFormatProperties);
  VK_DECL(GetPhysicalDeviceSurfaceFormatsKHR);
  VK_DECL(GetPhysicalDeviceSurfaceCapabilitiesKHR);
  VK_DECL(GetPhysicalDeviceSurfacePresentModesKHR);
  VK_DECL(GetPhysicalDeviceSurfaceSupportKHR);
  VK_DECL(EnumerateDeviceExtensionProperties);
  VK_DECL(CreateDevice);
  VK_DECL(DestroySurfaceKHR);

  // Device functions.
  VK_DECL(GetDeviceQueue);
  VK_DECL(DeviceWaitIdle);
  VK_DECL(CreateCommandPool);
  VK_DECL(DestroyCommandPool);
  VK_DECL(DestroyDevice);
  VK_DECL(CreateSwapchainKHR);
  VK_DECL(DestroySwapchainKHR);
  VK_DECL(GetSwapchainImagesKHR);
  VK_DECL(CreateImage);
  VK_DECL(CreateImageView);
  VK_DECL(DestroyImage);
  VK_DECL(DestroyImageView);
  VK_DECL(GetImageMemoryRequirements);
  VK_DECL(AllocateMemory);
  VK_DECL(FreeMemory);
  VK_DECL(AllocateCommandBuffers);
  VK_DECL(FreeCommandBuffers);
  VK_DECL(BeginCommandBuffer);
  VK_DECL(EndCommandBuffer);
  VK_DECL(BindImageMemory);
  VK_DECL(CreateSemaphore);
  VK_DECL(DestroySemaphore);
  VK_DECL(CreateFence);
  VK_DECL(DestroyFence);
  VK_DECL(WaitForFences);
  VK_DECL(AcquireNextImageKHR);
  VK_DECL(ResetFences);
  VK_DECL(CreateDescriptorSetLayout);
  VK_DECL(DestroyDescriptorSetLayout);
  VK_DECL(CreateDescriptorPool);
  VK_DECL(DestroyDescriptorPool);
  VK_DECL(CreateShaderModule);
  VK_DECL(DestroyShaderModule);
  VK_DECL(CreateSampler);
  VK_DECL(DestroySampler);
  VK_DECL(CreateBuffer);
  VK_DECL(DestroyBuffer);
  VK_DECL(GetBufferMemoryRequirements);
  VK_DECL(BindBufferMemory);
  VK_DECL(MapMemory);
  VK_DECL(UnmapMemory);
  VK_DECL(FlushMappedMemoryRanges);
  VK_DECL(CreatePipelineLayout);
  VK_DECL(DestroyPipelineLayout);
  VK_DECL(CreateGraphicsPipelines);
  VK_DECL(DestroyPipeline);
  VK_DECL(AllocateDescriptorSets);
  VK_DECL(FreeDescriptorSets);
  VK_DECL(UpdateDescriptorSets);

  VK_DECL(CmdBindPipeline);
  VK_DECL(CmdPipelineBarrier);
  VK_DECL(CmdBlitImage);
  VK_DECL(CmdCopyBuffer);
  VK_DECL(CmdCopyBufferToImage);
  VK_DECL(CmdCopyImageToBuffer);
  VK_DECL(CmdExecuteCommands);
  VK_DECL(CmdSetViewport);
  VK_DECL(CmdSetScissor);
  VK_DECL(CmdSetFrontFace);
  VK_DECL(CmdSetCullMode);
  VK_DECL(CmdSetStencilTestEnable);
  VK_DECL(CmdSetDepthTestEnable);
  VK_DECL(CmdSetDepthWriteEnable);
  VK_DECL(CmdSetStencilReference);
  VK_DECL(CmdSetStencilOp);
  VK_DECL(CmdBeginRendering);
  VK_DECL(CmdEndRendering);
  VK_DECL(CmdSetStencilCompareMask);
  VK_DECL(CmdSetStencilWriteMask);
  VK_DECL(CmdClearColorImage);
  VK_DECL(CmdClearDepthStencilImage);
  VK_DECL(CmdSetPrimitiveTopology);
  VK_DECL(CmdPushConstants);
  VK_DECL(CmdBindVertexBuffers);
  VK_DECL(CmdBindIndexBuffer);
  VK_DECL(CmdDraw);
  VK_DECL(CmdDrawIndexed);
  VK_DECL(CmdBindDescriptorSets);

  VK_DECL(QueueSubmit);
  VK_DECL(QueueWaitIdle);
  VK_DECL(QueuePresentKHR);
};

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

VK_State vk;

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
  return idx;
}

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
    case VK_RESULT_MAX_ENUM:                                    break;
    case VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT:
      break;
    }
  return "NO IDEA";
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

intern VkCommandBuffer vk_cmd_alloc_and_begin_single_use() {
  VkCommandBuffer result = vk_cmd_alloc(vk.device.cmd_pool);
  vk_cmd_begin(result);
  return result;
}

intern void vk_cmd_end_single_use(VkCommandBuffer cmd) {
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
  Assert(buffer.memory && buffer.handle);
  vk.FreeMemory(vkdevice, buffer.memory, vk.allocator);
  vk.DestroyBuffer(vkdevice, buffer.handle, vk.allocator);
}

intern void vk_buffer_map_memory(VK_Buffer& buffer, u64 offset, u64 size) {
  VK_CHECK(vk.MapMemory(vkdevice, buffer.memory, offset, size, 0, (void**)&buffer.maped_memory));
}

intern void vk_buffer_unmap_memory(VK_Buffer buffer) {
  vk.UnmapMemory(vkdevice, buffer.memory);
}

intern void vk_buffer_upload_to_gpu(VK_Buffer buffer, Range range, void* data) {
  MemCopy(vk.stage_buffer.maped_memory, data, range.size);
  VkCommandBuffer temp_cmd = vk_cmd_alloc_and_begin_single_use();
  VkBufferCopy copy_region = {
    .srcOffset = 0,
    .dstOffset = range.offset,
    .size = range.size,
  };
  vk.CmdCopyBuffer(temp_cmd, vk.stage_buffer.handle, buffer.handle, 1, &copy_region);
  vk_cmd_end_single_use(temp_cmd);
}

////////////////////////////////////////////////////////////////////////
// @Pipeline

intern void vk_shader_pipeline_init() {
  // Push constants
  VkPushConstantRange push_constant = {
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    .offset = 0,
    .size = sizeof(u8)*128,
  };
  
  // Pipeline layout
  VkDescriptorSetLayout set_layouts[] = {
    vk.descriptor_set_layout,
  };
  VkPipelineLayoutCreateInfo pipeline_layout_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = ArrayCount(set_layouts),
    .pSetLayouts = set_layouts,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges = &push_constant,
  };
  VK_CHECK(vk.CreatePipelineLayout(vkdevice, &pipeline_layout_info, vk.allocator, &vk.pipeline_layout));
}

intern VkPipeline vk_shader_pipeline_create(ShaderType type, Array<VkPipelineShaderStageCreateInfo, 2> stages) {
  ShaderInfo shader_info = shader_type[type];

  // Dynamic rendering
  // VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
  VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;

  VkPipelineRenderingCreateInfo renderingCreateInfo = {
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
  VkPipelineVertexInputStateCreateInfo vertex_input_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &binding_description,
    .vertexAttributeDescriptionCount = ArrayCount(attribute_desriptions),
    .pVertexAttributeDescriptions = attribute_desriptions,
  };

  // Input assembly
  VkPrimitiveTopology topology;
  switch (shader_info.primitive) {
    case ShaderTopology_Triangle: topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
    case ShaderTopology_Line:     topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
    case ShaderTopology_Point:    topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; break;
    break;
  }
  VkPipelineInputAssemblyStateCreateInfo input_assembly = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = topology,
    .primitiveRestartEnable = VK_FALSE,
  };
  
  // Viewport
  VkViewport viewport = {
    .x = 0.0f,
    .y = (f32)vk.height,
    .width = (f32)vk.width,
    .height = (f32)vk.height,
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
  VkPipelineRasterizationStateCreateInfo rasterizer_create_info = {
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
  VkPipelineMultisampleStateCreateInfo multisampling_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    .sampleShadingEnable = VK_FALSE,
    .minSampleShading = 1.0f,
    .pSampleMask = 0,
    .alphaToCoverageEnable = VK_FALSE,
    .alphaToOneEnable = VK_FALSE,
  };

  // Depth and stencil testing
  VkPipelineDepthStencilStateCreateInfo depth_stencil = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  if (shader_info.use_depth) {
    depth_stencil.depthTestEnable = VK_TRUE;
    if (IsInRange(ShaderType_Cubemap, type, ShaderType_Cubemap_COUNT)) {
      depth_stencil.depthWriteEnable = VK_FALSE;
      depth_stencil.depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL;
    } else {
      depth_stencil.depthWriteEnable = VK_TRUE;
      depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    }
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;
  }
  
  // Blending
  VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
  if (shader_info.is_transparent) {
    color_blend_attachment_state.blendEnable = VK_TRUE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;

    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
  } else {
    color_blend_attachment_state.blendEnable = VK_FALSE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;

    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
  }
  color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_COPY,
    .attachmentCount = 1,
    .pAttachments = &color_blend_attachment_state,
  };
   
  // Dynamic state
  VkDynamicState dynamic_state[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
  };
  VkPipelineDynamicStateCreateInfo dynamic_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = ArrayCount(dynamic_state),
    .pDynamicStates = dynamic_state,
  };
  
  // Pipeline
  VkGraphicsPipelineCreateInfo pipeline_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = &renderingCreateInfo,
    .stageCount = shader_info.stage_count,
    .pStages = stages.data,
    .pVertexInputState = &vertex_input_info,
    .pInputAssemblyState = &input_assembly,
    .pTessellationState = null,
    .pViewportState = &viewport_state,
    .pRasterizationState = &rasterizer_create_info,
    .pMultisampleState = &multisampling_create_info,
    .pDepthStencilState = &depth_stencil,
    .pColorBlendState = &color_blend_state_create_info,
    .pDynamicState = &dynamic_state_info,
    .layout = vk.pipeline_layout,
    .renderPass = null,
    .basePipelineHandle = VK_NULL_HANDLE,
    .basePipelineIndex = -1,
  };
  VkPipelineCacheCreateInfo info;
  VkPipeline result;
  VK_CHECK(vk.CreateGraphicsPipelines(vkdevice, VK_NULL_HANDLE, 1, &pipeline_info, vk.allocator, &result));
  return result;
}

intern Array<VkPipelineShaderStageCreateInfo, 2> vk_shader_module_create(String name) {
  Scratch scratch;
  Array<VkPipelineShaderStageCreateInfo, 2> stages;
  Loop (i, 2) {
    String stage_type_strs[] = {"vert", "frag"};
    VkShaderStageFlagBits stage_types[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    String filepath = push_strf(scratch, "%s/shaders/compiled/%s.%s.spv", asset_base_path(), name, stage_type_strs[i]);
    Buffer binary = os_file_read_all(scratch, filepath);
    VkShaderModuleCreateInfo module_create_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = binary.size,
      .pCode = (u32*)binary.data,
    };
    VkShaderModule handle;
    VK_CHECK(vk.CreateShaderModule(vkdevice, &module_create_info, vk.allocator, &handle));
    stages[i] = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = stage_types[i],
      .module = handle,
      .pName = "main",
    };
  }
  return stages;
}

u32 vk_shader_load(String name, ShaderType type) {
  Scratch scratch;
  ShaderInfo shader_info = shader_type[type];
  if (IsInRange(ShaderType_Drawing, type, ShaderType_Drawing_COUNT)) {
    VK_Shader shader = {
      .name = name,
      .type = type,
      .stages = vk_shader_module_create(name),
    };
    shader.pipeline = vk_shader_pipeline_create(type, shader.stages);
    u32 id = vk.shaders.count;
    vk.shaders.append(shader);
    return id;
  }
  else if (IsInRange(ShaderType_Screen, type, ShaderType_Screen_COUNT)) {
    VK_Shader &shader = vk.screen_shader;
    shader = {
      .name = name,
      .type = type,
      .stages = vk_shader_module_create(name),
    };
    shader.pipeline = vk_shader_pipeline_create(type, shader.stages);
    u32 id = vk.shaders.count + 100;
    // append(vk.shaders, shader);
    return id;
  }
  else if (IsInRange(ShaderType_Cubemap, type, ShaderType_Cubemap_COUNT)) {
    VK_Shader &shader = vk.cubemap_shader;
    shader = {
      .name = name,
      .type = type,
      .stages = vk_shader_module_create(name),
    };
    shader.pipeline = vk_shader_pipeline_create(type, shader.stages);
    return 200;
  }
  Assert(true);
  return 0;
}

void vk_shader_reload(String name, u32 id) {
  if (id >= 200) {
    VK_Shader& shader = vk.cubemap_shader;
    vk.DeviceWaitIdle(vkdevice);
    vk.DestroyPipeline(vkdevice, shader.pipeline, vk.allocator);
    Loop (i, 2) {
      vk.DestroyShaderModule(vkdevice, shader.stages[i].module, vk.allocator);
    }
    shader.stages = vk_shader_module_create(name);
    shader.pipeline = vk_shader_pipeline_create(shader.type, shader.stages);
  } else if (id >= 100) {
    VK_Shader& shader = vk.screen_shader;
    vk.DeviceWaitIdle(vkdevice);
    vk.DestroyPipeline(vkdevice, shader.pipeline, vk.allocator);
    Loop (i, 2) {
      vk.DestroyShaderModule(vkdevice, shader.stages[i].module, vk.allocator);
    }
    shader.stages = vk_shader_module_create(name);
    shader.pipeline = vk_shader_pipeline_create(shader.type, shader.stages);
  } else {
    VK_Shader& shader = vk.shaders[id];
    vk.DeviceWaitIdle(vkdevice);
    vk.DestroyPipeline(vkdevice, shader.pipeline, vk.allocator);
    Loop (i, 2) {
      vk.DestroyShaderModule(vkdevice, shader.stages[i].module, vk.allocator);
    }
    shader.stages = vk_shader_module_create(name);
    shader.pipeline = vk_shader_pipeline_create(shader.type, shader.stages);
  }
}

intern void vk_descriptor_init() {
  Scratch scratch;

  // Pool
  #define MaxStorageBuffer 1
  #define MaxSets 1
  #define MaxSamplers 1
  #define MaxCubeTextures 1
  {
    VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MaxStorageBuffer},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MaxTextures},
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
    };
    VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
      .pNext = null,
      .bindingCount = ArrayCount(flags),
      .pBindingFlags = flags,
    };
    VkDescriptorSetLayoutBinding layout_bindings[] = {
      {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = MaxTextures - MaxCubeTextures,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      {
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .descriptorCount = MaxSamplers,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      {
        .binding = 3,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = MaxCubeTextures,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
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

  // Descriptor
  {
    VkDescriptorSetAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = vk.descriptor_pool,
      .descriptorSetCount = 1,
      .pSetLayouts = &vk.descriptor_set_layout,
    };
    VK_CHECK(vk.AllocateDescriptorSets(vkdevice, &alloc_info, &vk.descriptor_sets));
  }

  // Mem for shaders
  {
    u64 entities_start = AlignUp(sizeof(ShaderGlobalState), alignof(ShaderEntity));
    Assign(vk.entities_data, Offset(vk.storage_buffer.maped_memory, entities_start));
  }

  // Shader Data
  {
    VkDescriptorBufferInfo buffer_info = {
      .buffer = vk.storage_buffer.handle,
      .offset = 0,
      .range = VK_WHOLE_SIZE,
    };
    VkWriteDescriptorSet ubo_descriptor = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = vk.descriptor_sets,
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .pBufferInfo = &buffer_info,
    };
    VkWriteDescriptorSet descriptors[] = {ubo_descriptor};
    vk.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  }
}

////////////////////////////////////////////////////////////////////////
// @Image

struct VK_ImageInfo {
  VkImageType image_type;
  u32 width;
  u32 height;
  VkImageCreateFlags flags;
  VkFormat format;
  u32 arrayLayers;
  VkImageTiling tiling;
  VkImageUsageFlags usage;
  VkMemoryPropertyFlags memory_flags;
  b32 create_view;
  VkImageAspectFlags aspect;
  VkImageViewType view_type;
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
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = info.arrayLayers,
    },
  };
  VkImageView result;
  VK_CHECK(vk.CreateImageView(vkdevice, &view_create_info, vk.allocator, &result));
  return result;
}

intern VK_ImageInfo vk_image_info_default(u32 width, u32 height) {
  VK_ImageInfo result = {
    .image_type = VK_IMAGE_TYPE_2D,
    .width = width,
    .height = height,
    .flags = 0,
    .format = VK_FORMAT_R8G8B8A8_UNORM,
    .arrayLayers = 1,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    .create_view = true,
    .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
    .view_type = VK_IMAGE_VIEW_TYPE_2D,
  };
  return result;
};

intern VK_Image vk_image_create(VK_ImageInfo info) {
  VkImageCreateInfo image_create_info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .flags = info.flags,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = info.format,
    .extent = {info.width, info.height, 1},
    .mipLevels = 1,
    .arrayLayers = info.arrayLayers,
    .samples = VK_SAMPLE_COUNT_1_BIT,
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
  VkImageView view = 0;
  if (info.create_view) {
    view = vk_image_view_create(handle, info);
  }
  VK_Image result = {
    .handle = handle,
    .memory = memory,
    .view = view,
    .width = info.width,
    .height = info.height,
    .array_layers = info.arrayLayers
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
		case VK_IMAGE_LAYOUT_UNDEFINED:
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:                              return NoFlags;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:                               return VK_ACCESS_HOST_WRITE_BIT;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:                     return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:                     return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:                     return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:                         return VK_ACCESS_TRANSFER_READ_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:                         return VK_ACCESS_TRANSFER_WRITE_BIT;
    default: Assert(false); return 0;
	}
}

intern VkPipelineStageFlags vk_image_layout_to_pipeline_stage_flags(VkImageLayout layout) {
	switch (layout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:                                    return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:                               return VK_PIPELINE_STAGE_HOST_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:                         return VK_PIPELINE_STAGE_TRANSFER_BIT; 
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:                     return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:                     return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:                     return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:                              return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    default: Assert(false); return 0;
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
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = image.array_layers,
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
      .layerCount = image.array_layers,
    },
    .imageExtent = { image.width, image.height, 1 },
  };
  vk.CmdCopyBufferToImage(cmd, vk.stage_buffer.handle, image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

intern void vk_texture_init() {
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
    .maxAnisotropy = 16,
    .compareEnable = VK_FALSE,
    .compareOp = VK_COMPARE_OP_ALWAYS,
    .minLod = 0.0f,
    .maxLod = VK_LOD_CLAMP_NONE,
    .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
    .unnormalizedCoordinates = VK_FALSE,
  };
  VK_CHECK(vk.CreateSampler(vkdevice, &sampler_info, vk.allocator, &vk.sampler));

  // common texture sampler
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

  // cubemap sampler
  // {
  //   VkDescriptorImageInfo descriptor_image_info = {
  //     .sampler = vk.sampler,
  //     .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  //   };
  //   VkWriteDescriptorSet texture_descriptor = {
  //     .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
  //     .dstSet = vk.descriptor_sets,
  //     .dstBinding = 3,
  //     .dstArrayElement = 0,
  //     .descriptorCount = 1,
  //     .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
  //     .pImageInfo = &descriptor_image_info,
  //   };
  //   VkWriteDescriptorSet descriptors[] = {texture_descriptor};
  //   vk.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  // }
};

u32 vk_texture_load(Texture t) {
  u64 size = t.width * t.height * 4;
  MemCopy(vk.stage_buffer.maped_memory, t.data, size);
  VK_ImageInfo image_info = vk_image_info_default(t.width, t.height);
  image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  VK_Image image = vk_image_create(image_info);
  VkCommandBuffer cmd = vk_cmd_alloc_and_begin_single_use();
  vk_image_layout_transition(cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  vk_image_upload_to_gpu(cmd, image);
  vk_image_layout_transition(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  vk_cmd_end_single_use(cmd);
  VkDescriptorImageInfo descriptor_image_info = {
    .sampler = vk.sampler,
    .imageView = image.view,
    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
  VkWriteDescriptorSet texture_descriptor = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = vk.descriptor_sets,
    .dstBinding = 1,
    .dstArrayElement = vk.texture.count,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    .pImageInfo = &descriptor_image_info,
  };
  VkWriteDescriptorSet descriptors[] = {texture_descriptor};
  vk.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  u32 id = vk.texture.count;
  vk.texture.append(image);
  return id;
}

u32 vk_cubemap_load(Texture* textures) {

  u32 width = textures->width;
  u32 height = textures->height;
  u64 size = width * height * 4;

  Loop (i, 6) {
    MemCopy(Offset(vk.stage_buffer.maped_memory, size * i), textures[i].data, size);
  }

  VK_ImageInfo image_info = vk_image_info_default(width, height);
  image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  image_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
  image_info.arrayLayers = 6;
  image_info.view_type = VK_IMAGE_VIEW_TYPE_CUBE;
  VK_Image image = vk_image_create(image_info);
  VkCommandBuffer cmd = vk_cmd_alloc_and_begin_single_use();
  vk_image_layout_transition(cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  vk_image_upload_to_gpu(cmd, image);
  vk_image_layout_transition(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  vk_cmd_end_single_use(cmd);

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
  return 0;
}

intern void vk_resize_texture_target() {
  VK_CHECK(vk.DeviceWaitIdle(vkdevice));
  Debug("texture target resized: x = %u y = %u", vk.width, vk.height);
  Loop (i, vk.images_in_flight) {
    // vk_image_destroy(vk.texture_targets[i]);
    vk_image_destroy(vk.offscreen_depth_buffer);
    VK_ImageInfo texture_info = vk_image_info_default(vk.width, vk.height);
    texture_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    // vk.texture_targets[i].image = vk_image_create(texture_info);
    VK_ImageInfo depth_info = vk_image_info_default(vk.width, vk.height);
    depth_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depth_info.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    vk.offscreen_depth_buffer = vk_image_create(depth_info);
  }
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

  auto missing = [](b32 required, u32 index) {
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
    Assert(features.samplerAnisotropy);

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
  VkPhysicalDeviceDescriptorIndexingFeatures indexing_features = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
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
  Array<u32, queue_count> indices;
  indices.append(vk.device.graphics_queue_index);
  if (!indices.exists(vk.device.present_queue_index)) {
    indices.append(vk.device.present_queue_index);
  }
  if (!indices.exists(vk.device.transfer_queue_index)) {
    indices.append(vk.device.transfer_queue_index);
  }
  if (!indices.exists(vk.device.compute_queue_index)) {
    indices.append(vk.device.compute_queue_index);
  }

  Array<VkDeviceQueueCreateInfo, queue_count> queue_create_infos;
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
    queue_create_infos.append(device_queue_create_info);
  }
  
  const char* extension_names[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };
  VkPhysicalDeviceFeatures device_features = {
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
      .array_layers = 1,
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
    VK_CHECK(vk.CreateImageView(vkdevice, &view_create_info, vk.allocator, &vk.swapchain.views[i]));
  }
  VK_ImageInfo depth_info = vk_image_info_default(swapchain_extent.width, swapchain_extent.height);
  depth_info.format = vk.device.depth_format;
  depth_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  depth_info.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
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
    vk.DestroyImageView(vkdevice, swapchain.views[i], vk.allocator);
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
  VkResult result = vk.AcquireNextImageKHR(vkdevice, vk.swapchain.handle, U64_MAX,
                                           image_available_semaphore, null, &image_index);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    AssertMsg(false, "vkAcquireNextImageKHR failed with: %s", vk_result_string(result));
  }
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

  // VK_CHECK(vkQueuePresentKHR(vk.device.graphics_queue, &present_info));
  VkResult result = vk.QueuePresentKHR(vk.device.graphics_queue, &present_info);
  // Info("vkQueuePresentKHR failed with: %s", vk_result_string(result));
  // Assert(result == VK_SUCCESS);
  
  vk.current_frame_idx = (vk.current_frame_idx + 1) % vk.frames_in_flight;
}

////////////////////////////////////////////////////////////////////////
// @Mesh

u32 vk_mesh_load(Mesh mesh) {
  VK_Buffer& vert_buff = vk.vert_buffer;
  u64 vert_size = mesh.vert_count*sizeof(Vertex);
  u64 vert_offset = vert_buff.size;
  vert_buff.size += vert_size;
  Range vert_range = { .offset = vert_offset, .size = vert_size };

  VK_Buffer& index_buff = vk.index_buffer;
  u64 index_size = mesh.index_count*sizeof(u32);
  u64 index_offset = index_buff.size;
  index_buff.size += index_size;
  Range index_range = { .offset = index_offset, .size = index_size };

  VK_Mesh vk_mesh = {
    .vert_count = mesh.vert_count,
    .vert_offset = vert_range.offset,
    .index_count = mesh.index_count,
    .index_offset = index_range.offset,
  };

  vk_buffer_upload_to_gpu(vk.vert_buffer, vert_range, mesh.vertices);
  if (mesh.indexes) {
    vk_buffer_upload_to_gpu(vk.index_buffer, index_range, mesh.indexes);
  }

  u32 id = vk.meshes.count;
  vk.meshes.append(vk_mesh);
  return id;
}

////////////////////////////////////////////////////////////////////////
// @Drawing

void vk_draw() {
  VkCommandBuffer cmd = vk_get_current_cmd();

  VK_Shader& shader = vk.shaders[0];
  vk.CmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.pipeline_layout, 0, 1, &vk.descriptor_sets, 0, null);

  for (VK_Shader shader : vk.shaders) {
    vk.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.pipeline);

    for (u32 id : shader.entities) {
      u32 mesh_id = vk.entities_to_mesh[id];
      VK_Mesh mesh = vk.meshes[mesh_id];
      PushConstant* push = &vk.push_constants[id];
      push->entity_idx = id;
      ShaderEntity& e = vk_get_entity(id);
      e.model = mat4_transform(entities_transforms[id]);

      vk.CmdPushConstants(cmd, vk.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), push);
      vk.CmdBindVertexBuffers(cmd, 0, 1, &vk.vert_buffer.handle, &mesh.vert_offset);
      
      if (mesh.index_count) {
        vk.CmdBindIndexBuffer(cmd, vk.index_buffer.handle, mesh.index_offset, VK_INDEX_TYPE_UINT32);
        vk.CmdDrawIndexed(cmd, mesh.index_count, 1, 0, 0, 0);
      } else {
        vk.CmdDraw(cmd, mesh.vert_count, 1, 0, 0);
      }
    }
  }

  vk.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.cubemap_shader.pipeline);
  VK_Mesh mesh = vk.meshes[0];
  vk.CmdBindVertexBuffers(cmd, 0, 1, &vk.vert_buffer.handle, &mesh.vert_offset);
  vk.CmdBindIndexBuffer(cmd, vk.index_buffer.handle, mesh.index_offset, VK_INDEX_TYPE_UINT32);
  vk.CmdDrawIndexed(cmd, mesh.index_count, 1, 0, 0, 0);
}

void vk_draw_screen() {
  VkCommandBuffer cmd = vk_get_current_cmd();

  // VkPipelineLayout pipeline_layout = vk.screen_shader.pipeline.pipeline_layout;
  // VkPipeline pipeline = vk.screen_shader.pipeline.handle;

  // vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &vk.screen_descriptor_sets[vk.current_frame], 0, null);
  // vk.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  // vk.CmdDraw(cmd, 3, 1, 0, 0);
}

// void vk_draw_compute() {
//   // vk_compute_descriptor_update();
//   VkCommandBuffer cmd = vk.compute_cmds[vk.current_frame];

//   VkCommandBufferBeginInfo beginInfo = {};
//   beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//   VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));
  
//   // UniformBufferObject* ubo; Assign(ubo, vk.compute_uniform_buffer.maped_memory);
//   // ubo->projection_view = *vk.projection_view;
//   // ubo->delta_time = delta_time;
  
//   vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, vk.compute_shader.pipeline.handle);
//   vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, vk.compute_shader.pipeline.pipeline_layout, 0, 1, &vk.compute_descriptor_sets[vk.current_frame], 0, null);

//   vkCmdDispatch(cmd, ParticleCount / 256, 1, 1);
  
//   VK_CHECK(vkEndCommandBuffer(cmd));
// }

// void vk_compute_descriptor_update() {
//   i32 i = vk.current_frame;
//   VkWriteDescriptorSet descriptor_writes[2];

//   VkDescriptorBufferInfo storage_buffer_info_last_frame = {
//     .buffer = vk.compute_storage_buffers[i].handle,
//     .offset = 0,
//     .range = sizeof(Particle) * ParticleCount,
//   };
//   descriptor_writes[0] = {
//     .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//     .dstSet = vk.compute_descriptor_sets[i],
//     .dstBinding = 1,
//     .dstArrayElement = 0,
//     .descriptorCount = 1,
//     .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//     .pBufferInfo = &storage_buffer_info_last_frame,
//   };

//   VkDescriptorBufferInfo storage_buffer_info_current_frame = {
//     .buffer = vk.compute_storage_buffers[(i + 1) % FramesInFlight].handle,
//     .offset = 0,
//     .range = sizeof(Particle) * ParticleCount,
//   };
//   descriptor_writes[1] = {
//     .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//     .dstSet = vk.compute_descriptor_sets[i],
//     .dstBinding = 2,
//     .dstArrayElement = 0,
//     .descriptorCount = 1,
//     .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//     .pBufferInfo = &storage_buffer_info_current_frame,
//   };

//   vkUpdateDescriptorSets(vkdevice, ArrayCount(descriptor_writes), descriptor_writes, 0, null);
// }

void vk_loader_load_core() {
  vk.lib = os_lib_open("libvulkan.so");
  VK_FUNCTION(GetInstanceProcAddr);
  VK_FUNCTION(EnumerateInstanceVersion);
  VK_FUNCTION(EnumerateInstanceExtensionProperties);
  VK_FUNCTION(EnumerateInstanceLayerProperties);
  VK_FUNCTION(CreateInstance);
}

void vk_loader_load_instance() {
  VK_INSTANCE_FUNCTION(GetDeviceProcAddr);
  VK_INSTANCE_FUNCTION(DestroyInstance);
  VK_INSTANCE_FUNCTION(EnumeratePhysicalDevices);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceProperties);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceFeatures);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceMemoryProperties);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceQueueFamilyProperties);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceFormatProperties);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceSurfaceCapabilitiesKHR);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceSurfaceFormatsKHR);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceSurfacePresentModesKHR);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceSurfaceSupportKHR);
  VK_INSTANCE_FUNCTION(EnumerateDeviceExtensionProperties);
  VK_INSTANCE_FUNCTION(CreateDevice);
  VK_INSTANCE_FUNCTION(DestroySurfaceKHR);
}

void vk_loader_load_device() {
  VK_DEVICE_FUNCTION(GetDeviceQueue);
  VK_DEVICE_FUNCTION(DeviceWaitIdle);
  VK_DEVICE_FUNCTION(CreateCommandPool);
  VK_DEVICE_FUNCTION(DestroyCommandPool);
  VK_DEVICE_FUNCTION(DestroyDevice);
  VK_DEVICE_FUNCTION(CreateSwapchainKHR);
  VK_DEVICE_FUNCTION(DestroySwapchainKHR);
  VK_DEVICE_FUNCTION(GetSwapchainImagesKHR);
  VK_DEVICE_FUNCTION(CreateImage);
  VK_DEVICE_FUNCTION(CreateImageView);
  VK_DEVICE_FUNCTION(DestroyImage);
  VK_DEVICE_FUNCTION(DestroyImageView);
  VK_DEVICE_FUNCTION(GetImageMemoryRequirements);
  VK_DEVICE_FUNCTION(AllocateMemory);
  VK_DEVICE_FUNCTION(FreeMemory);
  VK_DEVICE_FUNCTION(AllocateCommandBuffers);
  VK_DEVICE_FUNCTION(FreeCommandBuffers);
  VK_DEVICE_FUNCTION(BeginCommandBuffer);
  VK_DEVICE_FUNCTION(EndCommandBuffer);
  VK_DEVICE_FUNCTION(BindImageMemory);
  VK_DEVICE_FUNCTION(CreateSemaphore);
  VK_DEVICE_FUNCTION(DestroySemaphore);
  VK_DEVICE_FUNCTION(CreateFence);
  VK_DEVICE_FUNCTION(DestroyFence);
  VK_DEVICE_FUNCTION(WaitForFences);
  VK_DEVICE_FUNCTION(AcquireNextImageKHR);
  VK_DEVICE_FUNCTION(ResetFences);
  VK_DEVICE_FUNCTION(CreateDescriptorSetLayout);
  VK_DEVICE_FUNCTION(DestroyDescriptorSetLayout);
  VK_DEVICE_FUNCTION(CreateDescriptorPool);
  VK_DEVICE_FUNCTION(DestroyDescriptorPool);
  VK_DEVICE_FUNCTION(CreateShaderModule);
  VK_DEVICE_FUNCTION(DestroyShaderModule);
  VK_DEVICE_FUNCTION(CreateSampler);
  VK_DEVICE_FUNCTION(DestroySampler);
  VK_DEVICE_FUNCTION(CreateBuffer);
  VK_DEVICE_FUNCTION(DestroyBuffer);
  VK_DEVICE_FUNCTION(GetBufferMemoryRequirements);
  VK_DEVICE_FUNCTION(BindBufferMemory);
  VK_DEVICE_FUNCTION(MapMemory);
  VK_DEVICE_FUNCTION(UnmapMemory);
  VK_DEVICE_FUNCTION(FlushMappedMemoryRanges);
  VK_DEVICE_FUNCTION(CreatePipelineLayout);
  VK_DEVICE_FUNCTION(DestroyPipelineLayout);
  VK_DEVICE_FUNCTION(CreateGraphicsPipelines);
  VK_DEVICE_FUNCTION(DestroyPipeline);
  VK_DEVICE_FUNCTION(CmdBindPipeline);
  VK_DEVICE_FUNCTION(AllocateDescriptorSets);
  VK_DEVICE_FUNCTION(FreeDescriptorSets);
  VK_DEVICE_FUNCTION(UpdateDescriptorSets);

  VK_DEVICE_FUNCTION(CmdPipelineBarrier);
  VK_DEVICE_FUNCTION(CmdBlitImage);
  VK_DEVICE_FUNCTION(CmdCopyBuffer);
  VK_DEVICE_FUNCTION(CmdCopyBufferToImage);
  VK_DEVICE_FUNCTION(CmdCopyImageToBuffer);
  VK_DEVICE_FUNCTION(CmdExecuteCommands);
  VK_DEVICE_FUNCTION(CmdSetViewport);
  VK_DEVICE_FUNCTION(CmdSetScissor);
  VK_DEVICE_FUNCTION(CmdSetFrontFace);
  VK_DEVICE_FUNCTION(CmdSetCullMode);
  VK_DEVICE_FUNCTION(CmdSetStencilTestEnable);
  VK_DEVICE_FUNCTION(CmdSetDepthTestEnable);
  VK_DEVICE_FUNCTION(CmdSetDepthWriteEnable);
  VK_DEVICE_FUNCTION(CmdSetStencilReference);
  VK_DEVICE_FUNCTION(CmdSetStencilOp);
  VK_DEVICE_FUNCTION(CmdBeginRendering);
  VK_DEVICE_FUNCTION(CmdEndRendering);
  VK_DEVICE_FUNCTION(CmdSetStencilCompareMask);
  VK_DEVICE_FUNCTION(CmdSetStencilWriteMask);
  VK_DEVICE_FUNCTION(CmdClearColorImage);
  VK_DEVICE_FUNCTION(CmdClearDepthStencilImage);
  VK_DEVICE_FUNCTION(CmdSetPrimitiveTopology);
  VK_DEVICE_FUNCTION(CmdPushConstants);
  VK_DEVICE_FUNCTION(CmdBindVertexBuffers);
  VK_DEVICE_FUNCTION(CmdBindIndexBuffer);
  VK_DEVICE_FUNCTION(CmdDraw);
  VK_DEVICE_FUNCTION(CmdDrawIndexed);
  VK_DEVICE_FUNCTION(CmdBindDescriptorSets);

  VK_DEVICE_FUNCTION(QueueSubmit);
  VK_DEVICE_FUNCTION(QueueWaitIdle);
  VK_DEVICE_FUNCTION(QueuePresentKHR);
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
  required_validation_layer_names.append("VK_LAYER_KHRONOS_validation");
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
  required_extensions.append(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
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
          Error(String(callback_data->pMessage))
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

    v2i win_size = os_get_window_size();
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
    Debug("Command buffers created");
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
    vk.vert_buffer = vk_buffer_create(
      MB(1),
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vk.index_buffer = vk_buffer_create(
      MB(1),
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vk.stage_buffer = vk_buffer_create(
      MB(110),
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vk_buffer_map_memory(vk.stage_buffer, 0, vk.stage_buffer.cap);
    vk.storage_buffer = vk_buffer_create(
      MB(1),
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vk_buffer_map_memory(vk.storage_buffer, 0, vk.storage_buffer.cap);
  }

  vk_descriptor_init();
  vk_texture_init();
  vk_shader_pipeline_init();

  // Target texture render
  // {
  //   vk.texture_targets = push_array(vk.arena, VK_Texture, vk.images_in_flight);
  //   Loop (i, vk.images_in_flight) {
  //     vk.texture_targets[i].image = vk_image_create(
  //       VK_IMAGE_TYPE_2D,
  //       vk.width, vk.height,
  //       VK_FORMAT_B8G8R8A8_UNORM,
  //       // VK_FORMAT_R8G8B8A8_UNORM,
  //       VK_IMAGE_TILING_OPTIMAL,
  //       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
  //       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
  //       true,
  //       VK_IMAGE_ASPECT_COLOR_BIT);

  //     VkSamplerCreateInfo sampler_info = {
  //       .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
  //       .magFilter = VK_FILTER_LINEAR,
  //       .minFilter = VK_FILTER_LINEAR,
  //       .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
  //       .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
  //       .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
  //       .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
  //       .mipLodBias = 0.0f,
  //       .anisotropyEnable = VK_FALSE,
  //       .maxAnisotropy = 1,
  //       .compareEnable = VK_FALSE,
  //       .compareOp = VK_COMPARE_OP_ALWAYS,
  //       .minLod = 0.0f,
  //       .maxLod = 0.0f,
  //       .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
  //       .unnormalizedCoordinates = VK_FALSE,
  //     };
  //     VK_CHECK(vkCreateSampler(vkdevice, &sampler_info, vk.allocator, &vk.texture_targets[i].sampler));
  //   }

    // vk.offscreen_depth_buffer = vk_image_create(
    //   VK_IMAGE_TYPE_2D,
    //   vk.width,
    //   vk.height,
    //   vk.device.depth_format,
    //   VK_IMAGE_TILING_OPTIMAL,
    //   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    //   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    //   true,
    //   VK_IMAGE_ASPECT_DEPTH_BIT);
  // }

  Info("Vulkan renderer initialized");

  // {
  //   String name = "orange_lines_512.png";
  //   Texture texture = res_texture_load(name);
  //   vk_texture_load(texture);

  //   Shader shader = {
  //     .name = "texture_shader",
  //     .attribut = {3,3,2},
  //   };
  //   shader_create(shader);
  // }
}

void vk_shutdown() {
  VK_CHECK(vk.DeviceWaitIdle(vkdevice));
  
  Loop (i, vk.frames_in_flight) {
    vk.DestroySemaphore(vkdevice, vk.sync.image_available_semaphores[i], vk.allocator);
    vk.DestroySemaphore(vkdevice, vk.sync.render_complete_semaphores[i], vk.allocator);
    vk.DestroySemaphore(vkdevice, vk.sync.compute_complete_semaphores[i], vk.allocator);
    vk.DestroyFence(vkdevice, vk.sync.in_flight_fences[i], vk.allocator);
    vk_cmd_free(vk.device.cmd_pool, vk.cmds[i]);
    // vk_cmd_free(vk.device.cmd_pool, vk.compute_cmds[i]);
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
  VK_CHECK(vk.WaitForFences(vkdevice, 1, &vk.sync.in_flight_fences[vk.current_frame_idx], true, U64_MAX));

  v2i win_size = os_get_window_size();
  if (vk.width != win_size.x || vk.height != win_size.y) {
    vk.width = win_size.x;
    vk.height = win_size.y;
    VK_CHECK(vk.DeviceWaitIdle(vkdevice));
    vk_swapchain_recreate();
    // vk_resize_texture_target();
  }

  vk.current_image_idx = vk_swapchain_acquire_next_image_index(vk_get_current_image_available_semaphore());

  VkCommandBuffer cmd = vk_get_current_cmd();
  vk_cmd_begin(cmd);
  
  // VkViewport viewport = {
  //   .x = 0.0f,
  //   .y = 0,
  //   .width = (f32)vk.width,
  //   .height = (f32)vk.height,
  //   .minDepth = 0.0f,
  //   .maxDepth = 1.0f,
  // };
  // NOTE: we flip Y coordinate so Y:0 is on bottom of screen
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
}

void vk_end_frame() {
  VkCommandBuffer cmd = vk_get_current_cmd();
  vk_cmd_end(cmd);
  
  VK_CHECK(vk.ResetFences(vkdevice, 1, &vk.sync.in_flight_fences[vk.current_frame_idx]));
  
  // // compute
  // VkSubmitInfo compute_submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  // compute_submit_info.commandBufferCount = 1;
  // compute_submit_info.pCommandBuffers = &vk.compute_cmds[vk.current_frame].handle;
  // compute_submit_info.signalSemaphoreCount = 1;
  // compute_submit_info.pSignalSemaphores = &vk.sync.compute_complete_semaphores[vk.current_frame];
  
  // VK_CHECK(vkQueueSubmit(vk.device.graphics_queue, 1, &compute_submit_info, null));

  // graphics
  VkSemaphore semaphores_wait[] = {
    vk_get_current_image_available_semaphore(),
    // vk.sync.compute_complete_semaphores[vk.current_frame]
  };
  VkPipelineStageFlags sync_flags[] = {
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
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
      // Color
      vk_image_layout_transition(cmd, vk.swapchain.images[vk.current_image_idx], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      // Depth
      vk_image_layout_transition(cmd, vk.swapchain.depth_attachment, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
      // Render
      VkRenderingAttachmentInfo color_attachment = vk_default_color_attachment_info(vk.swapchain.views[vk.current_image_idx]);
      VkRenderingAttachmentInfo depth_attachment = vk_default_depth_attachment_info(vk.swapchain.depth_attachment.view);
      VkRenderingInfo render_info = vk_default_rendering_info(&color_attachment, &depth_attachment);
      vk.CmdBeginRendering(cmd, &render_info);
    } break;
    case RenderpassType_UI: {
    } break;
    case RenderpassType_Screen: {
    } break;
  }
  return;
}

void vk_end_renderpass(RenderpassType renderpass) {
  VkCommandBuffer cmd = vk_get_current_cmd();
  switch (renderpass) {
    case RenderpassType_World: {
      vk.CmdEndRendering(cmd);
      vk_image_layout_transition(cmd, vk.swapchain.images[vk.current_image_idx], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    } break;
    case RenderpassType_UI: {
    } break;
    case RenderpassType_Screen: {
    } break;
  }
  return;
}

void vk_update_transform(u32 entity_id, Transform trans) {
  PushConstant& push = vk_get_push_constant(entity_id);
  // push.model = mat4_transform(trans.pos, trans.rot, trans.scale);
}

////////////////////////////////////////////////////////////////////////
// Entity

u32 vk_make_renderable(u32 entity_id, u32 mesh_id, u32 shader_id, u32 texture_id) {
  vk.entities[entity_id].shader_handle = vk.shaders[shader_id].entities.append(entity_id);
  vk.entities_to_mesh[entity_id] = mesh_id;
  vk.entities_to_shader[entity_id] = shader_id;
  PushConstant& push = vk.push_constants[entity_id];
  push.texture_id = texture_id;
  return entity_id;
}

void vk_remove_renderable(u32 entity_id) {
  u32 shader_id = vk.entities_to_shader[entity_id];
  u32 shader_handler = vk.entities[entity_id].shader_handle;
  VK_Shader& shader = vk.shaders[shader_id];
  shader.entities.remove(shader_handler);
}

ShaderEntity& vk_get_entity(u32 entity_id) {
  return vk.entities_data[entity_id];
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
PushConstant& vk_get_push_constant(u32 entity_id) {
  return vk.push_constants[entity_id];
}

ShaderGlobalState* vk_get_shader_state() {
  return (ShaderGlobalState*)vk.storage_buffer.maped_memory;
}

// f32 ease_in_exp(f32 x) {
// 	return x <= 0.0 ? 0.0 : Pow(2, 10.0 * x - 10.0);
// }

// void compute_shader() {
//   Scratch scratch;
//   VK_ShaderCompute* shader = &vk.compute_shader;
//   // shader->stage.pipeline_shader_stage_create_info = vk_shader_module_create("compute", "comp", VK_SHADER_STAGE_COMPUTE_BIT);
//   // shader->pipeline_shader_stage_create_info = vk_shader_module_create("compute", "comp", VK_SHADER_STAGE_COMPUTE_BIT);
  
//   Particle* particles = push_array(scratch, Particle, ParticleCount);
  
//   // v2i frame = os_get_window_size();
//   f32 min = 0, max = 1;
  
//   Loop (i, ParticleCount) {
//     f32 r = 0.25f * rand_range_f32(min, max);
//     f32 theta = rand_range_f32(min, max) * Tau;          // angle around Y axis
//     f32 phi = Acos(2.0f * rand_range_f32(min, max) - 1.0f); // angle from Z axis (0..pi)

//     f32 x = r * Sin(phi) * Cos(theta);
//     f32 y = r * Sin(phi) * Sin(theta);
//     f32 z = r * Cos(phi);

//     particles[i].pos = v3(x, y, z);
//     particles[i].velocity = v3_norm(v3(x, y, z)) * 0.25f;
//     particles[i].color = v4(rand_range_f32(min, max), rand_range_f32(min, max), rand_range_f32(min, max), 1.0f);
//   }
  
//   u32 num_elipses = 14;
//   f32 tilt_step = radtodeg(0.1);
//   f32 radius_step = 1.1;
//   // Loop (i, num_elipses) {
//   //   f32 angle_offset = i * tilt_step;
//   //   f32 a = (i + 1) * radius_step / 5; // semi-major axis
//   //   // f32 b = a * 0.9f * i;              // semi-minor axis (flattening)
//   //   f32 b = a * 0.91f;              // semi-minor axis (flattening)

//   //   u32 stars_per_ellipse = ParticleCount / num_elipses;
//   //   Loop (j, stars_per_ellipse) {
//   //     f32 t = 2 * PI * j / stars_per_ellipse;
//   //     f32 rand = rand_range_f32(0,0.2);
//   //     f32 x = a * Cos(t) + rand;
//   //     f32 y = b * Sin(t) + rand;

//   //     // Apply rotation
//   //     f32 xr = Cos(angle_offset) * x - Sin(angle_offset) * y;
//   //     f32 yr = Sin(angle_offset) * x + Cos(angle_offset) * y;

//   //     particles[i*stars_per_ellipse + j].position = {xr,yr, 1};
//   //   }
//   // }

//   // Loop(i, num_elipses) {
//   //   f32 angle_offset = i * tilt_step;
//   //   f32 a = (i + 1) * radius_step / 5.0f; // semi-major axis
//   //   f32 b = a * 0.91f;                    // semi-minor axis

//   //   u32 stars_per_ellipse = ParticleCount / num_elipses;

//   //   Loop(j, stars_per_ellipse) {
//   //     f32 t = 2.0f * PI * j / stars_per_ellipse;

//   //     // Base ellipse position
//   //     f32 x = a * Cos(t);
//   //     f32 y = b * Sin(t);

//   //     // Add random jitter (scatter)
//   //     f32 rand_angle = rand_range_f32(-0.1f, 0.1f);  // scatter angle
//   //     f32 rand_radius = rand_range_f32(-0.1f, 0.2f); // scatter radius

//   //     x += rand_radius * Cos(t + rand_angle);
//   //     y += rand_radius * Sin(t + rand_angle);

//   //     // Rotate ellipse
//   //     f32 xr = Cos(angle_offset) * x - Sin(angle_offset) * y;
//   //     f32 yr = Sin(angle_offset) * x + Cos(angle_offset) * y;
//   //     f32 z = rand_range_f32(-0.1, 0.1);

//   //     particles[i * stars_per_ellipse + j].pos = {xr, yr, z};

//   //     f32 dist = v3_length(particles[i].pos);

//   //     f32 max_dist = 2;
//   //     f32 brightness = Clamp(0.0f, 1.0f - dist / max_dist, 1.0);

//   //     v4 baseColor = v4(0.5f, 0.5f, 0.5f, 1.0f);
      
//   //     particles[i*stars_per_ellipse + j].color = v4(baseColor.r * brightness, baseColor.g * brightness, baseColor.b * brightness, baseColor.a);
      
//   //     particles[i*stars_per_ellipse + j].pos = {xr,yr, 1};
//   //   }
//   // }

//   Range range = {0, ParticleCount * sizeof(Particle)};
//   vk_buffer_upload_to_gpu(vk.compute_storage_buffers[0], range, particles);
//   vk_buffer_upload_to_gpu(vk.compute_storage_buffers[1], range, particles);
  
//   VkDescriptorSetLayoutBinding layout_bindings[3] = {};
//   layout_bindings[0].binding = 0;
//   layout_bindings[0].descriptorCount = 1;
//   layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//   layout_bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
  
//   layout_bindings[1].binding = 1;
//   layout_bindings[1].descriptorCount = 1;
//   layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
//   layout_bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
  
//   layout_bindings[2].binding = 2;
//   layout_bindings[2].descriptorCount = 1;
//   layout_bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
//   layout_bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
  
//   VkDescriptorSetLayoutCreateInfo layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
//   layout_info.bindingCount = ArrayCount(layout_bindings);
//   layout_info.pBindings = layout_bindings;
//   VK_CHECK(vkCreateDescriptorSetLayout(vkdevice, &layout_info, vk.allocator, &vk.compute_descriptor_set_layout));
  
//   VkDescriptorSetLayout layouts[] = {
//     vk.compute_descriptor_set_layout, 
//     vk.compute_descriptor_set_layout, 
//   };
//   VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
//   alloc_info.descriptorPool = vk.descriptor_pool;
//   alloc_info.descriptorSetCount = ArrayCount(layouts);
//   alloc_info.pSetLayouts = layouts;
//   VK_CHECK(vkAllocateDescriptorSets(vkdevice, &alloc_info, vk.compute_descriptor_sets));

//   Loop (i, FramesInFlight) {
//     VkWriteDescriptorSet descriptor_writes[3];
    
//     VkDescriptorBufferInfo uniform_buffer_info = {};
//     // uniform_buffer_info.buffer = vk.compute_uniform_buffer.handle;
//     uniform_buffer_info.offset = 0;
//     uniform_buffer_info.range = sizeof(UniformBufferObject);
    
//     descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//     descriptor_writes[0].dstSet = vk.compute_descriptor_sets[i];
//     descriptor_writes[0].dstBinding = 0;
//     descriptor_writes[0].dstArrayElement = 0;
//     descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//     descriptor_writes[0].descriptorCount = 1;
//     descriptor_writes[0].pBufferInfo = &uniform_buffer_info;

//     VkDescriptorBufferInfo storage_buffer_info_last_frame{};
//     storage_buffer_info_last_frame.buffer = vk.compute_storage_buffers[(i + 1) % FramesInFlight].handle;
//     storage_buffer_info_last_frame.offset = 0;
//     storage_buffer_info_last_frame.range = sizeof(Particle) * ParticleCount;

//     descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//     descriptor_writes[1].dstSet = vk.compute_descriptor_sets[i];
//     descriptor_writes[1].dstBinding = 1;
//     descriptor_writes[1].dstArrayElement = 0;
//     descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
//     descriptor_writes[1].descriptorCount = 1;
//     descriptor_writes[1].pBufferInfo = &storage_buffer_info_last_frame;

//     VkDescriptorBufferInfo storage_buffer_info_current_frame{};
//     storage_buffer_info_current_frame.buffer = vk.compute_storage_buffers[i].handle;
//     storage_buffer_info_current_frame.offset = 0;
//     storage_buffer_info_current_frame.range = sizeof(Particle) * ParticleCount;

//     descriptor_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//     descriptor_writes[2].dstSet = vk.compute_descriptor_sets[i];
//     descriptor_writes[2].dstBinding = 2;
//     descriptor_writes[2].dstArrayElement = 0;
//     descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
//     descriptor_writes[2].descriptorCount = 1;
//     descriptor_writes[2].pBufferInfo = &storage_buffer_info_current_frame;

//     vkUpdateDescriptorSets(vkdevice, 3, descriptor_writes, 0, null);
//   }

//   VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
//   pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//   pipelineLayoutInfo.setLayoutCount = 1;
//   pipelineLayoutInfo.pSetLayouts = &vk.compute_descriptor_set_layout;
//   VK_CHECK(vkCreatePipelineLayout(vkdevice, &pipelineLayoutInfo, vk.allocator, &vk.compute_shader.pipeline.pipeline_layout));

//   VkComputePipelineCreateInfo pipeline_info{};
//   pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
//   pipeline_info.layout = vk.compute_shader.pipeline.pipeline_layout;
//   pipeline_info.stage = vk.compute_shader.pipeline_shader_stage_create_info;
//   VK_CHECK(vkCreateComputePipelines(vkdevice, VK_NULL_HANDLE, 1, &pipeline_info, vk.allocator, &vk.compute_shader.pipeline.handle));
  

// }

// void foo() {
//   Shader s = {
//     .has_position = true,
//     .has_color = true
//   };
//   vk_Shader* shader = &vk.graphics_shader_compute;
//   String stage_type_strs[] = { "vert", "frag"};
//   #define ShaderStageCount 2
//   VkShaderStageFlagBits stage_types[ShaderStageCount] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
//   Loop (i, 2) {
//     shader->stages[i] = vk_shader_module_create("compute", stage_type_strs[i], stage_types[i]);
//   }
  
//   #define AttributeCount 10
//   u32 stages_count = 0;
//   u32 vert_stride = 0;
//   u32 attribute_count = 0;
//   u32 offset = 0;
//   VkVertexInputAttributeDescription attribute_desriptions[AttributeCount];
//   if (s.has_position) {
//     attribute_desriptions[attribute_count].binding = 0;
//     attribute_desriptions[attribute_count].location = attribute_count;
//     attribute_desriptions[attribute_count].format = VK_FORMAT_R32G32B32_SFLOAT;
//     attribute_desriptions[attribute_count].offset = OffsetOf(Particle, pos);
//     ++attribute_count;
//   }
//   if (s.has_color) {
//     attribute_desriptions[attribute_count].binding = 0;
//     attribute_desriptions[attribute_count].location = attribute_count;
//     attribute_desriptions[attribute_count].format = VK_FORMAT_R32G32B32A32_SFLOAT;
//     attribute_desriptions[attribute_count].offset = OffsetOf(Particle, color);
//     ++attribute_count;
//   }
//   vert_stride = sizeof(Particle);
  
//   VkPipelineShaderStageCreateInfo stage_create_infos[ShaderStageCount] = {};
//   Loop (i, ShaderStageCount) {
//     stage_create_infos[i] = shader->stages[i].shader_state_create_info;
//   }
//   mem_copy(&shader->attribute_desriptions, &attribute_desriptions, attribute_count*sizeof(VkVertexInputAttributeDescription));
//   shader->pipeline = vk_pipeline_create(vert_stride, attribute_count, attribute_desriptions,
//                                         2, stage_create_infos, shader->topology, false, false);
// }
