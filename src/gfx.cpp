#include "com.h"

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
  void vk_surface_create() {
    VkWin32SurfaceCreateInfoKHR surface_create_info = {
      .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
      .hinstance = (HINSTANCE)os_get_handle_info(),
      .hwnd = (HWND)os_get_window_handle()
    };
    VK_CHECK(vkCreateWin32SurfaceKHR(g_st->vk.instance, &surface_create_info, g_st->vk.allocator, &g_st->vk.surface));
    Info("Vulkan win32 surface created");
  }
  #define VK_SURFACE_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
  
#elif OS_LINUX
  #if GFX_X11
    #include <xcb/xcb.h>
    #include <vulkan/vulkan_xcb.h>
    void vk_surface_create() {
      VK_State& g = st->vk;
      struct VK_Surface {
        xcb_connection_t* connection;
        xcb_window_t window;
      } vk_surface; 
      os_get_gfx_handlers(&vk_surface);
      VkXcbSurfaceCreateInfoKHR surfaceInfo = {
        .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        .connection = vk_surface.connection,
        .window = vk_surface.window,
      };
      PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)g.GetInstanceProcAddr(g.instance, "vkCreateXcbSurfaceKHR");
      VK_CHECK(vkCreateXcbSurfaceKHR(g.instance, &surfaceInfo, g.allocator, &g.surface));
      Info("Vulkan XCB surface created");
    }
    #define VK_SURFACE_NAME VK_KHR_XCB_SURFACE_EXTENSION_NAME

  #else
    #include <vulkan/vulkan_wayland.h>
    void vk_surface_create() {
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
      PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)g_st->vk.GetInstanceProcAddr(g_st->vk.instance, "vkCreateWaylandSurfaceKHR");
      VK_CHECK(vkCreateWaylandSurfaceKHR (g_st->vk.instance, &surfaceInfo, g_st->vk.allocator, &g_st->vk.surface));
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
        void* result = mem_alloc(g_st->vk.alloc, size);
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

