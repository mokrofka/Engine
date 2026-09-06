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
			Gfx_State& g = st->gfx;
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

VkImageAspectFlags vk_aspect_mask(Gfx_PixelFormat fmt) {
	switch (fmt) {
		default:                           return VK_IMAGE_ASPECT_COLOR_BIT;
		case Gfx_PixelFormat_Depth:        return VK_IMAGE_ASPECT_DEPTH_BIT;
		case Gfx_PixelFormat_DepthStencil: return VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
	}
}

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
	res |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if (flag_has(usg, Gfx_ImageUsage_StorageImage)) {
		res |= VK_IMAGE_USAGE_STORAGE_BIT;
	}
	if (flag_has(usg, Gfx_ImageUsage_ColorAttachment) || flag_has(usg, Gfx_ImageUsage_ResolveAttachment)) {
		res |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	if (flag_has(usg, Gfx_ImageUsage_DepthStencilAttachment)) {
		res |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	return res;
}

VkFormat vk_format(Gfx_PixelFormat fmt) {
	switch (fmt) {
		InvalidDefaultCase;
		case Gfx_PixelFormat_R8_UI:          return VK_FORMAT_R8_UINT;
		case Gfx_PixelFormat_R8:             return VK_FORMAT_R8_UNORM;
		case Gfx_PixelFormat_BGRA8:          return VK_FORMAT_B8G8R8A8_UNORM;
		case Gfx_PixelFormat_BGRA8_SRGB:     return VK_FORMAT_B8G8R8A8_SRGB;
		case Gfx_PixelFormat_RGBA8:          return VK_FORMAT_R8G8B8A8_UNORM;
		case Gfx_PixelFormat_RGBA8_SRGB:     return VK_FORMAT_R8G8B8A8_SRGB;
		case Gfx_PixelFormat_Depth:          return VK_FORMAT_D32_SFLOAT;
		case Gfx_PixelFormat_DepthStencil:   return VK_FORMAT_D32_SFLOAT_S8_UINT;
	}
}

VkPrimitiveTopology vk_primitive_topology(Gfx_PrimitiveType t) {
	switch (t) {
		InvalidDefaultCase;
		case Gfx_PrimitiveType_Point:         return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case Gfx_PrimitiveType_Line:          return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case Gfx_PrimitiveType_Triangle:      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case Gfx_PrimitiveType_TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
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

b32 vk_is_read_access(VK_Access access) {
	// VK_Access read_bits = VK_Access_IndexBuffer | VK_Access_VertBuffer | VK_Access_StorageBuffer_RO | VK_Access_Texture | VK_Access_Present;
	VK_Access read_bits = VK_Access_StorageBuffer_RO | VK_Access_Texture | VK_Access_Present;
	return flag_has(read_bits, access);
}

VkPipelineStageFlags2 vk_stage_mask(VK_Access access, b32 is_dst_access) {
	access &= ~VK_Access_Discard;
	if (is_dst_access) Assert(access != VK_Access_None);

	if (access == VK_Access_None) return VK_PIPELINE_STAGE_2_NONE;
	if (access & VK_Access_Present) return VK_PIPELINE_STAGE_2_NONE;

	VkPipelineStageFlags2 f = 0;
	// Transfer
	if (access & VK_Access_TransferDst || access & VK_Access_TransferSrc) f |= VK_PIPELINE_STAGE_2_COPY_BIT;

	// Vertex
	// if (access & VK_Access_VertBuffer)  f |= VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
	// if (access & VK_Access_IndexBuffer) f |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;

	// Determine which shader stages are needed
	b32 has_stage_qualifier = access & (VK_Access_VertexShader | VK_Access_FragmentShader | VK_Access_ComputeShader);
	b32 has_shader_access = access & (VK_Access_StorageBuffer_RO | VK_Access_StorageBuffer_RW |
																																			VK_Access_Texture |
																																			VK_Access_StorageImage_RO | VK_Access_StorageImage_RW);
	if (has_shader_access) {
			if (!has_stage_qualifier) {
					f |= VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
			} else {
					if (access & VK_Access_VertexShader) f |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
					if (access & VK_Access_FragmentShader) f |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
					if (access & VK_Access_ComputeShader) f |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			}
	}

	// Attachments
	if (access & VK_Access_ColorAttachment) f |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	if (access & VK_Access_ResolveAttachment) f |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

	if (access & (VK_Access_DepthAttachment | VK_Access_StencilAttachment | VK_Access_DepthRead)) {
			f |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
	}

	// Indirect
	if (access & VK_Access_IndirectBuffer) f |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;

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
	if (access == VK_Access_None) return VK_ACCESS_2_NONE;
	if (access & VK_Access_Present) return VK_ACCESS_2_NONE;

	VkAccessFlags2 f = VK_ACCESS_2_NONE;
	// Transfer
	if (access & VK_Access_TransferSrc) f |= VK_ACCESS_2_TRANSFER_READ_BIT;
	if (access & VK_Access_TransferDst) f |= VK_ACCESS_2_TRANSFER_WRITE_BIT;

	// Vertex/Index input — read only, always dst
	// if (access & VK_Access_VertBuffer)     f |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
	// if (access & VK_Access_IndexBuffer)    f |= VK_ACCESS_2_INDEX_READ_BIT;

	// Shader read accesses
	if (access & VK_Access_StorageBuffer_RO)  f |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
	if (access & VK_Access_Texture)           f |= VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
	if (access & VK_Access_StorageImage_RO)   f |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT;

	// Shader read+write accesses
	if (access & VK_Access_StorageBuffer_RW) {
			f |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
	}
	if (access & VK_Access_StorageImage_RW) {
			f |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
	}

	// Color attachments — write (and read if dst for blending)
	if (access & VK_Access_ColorAttachment) {
			f |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			// if (is_dst_access) f |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT; // for blending
	}
	if (access & VK_Access_ResolveAttachment) {
			f |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
	}

	// Depth/stencil
	if (access & (VK_Access_DepthAttachment | VK_Access_StencilAttachment)) {
			f |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	if (access & VK_Access_DepthRead) f |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

	// Indirect buffer
	if (access & VK_Access_IndirectBuffer) f |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;

	return f;
}

VkAccessFlags2 vk_src_access_mask(VK_Access access) {
	return vk_access_mask(access, false);
}

VkAccessFlags2 vk_dst_access_mask(VK_Access access) {
	return vk_access_mask(access, true);
}

VkImageLayout vk_image_layout(VK_Access access) {
	if (access & VK_Access_Discard) return VK_IMAGE_LAYOUT_UNDEFINED;
	if (access == VK_Access_None) return VK_IMAGE_LAYOUT_UNDEFINED;
	if (access & VK_Access_Present) return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Transfer
	if (access & VK_Access_TransferSrc) return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	if (access & VK_Access_TransferDst) return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	// Shader read
	if (access & VK_Access_Texture) return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	// Storage image
	if (access & (VK_Access_StorageImage_RO | VK_Access_StorageImage_RW)) return VK_IMAGE_LAYOUT_GENERAL;

	// Attachments
	if (access & (VK_Access_ColorAttachment | VK_Access_ResolveAttachment)) return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	if (access & (VK_Access_DepthAttachment | VK_Access_StencilAttachment)) return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	if (access & VK_Access_DepthRead) return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	return VK_IMAGE_LAYOUT_GENERAL;
}

void vk_swapchain_beginpass_barrier(VkImage vkimg, VK_Access pass_access) {
	Gfx_State& g = st->gfx;
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
	g.CmdPipelineBarrier2(vk_cur_cmd(), &dep_info);
}

void vk_swapchain_end_barrier(VkImage vkimg, VK_Access pass_access) {
	Gfx_State& g = st->gfx;
	VkImageMemoryBarrier2 barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcStageMask = vk_src_stage_mask(pass_access),
		.srcAccessMask = vk_src_access_mask(pass_access),
		.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
		.dstStageMask = vk_src_stage_mask(pass_access),
		.dstAccessMask = vk_src_access_mask(pass_access),
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
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
	g.CmdPipelineBarrier2(vk_cur_cmd(), &dep_info);
}

void vk_image_barrier(VkCommandBuffer cmd, VK_Image* img, VK_Access new_access) {
	Gfx_State& g = st->gfx;
	if (vk_is_read_access(img->cur_access) && vk_is_read_access(new_access)) return;
	VkImageMemoryBarrier2 barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcStageMask = vk_src_stage_mask(img->cur_access),
		.srcAccessMask = vk_src_access_mask(img->cur_access),
		.oldLayout = vk_image_layout(img->cur_access),
		.dstStageMask = vk_dst_stage_mask(new_access),
		.dstAccessMask = vk_dst_access_mask(new_access),
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
	g.CmdPipelineBarrier2(cmd, &dep_info);
	img->cur_access = new_access;
}

void vk_buffer_barrier(VK_BufferRegion buf, VK_Access new_access) {
	Gfx_State& g = st->gfx;
	if (vk_is_read_access(buf.cur_access) && vk_is_read_access(new_access)) return;
	VkBufferMemoryBarrier2 barrier = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
		.srcStageMask = vk_src_stage_mask(buf.cur_access),
		.srcAccessMask = vk_src_access_mask(buf.cur_access),
		.dstStageMask = vk_dst_stage_mask(new_access),
		.dstAccessMask = vk_dst_access_mask(new_access),
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.buffer = buf.h,
		.offset = buf.base,
		.size = buf.pos,
	};
	VkDependencyInfo dep_info = {
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.bufferMemoryBarrierCount = 1,
		.pBufferMemoryBarriers = &barrier,
	};
	g.CmdPipelineBarrier2(vk_cur_cmd(), &dep_info);
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

VkSemaphore vk_get_cur_image_available_semaphore() { return st->gfx.image_available_semaphores[st->gfx.current_frame_idx]; }
VkSemaphore vk_get_cur_render_complete_semaphore() { return st->gfx.render_complete_semaphores[st->gfx.current_image_idx]; }
VkCommandBuffer vk_cur_cmd()                       { return st->gfx.render_cmds[st->gfx.current_frame_idx]; }

u32 vk_find_memory_idx(u32 type_filter, u32 property_flags) {
	VkPhysicalDeviceMemoryProperties memory_properties = st->gfx.device.memory;
	u32 idx = -1;
	Loop (i, memory_properties.memoryTypeCount) {
		if (bit_has(type_filter, i) && flag_has(memory_properties.memoryTypes[i].propertyFlags, property_flags)) {
			idx = i;
			break;
		}
	}
	Assert(idx != -1);
	return idx;
}

void vk_push_constants(VK_PushConstant constants) {
	st->gfx.CmdPushConstants(vk_cur_cmd(), st->gfx.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(VK_PushConstant), &constants);
}

VkSemaphore vk_make_semaphore(u64 initial_counter) {
	Gfx_State& g = st->gfx;
	VkSemaphoreTypeCreateInfo timelineInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
		.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
		.initialValue = initial_counter
	};
	VkSemaphoreCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = &timelineInfo
	};
	VkSemaphore res;
	VK_CHECK(g.CreateSemaphore(vkdevice, &createInfo, g.allocator, &res));
	return res;
}

// void vk_semaphore_wait(VK_Semaphore semaphore, u64 wait_counter) {
//   Gfx_State& g = st->gfx;
//   u64 wait = semaphore.counter + wait_counter;
//   VkSemaphoreWaitInfo waitInfo = {
//     .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
//     .semaphoreCount = 1,
//     .pSemaphores = &semaphore.h,
//     .pValues = &wait
//   };
//   g.WaitSemaphores(vkdevice, &waitInfo, UINT64_MAX);
// }

void vk_cmd_alloc(VkCommandPool pool, u32 count, VkCommandBuffer* out) {
	VkCommandBufferAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = count,
	};
	VK_CHECK(st->gfx.AllocateCommandBuffers(vkdevice, &allocate_info, out));
}

void vk_cmd_begin(VkCommandBuffer cmd) {
	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	VK_CHECK(st->gfx.BeginCommandBuffer(cmd, &begin_info));
}

void vk_cmd_end(VkCommandBuffer cmd) {
	VK_CHECK(st->gfx.EndCommandBuffer(cmd));
}

void vk_cmd_submit(VkCommandBuffer cmd) {
	Gfx_State& g = st->gfx;
	VkCommandBufferSubmitInfo command_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = cmd,
	};
	VkSubmitInfo2 submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &command_info,
	};
	VK_CHECK(g.QueueSubmit2(g.device.graphics_queue, 1, &submit_info, 0));
	gfx_idle();
}

