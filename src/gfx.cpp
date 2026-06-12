#include "com.h"

VkImageAspectFlags vk_aspect_mask(Gfx_PixelFormat fmt) {
  switch (fmt) {
    default:                           return VK_IMAGE_ASPECT_COLOR_BIT;
    case Gfx_PixelFormat_Depth:        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case Gfx_PixelFormat_DepthStencil: return VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
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
  if (usg == Gfx_ImageUsage_StorageImage) {
    res |= VK_IMAGE_USAGE_STORAGE_BIT;
  }
  if (usg == Gfx_ImageUsage_ColorAttachment || usg == Gfx_ImageUsage_ResolveAttachment) {
    res |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }
  if (usg == Gfx_ImageUsage_DepthStencilAttachment) {
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

b32 vk_barrier_needed(VK_Access prev, VK_Access next) {
  VK_Access read_bits = VK_Access_IndexBuffer | VK_Access_VertBuffer | VK_Access_StorageBuffer_RO | VK_Access_Texture | VK_Access_Present;
  return !((prev & ~read_bits) == 0) && ((next & ~read_bits) == 0);
}

VkPipelineStageFlags2 vk_stage_mask(VK_Access access, b32 is_dst_access) {
  access &= ~VK_Access_Discard;
  if (is_dst_access) {
    Assert(access != VK_Access_None);
  }
  VkPipelineStageFlags2 f = 0;
  if (access == VK_Access_None) {
    return VK_PIPELINE_STAGE_2_NONE;
  }
  if (access & VK_Access_Present) {
    return VK_PIPELINE_STAGE_2_NONE;
  }
  if (access & VK_Access_Staging) {
    f |= VK_PIPELINE_STAGE_2_COPY_BIT;
  }
  if (access & VK_Access_VertBuffer) {
    f |= VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
  }
  if (access & VK_Access_IndexBuffer) {
    f |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
  }
  if (access & (VK_Access_StorageBuffer_RO|VK_Access_Texture)) {
    f |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
  }
  if (access & VK_Access_StorageBuffer_RW) {
    f |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
  }
  if (access & VK_Access_StorageImage) {
    f |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
  }
  if (access & VK_Access_ColorAttachment) {
    f |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  }
  if (access & VK_Access_ResolveAttachment) {
    f |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  }
  if (access & (VK_Access_DepthAttachment|VK_Access_StencilAttachment)) {
    f |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
  }
  Assert(f != 0);
  return f;
}

VkPipelineStageFlags2 vk_src_stage_mask(VK_Access access) {
  return vk_stage_mask(access, false);
}

VkPipelineStageFlags2 vk_dst_stage_mask(VK_Access access) {
  return vk_stage_mask(access, true);
}

VkAccessFlags2 vk_access_mask(VK_Access access, b32 is_dst_access) {
  access &= ~VK_Access_Discard;
  if (access == VK_Access_None) {
    return VK_ACCESS_2_NONE;
  }
  if (access & VK_Access_Present) {
    return VK_ACCESS_2_NONE;
  }
  VkAccessFlags2 f = VK_ACCESS_2_NONE;
  if (is_dst_access) {
    if (access & VK_Access_VertBuffer) {
      f |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
    }
    if (access & VK_Access_IndexBuffer) {
      f |= VK_ACCESS_2_INDEX_READ_BIT;
    }
    if (access & VK_Access_StorageBuffer_RO) {
      f |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
    }
    if (access & VK_Access_Texture) {
      f |= VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    }
  }
  if (access & VK_Access_Staging) {
    f |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
  }
  if (access & VK_Access_StorageBuffer_RW) {
    f |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
  }
  if (access & VK_Access_StorageImage) {
    f |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
  }
  if (access & VK_Access_ColorAttachment) {
    f |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
  }
  if (access & VK_Access_ResolveAttachment) {
    f |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
  }
  if (access & (VK_Access_DepthAttachment | VK_Access_StencilAttachment)) {
    f |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    if (is_dst_access) {
      f |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    }
  }
  return f;
}

VkAccessFlags2 vk_src_access_mask(VK_Access access) {
  return vk_access_mask(access, false);
}

VkAccessFlags2 vk_dst_access_mask(VK_Access access) {
  return vk_access_mask(access, true);
}

VkImageLayout vk_image_layout(VK_Access access) {
  if (access & VK_Access_Discard) {
    return VK_IMAGE_LAYOUT_UNDEFINED;
  }
  switch (access) {
    InvalidDefaultCase;
    case VK_Access_None:               return VK_IMAGE_LAYOUT_UNDEFINED;
    case VK_Access_Staging:            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case VK_Access_Texture:            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case VK_Access_StorageImage:       return VK_IMAGE_LAYOUT_GENERAL;
    case VK_Access_ColorAttachment:
    case VK_Access_ResolveAttachment:
    case VK_Access_DepthAttachment:
    case VK_Access_DepthAttachment |
         VK_Access_StencilAttachment:  return VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    case VK_Access_Present:            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  }
}

void vk_swapchain_beginpass_barrier(VkImage vkimg, VK_Access pass_access) {
  VK_State& g = st->vk;
  VkImageMemoryBarrier2 barrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    .srcStageMask = vk_src_stage_mask(pass_access),
    .srcAccessMask = vk_src_access_mask(pass_access),
    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .dstStageMask = vk_src_stage_mask(pass_access),
    .dstAccessMask = vk_src_access_mask(pass_access),
    .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = vkimg,
    .subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .levelCount = 1,
      .layerCount = 1,
    },
  };
  VkDependencyInfo dep_info = {
    .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    .imageMemoryBarrierCount = 1,
    .pImageMemoryBarriers = &barrier,
  };
  g.CmdPipelineBarrier2(vk_get_cur_cmd(), &dep_info);
}

void vk_swapchain_end_barrier(VkImage vkimg, VK_Access pass_access) {
  VK_State& g = st->vk;
  VkImageMemoryBarrier2 barrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    .srcStageMask = vk_src_stage_mask(pass_access),
    .srcAccessMask = vk_src_access_mask(pass_access),
    .oldLayout = vk_image_layout(pass_access),
    .dstStageMask = vk_src_stage_mask(pass_access),
    .dstAccessMask = vk_src_access_mask(pass_access),
    .newLayout = vk_image_layout(pass_access),
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = vkimg,
    .subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .levelCount = 1,
      .layerCount = 1,
    },
  };
  VkDependencyInfo dep_info = {
    .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    .imageMemoryBarrierCount = 1,
    .pImageMemoryBarriers = &barrier,
  };
  g.CmdPipelineBarrier2(vk_get_cur_cmd(), &dep_info);
}