String vk_result_str(VkResult result) {
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

#define _Def(val, def) (((val) == 0) ? (def) : (val))

VkImageCreateFlags vk_image_create_flags(Gfx_ImageType t) {
  switch (t) {
    InvalidDefaultCase;
    case Gfx_ImageType_2D:    return 0;
    case Gfx_ImageType_Cube:  return VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    case Gfx_ImageType_3D:    return 0;
    case Gfx_ImageType_Array: return 0;
  }
}

VkImageType vk_image_type(Gfx_ImageType t) {
  return t == Gfx_ImageType_3D ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
}

VkImageUsageFlags vk_image_usage(Gfx_ImageUsage usg) {
  VkImageUsageFlags res = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  res |= VK_IMAGE_USAGE_SAMPLED_BIT;
  if (FlagHas(usg, Gfx_ImageUsage_StorageImage)) {
    res |= VK_IMAGE_USAGE_STORAGE_BIT;
  }
  if (FlagHas(usg, Gfx_ImageUsage_ColorAttachment) || FlagHas(usg, Gfx_ImageUsage_ResolveAttachment)) {
    res |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }
  if (FlagHas(usg, Gfx_ImageUsage_DepthStencilAttachment)) {
    res |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }
  return res;
}

VkFormat vk_format(Gfx_PixelFormat fmt) {
  switch (fmt) {
    case Gfx_PixelFormat_None:           return VK_FORMAT_UNDEFINED;
    case Gfx_PixelFormat_BGRA8:          return VK_FORMAT_B8G8R8A8_UNORM;
    case Gfx_PixelFormat_BGRA8_SRGB:     return VK_FORMAT_B8G8R8A8_SRGB;
    case Gfx_PixelFormat_RGBA8:          return VK_FORMAT_R8G8B8A8_UNORM;
    case Gfx_PixelFormat_RGBA8_SRGB:     return VK_FORMAT_R8G8B8A8_SRGB;
    case Gfx_PixelFormat_Depth:          return VK_FORMAT_D32_SFLOAT;
    case Gfx_PixelFormat_DepthStencil:   return VK_FORMAT_D32_SFLOAT_S8_UINT;
    default:                             return VK_FORMAT_UNDEFINED;
  }
}

VkPrimitiveTopology vk_primitive_topology(Gfx_PrimitiveType t) {
  switch (t) {
    InvalidDefaultCase;
    case Gfx_PrimitiveType_Point:    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case Gfx_PrimitiveType_Line:     return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case Gfx_PrimitiveType_Triangle: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  }
}

VkCullModeFlags vk_cullmode(Gfx_CullMode cm) {
  switch (cm) {
    InvalidDefaultCase;
    case Gfx_CullMode_None:  return VK_CULL_MODE_NONE;
    case Gfx_CullMode_Front: return VK_CULL_MODE_FRONT_BIT;
    case Gfx_CullMode_Back:  return VK_CULL_MODE_BACK_BIT;
  }
}

VkFrontFace vk_frontface(Gfx_FaceWinding fw) {
  return fw == Gfx_FaceWinding_CCW ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
}

VkCompareOp vk_compare_op(Gfx_CompareOp op) {
  switch (op) {
    InvalidDefaultCase;
    case Gfx_CompareOp_Never:         return VK_COMPARE_OP_NEVER;
    case Gfx_CompareOp_Always:        return VK_COMPARE_OP_ALWAYS;
    case Gfx_CompareOp_Less:          return VK_COMPARE_OP_LESS;
    case Gfx_CompareOp_LessEqual:     return VK_COMPARE_OP_LESS_OR_EQUAL;
    case Gfx_CompareOp_Greater:       return VK_COMPARE_OP_GREATER;
    case Gfx_CompareOp_GreaterEqual:  return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case Gfx_CompareOp_Equal:         return VK_COMPARE_OP_EQUAL;
    case Gfx_CompareOp_NotEqual:      return VK_COMPARE_OP_NOT_EQUAL;
  }
}

VkStencilOp vk_stencil_op(Gfx_StencilOp op) {
  switch (op) {
    InvalidDefaultCase;
    case Gfx_StencilOp_Keep:      return VK_STENCIL_OP_KEEP;
    case Gfx_StencilOp_Zero:      return VK_STENCIL_OP_ZERO;
    case Gfx_StencilOp_Replace:   return VK_STENCIL_OP_REPLACE;
    case Gfx_StencilOp_IncrClamp: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case Gfx_StencilOp_DecrClamp: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case Gfx_StencilOp_Invert:    return VK_STENCIL_OP_INVERT;
    case Gfx_StencilOp_IncrWrap:  return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case Gfx_StencilOp_DecrWrap:  return VK_STENCIL_OP_DECREMENT_AND_WRAP;
  }
}

VkBlendOp vk_blend_op(Gfx_BlendOp op) {
  switch (op) {
    InvalidDefaultCase;
    case Gfx_BlendOp_Add:             return VK_BLEND_OP_ADD;
    case Gfx_BlendOp_Subtract:        return VK_BLEND_OP_SUBTRACT;
    case Gfx_BlendOp_ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
    case Gfx_BlendOp_Min:             return VK_BLEND_OP_MIN;
    case Gfx_BlendOp_Max:             return VK_BLEND_OP_MAX;
  }
}

VkBlendFactor vk_blend_factor(Gfx_BlendFactor f) {
  switch (f) {
    InvalidDefaultCase;
    case Gfx_BlendFactor_Zero:               return VK_BLEND_FACTOR_ZERO;
    case Gfx_BlendFactor_One:                return VK_BLEND_FACTOR_ONE;
    case Gfx_BlendFactor_SrcColor:           return VK_BLEND_FACTOR_SRC_COLOR;
    case Gfx_BlendFactor_OneMinusSrcColor:   return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case Gfx_BlendFactor_SrcAlpha:           return VK_BLEND_FACTOR_SRC_ALPHA;
    case Gfx_BlendFactor_OneMinusSrcAlpha:   return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case Gfx_BlendFactor_DstAlpha:           return VK_BLEND_FACTOR_DST_ALPHA;
    case Gfx_BlendFactor_OneMinusDstAlpha:   return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case Gfx_BlendFactor_SrcAlphaSaturated:  return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case Gfx_BlendFactor_BlendColor:         return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case Gfx_BlendFactor_OneMinusBlendColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
    case Gfx_BlendFactor_BlendAlpha:         return VK_BLEND_FACTOR_CONSTANT_ALPHA;
    case Gfx_BlendFactor_OneMinusBlendAlpha: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
    case Gfx_BlendFactor_Src1Color:          return VK_BLEND_FACTOR_SRC1_COLOR;
    case Gfx_BlendFactor_OneMinusSrc1Color:  return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    case Gfx_BlendFactor_Src1Alpha:          return VK_BLEND_FACTOR_SRC1_ALPHA;
    case Gfx_BlendFactor_OneMinusSrc1Alpha:  return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
  }
}

VkColorComponentFlags vk_color_write_mask(Gfx_ColorMask m) {
  u32 res = 0;
  if (m & Gfx_ColorMask_R) {
    res |= VK_COLOR_COMPONENT_R_BIT;
  }
  if (m & Gfx_ColorMask_G) {
    res |= VK_COLOR_COMPONENT_G_BIT;
  }
  if (m & Gfx_ColorMask_B) {
    res |= VK_COLOR_COMPONENT_B_BIT;
  }
  if (m & Gfx_ColorMask_A) {
    res |= VK_COLOR_COMPONENT_A_BIT;
  }
  return res;
}

// VkShaderStageFlags vk_shader_stage(Gfx_ShaderStage s) {
//   switch (s) {
//     InvalidDefaultCase;
//     case Gfx_ShaderStage_Vertex:
//     case Gfx_ShaderStage_Fragment:
//     case Gfx_ShaderStage_Compute:
//   }
// }

VkAttachmentLoadOp vk_load_op(Gfx_LoadAction a) {
  switch (a) {
    default:                      return VK_ATTACHMENT_LOAD_OP_LOAD;
    case Gfx_LoadAction_Clear:    return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case Gfx_LoadAction_DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  }
}

VkAttachmentStoreOp vk_store_op(Gfx_StoreAction a) {
  switch (a) {
    default:                    return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    case Gfx_StoreAction_Store: return VK_ATTACHMENT_STORE_OP_STORE;
  }
}

VkImageViewType vk_texture_image_view_type(Gfx_ImageType t) {
  switch (t) {
    InvalidDefaultCase;
    case Gfx_ImageType_2D:    return VK_IMAGE_VIEW_TYPE_2D;
    case Gfx_ImageType_Cube:  return VK_IMAGE_VIEW_TYPE_CUBE;
    case Gfx_ImageType_3D:    return VK_IMAGE_VIEW_TYPE_3D;
    case Gfx_ImageType_Array: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
  }
}

VkImageViewType vk_attachment_image_view_type(Gfx_ImageType t) {
  switch (t) {
    InvalidDefaultCase;
    case Gfx_ImageType_2D:    return VK_IMAGE_VIEW_TYPE_2D;
    case Gfx_ImageType_Cube:  return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    case Gfx_ImageType_3D:    return VK_IMAGE_VIEW_TYPE_2D;
    case Gfx_ImageType_Array: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
  }
}

VkFilter vk_sampler_minmag_filter(Gfx_Filter f) {
  switch (f) {
    InvalidDefaultCase;
    case Gfx_Filter_Nearest: return VK_FILTER_NEAREST;
    case Gfx_Filter_Linear:  return VK_FILTER_LINEAR;
  }
}

VkSamplerMipmapMode vk_sampler_mipmap_mode(Gfx_Filter f) {
  switch (f) {
    InvalidDefaultCase;
    case Gfx_Filter_Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case Gfx_Filter_Linear:  return VK_SAMPLER_MIPMAP_MODE_LINEAR;
  }
}

VkSamplerAddressMode vk_sampler_address_mode(Gfx_Wrap f) {
  switch (f) {
    InvalidDefaultCase;
    case Gfx_Wrap_Repeat:         return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case Gfx_Wrap_CalmpToEdge:    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case Gfx_Wrap_CalmpToBorder:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    case Gfx_Wrap_MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
  }
}

VkBorderColor vk_sampler_border_color(Gfx_BorderColor c) {
  switch (c) {
    InvalidDefaultCase;
    case Gfx_BorderColor_TransparentBlack: return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    case Gfx_BorderColor_OpaqueBlack:      return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    case Gfx_BorderColor_OpaqueWhite:      return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  }
}

////////////////////////////////////////////////////////////////////////
// @Misc

VkSemaphore vk_get_current_image_available_semaphore() { return st->vk.image_available_semaphores[st->vk.current_frame_idx]; }
VkSemaphore vk_get_current_render_complete_semaphore() { return st->vk.render_complete_semaphores[st->vk.current_image_idx]; }
VkCommandBuffer vk_get_current_cmd()                   { return st->vk.cmds_render[st->vk.current_frame_idx]; }

void vk_bind_pipeline(VkCommandBuffer cmd, VkPipeline pipeline) {
  st->vk.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

u32 vk_find_memory_idx(u32 type_filter, u32 property_flags) {
  VkPhysicalDeviceMemoryProperties memory_properties = st->vk.device.memory;
  u32 idx = -1;
  Loop (i, memory_properties.memoryTypeCount) {
    if (BitHas(type_filter, i) && FlagHas(memory_properties.memoryTypes[i].propertyFlags, property_flags)) {
      idx = i;
      break;
    }
  }
  Assert(idx != -1);
  return idx;
}

// VK_Semaphore vk_semaphore_make(u64 initial_counter) {
//   VK_State& g = st->vk;
//   VK_Semaphore res = {};
//   VkSemaphoreTypeCreateInfo timelineInfo = {
//     .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
//     .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
//     .initialValue = initial_counter
//   };
//   VkSemaphoreCreateInfo createInfo = {
//     .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
//     .pNext = &timelineInfo
//   };
//   g.CreateSemaphore(vkdevice, &createInfo, g.allocator, &res.h);
//   return res;
// }

// void vk_semaphore_wait(VK_Semaphore semaphore, u64 wait_counter) {
//   VK_State& g = st->vk;
//   u64 wait = semaphore.counter + wait_counter;
//   VkSemaphoreWaitInfo waitInfo = {
//     .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
//     .semaphoreCount = 1,
//     .pSemaphores = &semaphore.h,
//     .pValues = &wait
//   };
//   g.WaitSemaphores(vkdevice, &waitInfo, UINT64_MAX);
// }

////////////////////////////////////////////////////////////////////////
// @Cmd

VkCommandBuffer vk_cmd_alloc(VkCommandPool pool) {
  VkCommandBufferAllocateInfo allocate_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1,
  };
  VkCommandBuffer cmd;
  VK_CHECK(st->vk.AllocateCommandBuffers(vkdevice, &allocate_info, &cmd));
  return cmd;
}

void vk_cmd_free(VkCommandPool pool, VkCommandBuffer cmd) {
  st->vk.FreeCommandBuffers(vkdevice, pool, 1, &cmd);
}

void vk_cmd_begin(VkCommandBuffer cmd) {
  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  VK_CHECK(st->vk.BeginCommandBuffer(cmd, &begin_info));
}

void vk_cmd_end(VkCommandBuffer cmd) {
  VK_CHECK(st->vk.EndCommandBuffer(cmd));
}

void vk_cmd_submit(VkCommandBuffer cmd) {
  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
  };
  VK_CHECK(st->vk.QueueSubmit(st->vk.device.graphics_queue, 1, &submit_info, 0));
  VK_CHECK(st->vk.QueueWaitIdle(st->vk.device.graphics_queue));
}