VK_Memory vk_mem_make(Gfx_MemType type, u64 size) {
	Gfx_State& g = st->gfx;
	u32 mem_idx = 0;
	switch (type) {
		InvalidDefaultCase;
		case Gfx_MemType_Cpu: {
			// mem_idx = g.device.cpu_type_idx; 
			mem_idx = g.device.cpu_type_idx; 
		} break;
		case Gfx_MemType_Gpu: {
			// mem_idx = g.device.cpu_type_idx; 
			mem_idx = g.device.gpu_type_idx; 
		} break;
	}
	VkMemoryAllocateFlagsInfoKHR flags_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR,
		.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,
	};
	VkMemoryAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &flags_info,
		.allocationSize = size,
		.memoryTypeIndex = mem_idx,
	};
	VK_Memory res = {.cap = size};
	VK_CHECK(g.AllocateMemory(vkdevice, &alloc_info, g.allocator, &res.h));
	if (type == Gfx_MemType_Cpu) {
		VK_CHECK(g.MapMemory(vkdevice, res.h, 0, size, 0, (void**)&res.mapped_mem));
	}
	return res;
}

VK_Buffer vk_make_buffer(Gfx_MemType type, u64 size) {
	Gfx_State& g = st->gfx;
	VkBufferUsageFlags buf_usage_flags = 
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
		VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;
	VkBufferCreateInfo buffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = buf_usage_flags,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	VK_Buffer res = {.cap = size};
	VK_CHECK(g.CreateBuffer(vkdevice, &buffer_create_info, g.allocator, &res.h));
	u32 mem_prop_flags = 0;
	switch (type) {
		InvalidDefaultCase;
		case Gfx_MemType_Cpu: mem_prop_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; break;
		case Gfx_MemType_Gpu: mem_prop_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; break;
	}
	VkMemoryRequirements requirements;
	g.GetBufferMemoryRequirements(vkdevice, res.h, &requirements);

	// u32 mem_idx = vk_find_memory_idx(requirements.memoryTypeBits, mem_prop_flags);
	// VkMemoryAllocateInfo alloc_info = {
	//   .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
	//   .allocationSize = requirements.size,
	//   .memoryTypeIndex = mem_idx,
	// };
	// NoOp(alloc_info);

	VK_Memory* mem = null;
	u64 offset = 0;
	switch (type) {
		InvalidDefaultCase;
		case Gfx_MemType_Cpu: {
			mem = &g.cpu_mem;
			offset = offset_push(mem->pos, size, requirements.alignment);
			Assert(mem->pos <= mem->cap);
		} break;
		case Gfx_MemType_Gpu: {
			mem = &g.gpu_mem;
			offset = offset_push(mem->pos, size, requirements.alignment);
			Assert(mem->pos <= mem->cap);
		} break;
	}
	VK_CHECK(g.BindBufferMemory(vkdevice, res.h, mem->h, offset));
	res.base = Offset(mem->mapped_mem, offset);
	return res;
}

VK_Buffer vk_get_vkbuffer(Gfx_Buffer buf) {
	var& g = st->gfx;
	switch (buf.type) {
		InvalidDefaultCase;
		case Gfx_MemType_Cpu: return g.cpu_buf;
		case Gfx_MemType_Gpu: return g.gpu_buf;
	}
}

VkShaderModule vk_shader_create(Slice<u8> code) {
	Gfx_State& g = st->gfx;
	VkShaderModuleCreateInfo module_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size,
		.pCode = (u32*)code.data,
	};
	VkShaderModule res;
	VK_CHECK(g.CreateShaderModule(vkdevice, &module_info, g.allocator, &res));
	return res;
}

VkPipeline vk_pipeline_create(Gfx_PipelineDesc desc) {
	Gfx_State& g = st->gfx;

	VK_Shader shader = pool_get(g.shaders, desc.shader);
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
		VkPipeline res;
		VK_CHECK(g.CreateComputePipelines(vkdevice, VK_NULL_HANDLE, 1, &pip_create_info, 0, &res));
		return res;
	}
	
	///////////////////////////////////
	// Graphics pipeline
	// Dynamic rendering
	VkFormat color_formats[Gfx_MaxColorAttachments] = {};
	Loop (i, desc.color_count) {
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
		.stride = sizeof(R_Vertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
	};

	// Attributes
	VkVertexInputAttributeDescription attribute_desriptions[16] = {};
	u32 attribute_desriptions_count = 0;
	attribute_desriptions_count = 4;
	attribute_desriptions[0] = {
		.format = VK_FORMAT_R32G32B32_SFLOAT,
		.offset = (u32)OffsetOf(R_Vertex, pos),
	};
	attribute_desriptions[1] = {
		.format = VK_FORMAT_R32G32B32_SFLOAT,
		.offset = (u32)OffsetOf(R_Vertex, norm),
	};
	attribute_desriptions[2] = {
		.format = VK_FORMAT_R32G32_SFLOAT,
		.offset = (u32)OffsetOf(R_Vertex, uv),
	};
	attribute_desriptions[3] = {
		.format = VK_FORMAT_R32G32B32A32_SFLOAT,
		.offset = (u32)OffsetOf(R_Vertex, color),
	};
	Loop (i, attribute_desriptions_count) {
		attribute_desriptions[i].location = i;
	}
	VkPipelineVertexInputStateCreateInfo vertex_input_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &binding_description,
		.vertexAttributeDescriptionCount = attribute_desriptions_count,
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
	stages[stages_count++] = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = shader.h,
		.pName = "vs_main",
	};
	stages[stages_count++] = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = shader.h,
		.pName = "fs_main",
	};
	
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
	VkPipeline res;
	VK_CHECK(g.CreateGraphicsPipelines(vkdevice, VK_NULL_HANDLE, 1, &pipeline_info, g.allocator, &res));
	return res;
}

VkPipeline vk_pipeline_create2(Gfx_PipelineDesc2 desc) {
	var& g = st->gfx;

	// Dynamic rendering
	VkFormat color_formats = vk_format(desc.pixel_format);
	VkPipelineRenderingCreateInfo rendering_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &color_formats,
		.depthAttachmentFormat = vk_format(desc.depth_pixel_format),
		.stencilAttachmentFormat = desc.depth_pixel_format == Gfx_PixelFormat_DepthStencil ? vk_format(desc.depth_pixel_format) : VK_FORMAT_UNDEFINED,
	};
	
	// Vertex input
	VkVertexInputBindingDescription binding_description = {
		.binding = 0,
		.stride = sizeof(R_Vertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
	};

	// Attributes
	VkVertexInputAttributeDescription attribute_desriptions[16] = {};
	u32 attribute_desriptions_count = 0;
	attribute_desriptions_count = 4;
	attribute_desriptions[0] = {
		.format = VK_FORMAT_R32G32B32_SFLOAT,
		.offset = (u32)OffsetOf(R_Vertex, pos),
	};
	attribute_desriptions[1] = {
		.format = VK_FORMAT_R32G32B32_SFLOAT,
		.offset = (u32)OffsetOf(R_Vertex, norm),
	};
	attribute_desriptions[2] = {
		.format = VK_FORMAT_R32G32_SFLOAT,
		.offset = (u32)OffsetOf(R_Vertex, uv),
	};
	attribute_desriptions[3] = {
		.format = VK_FORMAT_R32G32B32A32_SFLOAT,
		.offset = (u32)OffsetOf(R_Vertex, color),
	};
	Loop (i, attribute_desriptions_count) {
		attribute_desriptions[i].location = i;
	}
	VkPipelineVertexInputStateCreateInfo vertex_input_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &binding_description,
		.vertexAttributeDescriptionCount = attribute_desriptions_count,
		.pVertexAttributeDescriptions = attribute_desriptions,
	};

	VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
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
		.lineWidth = 1.0f,
	};

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = (VkSampleCountFlagBits)desc.sample_count,
		.alphaToCoverageEnable = (u32)desc.alpha_to_coverage_enabled,
	};

	VkPipelineColorBlendStateCreateInfo color_blend_state_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		// .attachmentCount = 1,
		.blendConstants[0] = desc.blend_color.v[0],
		.blendConstants[1] = desc.blend_color.v[1],
		.blendConstants[2] = desc.blend_color.v[2],
		.blendConstants[3] = desc.blend_color.v[3],
	};

	// Dynamic state
	VkDynamicState dynamic_states[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
		VK_DYNAMIC_STATE_CULL_MODE,
		VK_DYNAMIC_STATE_FRONT_FACE,
		VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
		VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
		VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,
		VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
		VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT,
		VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT,
		// VK_DYNAMIC_STATE_BLEND_CONSTANTS,
	};
	VkPipelineDynamicStateCreateInfo dynamic_state_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = ArrayCount(dynamic_states),
		.pDynamicStates = dynamic_states,
	};

	// Shader stages
	VK_Shader shader = pool_get(g.shaders, desc.shader);
	u32 stages_count = 0;
	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[stages_count++] = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = shader.h,
		.pName = "vs_main",
	};
	stages[stages_count++] = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = shader.h,
		.pName = "fs_main",
	};
	
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
		.pDepthStencilState = null,
		.pColorBlendState = &color_blend_state_info,
		.pDynamicState = &dynamic_state_info,
		.layout = g.pipeline_layout,
	};
	VkPipeline res;
	VK_CHECK(g.CreateGraphicsPipelines(vkdevice, VK_NULL_HANDLE, 1, &pipeline_info, g.allocator, &res));
	return res;
}

void vk_loader_load_core() {
	Gfx_State& g = st->gfx;
	g.lib = os_lib_open("libvulkan.so");
#define X(name) VK_GET_PROC(name)
	VK_GET_PROC_LIST
#undef X
}

void vk_loader_load_instance() {
	Gfx_State& g = st->gfx;
#define X(name) VK_INSTANCE_GET_PROC(name)
	VK_INSTANCE_GET_PROC_LIST
#undef X
}

void vk_loader_load_device() {
	Gfx_State& g = st->gfx;
#define X(name) VK_DEVICE_GET_PROC(name)
	VK_DEVICE_GET_PROC_LIST
#undef X
}

VkBool32 vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_types, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data);
intern void vk_instance_init() {
	Scratch scratch;
	Gfx_State& g = st->gfx;
	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.apiVersion = VK_API_VERSION_1_4
	};
	var required_validation_layer_names = array_make(const char*, scratch);
	var required_extensions = array_make(const char*, scratch);
	const char* VK1_SURFACE_NAME = "VK_KHR_xcb_surface";
	array_push(required_extensions, VK_KHR_SURFACE_EXTENSION_NAME, VK1_SURFACE_NAME);

#if BUILD_DEBUG
	// Validation layer
	array_push(required_validation_layer_names, "VK_LAYER_KHRONOS_validation");
	Debug("Required layers:");
	Loop (i, required_validation_layer_names.count) {
		Debug(required_validation_layer_names[i]);
	}
	u32 available_layer_count = 0;
	VK_CHECK(g.EnumerateInstanceLayerProperties(&available_layer_count, null));
	VkLayerProperties* available_layers = push_array(scratch, VkLayerProperties, available_layer_count);
	VK_CHECK(g.EnumerateInstanceLayerProperties(&available_layer_count, available_layers));
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
	array_push(required_extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	Debug("Required extensions:");
	Loop (i, required_extensions.count) {
		Debug(required_extensions[i]);
	}
	u32 extension_count = 0;
	g.EnumerateInstanceExtensionProperties(null, &extension_count, null);
	VkExtensionProperties* props = push_array(scratch, VkExtensionProperties, extension_count);
	g.EnumerateInstanceExtensionProperties(null, &extension_count, props);
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
	VK_CHECK(g.CreateInstance(&instance_create_info, g.allocator, &g.instance));
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
		.pfnUserCallback = vk_debug_callback,
	};
	PFN_vkCreateDebugUtilsMessengerEXT func = null;
	Assign(func, g.GetInstanceProcAddr(g.instance, "vkCreateDebugUtilsMessengerEXT"));
	AssertMsg(func, "Failed to create debug messenger");
	VK_CHECK(func(g.instance, &debug_create_info, g.allocator, &g.debug_messenger));
	Debug("Vulkan debugger created");