void vk_image_barrier(VK_Image* img, VK_Access new_access) {
  VK_State& g = st->vk;
  if (!vk_barrier_needed(img->cur_access, new_access)) return;
  VkImageMemoryBarrier2 barrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    .srcStageMask = vk_src_stage_mask(img->cur_access),
    .srcAccessMask = vk_src_access_mask(img->cur_access),
    .oldLayout = vk_image_layout(img->cur_access),
    .dstStageMask = vk_src_stage_mask(new_access),
    .dstAccessMask = vk_src_access_mask(new_access),
    .newLayout = vk_image_layout(new_access),
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = img->h,
    .subresourceRange = {
      .aspectMask = vk_aspect_mask(img->pixel_format),
      .levelCount = VK_REMAINING_MIP_LEVELS,
      .layerCount = VK_REMAINING_ARRAY_LAYERS,
    },
  };
  VkDependencyInfo dep_info = {
    .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    .imageMemoryBarrierCount = 1,
    .pImageMemoryBarriers = &barrier,
  };
  g.CmdPipelineBarrier2(vk_get_cur_cmd(), &dep_info);
}

void vk_buffer_barrier(VK_BufferRegion buf, VK_Access new_access) {
  VK_State& g = st->vk;
  if (!vk_barrier_needed(buf.cur_access, new_access)) return;
  VkBufferMemoryBarrier2 barrier = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
    .srcStageMask = vk_src_stage_mask(buf.cur_access),
    .srcAccessMask = vk_src_access_mask(buf.cur_access),
    .dstStageMask = vk_src_stage_mask(new_access),
    .dstAccessMask = vk_src_access_mask(new_access),
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .buffer = buf.h,
    .offset = buf.base,
    .size = buf.pos,
  };
  VkDependencyInfo dep_info = {
    .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    .imageMemoryBarrierCount = 1,
    .pBufferMemoryBarriers = &barrier,
  };
  g.CmdPipelineBarrier2(vk_get_cur_cmd(), &dep_info);
}