void vk_cmd_end_submit(VkCommandBuffer cmd) {
  vk_cmd_end(cmd);
  vk_cmd_submit(cmd);
}

VkCommandBuffer vk_cmd_alloc_begin() {
  VkCommandBuffer result = vk_cmd_alloc(st->vk.device.cmd_pool);
  vk_cmd_begin(result);
  return result;
}

void vk_cmd_end_free(VkCommandBuffer cmd) {
  VK_State& g = st->vk;
  vk_cmd_end(cmd);
  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
  };
  VK_CHECK(g.QueueSubmit(g.device.graphics_queue, 1, &submit_info, 0));
  VK_CHECK(g.QueueWaitIdle(g.device.graphics_queue));
  vk_cmd_free(g.device.cmd_pool, cmd);
}

////////////////////////////////////////////////////////////////////////
// @Buffer

VK_Memory vk_mem_alloc(Gfx_MemType type, u64 size) {
  VK_State& g = st->vk;
  u32 mem_idx = 0;
  switch (type) {
    case Gfx_MemoryType_Gpu: {
      mem_idx = g.device.gpu_type_idx; 
    } break;
    case Gfx_MemoryType_Cpu: {
      mem_idx = g.device.cpu_type_idx; 
    } break;
  }
  VkMemoryAllocateInfo alloc_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = size,
    .memoryTypeIndex = mem_idx,
  };
  VK_Memory res = {.cap = size};
  VK_CHECK(g.AllocateMemory(vkdevice, &alloc_info, g.allocator, &res.h));
  if (type == Gfx_MemoryType_Cpu) {
    VK_CHECK(g.MapMemory(vkdevice, res.h, 0, size, 0, (void**)&res.mapped_mem));
  }
  return res;
}