#endif
}

intern void vk_device_init() {
	Scratch scratch;
	Gfx_State& g = st->gfx;

	///////////////////////////////////
	// Physical device
	{
		u32 physical_device_count;
		VK_CHECK(g.EnumeratePhysicalDevices(g.instance, &physical_device_count, null));
		VK_Device* devices = push_array_zero(scratch, VK_Device, physical_device_count);
		VkPhysicalDevice* physical_devices = push_array(scratch, VkPhysicalDevice, physical_device_count);
		VK_CHECK(g.EnumeratePhysicalDevices(g.instance, &physical_device_count, physical_devices));
		b32 discrete_available = false;
		u32 discrete_idx = 0;
		u32 integrated_idx = 0;
		Loop(i, physical_device_count) {
			VK_Device& device = devices[i];
			device.physical_device = physical_devices[i];
			devices[i].depth_format = VK_FORMAT_D32_SFLOAT;
			g.GetPhysicalDeviceProperties(physical_devices[i], &device.properties);
			g.GetPhysicalDeviceFeatures(physical_devices[i], &device.features);
			g.GetPhysicalDeviceMemoryProperties(physical_devices[i], &device.memory);

			///////////////////////////////////
			// Swapchain info
			{
				VK_CHECK(g.GetPhysicalDeviceSurfaceCapabilitiesKHR(device.physical_device, g.surface, &device.surface_capabilities));
				VK_CHECK(g.GetPhysicalDeviceSurfaceFormatsKHR(device.physical_device, g.surface, &device.surface_format_count, null));
				VK_CHECK(g.GetPhysicalDeviceSurfacePresentModesKHR(device.physical_device, g.surface, &device.present_mode_count, null));
				device.surface_formats = push_array(g.arena, VkSurfaceFormatKHR, device.surface_format_count);
				device.present_modes = push_array(g.arena, VkPresentModeKHR, device.present_mode_count);
				VK_CHECK(g.GetPhysicalDeviceSurfaceFormatsKHR(device.physical_device, g.surface, &device.surface_format_count, device.surface_formats));
				VK_CHECK(g.GetPhysicalDeviceSurfacePresentModesKHR(device.physical_device, g.surface, &device.present_mode_count, device.present_modes));
			}

			///////////////////////////////////
			// Select queue families
			{
				u32 queue_family_count = 0;
				g.GetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &queue_family_count, null);
				VkQueueFamilyProperties* queue_families = push_array(scratch, VkQueueFamilyProperties, queue_family_count);
				g.GetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &queue_family_count, queue_families);
				Loop (i, queue_family_count) {
					u32 flags = queue_families->queueFlags;
					if ((flags & VK_QUEUE_GRAPHICS_BIT)) {
						device.graphics_queue_family_idx = i;
					}
					if ((flags & VK_QUEUE_TRANSFER_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT)) {
						device.transfer_queue_family_idx = i;
					}
					if ((flags & VK_QUEUE_COMPUTE_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT) && !(flags & VK_QUEUE_TRANSFER_BIT)) {
						device.compute_queue_family_idx = i;
					}
				}
				Info("Queue Families: Graphics %i, Compute %i, Transfer %i", device.graphics_queue_family_idx, device.compute_queue_family_idx, device.transfer_queue_family_idx);
			}

			///////////////////////////////////
			// Gpu Info
			Info("Available device: '%s'", String(device.properties.deviceName));
			switch (device.properties.deviceType) {
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: {
					integrated_idx = i;
					Info("GPU type is Integrated");
				} break;
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: {
					discrete_available = true;
					discrete_idx = i;
					Info("GPU type is Descrete");
				} break;
				default:
			}
			Info("GPU Driver version: %i.%i.%i", VK_VERSION_MAJOR(device.properties.driverVersion), VK_VERSION_MINOR(device.properties.driverVersion), VK_VERSION_PATCH(device.properties.driverVersion));
			Info("GPU API version: %i.%i.%i", VK_VERSION_MAJOR(device.properties.apiVersion), VK_VERSION_MINOR(device.properties.apiVersion), VK_VERSION_PATCH(device.properties.apiVersion));
			Loop (i, device.memory.memoryHeapCount) {
				f64 mem_size = (f64)device.memory.memoryHeaps[i].size / GB(1);
				if (device.memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
					Info("Heap %i: GPU: %.2f GiB", i, mem_size);
				else
					Info("Heap %i: System: %.2f GiB", i, mem_size);
			}

			///////////////////////////////////
			// Find cpu/gpu memory type index
			Loop (i, device.memory.memoryTypeCount) {
				VkMemoryType t = device.memory.memoryTypes[i];
				// if (FlagEquals(t.propertyFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
				if (t.propertyFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
					device.gpu_type_idx = i;
					break;
				}
			}
			Loop (i, device.memory.memoryTypeCount) {
				VkMemoryType t = device.memory.memoryTypes[i];
				if (flag_any(t.propertyFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
					device.cpu_type_idx = i;
					break;
				}
			}
		}
		if (discrete_available) {
			g.device = devices[discrete_idx];
			Info("Discrete GPU was choosen");
		} else {
			g.device = devices[integrated_idx];
			Info("Integrated GPU was choosen");
		}
		Info("Physical device selected");
	}

	///////////////////////////////////
	// Logical device
	{
		// Find unique queues
		const u32 queue_count = 3;
		Array<u32, queue_count> indices = {};
		array_push(indices, g.device.graphics_queue_family_idx);
		if (!array_exists(indices, g.device.transfer_queue_family_idx)) {
			array_push(indices, g.device.transfer_queue_family_idx);
		}
		if (!array_exists(indices, g.device.compute_queue_family_idx)) {
			array_push(indices, g.device.compute_queue_family_idx);
		}
		Array<VkDeviceQueueCreateInfo, queue_count> queue_create_infos = {};
		Loop (i, indices.count) {
			f32 queue_priority = 1.0f;
			VkDeviceQueueCreateInfo device_queue_create_info = {
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext = null,
				.flags = 0,
				.queueFamilyIndex = indices[i],
				.queueCount = 1,
				.pQueuePriorities = &queue_priority,
			};
			array_push(queue_create_infos, device_queue_create_info);
		}
		
		///////////////////////////////////
		// Features
		VkPhysicalDeviceExtendedDynamicState3FeaturesEXT eds3 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
			.extendedDynamicState3PolygonMode = true,
			.extendedDynamicState3ColorBlendEnable = true,
			.extendedDynamicState3ColorBlendEquation = true,
			.extendedDynamicState3ColorWriteMask = true,
		};
		VkPhysicalDeviceVulkan14Features vulkan14 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
			.pNext = &eds3,
		};
		VkPhysicalDeviceVulkan13Features vulkan13 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.pNext = &vulkan14,
			.synchronization2 = true,
			.dynamicRendering = true,
		};
		VkPhysicalDeviceVulkan12Features vulkan12 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = &vulkan13,
			.bufferDeviceAddress = true,
			.shaderSampledImageArrayNonUniformIndexing = true,     // allows runtime indexing
			.descriptorBindingSampledImageUpdateAfterBind = true,  // allows update image descriptors
			.descriptorBindingPartiallyBound = true,               // allows not updated descriptors
			.runtimeDescriptorArray = true,                        // allows not specified size of descriptor array in shader
			.timelineSemaphore = true,
		};
		VkPhysicalDeviceVulkan11Features vulkan11 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
			.pNext = &vulkan12,
			.shaderDrawParameters = true, // Indirect drawing
		};
		VkPhysicalDeviceFeatures2 features = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &vulkan11,
			.features = {
				.multiDrawIndirect = true, // Request indirect drawing
				.fillModeNonSolid = true,  // Request anistrophy
				.samplerAnisotropy = true, // Request wireframe
				.shaderInt64 = true,       // Request 64bit integers
			}
		};
		const char* extension_names[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME
		};
		VkDeviceCreateInfo device_create_info = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &features,
			.queueCreateInfoCount = queue_create_infos.count,
			.pQueueCreateInfos = queue_create_infos.data,
			.enabledExtensionCount = ArrayCount(extension_names),
			.ppEnabledExtensionNames = extension_names,
		};
		VK_CHECK(g.CreateDevice(g.device.physical_device, &device_create_info, g.allocator, &g.device.logical_device));
		Info("Logical device created");
	}
}

void vk_swapchain_create() {
	Gfx_State& g = st->gfx;
	if (g.swapchain.h_old) {
		g.swapchain.h_old = g.swapchain.h;
		ArrayCopy(g.swapchain.old_view, g.swapchain.views);
	}
	VK_CHECK(g.GetPhysicalDeviceSurfaceCapabilitiesKHR(g.device.physical_device, g.surface, &g.device.surface_capabilities));
	v2u win_size = os_window_size();
	VkExtent2D swapchain_extent = {win_size.x, win_size.y};
	if (g.device.surface_capabilities.currentExtent.width != U32_MAX) {
		swapchain_extent = g.device.surface_capabilities.currentExtent;
	}
	VkExtent2D min = g.device.surface_capabilities.minImageExtent;
	VkExtent2D max = g.device.surface_capabilities.maxImageExtent;
	swapchain_extent.width = Clamp(min.width, swapchain_extent.width, max.width);
	swapchain_extent.height = Clamp(min.height, swapchain_extent.height, max.height);
	VkSwapchainCreateInfoKHR swapchain_create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = g.surface,
		.minImageCount = g.images_in_flight,
		.imageFormat = g.swapchain.format.format,
		.imageColorSpace = g.swapchain.format.colorSpace,
		.imageExtent = swapchain_extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.preTransform = g.device.surface_capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = g.swapchain.present_mode,
		.clipped = VK_TRUE,
		.oldSwapchain = g.swapchain.h_old ? g.swapchain.h : null,
	};
	VK_CHECK(g.CreateSwapchainKHR(vkdevice, &swapchain_create_info, g.allocator, &g.swapchain.h));
	u32 image_count = g.images_in_flight;
	VK_CHECK(g.GetSwapchainImagesKHR(vkdevice, g.swapchain.h, &image_count, g.swapchain.images));
	Loop (i, image_count) {
		VkImageViewCreateInfo view_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = g.swapchain.images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = g.swapchain.format.format,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
		VK_CHECK(g.CreateImageView(vkdevice, &view_info, g.allocator, &g.swapchain.views[i]));
	}

	///////////////////////////////////
	// Destroy old one
	if (g.swapchain.h_old) {
		Loop (i, g.images_in_flight) {
			g.DestroyImageView(vkdevice, g.swapchain.old_view[i], g.allocator);
		}
		g.DestroySwapchainKHR(vkdevice, g.swapchain.h_old, g.allocator);
	}
}

void vk_image_upload(VkCommandBuffer cmd, VK_Image image, u64 stage_offset) {
	var& g = st->gfx;
	VkBufferImageCopy2 region = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
		.bufferOffset = g.stage_buffer.base + stage_offset,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = image.slices_count,
		},
		.imageExtent = { image.width, image.height, 1 },
	};
	VkCopyBufferToImageInfo2 info = {
		.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
		.srcBuffer = g.cpu_buf.h,
		.dstImage = image.h,
		.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.regionCount = 1,
		.pRegions = &region,
	};
	g.CmdCopyBufferToImage2(cmd, &info);
}