void vk_init_color_attachment_info(VkRenderingAttachmentInfo* info, Gfx_ColorAttachmentAction action, VkImageView color_view, VkImageView resolve_view) {
  info->sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  info->imageView = color_view;
  info->imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  if (resolve_view) {
      info->resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
      info->resolveImageView = resolve_view;
      info->resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  } else {
      info->resolveMode = VK_RESOLVE_MODE_NONE;
      info->resolveImageView = 0;
      info->resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  }
  info->loadOp = vk_load_op(action.load_action);
  info->storeOp = vk_store_op(action.store_action);
  info->clearValue.color.float32[0] = action.clear_value.v[0];
  info->clearValue.color.float32[1] = action.clear_value.v[1];
  info->clearValue.color.float32[2] = action.clear_value.v[2];
  info->clearValue.color.float32[3] = action.clear_value.v[3];
}

void vk_init_depth_attachment_info(VkRenderingAttachmentInfo* info, Gfx_DepthAttachmentAction action, VkImageView ds_view) {
  info->sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  info->imageView = ds_view;
  info->imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
  info->resolveMode = VK_RESOLVE_MODE_NONE;
  info->loadOp = vk_load_op(action.load_action);
  info->storeOp = vk_store_op(action.store_action);
  info->clearValue.depthStencil.depth = action.clear_value;
}

void vk_init_stencil_attachment_info(VkRenderingAttachmentInfo* info, Gfx_StencilAttachmentAction action, VkImageView ds_view) {
  info->sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  info->imageView = ds_view;
  info->imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
  info->resolveMode = VK_RESOLVE_MODE_NONE;
  info->loadOp = vk_load_op(action.load_action);
  info->storeOp = vk_store_op(action.store_action);
  info->clearValue.depthStencil.stencil = action.clear_value;
}

////////////////////////////////////////////////////////////////////////
// @Misc

VkSemaphore vk_get_current_image_available_semaphore() { return st->vk.image_available_semaphores[st->vk.current_frame_idx]; }
VkSemaphore vk_get_current_render_complete_semaphore() { return st->vk.render_complete_semaphores[st->vk.current_image_idx]; }
VkCommandBuffer vk_get_cur_cmd()                   { return st->vk.cmds_render[st->vk.current_frame_idx]; }