VK_Buffer vk_buffer_alloc(u64 size, Gfx_BufferUsage usage, Gfx_MemType mem_type) {
  VK_State& g = st->vk;
  VkBufferUsageFlags buf_usage_flags = 0;
  if (FlagHas(usage, Gfx_BufferUsage_Vert)) buf_usage_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if (FlagHas(usage, Gfx_BufferUsage_Index)) buf_usage_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  if (FlagHas(usage, Gfx_BufferUsage_Dst)) buf_usage_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  if (FlagHas(usage, Gfx_BufferUsage_Src)) buf_usage_flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  if (FlagHas(usage, Gfx_BufferUsage_Storage)) buf_usage_flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  if (FlagHas(usage, Gfx_BufferUsage_Indirect)) buf_usage_flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
  VkBufferCreateInfo buffer_create_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = size,
    .usage = buf_usage_flags,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  VK_Buffer res = {.cap = size};
  VK_CHECK(g.CreateBuffer(vkdevice, &buffer_create_info, g.allocator, &res.h));

  u32 mem_prop_flags = 0;
  if (mem_type == Gfx_MemoryType_Gpu) mem_prop_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  if (mem_type == Gfx_MemoryType_Cpu) mem_prop_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  VkMemoryRequirements requirements;
  g.GetBufferMemoryRequirements(vkdevice, res.h, &requirements);
  // u32 mem_idx = vk_find_memory_idx(requirements.memoryTypeBits, mem_prop_flags);
  // VkMemoryAllocateInfo alloc_info = {
  //   .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
  //   .allocationSize = requirements.size,
  //   .memoryTypeIndex = mem_idx,
  // };
  // UnusedVariable(alloc_info);

  VK_Memory* mem = null;
  u64 offset = 0;
  switch (mem_type) {
    case Gfx_MemoryType_Gpu: {
      mem = &g.gpu_mem;
      offset = offset_push(mem->pos, size, requirements.alignment);
    } break;
    case Gfx_MemoryType_Cpu: {
      mem = &g.cpu_mem;
      offset = offset_push(mem->pos, size, requirements.alignment);
    } break;
  }
  VK_CHECK(st->vk.BindBufferMemory(vkdevice, res.h, mem->h, offset));
  res.base = Offset(mem->mapped_mem, offset);
  return res;
}

void vk_buffer_upload(VK_Buffer buffer, Region region, void* data) {
  VK_State& g = st->vk;
  MemCopy(g.stage_buffer.base, data, region.size);
  vk_cmd_begin(g.cmds_upload[0]);
  VkBufferCopy copy_region = {
    .srcOffset = 0,
    .dstOffset = region.offset,
    .size = region.size,
  };
  g.CmdCopyBuffer(g.cmds_upload[0], g.stage_buffer.h, buffer.h, 1, &copy_region);
  vk_cmd_end_submit(g.cmds_upload[0]);
}