void vk_texture_generate_mipmaps(VkCommandBuffer cmd, VK_Image* image) {
	var& g = st->gfx;
	VkImageMemoryBarrier2 barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image->h,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.layerCount = 1,
			.levelCount = 1,
		},
	};
	VkDependencyInfo dep = {
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier,
	};
	i32 width = image->width;
	i32 height = image->height;
	LoopOff (i, 1, image->mipmaps_count) {
		barrier.subresourceRange.baseMipLevel = i - 1;

		// DST -> SRC
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
		barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
		g.CmdPipelineBarrier2(cmd, &dep);

		i32 old_width = width;
		i32 old_height = height;
		width = width > 1 ? width / 2 : 1;
		height = height > 1 ? height / 2 : 1;
		VkImageBlit2 region = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
			.srcOffsets[1] = {old_width, old_height, 1},
			.dstOffsets[1] = {width, height, 1},
			.srcSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = (u32)i - 1,
				.layerCount = 1,
			},
			.dstSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = (u32)i,
				.layerCount = 1,
			},
		};

		VkBlitImageInfo2 info = {
			.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
			.srcImage = image->h,
			.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.dstImage = image->h,
			.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.regionCount = 1,
			.pRegions = &region,
			.filter = VK_FILTER_LINEAR,
		};
		g.CmdBlitImage2(cmd, &info);

		// SRC -> SHADER_READ
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
		g.CmdPipelineBarrier2(cmd, &dep);
	}
	barrier.subresourceRange.baseMipLevel = image->mipmaps_count - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
	barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
	barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
	g.CmdPipelineBarrier2(cmd, &dep);
	image->cur_access = VK_Access_Texture;
}

////////////////////////////////////////////////////////////////////////
// @Gfx

u32 gfx_pixelformat_bytesize(Gfx_PixelFormat fmt) {
	switch (fmt) {
		InvalidDefaultCase;
		case Gfx_PixelFormat_R8_UI:
		case Gfx_PixelFormat_R8:
			return 1;
		case Gfx_PixelFormat_BGRA8:
		case Gfx_PixelFormat_BGRA8_SRGB:
		case Gfx_PixelFormat_RGBA8:
		case Gfx_PixelFormat_RGBA8_SRGB:
		case Gfx_PixelFormat_Depth:
		case Gfx_PixelFormat_DepthStencil:
			return 4;
	}
}

void gfx_image_desc_defaults(Gfx_ImageDesc* desc) {
	_DefSet(desc->type, Gfx_ImageType_2D);
	desc->usage |= Gfx_ImageUsage_Immutable;
	_DefSet(desc->slices_count, desc->type == Gfx_ImageType_Cube ? 6 : 1);
	if (flag_has(desc->usage, Gfx_ImageUsage_ColorAttachment)) {
		_DefSet(desc->pixel_format, Gfx_PixelFormat_DefaulttAttachment);
		_DefSet(desc->sample_count, Gfx_DefaultSampleCount);
	} else if (flag_has(desc->usage, Gfx_ImageUsage_DepthStencilAttachment)) {
		_DefSet(desc->pixel_format, Gfx_PixelFormat_DefaultDepth);
		_DefSet(desc->sample_count, Gfx_DefaultSampleCount);
	} else if (flag_has(desc->usage, Gfx_ImageUsage_ResolveAttachment)) {
		_DefSet(desc->pixel_format, Gfx_PixelFormat_DefaulttAttachment);
		_DefSet(desc->sample_count, 1);
	} else {
		_DefSet(desc->pixel_format, Gfx_PixelFormat_RGBA8);
		_DefSet(desc->sample_count, 1);
	}
}

void gfx_sampler_desc_defaults(Gfx_SamplerDesc* desc) {
	_DefSet(desc->min_filter, Gfx_Filter_Nearest);
	_DefSet(desc->mag_filter, Gfx_Filter_Nearest);
	_DefSet(desc->mipmap_filter, Gfx_Filter_Nearest);
	_DefSet(desc->wrap_u, Gfx_Wrap_Repeat);
	_DefSet(desc->wrap_v, Gfx_Wrap_Repeat);
	_DefSet(desc->wrap_w, Gfx_Wrap_Repeat);
	_DefSet(desc->max_lod, VK_LOD_CLAMP_NONE);
	_DefSet(desc->border_color, Gfx_BorderColor_OpaqueBlack);
	_DefSet(desc->compare, Gfx_CompareOp_Never);
	_DefSet(desc->max_anisotropy, 1);
}

void gfx_pass_defaults(Gfx_Pass* pass) {
	Gfx_PassAction& action = pass->action;
	Loop (i, Gfx_MaxColorAttachments) {
		if (action.colors[i].load_action == Gfx_LoadAction_Default) {
			action.colors[i].load_action = Gfx_LoadAction_Clear;
			action.colors[i].clear_value.v[0] = 0.3;
			action.colors[i].clear_value.v[1] = 0.3;
			action.colors[i].clear_value.v[2] = 0.3;
			action.colors[i].clear_value.v[3] = 1.0;
		}
		if (action.colors[i].store_action == Gfx_StoreAction_Default) {
			action.colors[i].store_action = Gfx_StoreAction_Store;
		}
	}
	if (action.depth.load_action == Gfx_LoadAction_Default) {
		action.depth.load_action = Gfx_LoadAction_Clear;
		action.depth.clear_value = 1.0;
	}
	if (action.depth.store_action == Gfx_StoreAction_Default) {
		action.depth.store_action = Gfx_StoreAction_DontCare;
	}
	if (action.stencil.load_action == Gfx_LoadAction_Default) {
		action.stencil.load_action = Gfx_LoadAction_Clear;
		action.stencil.clear_value = 1.0;
	}
	if (action.stencil.store_action == Gfx_StoreAction_Default) {
		action.stencil.store_action = Gfx_StoreAction_DontCare;
	}
}

void gfx_pipeline_common_init(VK_Pipeline* pip, Gfx_PipelineDesc desc) {
	pip->shd_ref = desc.shader;
	pip->compute = desc.compute;
	pip->depth = desc.depth;
	pip->stencil = desc.stencil;
	pip->color_count = desc.color_count;
	pip->primitive_type = desc.primitive_type;
	pip->cull_mode = desc.cull_mode;
	pip->face_winding = desc.face_winding;
	pip->sample_count = desc.sample_count;
	pip->blend_color = desc.blend_color;
	pip->alpha_to_coverage_enabled = desc.alpha_to_coverage_enabled;
	ArrayCopy(pip->colors, desc.colors);
}

void gfx_pipeline_common_init2(VK_Pipeline* pip, Gfx_PipelineDesc2 desc) {
	pip->shd_ref = desc.shader;
	pip->compute = desc.compute;
	pip->colors[0].pixel_format = desc.pixel_format;
	pip->depth.pixel_format = desc.depth_pixel_format;
	pip->sample_count = desc.sample_count;
	pip->blend_color = desc.blend_color;
	pip->alpha_to_coverage_enabled = desc.alpha_to_coverage_enabled;
}

Gfx_Shader gfx_make_shader(Slice<u8> code) {
	Scratch scratch;
	Gfx_State& g = st->gfx;
	VK_Shader shader = {};
	shader.h = vk_shader_create(code);
	Gfx_Shader res = pool_push(g.shaders, shader);
	return res;
}

Gfx_Pipeline gfx_make_pipeline(Gfx_PipelineDesc desc) {
	Gfx_State& g = st->gfx;

	_DefSet(desc.depth.pixel_format, Gfx_PixelFormat_DefaultDepth);
	_DefSet(desc.depth.compare, Gfx_CompareOp_Always);
	_DefSet(desc.stencil.front.compare, Gfx_CompareOp_Always);
	_DefSet(desc.stencil.front.fail_op, Gfx_StencilOp_Keep);
	_DefSet(desc.stencil.front.depth_fail_op, Gfx_StencilOp_Keep);
	_DefSet(desc.stencil.front.pass_op, Gfx_StencilOp_Keep);
	_DefSet(desc.stencil.back.compare, Gfx_CompareOp_Always);
	_DefSet(desc.stencil.back.fail_op, Gfx_StencilOp_Keep);
	_DefSet(desc.stencil.back.depth_fail_op, Gfx_StencilOp_Keep);
	_DefSet(desc.stencil.back.pass_op, Gfx_StencilOp_Keep);
	_DefSet(desc.color_count, 1);
	Loop (i, desc.color_count) {
		Gfx_ColorTargetState& col = desc.colors[i];
		_DefSet(col.pixel_format, Gfx_PixelFormat_DefaulttAttachment);
		_DefSet(col.write_mask, Gfx_ColorMask_RGBA);
		_DefSet(col.blend.src_factor_rgb, Gfx_BlendFactor_One);
		_DefSet(col.blend.dst_factor_rgb, Gfx_BlendFactor_Zero);
		_DefSet(col.blend.op_rgb, Gfx_BlendOp_Add);
		_DefSet(col.blend.src_factor_alpha, Gfx_BlendFactor_One);
		_DefSet(col.blend.dst_factor_alpha, Gfx_BlendFactor_Zero);
		_DefSet(col.blend.op_alpha, Gfx_BlendOp_Add);
	}
	_DefSet(desc.primitive_type, Gfx_PrimitiveType_Triangle);
	_DefSet(desc.cull_mode, Gfx_CullMode_None);
	_DefSet(desc.face_winding, Gfx_FaceWinding_CCW);
	_DefSet(desc.sample_count, Gfx_DefaultSampleCount);

	VK_Pipeline pipeline = {};
	gfx_pipeline_common_init(&pipeline, desc);
	pipeline.h = vk_pipeline_create(desc);
	Gfx_Pipeline res = pool_push(g.pipelines, pipeline);
	return res;
}

Gfx_Pipeline gfx_make_pipeline2(Gfx_PipelineDesc2 desc) {
	Gfx_State& g = st->gfx;

	_DefSet(desc.sample_count, Gfx_DefaultSampleCount);
	_DefSet(desc.depth_pixel_format, Gfx_PixelFormat_DefaultDepth);
	_DefSet(desc.pixel_format, Gfx_PixelFormat_DefaulttAttachment);

	VK_Pipeline pipeline = {};
	gfx_pipeline_common_init2(&pipeline, desc);
	pipeline.h = vk_pipeline_create2(desc);
	Gfx_Pipeline res = pool_push(g.pipelines, pipeline);
	return res;
}

Gfx_Image gfx_make_image(Gfx_ImageDesc desc) {
	Scratch scratch;
	var& g = st->gfx;
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
		u64 offset = 0;
		u64 size = desc.width * desc.height * gfx_pixelformat_bytesize(desc.pixel_format);
		if (desc.type == Gfx_ImageType_Cube) {
			Slice<Slice<u8>> data = push_slice(scratch, Slice<u8>, 6);
			Loop (i, 6) {
				data[i] = Slice(desc.cube[i], size);
			}
			offset = gfx_push_stage_buffers(data);
		} else {
			offset = gfx_push_stage_buffer(Slice(desc.data, size));
		}
		VkCommandBuffer cmd = g.upload_cmd;
		vk_cmd_begin(cmd);
		vk_image_barrier(cmd, &image, VK_Access_TransferDst);
		vk_image_upload(cmd, image, offset);
		if (desc.mipmaps) {
			vk_texture_generate_mipmaps(cmd, &image);
		} else {
			vk_image_barrier(cmd, &image, VK_Access_Texture);
		}
		vk_cmd_end(cmd);
		vk_cmd_submit(cmd);
	}
	Gfx_Image res = pool_push(g.images, image);
	return res;
}