void vk_bind_pipeline(VkPipeline pipeline) {
  st->vk.CmdBindPipeline(vk_get_cur_cmd(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
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

void vk_push_constants(VK_PushConstant constants) {
  st->vk.CmdPushConstants(vk_get_cur_cmd(), st->vk.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(VK_PushConstant), &constants);
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

VK_Memory vk_mem_alloc(VK_MemType type, u64 size) {
  VK_State& g = st->vk;
  u32 mem_idx = 0;
  switch (type) {
    case VK_MemoryType_Gpu: {
      mem_idx = g.device.gpu_type_idx; 
    } break;
    case VK_MemoryType_Cpu: {
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
  if (type == VK_MemoryType_Cpu) {
    VK_CHECK(g.MapMemory(vkdevice, res.h, 0, size, 0, (void**)&res.mapped_mem));
  }
  return res;
}

VK_Buffer vk_buffer_alloc(u64 size, VK_BufferUsage usage, VK_MemType mem_type) {
  VK_State& g = st->vk;
  VkBufferUsageFlags buf_usage_flags = 0;
  if (FlagHas(usage, VK_BufferUsage_Vert)) buf_usage_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if (FlagHas(usage, VK_BufferUsage_Index)) buf_usage_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  if (FlagHas(usage, VK_BufferUsage_Dst)) buf_usage_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  if (FlagHas(usage, VK_BufferUsage_Src)) buf_usage_flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  if (FlagHas(usage, VK_BufferUsage_Storage)) buf_usage_flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  if (FlagHas(usage, VK_BufferUsage_Indirect)) buf_usage_flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
  VkBufferCreateInfo buffer_create_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = size,
    .usage = buf_usage_flags,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  VK_Buffer res = {.cap = size};
  VK_CHECK(g.CreateBuffer(vkdevice, &buffer_create_info, g.allocator, &res.h));

  u32 mem_prop_flags = 0;
  if (mem_type == VK_MemoryType_Gpu) mem_prop_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  if (mem_type == VK_MemoryType_Cpu) mem_prop_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
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
    case VK_MemoryType_Gpu: {
      mem = &g.gpu_mem;
      offset = offset_push(mem->pos, size, requirements.alignment);
    } break;
    case VK_MemoryType_Cpu: {
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
  Gfx_Shader res = {pool_push(g.shaders, shader)};
  return res;
}

void gfx_pipeline_desc_defaults(Gfx_PipelineDesc* desc) {
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
  gfx_pipeline_desc_defaults(&desc);
  VK_Shader shader = pool_get(g.shaders, desc.shader);
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
    Gfx_Pipeline res = {pool_push(g.pipelines, pipeline)};
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
  VkPipelineMultisampleStateCreateInfo multisampling_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .rasterizationSamples = (VkSampleCountFlagBits)desc.sample_count,
    .alphaToCoverageEnable = (u32)desc.alpha_to_coverage_enabled,
  };

  // Depth and stencil testing
  VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthTestEnable = (u32)!desc.depth.test_disable,
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
    stages[stages_count++] = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = shader.h,
      .pName = "vs_main",
    };
  }
  if (shader.frag) {
    stages[stages_count++] = {
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
    .pMultisampleState = &multisampling_info,
    .pDepthStencilState = &depth_stencil_info,
    .pColorBlendState = &color_blend_state_info,
    .pDynamicState = &dynamic_state_info,
    .layout = g.pipeline_layout,
  };
  VK_CHECK(g.CreateGraphicsPipelines(vkdevice, VK_NULL_HANDLE, 1, &pipeline_info, st->vk.allocator, &pipeline.h));
  Gfx_Pipeline res = {pool_push(g.pipelines, pipeline)};
  return res;
}

void gfx_image_desc_defaults(Gfx_ImageDesc* desc) {
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
  gfx_image_desc_defaults(&desc);
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

  VkMemoryAllocateInfo alloc_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = requirements.size,
    .memoryTypeIndex = g.device.gpu_type_idx,
  };
  VK_CHECK(g.AllocateMemory(vkdevice, &alloc_info, g.allocator, &image.memory));

  MemFormatSize s = mem_format_size(requirements.size);
  Info("image created: %f %s", s.size, s.format);
  VK_CHECK(g.BindImageMemory(vkdevice, image.h, image.memory, 0));

  if (desc.data) {
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

  Gfx_Image res = {pool_push(g.images, image)};
  return res;
}

Gfx_View gfx_view_make(Gfx_ViewDesc desc) {
  VK_State& g = st->vk;
  Gfx_ViewType type = Gfx_ViewType_Invalid;
  if (desc.texture.image.idx != Gfx_InvalidId) {
    type = Gfx_ViewType_Texture;
  }
  if (desc.color_attachment.image.idx != Gfx_InvalidId) {
    Assert(type == Gfx_ViewType_Invalid);
    type = Gfx_ViewType_ColorAttachment;
  }
  if (desc.resolve_attachment.image.idx != Gfx_InvalidId) {
    Assert(type == Gfx_ViewType_Invalid);
    type = Gfx_ViewType_ResolveAttachment;
  }
  if (desc.depth_stencil_attachment.image.idx != Gfx_InvalidId) {
    Assert(type == Gfx_ViewType_Invalid);
    type = Gfx_ViewType_DepthStencilAttachment;
  }
  Assert(type != Gfx_ViewType_Invalid);

  VK_View view = {.type = type};
  VK_Image img = {};
  switch (type) {
    InvalidDefaultCase;
    case Gfx_ViewType_Texture: {
      Gfx_Image ref = desc.texture.image;
      img = pool_get(g.images, ref);
      view.ref = ref;
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
      Gfx_Image ref = desc.color_attachment.image;
      img = pool_get(g.images, ref);
      view.ref = ref;
      view.mip_level = desc.color_attachment.mip_level;
      view.mip_level_count = 1;
      view.slice = desc.color_attachment.slice;
      view.slice_count = 1;
    } break;
    case Gfx_ViewType_ResolveAttachment: {
      Gfx_Image ref = desc.resolve_attachment.image;
      img = pool_get(g.images, ref);
      view.ref = ref;
      view.mip_level = desc.resolve_attachment.mip_level;
      view.mip_level_count = 1;
      view.slice = desc.resolve_attachment.slice;
      view.slice_count = 1;
    } break;
    case Gfx_ViewType_DepthStencilAttachment: {
      Gfx_Image ref = desc.depth_stencil_attachment.image;
      img = pool_get(g.images, ref);
      view.ref = ref;
      view.mip_level = desc.depth_stencil_attachment.mip_level;
      view.mip_level_count = 1;
      view.slice = desc.depth_stencil_attachment.slice;
      view.slice_count = 1;
    } break;
  }

  VkImageViewCreateInfo view_info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .image = img.h,
    .viewType = view.type == Gfx_ViewType_Texture ? vk_texture_image_view_type(img.type) : vk_attachment_image_view_type(img.type),
    .format = vk_format(img.pixel_format),
    .subresourceRange = {
      .aspectMask = vk_aspect_mask(img.pixel_format),
      .baseMipLevel = view.mip_level,
      .levelCount = view.mip_level_count,
      .baseArrayLayer = view.slice,
      .layerCount = view.slice_count,
    },
  };
  VK_CHECK(g.CreateImageView(vkdevice, &view_info, g.allocator, &view.h));

  Gfx_View res = {pool_push(g.views, view)};
  VkDescriptorImageInfo descriptor_image_info = {
    .imageView = view.h,
    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
  VkWriteDescriptorSet texture_descriptor = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = g.descriptor_sets,
    .dstBinding = (img.type == Gfx_ImageType_2D || img.type == Gfx_ImageType_Array) ? Bindings::Textures : Bindings::CubeTextures,
    .dstArrayElement = res.idx,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    .pImageInfo = &descriptor_image_info,
  };
  g.UpdateDescriptorSets(vkdevice, 1, &texture_descriptor, 0, null);

  return res;
}

void gfx_sampler_desc_defaults(Gfx_SamplerDesc* desc) {
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
  gfx_sampler_desc_defaults(&desc);
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
    .minLod = desc.min_lod,
    .maxLod = desc.max_lod,
    .borderColor = vk_sampler_border_color(desc.border_color),
  };
  VK_CHECK(g.CreateSampler(vkdevice, &sampler_info, g.allocator, &sampler.h));

  Gfx_Sampler res = {pool_push(g.samplers, sampler)};
  VkDescriptorImageInfo descriptor_image_info = {
    .sampler = sampler.h,
    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
  VkWriteDescriptorSet sampler_descriptor = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = g.descriptor_sets,
    .dstBinding = Bindings::Samplers,
    .dstArrayElement = res.idx,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
    .pImageInfo = &descriptor_image_info,
  };
  g.UpdateDescriptorSets(vkdevice, 1, &sampler_descriptor, 0, null);

  return res;
}

void gfx_image_destroy(Gfx_Image img) {
  VK_State& g = st->vk;
  g.DestroyImage(vkdevice, pool_get(g.images, img).h, g.allocator);
  pool_remove(g.images, img);
}

void gfx_view_destroy(Gfx_View view) {
  VK_State& g = st->vk;
  g.DestroyImageView(vkdevice, pool_get(g.views, view).h, g.allocator);
  pool_remove(g.views, view);
}

void gfx_pass_begin(Gfx_Pass pass) {
  VK_State& g = st->vk;
  
  ///////////////////////////////////
  // Barrier
  b32 is_swapchain_pass = true;
  Loop (i, Gfx_MaxColorAttachments) {
    if (pass.attachments.colors[i].idx != Gfx_InvalidId) is_swapchain_pass = false;
    if (pass.attachments.resolves[i].idx != Gfx_InvalidId) is_swapchain_pass = false;
  }
  if (pass.attachments.depth_stencil.idx != Gfx_InvalidId) is_swapchain_pass = false;
  if (is_swapchain_pass) {
    // vk_image_barrier(&g.swapchain.images[g.current_image_idx], VK_Access_ColorAttachment);
  }
  else {
    Loop (i, Gfx_MaxColorAttachments) {
      if (pass.attachments.colors[i].idx == Gfx_InvalidId) break;
      VK_Image& color_image = pool_get(g.images, pool_get(g.views, pass.attachments.colors[i]).ref);
      if (pass.action.colors[i].load_action != Gfx_LoadAction_Load) {
        color_image.cur_access |= VK_Access_Discard;
      }
      vk_image_barrier(&color_image, VK_Access_ColorAttachment);
      if (pass.attachments.resolves[i].idx != Gfx_InvalidId) {
        VK_Image& resolve_image = pool_get(g.images, pool_get(g.views, pass.attachments.resolves[i]).ref);
        resolve_image.cur_access |= VK_Access_Discard;
        vk_image_barrier(&resolve_image, VK_Access_ResolveAttachment);
      }
    }
    if (pass.attachments.depth_stencil.idx != Gfx_InvalidId) {
      VK_Image& ds_image = pool_get(g.images, pool_get(g.views, pass.attachments.depth_stencil).ref);
      b32 has_stencil = ds_image.pixel_format == Gfx_PixelFormat_DepthStencil;
      if ((pass.action.depth.load_action != Gfx_LoadAction_Load) && (pass.action.stencil.load_action != Gfx_LoadAction_Load)) {
        ds_image.cur_access |= VK_Access_Discard;
      }
      VK_Access dst_access = VK_Access_DepthAttachment;
      if (has_stencil) {
        dst_access |= VK_Access_StencilAttachment;
      }
      vk_image_barrier(&ds_image, dst_access);
    }
  }

  Gfx_PassAction action = pass.action;

  VkRenderingAttachmentInfo color_att_infos[Gfx_MaxColorAttachments] = {};
  VkRenderingAttachmentInfo depth_att_info = {};
  VkRenderingAttachmentInfo stencil_att_info = {};
  VkRenderingInfo render_info = {};

  ///////////////////////////////////
  // Begin
  if (is_swapchain_pass) {
    VkImageView vk_color_view = g.swapchain.views[g.current_image_idx];
    vk_init_color_attachment_info(&color_att_infos[0], action.colors[0], vk_color_view, null);
    render_info.colorAttachmentCount = 1;
    render_info.pColorAttachments = color_att_infos;
  } else {
    u32 color_view_count = 0;
    Loop (i, Gfx_MaxColorAttachments) {
      if (pass.attachments.colors[i].idx == Gfx_InvalidId) break;
      VkImageView vk_color_view = pool_get(g.views, pass.attachments.colors[i]).h;
      VkImageView vk_resolve_view = pool_get(g.views, pass.attachments.resolves[i]).h;
      vk_init_color_attachment_info(&color_att_infos[i], action.colors[0], vk_color_view, vk_resolve_view);
      ++color_view_count;
    }
    if (color_view_count) {
      render_info.colorAttachmentCount = color_view_count;
      render_info.pColorAttachments = color_att_infos;
    }
    if (pass.attachments.depth_stencil.idx != Gfx_InvalidId) {
      VK_View ds_view = pool_get(g.views, pass.attachments.depth_stencil);
      VK_Image ds_image = pool_get(g.images, ds_view.ref);
      b32 has_stencil = ds_image.pixel_format == Gfx_PixelFormat_DepthStencil;
      vk_init_depth_attachment_info(&depth_att_info, action.depth, ds_view.h);
      render_info.pDepthAttachment = &depth_att_info;
      if (has_stencil) {
        vk_init_stencil_attachment_info(&stencil_att_info, action.stencil, ds_view.h);
        render_info.pStencilAttachment = &stencil_att_info;
      }
    }
  }

  g.CmdBeginRendering(vk_get_cur_cmd(), &render_info);
}

void gfx_pass_end() {
  VK_State& g = st->vk;
  g.CmdEndRendering(vk_get_cur_cmd());
  b32 is_swapchain_pass = true;
  Loop (i, Gfx_MaxColorAttachments) {
    if (g.cur_pass.attachments.colors[i].idx != Gfx_InvalidId) is_swapchain_pass = false;
    if (g.cur_pass.attachments.resolves[i].idx != Gfx_InvalidId) is_swapchain_pass = false;
  }
  if (g.cur_pass.attachments.depth_stencil.idx != Gfx_InvalidId) is_swapchain_pass = false;

  if (is_swapchain_pass) {
    vk_swapchain_end_barrier(g.swapchain.images[g.current_image_idx], VK_Access_Present);
  }
  else {
    Loop (i, Gfx_MaxColorAttachments) {
      if (g.cur_pass.attachments.colors[i].idx == Gfx_InvalidId) break;
      if (g.cur_pass.attachments.colors[i].idx == Gfx_StoreAction_Store) {
        VK_Image& img = pool_get(g.images, pool_get(g.views, g.cur_pass.attachments.colors[i]).ref);
        vk_image_barrier(&img, VK_Access_Texture);
      }
      if (g.cur_pass.attachments.resolves[i].idx != Gfx_InvalidId) {
        VK_Image& img = pool_get(g.images, pool_get(g.views, g.cur_pass.attachments.resolves[i]).ref);
        vk_image_barrier(&img, VK_Access_Texture);
      }
    }
    if (g.cur_pass.attachments.depth_stencil.idx != Gfx_InvalidId) {
      VK_Image& img = pool_get(g.images, pool_get(g.views, g.cur_pass.attachments.depth_stencil).ref);
      if (g.cur_pass.action.depth.store_action == Gfx_StoreAction_Store) {
        vk_image_barrier(&img, VK_Access_Texture);
      }
    }
  }
}

void gfx_apply_viewport(f32 x, f32 y, f32 width, f32 height) {
  VK_State& g = st->vk;
  // NOTE: we flip Y coordinate so Y:0 is on bottom of screen
  VkViewport viewport = {
    .x = x,
    .y = (f32)g.height-y,
    .width = (f32)g.width,
    .height = -(f32)g.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  g.CmdSetViewport(vk_get_cur_cmd(), 0, 1, &viewport);
}

void gfx_apply_scissor(f32 x, f32 y, f32 width, f32 height) {
  VK_State& g = st->vk;
  VkRect2D scissor = {
    .offset = {.x = (i32)x, .y = (i32)y},
    .extent = {
      .width = (u32)width, 
      .height = (u32)height
    },
  };
  g.CmdSetScissor(vk_get_cur_cmd(), 0, 1, &scissor);
}

void gfx_pipeline_bind(Gfx_Pipeline pip) {
  VK_State& g = st->vk;
  g.CmdBindPipeline(vk_get_cur_cmd(), VK_PIPELINE_BIND_POINT_GRAPHICS, pool_get(g.pipelines, pip).h);
}

void gfx_draw(u32 base_vert, u32 vert_count, u32 instance_count, u32 base_instance) {
  st->vk.CmdDraw(vk_get_cur_cmd(), vert_count, instance_count, base_vert, base_instance);
}

void gfx_draw_indexed(u32 base_index, u32 index_count, u32 base_vert, u32 instance_count, u32 base_instance) {
  st->vk.CmdDrawIndexed(vk_get_cur_cmd(), index_count, instance_count, base_index, base_vert, base_instance);
}

void gfx_draw_indirect(u32 draw_base, u32 draw_count) {
  st->vk.CmdDrawIndirect(vk_get_cur_cmd(), st->vk.indirect_draw_buffer.h, draw_base*sizeof(VK_DrawCallInfo), draw_count, sizeof(VK_DrawCallInfo));
}

void gfx_draw_indexed_indirect(u32 draw_base, u32 draw_count) {
  st->vk.CmdDrawIndexedIndirect(vk_get_cur_cmd(), st->vk.indirect_draw_buffer.h, draw_base*sizeof(VK_DrawCallInfo), draw_count, sizeof(VK_DrawCallInfo));
}