Gfx_Shader gfx_shader_make(Gfx_ShaderDesc desc) {
  Scratch scratch;
  VK_State& g = st->vk;
  VK_Shader shader = {};
  shader.vert = desc.vert;
  shader.frag = desc.frag;
  shader.comp = desc.comp;
  if (!shader.vert || !shader.frag || shader.comp) {
    shader.vert = true;
    shader.frag = true;
  }
  VkShaderModuleCreateInfo module_info = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = desc.shader.size,
    .pCode = (u32*)desc.shader.str,
  };
  VK_CHECK(g.CreateShaderModule(vkdevice, &module_info, st->vk.allocator, &shader.h));
  Gfx_Shader res = {array_push(g.shaders, shader)};
  return res;
}

void gfx_pipeline_defaults(Gfx_PipelineDesc* desc) {
  desc->depth.pixel_format = _Def(desc->depth.pixel_format, Gfx_DefaultDepthFormat);
  desc->depth.compare = _Def(desc->depth.compare, Gfx_CompareOp_Always);

  desc->stencil.front.compare = _Def(desc->stencil.front.compare, Gfx_CompareOp_Always);
  desc->stencil.front.fail_op = _Def(desc->stencil.front.fail_op, Gfx_StencilOp_Keep);
  desc->stencil.front.depth_fail_op = _Def(desc->stencil.front.fail_op, Gfx_StencilOp_Keep);
  desc->stencil.front.pass_op = _Def(desc->stencil.front.fail_op, Gfx_StencilOp_Keep);
  desc->stencil.back.compare = _Def(desc->stencil.back.compare, Gfx_CompareOp_Always);
  desc->stencil.back.fail_op = _Def(desc->stencil.back.fail_op, Gfx_StencilOp_Keep);
  desc->stencil.back.depth_fail_op = _Def(desc->stencil.back.fail_op, Gfx_StencilOp_Keep);
  desc->stencil.back.pass_op = _Def(desc->stencil.back.fail_op, Gfx_StencilOp_Keep);

  desc->color_count = _Def(desc->color_count, 1);
  Loop (i, desc->color_count) {
    Gfx_ColorTargetState& col = desc->colors[i];
    col.pixel_format = _Def(col.pixel_format, Gfx_DefaultTextureTargetColorFormat);
    col.write_mask = _Def(col.write_mask, Gfx_ColorMask_RGBA);
    col.blend.src_factor_rgb = _Def(col.blend.src_factor_rgb, Gfx_BlendFactor_One);
    col.blend.dst_factor_rgb = _Def(col.blend.dst_factor_rgb, Gfx_BlendFactor_Zero);
    col.blend.op_rgb = _Def(col.blend.op_rgb, Gfx_BlendOp_Add);
    col.blend.src_factor_alpha = _Def(col.blend.src_factor_alpha, Gfx_BlendFactor_One);
    col.blend.dst_factor_alpha = _Def(col.blend.dst_factor_alpha, Gfx_BlendFactor_Zero);
    col.blend.op_alpha = _Def(col.blend.op_alpha, Gfx_BlendOp_Add);
  }
  desc->primitive_type = _Def(desc->primitive_type, Gfx_PrimitiveType_Triangle);
  desc->cull_mode = _Def(desc->cull_mode, Gfx_CullMode_None);
  desc->face_winding = _Def(desc->face_winding, Gfx_FaceWinding_CW);
  desc->sample_count = _Def(desc->sample_count, Gfx_DefaultSampleCount);
}