Gfx_View gfx_make_view(Gfx_ViewDesc desc) {
	var& g = st->gfx;
	VK_View view = {.type = desc.type};
	VK_Image img = {};
	switch (view.type) {
		InvalidDefaultCase;
		case Gfx_ViewType_Texture: {
			Gfx_Image ref = desc.image;
			img = pool_get(g.images, ref);
			view.ref = ref;
			view.mip_level = desc.mip_level;
			view.mip_level_count = _Def(desc.mip_level_count, img.mipmaps_count - desc.mip_level);
			view.slice = desc.slice;
			switch (img.type) {
				InvalidDefaultCase;
				case Gfx_ImageType_2D:    view.slice_count = 1; break;
				case Gfx_ImageType_Cube:  view.slice_count = 6; break;
				case Gfx_ImageType_3D:    view.slice_count = 1; break;
				case Gfx_ImageType_Array: view.slice_count = _Def(desc.slice_count, img.slices_count - view.slice); break;
			}
		} break;
		case Gfx_ViewType_ColorAttachment: {
			Gfx_Image ref = desc.image;
			img = pool_get(g.images, ref);
			view.ref = ref;
			view.mip_level = desc.mip_level;
			view.mip_level_count = 1;
			view.slice = desc.slice;
			view.slice_count = 1;
		} break;
		case Gfx_ViewType_ResolveAttachment: {
			Gfx_Image ref = desc.image;
			img = pool_get(g.images, ref);
			view.ref = ref;
			view.mip_level = desc.mip_level;
			view.mip_level_count = 1;
			view.slice = desc.slice;
			view.slice_count = 1;
		} break;
		case Gfx_ViewType_DepthStencilAttachment: {
			Gfx_Image ref = desc.image;
			img = pool_get(g.images, ref);
			view.ref = ref;
			view.mip_level = desc.mip_level;
			view.mip_level_count = 1;
			view.slice = desc.slice;
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

	Gfx_View res = {pool_push(img.type == Gfx_ImageType_Cube ? g.cubemap_views : g.views, view)};
	VkDescriptorImageInfo descriptor_image_info = {
		.imageView = view.h,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};
	VkWriteDescriptorSet texture_descriptor = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = g.descriptor_set,
		.dstBinding = (img.type == Gfx_ImageType_2D || img.type == Gfx_ImageType_Array) ? Bindings::Textures : Bindings::CubeTextures,
		.dstArrayElement = res.idx,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		.pImageInfo = &descriptor_image_info,
	};
	g.UpdateDescriptorSets(vkdevice, 1, &texture_descriptor, 0, null);

	return res;
}

Gfx_Sampler gfx_make_sampler(Gfx_SamplerDesc desc) {
	Gfx_State& g = st->gfx;
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
		.dstSet = g.descriptor_set,
		.dstBinding = Bindings::Samplers,
		.dstArrayElement = res.idx,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
		.pImageInfo = &descriptor_image_info,
	};
	g.UpdateDescriptorSets(vkdevice, 1, &sampler_descriptor, 0, null);
	return res;
}

Gfx_Buffer gfx_make_buffer(u64 size, Gfx_MemType type, u64 align) {
	var& g = st->gfx;
	u64 offset = 0;
	switch (type) {
		InvalidDefaultCase;
		case Gfx_MemType_Cpu: {
			var& buffer= g.cpu_buf;
			offset = offset_push(buffer.pos, size, align);
			Assert(buffer.pos <= buffer.cap);
		} break;
		case Gfx_MemType_Gpu: {
			var& buffer = g.gpu_buf;
			offset = offset_push(buffer.pos, size, align);
			Assert(buffer.pos <= buffer.cap);
		} break;
	}
	Gfx_Buffer res = {
		.type = type,
		.base = offset,
		.size = size,
	};
	return res;
}

void gfx_make_bind(Gfx_DescriptorDesc desc) {
	Gfx_State& g = st->gfx;
	if (desc.count == 0) {
		desc.count = 1;
	}
	_DefSet(desc.type, Gfx_BindType_Storage);
	VK_DescriptorWriter& writer = g.descriptor_writer;
	VkDescriptorBindingFlags flags = {};
	if (desc.type == Gfx_BindType_Image) {
		flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	}
	writer.binding_flags[writer.binds_count] = flags;
	VkDescriptorType descriptor_type;
	switch (desc.type) {
		InvalidDefaultCase;
		case Gfx_BindType_Storage: descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
		case Gfx_BindType_Image: descriptor_type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; break;
		case Gfx_BindType_Sampler: descriptor_type = VK_DESCRIPTOR_TYPE_SAMPLER; break;
	}
	writer.bindings[writer.binds_count] = {
		.binding = desc.binding,
		.descriptorType = descriptor_type,
		.descriptorCount = desc.count,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
	};
	++writer.binds_count;
}

Gfx_PipelineDesc gfx_query_pipeline_desc(Gfx_Pipeline pip) {
	Gfx_State& g = st->gfx;
	VK_Pipeline pipeline = pool_get(g.pipelines, pip);
	Gfx_PipelineDesc res = {
		.shader = pipeline.shd_ref,
		.compute = pipeline.compute,
		.depth = pipeline.depth,
		.stencil = pipeline.stencil,
		.color_count = pipeline.color_count,
		.primitive_type = pipeline.primitive_type,
		.cull_mode = pipeline.cull_mode,
		.face_winding = pipeline.face_winding,
		.sample_count = pipeline.sample_count,
		.blend_color = pipeline.blend_color,
		.alpha_to_coverage_enabled = pipeline.alpha_to_coverage_enabled,
	};
	Loop (i, pipeline.color_count) {
			res.colors[i] = pipeline.colors[i];
	}
	return res;
}

Gfx_PipelineDesc2 gfx_query_pipeline_desc2(Gfx_Pipeline pip) {
	var& g = st->gfx;
	VK_Pipeline pipeline = pool_get(g.pipelines, pip);
	Gfx_PipelineDesc2 res = {
		.shader = pipeline.shd_ref,
		.compute = pipeline.compute,
		.sample_count = pipeline.sample_count,
		.blend_color = pipeline.blend_color,
		.alpha_to_coverage_enabled = pipeline.alpha_to_coverage_enabled,
		.pixel_format = pipeline.colors[0].pixel_format,
		.depth_pixel_format = pipeline.depth.pixel_format,
	};
	return res;
}

Gfx_ImageDesc gfx_query_image_desc(Gfx_Image img) {
	var& g = st->gfx;
	VK_Image image = pool_get(g.images, img);
	Gfx_ImageDesc res = {
		.type = image.type,
		.width = image.width,
		.height = image.height,
		.slices_count = image.slices_count,
		.usage = image.usage,
		.pixel_format = image.pixel_format,
		.sample_count = image.sample_count,
	};
	return res;
}

void gfx_update_shader(Gfx_Shader shd, Slice<u8> code) {
	var& g = st->gfx;
	VK_Shader& shader = pool_get(g.shaders, shd);
	gfx_idle();
	g.DestroyShaderModule(vkdevice, shader.h, g.allocator);
	shader.h = vk_shader_create(code);
}

void gfx_update_pipeline(Gfx_Pipeline pip, Gfx_PipelineDesc desc) {
	var& g = st->gfx;
	VK_Pipeline& pipeline = pool_get(g.pipelines, pip);
	g.DestroyPipeline(vkdevice, pipeline.h, g.allocator);
	gfx_pipeline_common_init(&pipeline, desc);
	pipeline.h = vk_pipeline_create(desc);
}

void gfx_update_pipeline2(Gfx_Pipeline pip, Gfx_PipelineDesc2 desc) {
	var& g = st->gfx;
	VK_Pipeline& pipeline = pool_get(g.pipelines, pip);
	g.DestroyPipeline(vkdevice, pipeline.h, g.allocator);
	gfx_pipeline_common_init2(&pipeline, desc);
	pipeline.h = vk_pipeline_create2(desc);
}

void gfx_update_image(Gfx_Image img, u8* data) {
	// var& g = st->gfx;
	// VK_Image& image = pool_get(g.images, img);
	// Assert(image.type == Gfx_ImageType_2D);
	// MemCopy(gfx_buffer_base_ptr(g.stage_buffer), data, image.width * image.height * 4);
	// VkCommandBuffer cmd = g.cmds_upload[0];
	// vk_cmd_begin(cmd);
	// vk_image_barrier(cmd, &image, VK_Access_TransferDst);
	// // vk_image_upload(cmd, image);
	// if (image.mipmaps_count > 1) {
	//   vk_texture_generate_mipmaps(image);
	// }
	// vk_cmd_end_submit(cmd);
}

void gfx_update_image(Gfx_Image img, Gfx_ImageDesc desc) {
	// var& g = st->gfx;
	// VK_Image& vkimg = pool_get(g.images, img);
	// if (vkimg.width == desc.width && vkimg.height == desc.height) {
	//   gfx_update_image(img, desc.data);
	//   return;
	// }
	// g.DestroyImage(vkdevice, vkimg.h, g.allocator);

	// gfx_image_desc_defaults(&desc);
	// vkimg.width = desc.width;
	// vkimg.height = desc.height;
	// if (desc.mipmaps) {
	//   vkimg.mipmaps_count = Floor(Log2(Max(vkimg.width, vkimg.height))) + 1;
	// }
	// VkImageCreateInfo image_info = {
	//   .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
	//   .flags = vk_image_create_flags(desc.type),
	//   .imageType = vk_image_type(desc.type),
	//   .format = vk_format(desc.pixel_format),
	//   .extent = {desc.width, desc.height},
	//   .mipLevels = vkimg.mipmaps_count,
	//   .samples = (VkSampleCountFlagBits)desc.sample_count,
	//   .tiling = VK_IMAGE_TILING_OPTIMAL,
	//   .usage = vk_image_usage(desc.usage),
	//   .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	//   .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	// };
	// if (desc.type == Gfx_ImageType_3D) {
	//   image_info.extent.depth = desc.slices_count;
	//   image_info.arrayLayers = 1;
	// } else {
	//   image_info.extent.depth = 1;
	//   image_info.arrayLayers = desc.slices_count;
	// }
	// VK_CHECK(g.CreateImage(vkdevice, &image_info, g.allocator, &vkimg.h));
	// VkMemoryRequirements requirements;
	// g.GetImageMemoryRequirements(vkdevice, vkimg.h, &requirements);

	// VkMemoryAllocateInfo alloc_info = {
	//   .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
	//   .allocationSize = requirements.size,
	//   .memoryTypeIndex = g.device.gpu_type_idx,
	// };
	// VK_CHECK(g.AllocateMemory(vkdevice, &alloc_info, g.allocator, &vkimg.memory));

	// MemFormatSize s = mem_format_size(requirements.size);
	// Info("image created: %f %s", s.size, s.format);
	// VK_CHECK(g.BindImageMemory(vkdevice, vkimg.h, vkimg.memory, 0));

	// if (desc.data) {
	//   if (desc.type == Gfx_ImageType_Cube) {
	//     u64 size = desc.width * desc.height * gfx_pixelformat_bytesize(desc.pixel_format);
	//     Loop (i, 6) {
	//       MemCopy(Offset(gfx_buffer_base_ptr(st->gfx.stage_buffer), size * i), desc.cube[i], size);
	//     }
	//   } else {
	//     MemCopy(gfx_buffer_base_ptr(g.stage_buffer), desc.data, desc.width * desc.height * gfx_pixelformat_bytesize(desc.pixel_format));
	//   }
	//   VkCommandBuffer cmd = g.cmds_upload[0];
	//   vk_cmd_begin(cmd);
	//   vk_image_barrier(cmd, &vkimg, VK_Access_TransferDst);
	//   vk_image_upload(cmd, vkimg);
	//   if (desc.mipmaps) {
	//     vk_texture_generate_mipmaps(vkimg);
	//     vkimg.cur_access = VK_Access_Texture | VK_Access_FragmentShader;
	//   }
	//   else {
	//     vk_image_barrier(cmd, &vkimg, VK_Access_Texture);
	//   }
	//   vk_cmd_end_submit(cmd);
	// }
}

void gfx_update_view(Gfx_View view, Gfx_ViewDesc desc) {
	var& g = st->gfx;
	VK_View& vkview = pool_get(g.views, view);
	g.DestroyImageView(vkdevice, pool_get(g.views, view).h, g.allocator);
	VK_Image img = pool_get(g.images, vkview.ref);

	vkview.mip_level_count = _Def(desc.mip_level_count, img.mipmaps_count - desc.mip_level);
	switch (img.type) {
		InvalidDefaultCase;
		case Gfx_ImageType_2D:    vkview.slice_count = 1; break;
		case Gfx_ImageType_Cube:  vkview.slice_count = 6; break;
		case Gfx_ImageType_3D:    vkview.slice_count = 1; break;
		case Gfx_ImageType_Array: vkview.slice_count = _Def(desc.slice_count, img.slices_count - vkview.slice);
	}

	VkImageViewCreateInfo view_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = img.h,
		.viewType = vkview.type == Gfx_ViewType_Texture ? vk_texture_image_view_type(img.type) : vk_attachment_image_view_type(img.type),
		.format = vk_format(img.pixel_format),
		.subresourceRange = {
			.aspectMask = vk_aspect_mask(img.pixel_format),
			.baseMipLevel = vkview.mip_level,
			.levelCount = vkview.mip_level_count,
			.baseArrayLayer = vkview.slice,
			.layerCount = vkview.slice_count,
		},
	};
	VK_CHECK(g.CreateImageView(vkdevice, &view_info, g.allocator, &vkview.h));

	VkDescriptorImageInfo descriptor_image_info = {
		.imageView = vkview.h,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};
	VkWriteDescriptorSet texture_descriptor = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = g.descriptor_set,
		.dstBinding = (img.type == Gfx_ImageType_2D || img.type == Gfx_ImageType_Array) ? Bindings::Textures : Bindings::CubeTextures,
		.dstArrayElement = view.idx,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		.pImageInfo = &descriptor_image_info,
	};
	g.UpdateDescriptorSets(vkdevice, 1, &texture_descriptor, 0, null);
}