Gfx_Pipeline gfx_pipeline_make(Gfx_PipelineDesc desc) {
  Scratch scratch;
  VK_State& g = st->vk;
  gfx_pipeline_defaults(&desc);
  VK_Shader shader = g.shaders[desc.shader.id];
  VK_Pipeline pipeline = {};

  ///////////////////////////////////
  // Compute pipeline
  if (desc.compute) {
    VkComputePipelineCreateInfo pip_create_info = {
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
      .stage = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = shader.h,
        .pName = "cs_main",
      },
      .layout = g.pipeline_layout,
    };
    VkPipeline vk_pipeline;
    VK_CHECK(g.CreateComputePipelines(vkdevice, VK_NULL_HANDLE, 1, &pip_create_info, 0, &vk_pipeline));
    pipeline.h = vk_pipeline;
    Gfx_Pipeline res = {array_push(g.pipelines, pipeline)};
    return res;
  }

  ///////////////////////////////////
  // Graphics pipeline
  pipeline = {
    .depth = desc.depth,
    .stencil = desc.stencil,
    .color_count = desc.color_count,
    .primitive_type = desc.primitive_type,
    .cull_mode = desc.cull_mode,
    .face_winding = desc.face_winding,
    .sample_count = desc.sample_count,
    .blend_color = desc.blend_color,
    .alpha_to_coverage_enabled = desc.alpha_to_coverage_enabled,
  };
  ArrayCopy(pipeline.colors, desc.colors);

  // Dynamic rendering
  VkFormat color_formats[Gfx_MaxColorAttachments] = {};
  Loop (i, Gfx_MaxColorAttachments) {
    color_formats[i] = vk_format(desc.colors[i].pixel_format);
  }
  VkPipelineRenderingCreateInfo rendering_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .colorAttachmentCount = desc.color_count,
    .pColorAttachmentFormats = color_formats,
    .depthAttachmentFormat = vk_format(desc.depth.pixel_format),
    .stencilAttachmentFormat = desc.depth.pixel_format == Gfx_PixelFormat_DepthStencil ? vk_format(desc.depth.pixel_format) : VK_FORMAT_UNDEFINED,
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
  VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = vk_primitive_topology(desc.primitive_type),
    .primitiveRestartEnable = VK_FALSE,
  };
  
  // Viewport
  VkPipelineViewportStateCreateInfo viewport_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .scissorCount = 1,
  };
  
  // Rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = vk_cullmode(desc.cull_mode),
    .frontFace = vk_frontface(desc.face_winding),
    .depthBiasEnable = (u32)desc.depth.bias != 0,
    .depthBiasConstantFactor = desc.depth.bias,
    .depthBiasClamp = desc.depth.bias_clamp,
    .depthBiasSlopeFactor = desc.depth.bias_slope_scale,
    .lineWidth = 1.0f,
  };
  
  // Multisampling
  VkPipelineMultisampleStateCreateInfo multisampling_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .rasterizationSamples = (VkSampleCountFlagBits)desc.sample_count,
    .alphaToCoverageEnable = (u32)desc.alpha_to_coverage_enabled,
  };

  // Depth and stencil testing
  VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthTestEnable = desc.depth.compare != Gfx_CompareOp_Always,
    .depthWriteEnable = (u32)desc.depth.write_enabled,
    .depthCompareOp = vk_compare_op(desc.depth.compare),
    .depthBoundsTestEnable = false,
    .front = {
      .failOp = vk_stencil_op(desc.stencil.front.fail_op),
      .passOp = vk_stencil_op(desc.stencil.front.pass_op),
      .depthFailOp = vk_stencil_op(desc.stencil.front.depth_fail_op),
      .compareOp = vk_compare_op(desc.stencil.front.compare),
      .compareMask = desc.stencil.read_mask,
      .writeMask = desc.stencil.write_mask,
      .reference = desc.stencil.ref,
    },
    .back = {
      .failOp = vk_stencil_op(desc.stencil.back.fail_op),
      .passOp = vk_stencil_op(desc.stencil.back.pass_op),
      .depthFailOp = vk_stencil_op(desc.stencil.back.depth_fail_op),
      .compareOp = vk_compare_op(desc.stencil.back.compare),
      .compareMask = desc.stencil.read_mask,
      .writeMask = desc.stencil.write_mask,
      .reference = desc.stencil.ref,
    },
  };
  
  // Blending
  VkPipelineColorBlendAttachmentState attachment_states[Gfx_MaxColorAttachments] = {};
  Loop (i, desc.color_count) {
    attachment_states[i].blendEnable = desc.colors[i].blend.enabled;
    attachment_states[i].srcColorBlendFactor = vk_blend_factor(desc.colors[i].blend.src_factor_rgb);
    attachment_states[i].dstColorBlendFactor = vk_blend_factor(desc.colors[i].blend.dst_factor_rgb);
    attachment_states[i].colorBlendOp = vk_blend_op(desc.colors[i].blend.op_rgb);
    attachment_states[i].srcAlphaBlendFactor = vk_blend_factor(desc.colors[i].blend.src_factor_alpha);
    attachment_states[i].dstAlphaBlendFactor = vk_blend_factor(desc.colors[i].blend.dst_factor_alpha);
    attachment_states[i].alphaBlendOp = vk_blend_op(desc.colors[i].blend.op_alpha);
    attachment_states[i].colorWriteMask = vk_color_write_mask(desc.colors[i].write_mask);
  }
  VkPipelineColorBlendStateCreateInfo color_blend_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .attachmentCount = desc.color_count,
    .pAttachments = attachment_states,
    .blendConstants[0] = desc.blend_color.v[0],
    .blendConstants[1] = desc.blend_color.v[1],
    .blendConstants[2] = desc.blend_color.v[2],
    .blendConstants[3] = desc.blend_color.v[3],
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

  // Shader stages
  u32 stages_count = 0;
  VkPipelineShaderStageCreateInfo stages[2] = {};
  if (shader.vert) {
    stages[stages_count] = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = shader.h,
      .pName = "vs_main",
    };
  }
  if (shader.frag) {
    stages[stages_count] = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = shader.h,
      .pName = "fs_main",
    };
  }
  
  // Pipeline
  VkGraphicsPipelineCreateInfo pipeline_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = &rendering_info,
    .stageCount = stages_count,
    .pStages = stages,
    .pVertexInputState = &vertex_input_state,
    .pInputAssemblyState = &input_assembly_state,
    .pViewportState = &viewport_state,
    .pRasterizationState = &rasterizer_state,
    .pMultisampleState = &multisampling_state_info,
    .pDepthStencilState = &depth_stencil_state_info,
    .pColorBlendState = &color_blend_state_info,
    .pDynamicState = &dynamic_state_info,
    .layout = g.pipeline_layout,
  };
  VkPipeline pip;
  VK_CHECK(g.CreateGraphicsPipelines(vkdevice, VK_NULL_HANDLE, 1, &pipeline_info, st->vk.allocator, &pip));
  pipeline.h = pip;
  Gfx_Pipeline res = {array_push(g.pipelines, pipeline)};
  return res;
}

void gfx_image_defaults(Gfx_ImageDesc* desc) {
  desc->type = _Def(desc->type, Gfx_ImageType_2D);
  desc->usage = FlagSet(desc->usage, Gfx_ImageUsage_Immutable);
  desc->slices_count = _Def(desc->slices_count, desc->type == Gfx_ImageType_Cube ? 6 : 1);
  if (FlagHas(desc->usage, Gfx_ImageUsage_ColorAttachment)) {
    desc->pixel_format = _Def(desc->pixel_format, Gfx_DefaultTextureTargetColorFormat);
    desc->sample_count = _Def(desc->sample_count, Gfx_DefaultSampleCount);
  } else if (FlagHas(desc->usage, Gfx_ImageUsage_DepthStencilAttachment)) {
    desc->pixel_format = _Def(desc->pixel_format, Gfx_DefaultDepthFormat);
    desc->sample_count = _Def(desc->sample_count, Gfx_DefaultSampleCount);
  } else {
    desc->pixel_format = _Def(desc->pixel_format, Gfx_PixelFormat_RGBA8);
    desc->sample_count = _Def(desc->sample_count, 1);
  }
}

Gfx_Image gfx_image_make(Gfx_ImageDesc desc) {
  VK_State& g = st->vk;
  gfx_image_defaults(&desc);
  VK_Image image = {
    .type = desc.type,
    .usage = desc.usage,
    .width = desc.width,
    .height = desc.height,
    .slices_count = desc.slices_count,
    .mipmaps_count = 1,
    .pixel_format = desc.pixel_format,
    .sample_count = desc.sample_count,
  };
  if (desc.mipmaps) {
    image.mipmaps_count = Floor(Log2(Max(image.width, image.height))) + 1;
  }
  VkImageCreateInfo image_info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .flags = vk_image_create_flags(desc.type),
    .imageType = vk_image_type(desc.type),
    .format = vk_format(desc.pixel_format),
    .extent = {desc.width, desc.height},
    .mipLevels = image.mipmaps_count,
    .samples = (VkSampleCountFlagBits)desc.sample_count,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .usage = vk_image_usage(desc.usage),
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };
  if (desc.type == Gfx_ImageType_3D) {
    image_info.extent.depth = desc.slices_count;
    image_info.arrayLayers = 1;
  } else {
    image_info.extent.depth = 1;
    image_info.arrayLayers = desc.slices_count;
  }
  VK_CHECK(g.CreateImage(vkdevice, &image_info, g.allocator, &image.h));
  VkMemoryRequirements requirements;
  g.GetImageMemoryRequirements(vkdevice, image.h, &requirements);
  VK_Memory& mem = g.gpu_mem;
  u64 offset = offset_push(mem.pos, requirements.size, requirements.alignment);
  VK_CHECK(g.BindImageMemory(vkdevice, image.h, mem.h, offset));

  {
    VkCommandBuffer cmd = g.cmds_upload[0];
    vk_cmd_begin(cmd);
    vk_image_layout_transition(cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    MemCopy(g.stage_buffer.base, desc.data, desc.width * desc.height * 4);
    vk_image_upload_to_gpu(cmd, image);
    if (desc.mipmaps) {
      vk_texture_generate_mipmaps(image);
    }
    vk_cmd_end_submit(cmd);
  }

  Gfx_Image res = {array_push(g.images, image)};
  return res;
}