void gfx_update_buffer(Gfx_Buffer dst, u64 offset, Slice<u8> data) {
	var& g = st->gfx;
	u64 stage_off = gfx_push_stage_buffer(data);
	vk_cmd_begin(g.upload_cmd);
	VkBufferCopy2 copy_region = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
		.srcOffset = g.stage_buffer.base + stage_off,
		.dstOffset = dst.base + offset,
		.size = data.size,
	};
	VkCopyBufferInfo2 info = {
		.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
		.srcBuffer = g.cpu_buf.h,
		.dstBuffer = g.gpu_buf.h,
		.regionCount = 1,
		.pRegions = &copy_region,
	};
	g.CmdCopyBuffer2(g.upload_cmd, &info);
	vk_cmd_end(g.upload_cmd);
	vk_cmd_submit(g.upload_cmd);
}

void gfx_destroy_image(Gfx_Image img) {
	Gfx_State& g = st->gfx;
	g.DestroyImage(vkdevice, pool_get(g.images, img).h, g.allocator);
	pool_remove(g.images, img);
}

void gfx_destroy_view(Gfx_View view) {
	Gfx_State& g = st->gfx;
	g.DestroyImageView(vkdevice, pool_get(g.views, view).h, g.allocator);
	pool_remove(g.views, view);
}

void gfx_destroy_shader(Gfx_Shader shd) {
	Gfx_State& g = st->gfx;
	g.DestroyShaderModule(vkdevice, pool_get(g.shaders, shd).h, g.allocator);
	pool_remove(g.shaders, shd);
}

void gfx_destroy_pipeline(Gfx_Pipeline pip) {
	Gfx_State& g = st->gfx;
	VK_Pipeline pipeline = pool_get(g.pipelines, pip);
	g.DestroyPipeline(vkdevice, pipeline.h, g.allocator);
	pool_remove(g.pipelines, pip);
}

void gfx_apply_viewport(Rng2 rect, b32 y_origin_at_bottom) {
	var& g = st->gfx;
	f32 y = rect.min.y;
	f32 height = rng2_height(rect);
	if (y_origin_at_bottom) {
		y = height - rect.min.y;
		height = -height;
	}
	VkViewport viewport = {
		.x = rect.min.x,
		.y = y,
		.width = rng2_width(rect),
		.height = height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	g.CmdSetViewport(vk_cur_cmd(), 0, 1, &viewport);
}

void gfx_apply_scissor(Rng2 rect) {
	Gfx_State& g = st->gfx;
	v2 dim = rng2_dim(rect);
	VkRect2D scissor = {
		.offset = {.x = (i32)rect.min.x, .y = (i32)rect.min.y},
		.extent = {
			.width = (u32)dim.x, 
			.height = (u32)dim.y,
		},
	};
	g.CmdSetScissor(vk_cur_cmd(), 0, 1, &scissor);
}

void gfx_apply_primitive_type(Gfx_PrimitiveType t)     { st->gfx.CmdSetPrimitiveTopology(vk_cur_cmd(), vk_primitive_topology(t)); }
void gfx_apply_cull_mode(Gfx_CullMode cm)              { st->gfx.CmdSetCullMode(vk_cur_cmd(), vk_cullmode(cm)); }
void gfx_apply_face_winding(Gfx_FaceWinding fw)        { st->gfx.CmdSetFrontFace(vk_cur_cmd(), vk_frontface(fw)); }
void gfx_apply_depth_test(b32 disable)                 { st->gfx.CmdSetDepthTestEnable(vk_cur_cmd(), !disable); }
void gfx_apply_depth_write(b32 enable)                 { st->gfx.CmdSetDepthWriteEnable(vk_cur_cmd(), enable); }
void gfx_apply_depth_compare(Gfx_CompareOp op)         { st->gfx.CmdSetDepthCompareOp(vk_cur_cmd(), vk_compare_op(op)); }
void gfx_apply_color_blend_enable(b32 enable)          { VkBool32 enabled = enable; st->gfx.CmdSetColorBlendEnableEXT(vk_cur_cmd(), 0, 1, &enabled); }
void gfx_apply_color_blend_equation(Gfx_BlendState bs) {
	VkColorBlendEquationEXT equation = {
		.srcColorBlendFactor = vk_blend_factor(bs.src_factor_rgb),
		.dstColorBlendFactor = vk_blend_factor(bs.dst_factor_rgb),
		.colorBlendOp = vk_blend_op(bs.op_rgb),
		.srcAlphaBlendFactor = vk_blend_factor(bs.src_factor_alpha),
		.dstAlphaBlendFactor = vk_blend_factor(bs.dst_factor_alpha),
		.alphaBlendOp = vk_blend_op(bs.op_alpha),
	};
	st->gfx.CmdSetColorBlendEquationEXT(vk_cur_cmd(), 0, 1, &equation);
};
void gfx_apply_color_blend_mask(Gfx_BlendState bs) { 
	VkColorComponentFlags write_mask = vk_color_write_mask(bs.write_mask);
	st->gfx.CmdSetColorWriteMaskEXT(vk_cur_cmd(), 0, 1, &write_mask);
}
void gfx_draw(u32 base_vert, u32 vert_count, u32 instance_count, u32 base_instance) {
	st->gfx.CmdDraw(vk_cur_cmd(), vert_count, instance_count, base_vert, base_instance);
}
void gfx_draw_indexed(u32 base_index, u32 index_count, u32 base_vert, u32 instance_count, u32 base_instance) {
	st->gfx.CmdDrawIndexed(vk_cur_cmd(), index_count, instance_count, base_index, base_vert, base_instance);
}
void gfx_draw_indirect(Gfx_IndirectDrawCall drawcall) {
	var& g = st->gfx;
	g.CmdDrawIndirect(vk_cur_cmd(), g.cpu_buf.h, g.drawcalls_buf.base + drawcall.base*sizeof(VK_IndirectDrawCall), drawcall.count, sizeof(VK_IndirectDrawCall));
}

void gfx_draw_indirect_mesh(Gfx_Mesh mesh, u32 id) {
	// var& g = st->gfx;
	// Gfx_BufferRegion buf = pool_get(g.buffers, g.drawcalls_buf);
	vk_push_constants({id});
	gfx_draw_mesh(mesh);
	// g.CmdDrawIndirect(vk_cur_cmd(), g.cpu_buf.h, buf.base + g.indirect_drawcall_cursor++ * sizeof(VK_IndirectDrawCall), 1, sizeof(VK_IndirectDrawCall));
}

void gfx_draw_indexed_indirect(Gfx_IndirectDrawCall drawcall) {
	var& g = st->gfx;
	g.CmdDrawIndexedIndirect(vk_cur_cmd(), g.cpu_buf.h, g.drawcalls_buf.base + drawcall.base*sizeof(VK_IndirectDrawCall), drawcall.count, sizeof(VK_IndirectDrawCall));
}

void gfx_draw_mesh(Gfx_Mesh mesh) {
	if (mesh.index_count) {
		gfx_draw_indexed(mesh.base_index, mesh.index_count, mesh.base_vert);
	} else {
		gfx_draw(mesh.base_vert, mesh.vert_count);
	}
}

void gfx_begin_pass(Gfx_Pass pass) {
	Gfx_State& g = st->gfx;
	gfx_pass_defaults(&pass);
	v2u win_size = os_window_size();
	g.cur_pass.action = pass.action;
	g.cur_pass.attachments = pass.attachments;
	
	///////////////////////////////////
	// Is swapchain?
	b32 is_swapchain_pass = true;
	Loop (i, Gfx_MaxColorAttachments) {
		if (pass.attachments.colors[i].idx) is_swapchain_pass = false;
		if (pass.attachments.resolves[i].idx) is_swapchain_pass = false;
	}
	if (pass.attachments.depth_stencil.idx) is_swapchain_pass = false;

	///////////////////////////////////
	// Barrier
	if (is_swapchain_pass) {
		g.cur_pass.size = win_size;
		vk_swapchain_beginpass_barrier(g.swapchain.images[g.current_image_idx], VK_Access_ColorAttachment);
	}
	else {
		// Color + Resolve
		Loop (i, Gfx_MaxColorAttachments) {
			if (pass.attachments.colors[i].idx == 0) break;
			Gfx_View color_view_id = pass.attachments.colors[i];
			VK_View& color_view = pool_get(g.views, color_view_id);
			VK_Image& color_image = pool_get(g.images, color_view.ref);
			if (i == 0) g.cur_pass.size = {color_image.width, color_image.height};
			if (pass.action.colors[i].load_action != Gfx_LoadAction_Load) {
				color_image.cur_access |= VK_Access_Discard;
			}
			vk_image_barrier(vk_cur_cmd(), &color_image, VK_Access_ColorAttachment);
			if (pass.attachments.resolves[i].idx) {
				VK_Image& resolve_image = pool_get(g.images, pool_get(g.views, pass.attachments.resolves[i]).ref);
				resolve_image.cur_access |= VK_Access_Discard;
				vk_image_barrier(vk_cur_cmd(), &resolve_image, VK_Access_ResolveAttachment);
			}
		}

		// Depth / Stencil
		if (pass.attachments.depth_stencil.idx) {
			Gfx_Image img = pool_get(g.views, pass.attachments.depth_stencil).ref;
			VK_Image& ds_image = pool_get(g.images, img);
			b32 has_stencil = ds_image.pixel_format == Gfx_PixelFormat_DepthStencil;
			if ((pass.action.depth.load_action != Gfx_LoadAction_Load) && (pass.action.stencil.load_action != Gfx_LoadAction_Load)) {
				ds_image.cur_access |= VK_Access_Discard;
			}
			VK_Access dst_access = VK_Access_DepthAttachment;
			if (has_stencil) {
				dst_access |= VK_Access_StencilAttachment;
			}
			vk_image_barrier(vk_cur_cmd(), &ds_image, dst_access);
		}
	}

	///////////////////////////////////
	// Begin attachments
	Gfx_PassAction action = pass.action;
	VkRenderingAttachmentInfo color_att_infos[Gfx_MaxColorAttachments] = {};
	VkRenderingAttachmentInfo depth_att_info = {};
	VkRenderingAttachmentInfo stencil_att_info = {};
	VkRenderingInfo render_info = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea.extent = {
			.width = g.cur_pass.size.x,
			.height = g.cur_pass.size.y,
		},
		.layerCount = 1,
	};

	if (is_swapchain_pass) {
		VkImageView vk_color_view = g.swapchain.views[g.current_image_idx];
		vk_init_color_attachment_info(&color_att_infos[0], action.colors[0], vk_color_view, null);
		render_info.colorAttachmentCount = 1;
		render_info.pColorAttachments = color_att_infos;
	}
	else {
		// Color
		u32 color_view_count = 0;
		Loop (i, Gfx_MaxColorAttachments) {
			if (pass.attachments.colors[i].idx == 0) break;
			VkImageView vk_color_view = pool_get(g.views, pass.attachments.colors[i]).h;
			VkImageView vk_resolve_view = pool_get(g.views, pass.attachments.resolves[i]).h;
			vk_init_color_attachment_info(&color_att_infos[i], action.colors[i], vk_color_view, vk_resolve_view);
			++color_view_count;
		}
		if (color_view_count) {
			render_info.colorAttachmentCount = color_view_count;
			render_info.pColorAttachments = color_att_infos;
		}

		// Depth / Stencil
		if (pass.attachments.depth_stencil.idx) {
			VK_View ds_view = pool_get(g.views, pass.attachments.depth_stencil);
			VK_Image ds_image = pool_get(g.images, ds_view.ref);
			vk_init_depth_attachment_info(&depth_att_info, action.depth, ds_view.h);
			render_info.pDepthAttachment = &depth_att_info;
			b32 has_stencil = ds_image.pixel_format == Gfx_PixelFormat_DepthStencil;
			if (has_stencil) {
				vk_init_stencil_attachment_info(&stencil_att_info, action.stencil, ds_view.h);
				render_info.pStencilAttachment = &stencil_att_info;
			}
		}
	}
	g.CmdBeginRendering(vk_cur_cmd(), &render_info);
}