Gfx_View gfx_view_make(Gfx_ViewDesc desc) {
  VK_State& g = st->vk;
  Gfx_ViewType type = Gfx_ViewType_Invalid;
  if (desc.texture.image.id != Gfx_InvalidId) {
    type = Gfx_ViewType_Texture;
  }
  if (desc.color_attachment.image.id != Gfx_InvalidId) {
    Assert(type == Gfx_ViewType_Invalid);
    type = Gfx_ViewType_ColorAttachment;
  }
  if (desc.resolve_attachment.image.id != Gfx_InvalidId) {
    Assert(type == Gfx_ViewType_Invalid);
    type = Gfx_ViewType_ResolveAttachment;
  }
  if (desc.depth_stencil_attachment.image.id != Gfx_InvalidId) {
    Assert(type == Gfx_ViewType_Invalid);
    type = Gfx_ViewType_DepthStencilAttachment;
  }
  Assert(type != Gfx_ViewType_Invalid);

  VK_View view = {.type = type};
  switch (type) {
    InvalidDefaultCase;
    case Gfx_ViewType_Texture: {
      VK_Image img = g.images[desc.texture.image.id];
      view.ref = img;
      view.mip_level = desc.texture.mip_level;
      view.mip_level_count = _Def(desc.texture.mip_level_count, img.mipmaps_count - desc.texture.mip_level);
      view.slice = desc.texture.slice;
      switch (img.type) {
        InvalidDefaultCase;
        case Gfx_ImageType_2D:    view.slice_count = 1; break;
        case Gfx_ImageType_Cube:  view.slice_count = 6; break;
        case Gfx_ImageType_3D:    view.slice_count = 1; break;
        case Gfx_ImageType_Array: view.slice_count = _Def(desc.texture.slice_count, img.slices_count - view.slice);
      }
    } break;
    case Gfx_ViewType_ColorAttachment: {
      VK_Image img = g.images[desc.color_attachment.image.id];
      view.ref = img;
      view.mip_level = desc.color_attachment.mip_level;
      view.mip_level_count = 1;
      view.slice = desc.color_attachment.slice;
      view.slice_count = 1;
    } break;
    case Gfx_ViewType_ResolveAttachment: {
      VK_Image img = g.images[desc.resolve_attachment.image.id];
      view.ref = img;
      view.mip_level = desc.resolve_attachment.mip_level;
      view.mip_level_count = 1;
      view.slice = desc.resolve_attachment.slice;
      view.slice_count = 1;
    } break;
    case Gfx_ViewType_DepthStencilAttachment: {
      VK_Image img = g.images[desc.depth_stencil_attachment.image.id];
      view.ref = img;
      view.mip_level = desc.depth_stencil_attachment.mip_level;
      view.mip_level_count = 1;
      view.slice = desc.depth_stencil_attachment.slice;
      view.slice_count = 1;
    } break;
  }

  VK_Image img = view.ref;
  VkImageViewCreateInfo view_info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .image = img.h,
    .viewType = view.type == Gfx_ViewType_Texture ? vk_texture_image_view_type(img.type) : vk_attachment_image_view_type(img.type),
    .format = vk_format(img.pixel_format),
    .subresourceRange = {
      .baseMipLevel = view.mip_level,
      .levelCount = view.mip_level_count,
      .baseArrayLayer = view.slice,
      .layerCount = view.slice_count,
    },
  };
  if (view.type == Gfx_ViewType_DepthStencilAttachment) {
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (img.pixel_format == Gfx_PixelFormat_DepthStencil) {
      view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  } else {
    view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
  }
  VK_CHECK(g.CreateImageView(vkdevice, &view_info, g.allocator, &view.h));
  Gfx_View res = {array_push(g.views, view)};
  return res;
}

void gfx_sampler_defaults(Gfx_SamplerDesc* desc) {
  desc->min_filter = _Def(desc->min_filter, Gfx_Filter_Nearest);
  desc->mag_filter = _Def(desc->mag_filter, Gfx_Filter_Nearest);
  desc->mipmap_filter = _Def(desc->mipmap_filter, Gfx_Filter_Nearest);
  desc->wrap_u = _Def(desc->wrap_u, Gfx_Wrap_Repeat);
  desc->wrap_v = _Def(desc->wrap_v, Gfx_Wrap_Repeat);
  desc->wrap_w = _Def(desc->wrap_w, Gfx_Wrap_Repeat);
  desc->max_lod = _Def(desc->max_lod, VK_LOD_CLAMP_NONE);
  desc->border_color = _Def(desc->border_color, Gfx_BorderColor_OpaqueBlack);
  desc->compare = _Def(desc->compare, Gfx_CompareOp_Never);
  desc->max_anisotropy = _Def(desc->max_anisotropy, 1);
}

Gfx_Sampler gfx_sampler_make(Gfx_SamplerDesc desc) {
  VK_State& g = st->vk;
  gfx_sampler_defaults(&desc);
  VK_Sampler sampler = {
    .min_filter = desc.min_filter,
    .mag_filter = desc.mag_filter,
    .mipmap_filter = desc.mipmap_filter,
    .wrap_u = desc.wrap_u,
    .wrap_v = desc.wrap_v,
    .wrap_w = desc.wrap_w,
    .min_lod = desc.min_lod,
    .max_lod = desc.max_lod,
    .border_color = desc.border_color,
    .compare = desc.compare,
    .max_anisotropy = desc.max_anisotropy,
  };
  VkSamplerCreateInfo sampler_info = {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .magFilter = vk_sampler_minmag_filter(desc.mag_filter),
    .minFilter = vk_sampler_minmag_filter(desc.min_filter),
    .mipmapMode = vk_sampler_mipmap_mode(desc.mipmap_filter),
    .addressModeU = vk_sampler_address_mode(desc.wrap_u),
    .addressModeV = vk_sampler_address_mode(desc.wrap_v),
    .addressModeW = vk_sampler_address_mode(desc.wrap_w),
    .mipLodBias = 0.0f,
    .anisotropyEnable = desc.max_anisotropy > 1 ? true : false,
    .maxAnisotropy = (f32)desc.max_anisotropy,
    .compareEnable = desc.compare != Gfx_CompareOp_Never ? true : false,
    .compareOp = vk_compare_op(desc.compare),
    .minLod = desc.max_lod,
    .maxLod = desc.max_lod,
    .borderColor = vk_sampler_border_color(desc.border_color),
  };
  VK_CHECK(g.CreateSampler(vkdevice, &sampler_info, g.allocator, &sampler.h));
  Gfx_Sampler res = {array_push(g.samplers, sampler)};
  return res;
}

void gfx_pass_begin(Gfx_Pass pass) {
  
}