void gfx_end_pass() {
	Gfx_State& g = st->gfx;
	g.CmdEndRendering(vk_cur_cmd());

	///////////////////////////////////
	// Barrier
	b32 is_swapchain_pass = true;
	Loop (i, Gfx_MaxColorAttachments) {
		if (g.cur_pass.attachments.colors[i].idx) is_swapchain_pass = false;
		if (g.cur_pass.attachments.resolves[i].idx) is_swapchain_pass = false;
	}
	if (g.cur_pass.attachments.depth_stencil.idx) is_swapchain_pass = false;

	if (is_swapchain_pass) {
		vk_swapchain_end_barrier(g.swapchain.images[g.current_image_idx], VK_Access_Present);
	}
	else {
		Loop (i, Gfx_MaxColorAttachments) {
			if (g.cur_pass.attachments.colors[i].idx == 0) break;
			if (g.cur_pass.action.colors[i].store_action == Gfx_StoreAction_Store) {
				VK_Image& img = pool_get(g.images, pool_get(g.views, g.cur_pass.attachments.colors[i]).ref);
				vk_image_barrier(vk_cur_cmd(), &img, VK_Access_Texture | VK_Access_FragmentShader);
			}
			if (g.cur_pass.attachments.resolves[i].idx) {
				VK_Image& img = pool_get(g.images, pool_get(g.views, g.cur_pass.attachments.resolves[i]).ref);
				vk_image_barrier(vk_cur_cmd(), &img, VK_Access_Texture | VK_Access_FragmentShader);
			}
		}
		if (g.cur_pass.attachments.depth_stencil.idx) {
			VK_Image& img = pool_get(g.images, pool_get(g.views, g.cur_pass.attachments.depth_stencil).ref);
			if (g.cur_pass.action.depth.store_action == Gfx_StoreAction_Store) {
				vk_image_barrier(vk_cur_cmd(), &img, VK_Access_Texture | VK_Access_FragmentShader);
			}
		}
	}
}

u64 gfx_push_buffer(Gfx_Buffer& arena, u64 size) {
	u64 res = offset_push(arena.pos, size);
	Assert(arena.pos <= arena.size);
	return res;
}

u64 gfx_push_stage_buffer(Slice<u8> buf) {
	var& g = st->gfx;
	u32 stage_buffer_size = g.stage_buffer.size;
	os_mutex_lock(g.stage_buffer_reserve_mutex);
	if (g.stage_buffer.pos + buf.size > stage_buffer_size) {
		g.stage_buffer.pos = 0;
	}
	u64 pos = g.stage_buffer.pos;
	g.stage_buffer.pos += buf.size;
	os_mutex_unlock(g.stage_buffer_reserve_mutex);
	u8* base = g.cpu_buf.base+g.stage_buffer.base;
	MemCopy(base + pos, buf.data, buf.size);
	return pos;
}

u64 gfx_push_stage_buffers(Slice<Slice<u8>> buffers) {
	var& g = st->gfx;
	u64 size = 0;
	for (var buf : buffers) {
		size += buf.size;
	}
	u32 stage_buffer_size = g.stage_buffer.size;
	os_mutex_lock(g.stage_buffer_reserve_mutex);
	if (g.stage_buffer.pos + size > stage_buffer_size) {
		g.stage_buffer.pos = 0;
	}
	u64 pos = g.stage_buffer.pos;
	g.stage_buffer.pos += size;
	os_mutex_unlock(g.stage_buffer_reserve_mutex);
	u8* base = g.cpu_buf.base+g.stage_buffer.base;
	u64 off = 0;
	for (var buf : buffers) {
		MemCopy(base + pos+off, buf.data, buf.size);
		off += buf.size;
	}
	return pos;
}

u32 gfx_push_stage_buffer_cmd(Gfx_StageBufferCmd cmd) {
	var& g = st->gfx;
	queue_push(g.stage_cmd_queue, cmd);
	return g.stage_cmd_counter+1;
}

u32 gfx_ready_counter() {
	var& g = st->gfx;
	return g.stage_cmd_ready_counter;
}

u32 gfx_begin_indirect() {
	return st->gfx.indirect_drawcall_cursor;
}
Gfx_IndirectDrawCall gfx_end_indirect(u32 base) {
	Gfx_IndirectDrawCall res = {
		.base = base,
		.count = st->gfx.indirect_drawcall_cursor - base,
	};
	return res;
}
void gfx_push_indirect_mesh(Gfx_Mesh mesh, u32 id, u32 instance_count) {
	VK_IndirectDrawCall info = {.base_instance = id};
	if (mesh.index_count) {
		info.index_draw_command = {
			.indexCount = mesh.index_count,
			.instanceCount = instance_count,
			.firstIndex = mesh.base_index,
			.vertexOffset = (i32)mesh.base_vert,
			.firstInstance = 0,
		};
	} else {
		info.draw_command = {
			.vertexCount = mesh.vert_count,
			.instanceCount = instance_count,
			.firstVertex = mesh.base_vert,
			.firstInstance = 0,
		};
	}
	VK_IndirectDrawCall* indirect_drawcalls = (VK_IndirectDrawCall*)gfx_buffer_base_ptr(st->gfx.drawcalls_buf);
	indirect_drawcalls[st->gfx.indirect_drawcall_cursor++] = info;
}

void gfx_push_indirect_instanced(Gfx_Mesh mesh, u32 count) {
	gfx_push_indirect_mesh(mesh, st->gfx.entity_cursor, count);
	st->gfx.entity_cursor += count;
}
void gfx_instance_set_indices(u32* indices) {
	st->gfx.base_index = indices;
}

u32* gfx_indirect_indices() { return st->gfx.base_index + st->gfx.entity_cursor; }
u8* gfx_buffer_base_ptr(Gfx_Buffer buf) { return vk_get_vkbuffer(buf).base + buf.base; }

void gfx_bind_pipeline(Gfx_Pipeline pip) {
	Gfx_State& g = st->gfx;
	g.CmdBindPipeline(vk_cur_cmd(), VK_PIPELINE_BIND_POINT_GRAPHICS, pool_get(g.pipelines, pip).h);
}

void gfx_bind_vert(Gfx_Buffer buf) {
	var& g = st->gfx;
	VkBuffer vkbuf = {};
	switch (buf.type) {
		InvalidDefaultCase;
		case Gfx_MemType_Cpu: vkbuf = g.cpu_buf.h; break;
		case Gfx_MemType_Gpu: vkbuf = g.gpu_buf.h; break;
	}
	VkDeviceSize offset = buf.base;
	g.CmdBindVertexBuffers(vk_cur_cmd(), 0, 1, &vkbuf, &offset);
}

void gfx_bind_index(Gfx_Buffer buf) {
	var& g = st->gfx;
	VkBuffer vkbuf = {};
	switch (buf.type) {
		InvalidDefaultCase;
		case Gfx_MemType_Cpu: vkbuf = g.cpu_buf.h; break;
		case Gfx_MemType_Gpu: vkbuf = g.gpu_buf.h; break;
	}
	VkDeviceSize offset = buf.base;
	g.CmdBindIndexBuffer(vk_cur_cmd(), vkbuf, offset, VK_INDEX_TYPE_UINT32);
}

void gfx_bind_buffer(Gfx_Buffer buf, u32 binding) {
	Gfx_State& g = st->gfx;
	VK_DescriptorWriter& writer = g.descriptor_writer;
	VK_Buffer vkbuf = vk_get_vkbuffer(buf);
	writer.buffers[writer.writes_count] = {
		.buffer = vkbuf.h,
		.offset = buf.base,
		.range = buf.size,
	};
	writer.writes[writer.writes_count] = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = g.descriptor_set,
		.dstBinding = binding,
		.dstArrayElement = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pBufferInfo = &writer.buffers[writer.writes_count],
	};
	++writer.writes_count;
}

void gfx_make_binding_buffers(Slice<Gfx_BufferDesc> descs) {
	for (var desc : descs) {
		gfx_make_bind({.binding = desc.binding});
		Gfx_Buffer buf = gfx_make_buffer(desc.size, _Def(desc.mem_type, Gfx_MemType_Cpu));
		if (desc.out_buffer) {
			*desc.out_buffer = buf;
		}
		*desc.out_cpu_ptr = gfx_buffer_base_ptr(buf);
		gfx_bind_buffer(buf, desc.binding);
	}
}

void gfx_flush() {
	Gfx_State& g = st->gfx;

	///////////////////////////////////
	// Descriptors
	VK_DescriptorWriter& writer = st->gfx.descriptor_writer;
	VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		.pNext = null,
		.bindingCount = writer.binds_count,
		.pBindingFlags = writer.binding_flags,
	};
	VkDescriptorSetLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = &binding_flags_info,
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
		.bindingCount = writer.binds_count,
		.pBindings = writer.bindings,
	};
	g.CreateDescriptorSetLayout(vkdevice, &layout_info, g.allocator, &g.descriptor_set_layout);
	VkDescriptorSetAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = g.descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &g.descriptor_set_layout,
	};
	VK_CHECK(g.AllocateDescriptorSets(vkdevice, &alloc_info, &g.descriptor_set));
	Loop (i, writer.writes_count) {
		writer.writes[i].dstSet = g.descriptor_set;
	}
	g.UpdateDescriptorSets(vkdevice, writer.writes_count, writer.writes, 0, null);

	///////////////////////////////////
	// Pipeline layout
	{
		VkPushConstantRange push_constant = {
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			.offset = 0,
			.size = sizeof(VK_PushConstant),
		};
		VkPipelineLayoutCreateInfo pipeline_layout_info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &st->gfx.descriptor_set_layout,
			.pushConstantRangeCount = 1,
			.pPushConstantRanges = &push_constant,
		};
		VK_CHECK(st->gfx.CreatePipelineLayout(vkdevice, &pipeline_layout_info, st->gfx.allocator, &st->gfx.pipeline_layout));
	}
}

void gfx_idle() { st->gfx.DeviceWaitIdle(vkdevice); }

void gfx_readback_image(Gfx_Image img, u8* dst) {
	// var& g = st->gfx;
	// VK_Image& image = pool_get(g.images, img);
	// var cmd = g.upload_cmd;
	// vk_cmd_begin(cmd);
	// vk_image_barrier(cmd, &image, VK_Access_TransferSrc);
	// VkBufferImageCopy2 region = {
	//   .bufferOffset = 0,
	//   .imageSubresource = {
	//     .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	//     .mipLevel = 0,
	//     .baseArrayLayer = 0,
	//     .layerCount = 1,
	//   },
	//   .imageExtent = {image.width, image.height, 1}
	// };
	// VkCopyImageToBufferInfo2 info = {
	//   .sType = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2,
	//   .srcImage = image.h,
	//   .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	//   .dstBuffer = g.cpu_buf.h,
	//   .regionCount = 1,
	//   .pRegions = &region,
	// };
	// g.CmdCopyImageToBuffer2(cmd, &info);
	// vk_cmd_end_submit(cmd);
	// MemCopy(dst, g.cpu_buf.base, image.width*image.height*4);
}

void gfx_init(Gfx_Environment environment) {
	Gfx_State& g = st->gfx;
	g.environment = environment;
	g.arena = arena_make(.name = "gfx arena");
	g.stage_buffer_reserve_mutex = os_mutex_make();

#if VulkanUseAllocator
	g._allocator = vk_allocator_create();
	g.allocator = &g._allocator;
#endif

	{
		ProfBlock("vulkan loader");
		vk_loader_load_core();
		vk_instance_init();
		vk_loader_load_instance();
		vk_surface_create();
		vk_device_init();
		vk_loader_load_device();
	}

	///////////////////////////////////
	// Device stuff
	{
		g.GetDeviceQueue(vkdevice, g.device.graphics_queue_family_idx, 0, &g.device.graphics_queue);
		g.GetDeviceQueue(vkdevice, g.device.transfer_queue_family_idx, 0, &g.device.transfer_queue);
		g.GetDeviceQueue(vkdevice, g.device.compute_queue_family_idx, 0, &g.device.compute_queue);
		Info("Queues obtained");
		VkCommandPoolCreateInfo graphics_pool_create_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = g.device.graphics_queue_family_idx,
		};
		VK_CHECK(g.CreateCommandPool(vkdevice, &graphics_pool_create_info, g.allocator, &g.device.cmd_pool));
		VK_CHECK(g.CreateCommandPool(vkdevice, &graphics_pool_create_info, g.allocator, &g.device.upload_cmd_pool));
		Info("Graphics command pool created");
		g.images_in_flight = g.device.surface_capabilities.minImageCount;
		g.frames_in_flight = g.images_in_flight - 1;
		// Choose a image format
		g.swapchain.format = g.device.surface_formats[0];
		Loop (i, g.device.surface_format_count) {
			VkSurfaceFormatKHR format = g.device.surface_formats[i];
			if (format.format == VK_FORMAT_B8G8R8A8_UNORM && // darker
			// if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
							format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
			{
				g.swapchain.format = format;
				break;
			}
		}
		// Choose present mode
		VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		Loop (i, g.device.present_mode_count) {
			VkPresentModeKHR mode = g.device.present_modes[i];
			if (mode == VK_PRESENT_MODE_FIFO_KHR) {
				present_mode = mode;
				break;
			}
		}
		g.swapchain.present_mode = present_mode;
		vk_swapchain_create();
		Info("Swapchain created");
		vk_cmd_alloc(g.device.cmd_pool, g.frames_in_flight, g.render_cmds);
		vk_cmd_alloc(g.device.upload_cmd_pool, 1, &g.upload_cmd);
		Info("Command buffers created");
	}

	// Sync
	{
		Loop (i, g.images_in_flight) {
			VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
			g.CreateSemaphore(vkdevice, &semaphore_create_info, g.allocator, &g.render_complete_semaphores[i]);
			g.CreateSemaphore(vkdevice, &semaphore_create_info, g.allocator, &g.image_available_semaphores[i]);
		}
		Loop (i, g.frames_in_flight) {
			VkFenceCreateInfo fence_create_info = {
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.flags = VK_FENCE_CREATE_SIGNALED_BIT,
			};
			VK_CHECK(g.CreateFence(vkdevice, &fence_create_info, g.allocator, &g.in_flight_fences[i]));
			// g.frames_upload_semaphores[i] = vk_semaphore_make(1);
		}
		g.stage_semaphore = vk_make_semaphore(0);

// VkSemaphoreTypeCreateInfo timelineInfo = {
//   .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
//   .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
//   .initialValue = 0
// };
// VkSemaphoreCreateInfo createInfo = {
//   .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
//   .pNext = &timelineInfo
// };
// VkSemaphore semaphore;
// g.CreateSemaphore(vkdevice, &createInfo, g.allocator, &semaphore);
	}

	{
		VkDescriptorPoolSize pool_sizes[] = {
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, Gfx_MaxStorageBuffers},
			{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, Gfx_MaxImages},
			{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, Gfx_MaxCubeTextures},
			{VK_DESCRIPTOR_TYPE_SAMPLER, Gfx_MaxSamplers},
		};
		VkDescriptorPoolCreateInfo pool_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
			.maxSets = 1,
			.poolSizeCount = ArrayCount(pool_sizes),
			.pPoolSizes = pool_sizes,
		};
		g.CreateDescriptorPool(vkdevice, &pool_info, g.allocator, &g.descriptor_pool);
	}

	u32 drawcall_mem = MaxEntities * sizeof(VK_IndirectDrawCall);
	g.cpu_mem = vk_mem_make(Gfx_MemType_Cpu, environment.cpu_mem_size);
	g.gpu_mem = vk_mem_make(Gfx_MemType_Gpu, environment.gpu_mem_size + environment.image_mem_size);
	g.cpu_buf = vk_make_buffer(Gfx_MemType_Cpu, environment.cpu_mem_size - drawcall_mem);
	g.gpu_buf = vk_make_buffer(Gfx_MemType_Gpu, environment.gpu_mem_size);

	VkBufferDeviceAddressInfoKHR gpu_address_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR,
		.buffer = g.gpu_buf.h,
	};
	g.gpu_buf_address = g.GetBufferDeviceAddress(vkdevice, &gpu_address_info);
	VkBufferDeviceAddressInfoKHR cpu_address_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR,
		.buffer = g.cpu_buf.h,
	};
	g.cpu_buf_address = g.GetBufferDeviceAddress(vkdevice, &cpu_address_info);

	g.drawcalls_buf = gfx_make_buffer(drawcall_mem, Gfx_MemType_Cpu);
	gfx_make_bind({.binding = Bindings::Drawinfo});
	gfx_bind_buffer(g.drawcalls_buf, Bindings::Drawinfo);
	g.stage_buffer = gfx_make_buffer(MB(50), Gfx_MemType_Cpu);
}

void gfx_shutdown() {
	Gfx_State& g = st->gfx;
	gfx_idle();
	g.DestroySurfaceKHR(g.instance, g.surface, g.allocator);
#if BUILD_DEBUG
	PFN_vkDestroyDebugUtilsMessengerEXT func = null;
	Assign(func, g.GetInstanceProcAddr(g.instance, "vkDestroyDebugUtilsMessengerEXT"));
	func(g.instance, g.debug_messenger, g.allocator);
#endif
	g.DestroyInstance(g.instance, g.allocator);
}

void gfx_begin() {
	ProfFunc;
	Gfx_State& g = st->gfx;

	{
		ProfBlock("rendering waiting");
		VK_CHECK(g.WaitForFences(vkdevice, 1, &g.in_flight_fences[g.current_frame_idx], true, U64_MAX));
		VK_CHECK(g.ResetFences(vkdevice, 1, &g.in_flight_fences[g.current_frame_idx]));
	}

	VkCommandBuffer cmd = vk_cur_cmd();
	vk_cmd_begin(cmd);

	g.swapchain_resized = false;
	v2u win_size = os_window_size();
	if (win_size != g.size) {
		g.size = win_size;
		g.swapchain_resized = true;
		gfx_idle();
		vk_swapchain_create();
		Info("Swapchain recreated x: %i y: %i", win_size.x, win_size.y);
	}

	g.CmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipeline_layout, 0, 1, &g.descriptor_set, 0, null);
	// Next image
	{
		ProfBlock("swapchain flip waiting");
		#if GFX_X11 // NOTE: on x11 errors
		VkResult res = g.AcquireNextImageKHR(vkdevice, g.swapchain.h, U64_MAX, vk_get_cur_image_available_semaphore(), null, &g.current_image_idx);
		if (res != VK_SUCCESS) {
				// Warn("%s", vk_result_string(res));
		}
		#else
		VK_CHECK(g_g.AcquireNextImageKHR(vkdevice, g_g.swapchain.handle, U64_MAX, image_available_semaphore, null, &image_index));
		#endif
	}

	u64 counter = 0;
	VK_CHECK(g.GetSemaphoreCounterValue(vkdevice, g.stage_semaphore, &counter));
	if (counter == g.stage_cmd_counter) {
		if (g.stage_cmd_busy) {
			++g.stage_cmd_ready_counter;
			g.stage_cmd_busy = false;
		}
		if (g.stage_cmd_queue.count) {
			vk_cmd_begin(g.upload_cmd);
			Loop (i, g.stage_cmd_queue.count) {
				var stage_cmd = queue_pop(g.stage_cmd_queue);
				switch (stage_cmd.type) {
					InvalidDefaultCase;
					case Gfx_CmdType_Texture: {
						var& vkimg = pool_get(g.images, stage_cmd.img);
						vk_image_barrier(g.upload_cmd, &vkimg, VK_Access_TransferDst);
						vk_image_upload(g.upload_cmd, vkimg, stage_cmd.stage_offset);
						if (vkimg.mipmaps_count > 1) {
							vk_texture_generate_mipmaps(g.upload_cmd, &vkimg);
						} else {
							vk_image_barrier(g.upload_cmd, &vkimg, VK_Access_Texture);
						}
					} break;
					case Gfx_CmdType_Mesh: {
						VkBufferCopy2 copy_region = {
							.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
							.srcOffset = g.stage_buffer.base + stage_cmd.stage_offset,
							.dstOffset = stage_cmd.buf.base + stage_cmd.buf_offset,
							.size = stage_cmd.size,
						};
						VkCopyBufferInfo2 info = {
							.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
							.srcBuffer = g.cpu_buf.h,
							.dstBuffer = g.gpu_buf.h,
							.regionCount = 1,
							.pRegions = &copy_region,
						};
						g.CmdCopyBuffer2(g.upload_cmd, &info);
					} break;
				}
			}
			vk_cmd_end(g.upload_cmd);
			VkCommandBufferSubmitInfo command_info = {
					.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
					.commandBuffer = g.upload_cmd,
			};
			VkSemaphoreSubmitInfo signal_info = {
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = g.stage_semaphore,
					.value = ++g.stage_cmd_counter,
			};
			VkSubmitInfo2 info = {
					.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
					.commandBufferInfoCount = 1,
					.pCommandBufferInfos = &command_info,
					.signalSemaphoreInfoCount = 1,
					.pSignalSemaphoreInfos = &signal_info,
			};
			VK_CHECK(g.QueueSubmit2(g.device.graphics_queue, 1, &info, null));
			g.stage_cmd_busy = true;
		}
	}
}

void gfx_end() {
	ProfFunc;
	var& g = st->gfx;

	g.base_index = null;
	g.entity_cursor = 0;
	g.indirect_drawcall_cursor = 0;

	VkCommandBuffer cmd = vk_cur_cmd();
	vk_cmd_end(cmd);

	///////////////////////////////////
	// Render
	VkSemaphoreSubmitInfo wait_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = vk_get_cur_image_available_semaphore(),
		.stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	};
	VkCommandBufferSubmitInfo command_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = vk_cur_cmd(),
	};
	VkSemaphoreSubmitInfo signal_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = vk_get_cur_render_complete_semaphore(),
	};
	VkSubmitInfo2 info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.waitSemaphoreInfoCount = 1,
		.pWaitSemaphoreInfos = &wait_info,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &command_info,
		.signalSemaphoreInfoCount = 1,
		.pSignalSemaphoreInfos = &signal_info,
	};
	VK_CHECK(g.QueueSubmit2(g.device.graphics_queue, 1, &info, g.in_flight_fences[g.current_frame_idx]));

	///////////////////////////////////
	// Present
	{
		VkSemaphore render_complete = vk_get_cur_render_complete_semaphore();
		VkPresentInfoKHR present_info = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &render_complete,
			.swapchainCount = 1,
			.pSwapchains = &g.swapchain.h,
			.pImageIndices = &g.current_image_idx,
		};
#if GFX_X11 // NOTE: on x11 errors
		VkResult res = g.QueuePresentKHR(g.device.graphics_queue, &present_info);
		NoOp(res);
		// if (res != VK_SUCCESS) {
		//   Error("%s", vk_result_string(res));
		// }
#else
		VK_CHECK(g_g.QueuePresentKHR(g_g.device.graphics_queue, &present_info));
#endif
	}
	g.current_frame_idx = (g.current_frame_idx + 1) % g.frames_in_flight;
	g.current_frame_idx_plus_one = (g.current_frame_idx_plus_one + 1) % (g.frames_in_flight+1);
}
