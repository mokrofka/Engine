#include "com.h"

VK_SyncInfo vk_sync_table[VK_Sync_COUNT] {
	[VK_Sync_Undefined] = { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0,
		VK_IMAGE_LAYOUT_UNDEFINED, false },
	[VK_Sync_TransferSrc] = { VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, false },
	[VK_Sync_TransferDst] = { VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, true },
	[VK_Sync_VertexIndexBuffer] = { VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT, VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_2_INDEX_READ_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED, false },
	[VK_Sync_IndirectBuffer] = { VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT, VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED, false },
	[VK_Sync_ShaderRead] = { VK_SyncShaderStages, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_UNIFORM_READ_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, false },
	[VK_Sync_Storage] = { VK_SyncShaderStages, VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
		VK_IMAGE_LAYOUT_GENERAL, true },
	[VK_Sync_ColorAttachment] = { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, true },
	[VK_Sync_DepthAttachment] = { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, true },
	[VK_Sync_Present] = { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, false },
	[VK_Sync_HostRead] = { VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_READ_BIT,
		VK_IMAGE_LAYOUT_GENERAL, false },
	[VK_Sync_General] = { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
		VK_IMAGE_LAYOUT_GENERAL, true },
};

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
		typedef u32 xcb_window_t;
		typedef struct xcb_connection_t xcb_connection_t;
		typedef u32 xcb_visualid_t;
		#include <vulkan/vulkan_xcb.h>
		void vk_surface_create() {
			ProfFunc;
			var& g = st->gfx;
			struct {
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
				if(!memory) { // NOTE: it happens
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
	switch(result) {
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

VkImageCreateFlags vk_image_create_flags(Gfx_ImageType t) {
	switch(t) {
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

VkImageUsageFlags vk_image_usage(Gfx_Format fmt) {
	VkImageUsageFlags res = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	if(fmt == Gfx_Format_DepthStencil) res |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	else res |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	return res;
}

VkFormat vk_format(Gfx_Format fmt) {
	switch(fmt) {
		InvalidDefaultCase;
		case Gfx_Format_R8_UI:          return VK_FORMAT_R8_UINT;
		case Gfx_Format_R8:             return VK_FORMAT_R8_UNORM;
		case Gfx_Format_BGRA8:          return VK_FORMAT_B8G8R8A8_UNORM;
		case Gfx_Format_BGRA8_SRGB:     return VK_FORMAT_B8G8R8A8_SRGB;
		case Gfx_Format_RGBA8:          return VK_FORMAT_R8G8B8A8_UNORM;
		case Gfx_Format_RGBA8_SRGB:     return VK_FORMAT_R8G8B8A8_SRGB;
		// case Gfx_Format_Depth:          return VK_FORMAT_D32_SFLOAT;
		case Gfx_Format_DepthStencil:   return VK_FORMAT_D32_SFLOAT_S8_UINT;
	}
}

VkImageAspectFlagBits vk_aspect(Gfx_Format fmt) {
	switch(fmt) {
		default:                      return VK_IMAGE_ASPECT_COLOR_BIT;
		case Gfx_Format_DepthStencil: return VkImageAspectFlagBits(VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT);
	};
}

VkPrimitiveTopology vk_primitive_topology(Gfx_PrimitiveType t) {
	switch(t) {
		InvalidDefaultCase;
		case Gfx_PrimitiveType_Point:         return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case Gfx_PrimitiveType_Line:          return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case Gfx_PrimitiveType_Triangle:      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case Gfx_PrimitiveType_TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	}
}

VkCullModeFlags vk_cullmode(Gfx_CullMode cm) {
	switch(cm) {
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
	switch(op) {
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
	switch(op) {
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
	switch(op) {
		InvalidDefaultCase;
		case Gfx_BlendOp_Add:             return VK_BLEND_OP_ADD;
		case Gfx_BlendOp_Subtract:        return VK_BLEND_OP_SUBTRACT;
		case Gfx_BlendOp_ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
		case Gfx_BlendOp_Min:             return VK_BLEND_OP_MIN;
		case Gfx_BlendOp_Max:             return VK_BLEND_OP_MAX;
	}
}

VkBlendFactor vk_blend_factor(Gfx_BlendFactor f) {
	switch(f) {
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
	if(m & Gfx_ColorMask_R) {
		res |= VK_COLOR_COMPONENT_R_BIT;
	}
	if(m & Gfx_ColorMask_G) {
		res |= VK_COLOR_COMPONENT_G_BIT;
	}
	if(m & Gfx_ColorMask_B) {
		res |= VK_COLOR_COMPONENT_B_BIT;
	}
	if(m & Gfx_ColorMask_A) {
		res |= VK_COLOR_COMPONENT_A_BIT;
	}
	return res;
}

VkAttachmentLoadOp vk_load_op(Gfx_LoadOp a) {
	switch(a) {
		default:                  return VK_ATTACHMENT_LOAD_OP_LOAD;
		case Gfx_LoadOp_Clear:    return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case Gfx_LoadOp_DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}
}

VkAttachmentStoreOp vk_store_op(Gfx_StoreOp a) {
	switch(a) {
		default:                return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		case Gfx_StoreOp_Store: return VK_ATTACHMENT_STORE_OP_STORE;
	}
}

VkImageViewType vk_image_view_type(Gfx_ImageType t) {
	switch(t) {
		InvalidDefaultCase;
		case Gfx_ImageType_2D:    return VK_IMAGE_VIEW_TYPE_2D;
		case Gfx_ImageType_Cube:  return VK_IMAGE_VIEW_TYPE_CUBE;
		case Gfx_ImageType_3D:    return VK_IMAGE_VIEW_TYPE_3D;
		case Gfx_ImageType_Array: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	}
}

VkFilter vk_sampler_minmag_filter(Gfx_Filter f) {
	switch(f) {
		InvalidDefaultCase;
		case Gfx_Filter_Nearest: return VK_FILTER_NEAREST;
		case Gfx_Filter_Linear:  return VK_FILTER_LINEAR;
	}
}

VkSamplerMipmapMode vk_sampler_mipmap_mode(Gfx_Filter f) {
	switch(f) {
		InvalidDefaultCase;
		case Gfx_Filter_Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case Gfx_Filter_Linear:  return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}
}

VkSamplerAddressMode vk_sampler_address_mode(Gfx_Wrap f) {
	switch(f) {
		InvalidDefaultCase;
		case Gfx_Wrap_Repeat:         return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case Gfx_Wrap_CalmpToEdge:    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case Gfx_Wrap_CalmpToBorder:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case Gfx_Wrap_MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	}
}

VkBorderColor vk_sampler_border_color(Gfx_BorderColor c) {
	switch(c) {
		InvalidDefaultCase;
		case Gfx_BorderColor_TransparentBlack: return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		case Gfx_BorderColor_OpaqueBlack:      return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		case Gfx_BorderColor_OpaqueWhite:      return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	}
}

b32 vk_sync_needed(VK_Sync prev, VK_Sync next) {
	return prev != next || vk_sync_table[next].writes;
}

VkImageMemoryBarrier2 vk_sync_make_image_barrier(VkImage image, VkImageSubresourceRange range, VK_Sync prev, VK_Sync next) {
	var p = vk_sync_table[prev];
	var n = vk_sync_table[next];
	Assert(next != VK_Sync_Undefined && "UNDEFINED is only a source state");
	Assert(n.layout != VK_IMAGE_LAYOUT_UNDEFINED && "this state is buffer-only");
	VkImageMemoryBarrier2 br = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcStageMask = p.stage,
		.srcAccessMask = p.writes ? p.access : 0,
		.dstStageMask = n.stage,
		.dstAccessMask = n.access,
		.oldLayout = p.layout,
		.newLayout = n.layout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange = range,
	};
	return br;
}

// VkBufferMemoryBarrier2 vk_sync_make_buffer_barrier(VK_SyncBuffer buf, VK_Sync prev, VK_Sync next) {
// 	var p = vk_sync_table[prev];
// 	var n = vk_sync_table[next];
// 	assert(next != VK_Sync_Undefined && "UNDEFINED is only a source state");
// 	VkBufferMemoryBarrier2 br = {
// 		.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
// 		.srcStageMask = p.stage,
// 		.srcAccessMask = p.writes ? p.access : 0,
// 		.dstStageMask = n.stage,
// 		.dstAccessMask = n.access,
// 		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
// 		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
// 		.buffer = buf.buffer,
// 		.offset = buf.offset,
// 		.size = buf.size,
// 	};
// 	return br;
// }

VK_SyncBatch vk_sync_begin(VkCommandBuffer cmd) {
	VK_SyncBatch res = {.cmd = cmd};
	return res;
}

void vk_sync_end(VK_SyncBatch& b) {
	var& g = st->gfx;
	if(b.images.count == 0 && b.buffers.count == 0) return;
	VkDependencyInfo dep = {
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.bufferMemoryBarrierCount = b.buffers.count,
		.pBufferMemoryBarriers = b.buffers.data,
		.imageMemoryBarrierCount = b.images.count,
		.pImageMemoryBarriers = b.images.data,
	};
	g.CmdPipelineBarrier2(b.cmd, &dep);
	array_clear(b.images);
	array_clear(b.buffers);
}

void vk_sync_image(VK_SyncBatch& b, VK_Image& img, VK_Sync next) {
	if(!vk_sync_needed(img.sync, next)) return;
	VkImageSubresourceRange range = {
		.aspectMask = vk_aspect(img.format),
		.baseMipLevel = 0,
		.levelCount = img.mip_count,
		.baseArrayLayer = 0,
		.layerCount = img.count,
	};
	array_push(b.images, vk_sync_make_image_barrier(img.h, range, img.sync, next));
	img.sync = next;
}

// void vk_sync_buffer(VK_SyncBatch& b, VK_SyncBuffer& buf, VK_Sync next) {
// 	if(!vk_sync_needed(buf.state, next)) return;
// 	array_push(b.buffers, vk_sync_make_buffer_barrier(buf, buf.state, next));
// 	buf.state = next;
// }

void vk_sync_image_now(VkCommandBuffer cmd, VK_Image& img, VK_Sync next) {
	var b = vk_sync_begin(cmd);
	vk_sync_image(b, img, next);
	vk_sync_end(b);
}

// void vk_sync_buffer_now(VkCommandBuffer cmd, VK_SyncBuffer& buf, VK_Sync next) {
// 	var b = vk_sync_begin(cmd);
// 	vk_sync_buffer(b, buf, next);
// 	vk_sync_end(b);
// }

void vk_sync_everything(VkCommandBuffer cmd) {
	var& g = st->gfx;
	VkMemoryBarrier2 mb = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
		.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
	};
	VkDependencyInfo dep = {
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.memoryBarrierCount = 1,
		.pMemoryBarriers = &mb,
	};
	g.CmdPipelineBarrier2(cmd, &dep);
}

VkSemaphore vk_get_cur_image_available_semaphore() { return st->gfx.image_available_semaphores[st->gfx.cur_frame_idx]; }
VkSemaphore vk_get_cur_render_complete_semaphore() { return st->gfx.render_complete_semaphores[st->gfx.current_image_idx]; }
VkCommandBuffer vk_cur_cmd()                       { return st->gfx.render_cmds[st->gfx.cur_frame_idx]; }

u32 vk_find_memory_idx(u32 type_filter, u32 property_flags) {
	VkPhysicalDeviceMemoryProperties memory_properties = st->gfx.device.memory;
	u32 idx = -1;
	Loop(i, memory_properties.memoryTypeCount) {
		if(bit_has(type_filter, i) && flag_has(memory_properties.memoryTypes[i].propertyFlags, property_flags)) {
			idx = i;
			break;
		}
	}
	Assert(idx != -1);
	return idx;
}

VkSemaphore vk_make_semaphore(u64 initial_counter) {
	var& g = st->gfx;
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
//   var& g = st->gfx;
//   u64 wait = semaphore.counter + wait_counter;
//   VkSemaphoreWaitInfo waitInfo = {
//     .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
//     .semaphoreCount = 1,
//     .pSemaphores = &semaphore.h,
//     .pValues = &wait
//   };
//   g.WaitSemaphores(vkdevice, &waitInfo, UINT64_MAX);
// }

void vk_make_cmd(VkCommandPool pool, u32 count, VkCommandBuffer* out) {
	VkCommandBufferAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = count,
	};
	VK_CHECK(st->gfx.AllocateCommandBuffers(vkdevice, &allocate_info, out));
}

void vk_begin_cmd(VkCommandBuffer cmd) {
	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	VK_CHECK(st->gfx.BeginCommandBuffer(cmd, &begin_info));
}

void vk_end_cmd(VkCommandBuffer cmd) {
	VK_CHECK(st->gfx.EndCommandBuffer(cmd));
}

void vk_submit_cmd(VkCommandBuffer cmd) {
	var& g = st->gfx;
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

VK_Memory vk_make_mem(u64 size, b32 gpu) {
	var& g = st->gfx;
	u32 mem_idx = gpu ? g.device.gpu_type_idx : g.device.cpu_type_idx;
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
	if(!gpu) {
		VK_CHECK(g.MapMemory(vkdevice, res.h, 0, size, 0, (void**)&res.mapped_mem));
	}
	return res;
}

VK_Buffer vk_make_buffer(u64 size, b32 gpu) {
	var& g = st->gfx;
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
	VkMemoryRequirements requirements;
	g.GetBufferMemoryRequirements(vkdevice, res.h, &requirements);

	// u32 mem_prop_flags = gpu ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	// u32 mem_idx = vk_find_memory_idx(requirements.memoryTypeBits, mem_prop_flags);
	// VkMemoryAllocateInfo alloc_info = {
	//   .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
	//   .allocationSize = requirements.size,
	//   .memoryTypeIndex = mem_idx,
	// };
	// NoOp(alloc_info);

	VK_Memory* mem = null;
	u64 offset = 0;
	if(gpu) {
		mem = &g.gpu_mem;
		offset = offset_push(mem->pos, size, requirements.alignment);
		Assert(mem->pos <= mem->cap);
	} else {
		mem = &g.cpu_mem;
		offset = offset_push(mem->pos, size, requirements.alignment);
		Assert(mem->pos <= mem->cap);
	}
	VK_CHECK(g.BindBufferMemory(vkdevice, res.h, mem->h, offset));
	res.base = Offset(mem->mapped_mem, offset);
	return res;
}

VK_Buffer vk_get_vkbuffer(Gfx_Buffer buf) {
	var& g = st->gfx;
	if(buf.gpu) return g.gpu_buf;
	return g.cpu_buf;
}

VkShaderModule vk_make_shader(Slice<u8> code) {
	var& g = st->gfx;
	VkShaderModuleCreateInfo module_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size,
		.pCode = (u32*)code.data,
	};
	VkShaderModule res;
	VK_CHECK(g.CreateShaderModule(vkdevice, &module_info, g.allocator, &res));
	return res;
}

VkPipeline vk_make_pipeline(Gfx_PipelineDesc desc) {
	var& g = st->gfx;

	// Dynamic rendering
	VkFormat color_formats = vk_format(desc.pixel_format);
	VkPipelineRenderingCreateInfo rendering_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &color_formats,
		.depthAttachmentFormat = vk_format(desc.depth_pixel_format),
		.stencilAttachmentFormat = desc.depth_pixel_format == Gfx_Format_DepthStencil ? vk_format(desc.depth_pixel_format) : VK_FORMAT_UNDEFINED,
	};

	VkPipelineVertexInputStateCreateInfo vertex_input_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	};

	VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.primitiveRestartEnable = false,
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
		.depthClampEnable = false,
		.rasterizerDiscardEnable = false,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.lineWidth = 1.0f,
	};

	VkPipelineColorBlendStateCreateInfo color_blend_state_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
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
		VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT,
		VK_DYNAMIC_STATE_SAMPLE_MASK_EXT,
		VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT
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
		// .pMultisampleState = &multisampling_info,
		.pDepthStencilState = null,
		.pColorBlendState = &color_blend_state_info,
		.pDynamicState = &dynamic_state_info,
		.layout = g.pipeline_layout,
	};
	VkPipeline res;
	VK_CHECK(g.CreateGraphicsPipelines(vkdevice, null, 1, &pipeline_info, g.allocator, &res));
	return res;
}

void vk_loader_load_core() {
	ProfFunc;
	var& g = st->gfx;
	g.lib = os_lib_open("libvulkan.so");
#define X(name) VK_GET_PROC(name)
	VK_GET_PROC_LIST
#undef X
}

void vk_loader_load_instance() {
	ProfFunc;
	var& g = st->gfx;
#define X(name) VK_INSTANCE_GET_PROC(name)
	VK_INSTANCE_GET_PROC_LIST
#undef X
}

void vk_loader_load_device() {
	ProfFunc;
	var& g = st->gfx;
#define X(name) VK_DEVICE_GET_PROC(name)
	VK_DEVICE_GET_PROC_LIST
#undef X
}

void vk_instance_init() {
	ProfFunc;
	Scratch scratch;
	var& g = st->gfx;
	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.apiVersion = VK_API_VERSION_1_4
	};
	var required_validation_layer_names = array_make<const char*>(scratch);
	var required_extensions = array_make<const char*>(scratch);
	const char* VK1_SURFACE_NAME = "VK_KHR_xcb_surface";
	array_push(required_extensions, VK_KHR_SURFACE_EXTENSION_NAME);
	array_push(required_extensions, VK1_SURFACE_NAME);

#if BUILD_DEBUG
	// Validation layer
	array_push(required_validation_layer_names, "VK_LAYER_KHRONOS_validation");
	Debug("Required layers:");
	Loop(i, required_validation_layer_names.count) {
		Debug(required_validation_layer_names[i]);
	}
	u32 available_layer_count = 0;
	VkLayerProperties* available_layers;
	{ProfBlock("enum inst props");
	VK_CHECK(g.EnumerateInstanceLayerProperties(&available_layer_count, null));
	available_layers = push_array(scratch, VkLayerProperties, available_layer_count);
	VK_CHECK(g.EnumerateInstanceLayerProperties(&available_layer_count, available_layers));
	}
	Loop(i, required_validation_layer_names.count) {
		b32 found = false;
		Loop(j, available_layer_count) {
			if(str_match(required_validation_layer_names[i], available_layers[j].layerName)) {
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
	Loop(i, required_extensions.count) {
		Debug(required_extensions[i]);
	}
	u32 extension_count = 0;
	VkExtensionProperties* props;
	{ProfBlock("enum inst ext");
	g.EnumerateInstanceExtensionProperties(null, &extension_count, null);
	props = push_array(scratch, VkExtensionProperties, extension_count);
	g.EnumerateInstanceExtensionProperties(null, &extension_count, props);
	}
	Loop(i, required_extensions.count) {
		b32 found = false;
		Loop(j, extension_count) {
			if(str_match(required_extensions[i], props[j].extensionName)) {
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
	{
		ProfBlock("init");
		VK_CHECK(g.CreateInstance(&instance_create_info, g.allocator, &g.instance));
	}
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

void vk_device_init() {
	ProfFunc;
	Scratch scratch;
	var& g = st->gfx;

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
				Loop(i, queue_family_count) {
					u32 flags = queue_families->queueFlags;
					if((flags & VK_QUEUE_GRAPHICS_BIT)) {
						device.graphics_queue_family_idx = i;
					}
					if((flags & VK_QUEUE_TRANSFER_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT)) {
						device.transfer_queue_family_idx = i;
					}
					if((flags & VK_QUEUE_COMPUTE_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT) && !(flags & VK_QUEUE_TRANSFER_BIT)) {
						device.compute_queue_family_idx = i;
					}
				}
				Info("Queue Families: Graphics %i, Compute %i, Transfer %i", device.graphics_queue_family_idx, device.compute_queue_family_idx, device.transfer_queue_family_idx);
			}

			///////////////////////////////////
			// Gpu Info
			Info("Available device: '%s'", String(device.properties.deviceName));
			switch(device.properties.deviceType) {
				InvalidDefaultCase;
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: {
					integrated_idx = i;
					Info("GPU type is Integrated");
				}break;
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: {
					discrete_available = true;
					discrete_idx = i;
					Info("GPU type is Descrete");
				}break;
			}
			Info("GPU Driver version: %i.%i.%i", VK_VERSION_MAJOR(device.properties.driverVersion), VK_VERSION_MINOR(device.properties.driverVersion), VK_VERSION_PATCH(device.properties.driverVersion));
			Info("GPU API version: %i.%i.%i", VK_VERSION_MAJOR(device.properties.apiVersion), VK_VERSION_MINOR(device.properties.apiVersion), VK_VERSION_PATCH(device.properties.apiVersion));
			Loop(i, device.memory.memoryHeapCount) {
				f64 mem_size = (f64)device.memory.memoryHeaps[i].size / GB(1);
				if(device.memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
					Info("Heap %i: GPU: %.2f GiB", i, mem_size);
				else
					Info("Heap %i: System: %.2f GiB", i, mem_size);
			}

			///////////////////////////////////
			// Find cpu/gpu memory type index
			Loop(i, device.memory.memoryTypeCount) {
				VkMemoryType t = device.memory.memoryTypes[i];
				if(t.propertyFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
					device.gpu_type_idx = i;
					break;
				}
			}
			Loop(i, device.memory.memoryTypeCount) {
				VkMemoryType t = device.memory.memoryTypes[i];
				if(flag_any(t.propertyFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
					device.cpu_type_idx = i;
					break;
				}
			}
		}
		if(discrete_available) {
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
		b32 transfer_has = false;
		b32 compute_has = false;
		for(u32 x : indices) {
			if(x == g.device.transfer_queue_family_idx) transfer_has = true;
			if(x == g.device.compute_queue_family_idx) compute_has = true;
		}
		if(!transfer_has) array_push(indices, g.device.transfer_queue_family_idx);
		if(!compute_has) array_push(indices, g.device.compute_queue_family_idx);
		Array<VkDeviceQueueCreateInfo, queue_count> queue_create_infos = {};
		Loop(i, indices.count) {
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
			.extendedDynamicState3RasterizationSamples = true,
			.extendedDynamicState3SampleMask = true,
			.extendedDynamicState3AlphaToCoverageEnable = true,
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
			.shaderInt8 = true,
			.storageBuffer8BitAccess = true,
			.uniformAndStorageBuffer8BitAccess = true,
		};
		VkPhysicalDeviceVulkan11Features vulkan11 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
			.pNext = &vulkan12,
			.shaderDrawParameters = true, // Indirect drawing
			.storageBuffer16BitAccess = true,
			.uniformAndStorageBuffer16BitAccess = true,
		};
		VkPhysicalDeviceFeatures2 features = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &vulkan11,
			.features = {
				.multiDrawIndirect = true, // Request indirect drawing
				.fillModeNonSolid = true,  // Request anistrophy
				.samplerAnisotropy = true, // Request wireframe
				.shaderInt64 = true,       // Request 64bit integers
#if BUILD_DEV
				.pipelineStatisticsQuery = true,
#endif
				.shaderInt16 = true,
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
	var& g = st->gfx;
	if(g.swapchain.h_old) {
		g.swapchain.h_old = g.swapchain.h;
		ArrayCopy(g.swapchain.old_view, g.swapchain.views);
	}
	VK_CHECK(g.GetPhysicalDeviceSurfaceCapabilitiesKHR(g.device.physical_device, g.surface, &g.device.surface_capabilities));
	v2u win_size = os_win_size();
	VkExtent2D swapchain_extent = {win_size.x, win_size.y};
	if(g.device.surface_capabilities.currentExtent.width != U32_MAX) {
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
		.imageFormat = g.swapchain.surface_format.format,
		.imageColorSpace = g.swapchain.surface_format.colorSpace,
		.imageExtent = swapchain_extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.preTransform = g.device.surface_capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = g.swapchain.present_mode,
		.clipped = true,
		.oldSwapchain = g.swapchain.h_old ? g.swapchain.h : null,
	};
	VK_CHECK(g.CreateSwapchainKHR(vkdevice, &swapchain_create_info, g.allocator, &g.swapchain.h));
	u32 image_count = g.images_in_flight;
	VK_CHECK(g.GetSwapchainImagesKHR(vkdevice, g.swapchain.h, &image_count, g.swapchain.images));
	Loop(i, image_count) {
		VkImageViewCreateInfo view_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = g.swapchain.images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = g.swapchain.surface_format.format,
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
	if(g.swapchain.h_old) {
		Loop(i, g.images_in_flight) {
			g.DestroyImageView(vkdevice, g.swapchain.old_view[i], g.allocator);
		}
		g.DestroySwapchainKHR(vkdevice, g.swapchain.h_old, g.allocator);
	}
}

void vk_upload_image(VkCommandBuffer cmd, VK_Image image, u64 stage_offset) {
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
			.layerCount = image.count,
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

void vk_make_mipmaps(VkCommandBuffer cmd, VK_Image& image) {
	var& g = st->gfx;
	VkImageMemoryBarrier2 barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image.h,
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
	i32 width = image.width;
	i32 height = image.height;
	LoopOff(i, 1, image.mip_count) {
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
			.srcImage = image.h,
			.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.dstImage = image.h,
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
	barrier.subresourceRange.baseMipLevel = image.mip_count - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
	barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
	barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
	g.CmdPipelineBarrier2(cmd, &dep);
	image.sync = VK_Sync_ShaderRead;
}

////////////////////////////////////////////////////////////////////////
// @Gfx
u32 gfx_format_bytesize(Gfx_Format fmt) {
	switch(fmt) {
		InvalidDefaultCase;
		case Gfx_Format_R8_UI:
		case Gfx_Format_R8:
			return 1;
		case Gfx_Format_BGRA8:
		case Gfx_Format_BGRA8_SRGB:
		case Gfx_Format_RGBA8:
		case Gfx_Format_RGBA8_SRGB:
		// case Gfx_Format_Depth:
		case Gfx_Format_DepthStencil:
			return 4;
	}
}

void gfx_pipeline_common_init(VK_Pipeline* pip, Gfx_PipelineDesc desc) {
	pip->shader = desc.shader;
	pip->compute = desc.compute;
	pip->pixel_format = desc.pixel_format;
	pip->depth_pixel_format = desc.depth_pixel_format;
	pip->blend_color = desc.blend_color;
}

Gfx_Shader gfx_make_shader(Slice<u8> code) {
	Scratch scratch;
	var& g = st->gfx;
	VK_Shader shader = {};
	shader.h = vk_make_shader(code);
	Gfx_Shader res = pool_push(g.shaders, shader);
	return res;
}

Gfx_Pipeline gfx_make_pipeline(Gfx_PipelineDesc desc) {
	var& g = st->gfx;
	_DefSet(desc.depth_pixel_format, Gfx_Format_DepthStencil);
	_DefSet(desc.pixel_format, g.swapchain.format);
	VK_Pipeline pipeline = {};
	gfx_pipeline_common_init(&pipeline, desc);
	pipeline.h = vk_make_pipeline(desc);
	Gfx_Pipeline res = pool_push(g.pipelines, pipeline);
	return res;
}

Gfx_Image gfx_make_image(Gfx_ImageDesc desc) {
	Scratch scratch;
	var& g = st->gfx;
	_DefSet(desc.type, Gfx_ImageType_2D);
	_DefSet(desc.slices_count, desc.type == Gfx_ImageType_Cube ? 6 : 1);
	_DefSet(desc.format, Gfx_Format_RGBA8);
	_DefSet(desc.samples, 1);
	VK_Image image = {
		.type = desc.type,
		.width = desc.width,
		.height = desc.height,
		.count = desc.slices_count,
		.mip_count = 1,
		.format = desc.format,
		.sample_count = desc.samples,
	};
	if(desc.mipmaps) {
		image.mip_count = Floor(Log2(Max(image.width, image.height))) + 1;
	}
	VkImageCreateInfo image_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.flags = vk_image_create_flags(desc.type),
		.imageType = vk_image_type(desc.type),
		.format = vk_format(desc.format),
		.extent = {desc.width, desc.height},
		.mipLevels = image.mip_count,
		.samples = VkSampleCountFlagBits(desc.samples),
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = vk_image_usage(desc.format),
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};
	if(desc.type == Gfx_ImageType_3D) {
		image_info.extent.depth = desc.slices_count;
		image_info.arrayLayers = 1;
	} else {
		image_info.extent.depth = 1;
		image_info.arrayLayers = desc.slices_count;
	}
	VK_CHECK(g.CreateImage(vkdevice, &image_info, g.allocator, &image.h));
	VkMemoryRequirements requirements;
	g.GetImageMemoryRequirements(vkdevice, image.h, &requirements);
	g.images_mem_usage += requirements.size;
	image.mem_size = requirements.size;

	VkMemoryAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = requirements.size,
		.memoryTypeIndex = g.device.gpu_type_idx,
	};
	VK_CHECK(g.AllocateMemory(vkdevice, &alloc_info, g.allocator, &image.memory));

	MemFormatSize s = mem_format_size(requirements.size);
	Info("image created: %f %s", s.size, s.format);
	VK_CHECK(g.BindImageMemory(vkdevice, image.h, image.memory, 0));
	Gfx_Image res = pool_push(g.images, image);
	return res;
}

Gfx_View gfx_make_view(Gfx_ViewDesc desc) {
	var& g = st->gfx;
	VK_Image img = pool_get(g.images, desc.image);
	VK_View view = {
		.img = desc.image,
		.mip_level = desc.mip_level,
		.mip_level_count = _Def(desc.mip_level_count, img.mip_count - desc.mip_level),
		.slice = desc.slice,
	};
	switch(img.type) {
		InvalidDefaultCase;
		case Gfx_ImageType_2D:    view.slice_count = 1; break;
		case Gfx_ImageType_Cube:  view.slice_count = 6; break;
		case Gfx_ImageType_3D:    view.slice_count = 1; break;
		case Gfx_ImageType_Array: view.slice_count = _Def(desc.slice_count, img.count - view.slice); break;
	}

	u32 aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	if(img.format == Gfx_Format_DepthStencil) aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
	if(desc.stencil) aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
	VkImageViewCreateInfo view_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = img.h,
		.viewType = vk_image_view_type(img.type),
		.format = vk_format(img.format),
		.subresourceRange = {
			.aspectMask = aspect,
			.baseMipLevel = view.mip_level,
			.levelCount = view.mip_level_count,
			.baseArrayLayer = view.slice,
			.layerCount = view.slice_count,
		},
	};
	VK_CHECK(g.CreateImageView(vkdevice, &view_info, g.allocator, &view.h));

	Gfx_View res = {};
	if(img.type == Gfx_ImageType_Cube) res = pool_push(g.cubemap_views, view);
	else res = pool_push(g.views, view);
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
	var& g = st->gfx;
	_DefSet(desc.min_filter, Gfx_Filter_Nearest);
	_DefSet(desc.mag_filter, Gfx_Filter_Nearest);
	_DefSet(desc.mipmap_filter, Gfx_Filter_Nearest);
	_DefSet(desc.wrap_u, Gfx_Wrap_Repeat);
	_DefSet(desc.wrap_v, Gfx_Wrap_Repeat);
	_DefSet(desc.wrap_w, Gfx_Wrap_Repeat);
	_DefSet(desc.max_lod, VK_LOD_CLAMP_NONE);
	_DefSet(desc.border_color, Gfx_BorderColor_OpaqueBlack);
	_DefSet(desc.compare, Gfx_CompareOp_Never);
	_DefSet(desc.max_anisotropy, 1);
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

Gfx_Buffer gfx_make_buffer(u64 size, b32 gpu, u64 align) {
	var& g = st->gfx;
	u64 offset = 0;
	if(gpu) {
		var& buffer = g.gpu_buf;
		offset = offset_push(buffer.pos, size, align);
		Assert(buffer.pos <= buffer.cap);
	} else {
		var& buffer = g.cpu_buf;
		offset = offset_push(buffer.pos, size, align);
		Assert(buffer.pos <= buffer.cap);
	}
	Gfx_Buffer res = {
		.gpu = gpu,
		.base = offset,
		.size = size,
	};
	return res;
}

Gfx_PipelineDesc gfx_query_pipeline_desc(Gfx_Pipeline pip) {
	var& g = st->gfx;
	VK_Pipeline pipeline = pool_get(g.pipelines, pip);
	Gfx_PipelineDesc res = {
		.shader = pipeline.shader,
		.compute = pipeline.compute,
		.blend_color = pipeline.blend_color,
		.pixel_format = pipeline.pixel_format,
		.depth_pixel_format = pipeline.depth_pixel_format,
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
		.slices_count = image.count,
		// .usage = image.usage,
		.format = image.format,
		.samples = image.sample_count,
	};
	return res;
}

void gfx_update_shader(Gfx_Shader shd, Slice<u8> code) {
	var& g = st->gfx;
	VK_Shader& shader = pool_get(g.shaders, shd);
	gfx_idle();
	g.DestroyShaderModule(vkdevice, shader.h, g.allocator);
	shader.h = vk_make_shader(code);
}

void gfx_update_pipeline(Gfx_Pipeline pip, Gfx_PipelineDesc desc) {
	var& g = st->gfx;
	VK_Pipeline& pipeline = pool_get(g.pipelines, pip);
	g.DestroyPipeline(vkdevice, pipeline.h, g.allocator);
	gfx_pipeline_common_init(&pipeline, desc);
	pipeline.h = vk_make_pipeline(desc);
}

u64 gfx_update_image(Gfx_Image img, Slice<Slice<u8>> data, b32 sync) {
	var& g = st->gfx;
	var image = pool_get(g.images, img);
	u64 whole_size = 0;
	for(var buf : data) whole_size += buf.size;
	For {
		os_mutex_lock(g.stage_mutex);
		u64 stage_off;
		if(ring_try_alloc(g.stage_ring, whole_size, &stage_off)) {
			u64 off = 0;
			for(var buf : data) {
				MemCopy(g.stage_ring.base + stage_off + off, buf);
				off += buf.size;
			}
			if(sync) {
				var cmd = g.upload_cmd;
				vk_begin_cmd(cmd);
				vk_sync_image_now(cmd, image, VK_Sync_TransferDst);
				vk_upload_image(cmd, image, stage_off);
				if(image.mip_count > 1) vk_make_mipmaps(cmd, image);
				vk_sync_image_now(cmd, image, VK_Sync_ShaderRead);
				vk_end_cmd(cmd);
				vk_submit_cmd(cmd);
				os_mutex_unlock(g.stage_mutex);
				return 0;
			} else {
				queue_push(g.stage_cmd_queue, {.type = Gfx_StageType_Texture, .stage_off = stage_off, .img = img});
				os_mutex_unlock(g.stage_mutex);
				return atomic_load(&g.stage_next_value);
			}
		}
		os_mutex_unlock(g.stage_mutex);
		os_sleep_ms(1);
	}
}

u64 gfx_update_buffer(Gfx_Buffer buf, u64 off, Slice<u8> data, b32 sync) {
	var& g = st->gfx;
	For {
		os_mutex_lock(g.stage_mutex);
		u64 stage_off;
		if(ring_try_alloc(g.stage_ring, data.size, &stage_off)) {
			MemCopy(g.stage_ring.base + stage_off, data.data, data.size);
			if(sync) {
				var cmd = g.upload_cmd;
				vk_begin_cmd(cmd);
				VkBufferCopy2 copy_region = {
					.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
					.srcOffset = g.stage_buffer.base + stage_off,
					.dstOffset = buf.base + off,
					.size = data.size,
				};
				VkCopyBufferInfo2 info = {
					.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
					.srcBuffer = g.cpu_buf.h,
					.dstBuffer = g.gpu_buf.h,
					.regionCount = 1,
					.pRegions = &copy_region,
				};
				g.CmdCopyBuffer2(cmd, &info);
				vk_end_cmd(cmd);
				vk_submit_cmd(cmd);
				os_mutex_unlock(g.stage_mutex);
				return 0;
			} else {
				queue_push(g.stage_cmd_queue, {.type = Gfx_StageType_Buffer, .stage_off = stage_off, .buf = buf, .buf_off = off});
				os_mutex_unlock(g.stage_mutex);
				return atomic_load(&g.stage_next_value);
			}
		}
		os_mutex_unlock(g.stage_mutex);
		os_sleep_ms(1);
	}
}

u64 gfx_update_vert_index(Gfx_Buffer vert_buf, u64 vert_off, Gfx_Buffer index_buf, u64 index_off, Slice<u8> vert_data, Slice<u8> index_data) {
	var& g = st->gfx;
	For {
		os_mutex_lock(g.stage_mutex);
		u64 stage_off;
		if(ring_try_alloc(g.stage_ring, vert_data.size + index_data.size, &stage_off)) {
			MemCopy(g.stage_ring.base + stage_off, vert_data);
			MemCopy(g.stage_ring.base + stage_off + vert_data.size, index_data);
			queue_push(g.stage_cmd_queue, {
				.type = Gfx_StageType_VertIndex,
				.stage_off = stage_off,
				.buf = vert_buf,
				.buf_off = vert_off,
				.index_buf = index_buf,
				.index_off = index_off,
				.buf_size = vert_data.size,
				.index_size = index_data.size,
			});
			os_mutex_unlock(g.stage_mutex);
			return atomic_load(&g.stage_next_value);
		}
		os_mutex_unlock(g.stage_mutex);
		os_sleep_ms(1);
	}
}

void gfx_destroy_image(Gfx_Image img) {
	var& g = st->gfx;
	var& vkimg = pool_get(g.images, img);
	g.DestroyImage(vkdevice, vkimg.h, g.allocator);
	g.images_mem_usage -= vkimg.mem_size;
	pool_remove(g.images, img);
}

void gfx_destroy_view(Gfx_View view) {
	var& g = st->gfx;
	g.DestroyImageView(vkdevice, pool_get(g.views, view).h, g.allocator);
	pool_remove(g.views, view);
}

void gfx_destroy_shader(Gfx_Shader shd) {
	var& g = st->gfx;
	g.DestroyShaderModule(vkdevice, pool_get(g.shaders, shd).h, g.allocator);
	pool_remove(g.shaders, shd);
}

void gfx_destroy_pipeline(Gfx_Pipeline pip) {
	var& g = st->gfx;
	VK_Pipeline pipeline = pool_get(g.pipelines, pip);
	g.DestroyPipeline(vkdevice, pipeline.h, g.allocator);
	pool_remove(g.pipelines, pip);
}

void gfx_apply_viewport(Rng2 rect, b32 y_origin_at_bottom) {
	var& g = st->gfx;
	f32 y = rect.min.y;
	f32 height = rng2_height(rect);
	if(y_origin_at_bottom) {
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
	var& g = st->gfx;
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
void gfx_apply_sample(u32 sample_count) {
	VkSampleCountFlagBits s = VkSampleCountFlagBits(sample_count);
	st->gfx.CmdSetRasterizationSamplesEXT(vk_cur_cmd(), s);
}
void gfx_apply_sample_mask(u32 sample_count, u32 mask) {
	VkSampleCountFlagBits s = VkSampleCountFlagBits(sample_count);
	VkSampleMask m = ~VkSampleMask();
	st->gfx.CmdSetSampleMaskEXT(vk_cur_cmd(), s, &m);
}
void gfx_apply_alpha_to_coverage(b32 enable) {
	st->gfx.CmdSetAlphaToCoverageEnableEXT(vk_cur_cmd(), false);
}

void gfx_push_ctx(Gfx_DrawCtx constants) {
	st->gfx.CmdPushConstants(vk_cur_cmd(), st->gfx.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Gfx_DrawCtx), &constants);
}
void gfx_draw(u32 base_vert, u32 vert_count, u32 instance_count, u32 base_instance) {
	st->gfx.CmdDraw(vk_cur_cmd(), vert_count, instance_count, base_vert, base_instance);
}
void gfx_draw_indirect(Region region, Gfx_Buffer buf, u32 offset, u32 stride) {
	var& g = st->gfx;
	g.CmdDrawIndirect(vk_cur_cmd(), g.cpu_buf.h, buf.base + region.base*stride, region.count, stride);
}

void gfx_draw_mesh(Gfx_Mesh mesh) {
	if(mesh.index_count) {
		gfx_draw(0, mesh.index_count);
	} else {
		gfx_draw(0, mesh.vert_count);
	}
}

void gfx_begin_pass(Gfx_Pass pass) {
	var& g = st->gfx;
	Loop(i, Gfx_MaxColorAttachments) {
		if(pass.colors_load[i] == Gfx_LoadOp_Default) {
			pass.colors_load[i] = Gfx_LoadOp_Clear;
			pass.colors_clear[i].v[0] = 0.3f;
			pass.colors_clear[i].v[1] = 0.3f;
			pass.colors_clear[i].v[2] = 0.3f;
			pass.colors_clear[i].v[3] = 1.0f;
		}
		if(pass.colors_store[i] == Gfx_StoreOp_Default) {
			pass.colors_store[i] = Gfx_StoreOp_Store;
		}
	}
	if(pass.depth_load == Gfx_LoadOp_Default) {
		pass.depth_load = Gfx_LoadOp_Clear;
		pass.depth_clear = 1.0f;
	}
	if(pass.depth_store == Gfx_StoreOp_Default) {
		pass.depth_store = Gfx_StoreOp_DontCare;
	}
	if(pass.stencil_load == Gfx_LoadOp_Default) {
		pass.stencil_load = Gfx_LoadOp_Clear;
		pass.stencil_clear = 1;
	}
	if(pass.stencil_store == Gfx_StoreOp_Default) {
		pass.stencil_store = Gfx_StoreOp_DontCare;
	}
	var cmd = vk_cur_cmd();
	var b = vk_sync_begin(cmd);

	if(!g.is_first_pass) {
		if(g.prev_pass.colors[0].idx || g.prev_pass.resolves[0].idx || g.prev_pass.depth_stencil.idx) {
			Loop(i, Gfx_MaxColorAttachments) {
				if(g.prev_pass.colors[i].idx == 0) break;
				if(g.prev_pass.colors_store[i] == Gfx_StoreOp_Store) {
					var& img = pool_get(g.images, pool_get(g.views, g.prev_pass.colors[i]).img);
					vk_sync_image(b, img, VK_Sync_ShaderRead);
				}
				if(g.prev_pass.resolves[i].idx) {
					var& img = pool_get(g.images, pool_get(g.views, g.prev_pass.resolves[i]).img);
					vk_sync_image(b, img, VK_Sync_ShaderRead);
				}
			}
			if(g.prev_pass.depth_stencil.idx) {
				var& img = pool_get(g.images, pool_get(g.views, g.prev_pass.depth_stencil).img);
				if(g.prev_pass.depth_store == Gfx_StoreOp_Store) {
					vk_sync_image(b, img, VK_Sync_ShaderRead);
				}
			}
		}
	}
	g.is_first_pass = false;
	g.prev_pass = pass;

	VkRenderingAttachmentInfo color_infos[Gfx_MaxColorAttachments] = {};
	VkRenderingAttachmentInfo depth_info = {};
	VkRenderingAttachmentInfo stencil_info = {};
	VkRenderingInfo render_info = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.layerCount = 1,
	};
	if(pass.colors[0].idx || pass.resolves[0].idx || pass.depth_stencil.idx) {
		var color_view = pool_get(g.views, pass.colors[0]);
		var color_image = pool_get(g.images, color_view.img);
		render_info.renderArea.extent = {color_image.width, color_image.height};
		Loop(i, Gfx_MaxColorAttachments) {
			if(pass.colors[i].idx == 0) break;
			Gfx_View color_view_id = pass.colors[i];
			var& color_view = pool_get(g.views, color_view_id);
			var& color_image = pool_get(g.images, color_view.img);
			if(pass.colors_load[i] != Gfx_LoadOp_Load) {
				color_image.sync = VK_Sync_Undefined;
			}
			vk_sync_image(b, color_image, VK_Sync_ColorAttachment);
			if(pass.resolves[i].idx) {
				var& resolve_image = pool_get(g.images, pool_get(g.views, pass.resolves[i]).img);
				resolve_image.sync = VK_Sync_Undefined;
				vk_sync_image(b, resolve_image, VK_Sync_ColorAttachment);
			}
		}
		if(pass.depth_stencil.idx) {
			Gfx_Image img = pool_get(g.views, pass.depth_stencil).img;
			var& image = pool_get(g.images, img);
			if((pass.depth_load != Gfx_LoadOp_Load) && (pass.stencil_load != Gfx_LoadOp_Load)) {
				image.sync = VK_Sync_Undefined;
			}
			vk_sync_image(b, image, VK_Sync_DepthAttachment);
		}
		u32 color_view_count = 0;
		Loop(i, Gfx_MaxColorAttachments) {
			if(pass.colors[i].idx == 0) break;
			var color_vkview = pool_get(g.views, pass.colors[i]).h;
			var resolve_vkview = pool_get(g.views, pass.resolves[i]).h;
			var& info = color_infos[i];
			info = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				.imageView = color_vkview,
				.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.loadOp = vk_load_op(pass.colors_load[i]),
				.storeOp = vk_store_op(pass.colors_store[i]),
				.clearValue.color.float32[0] = pass.colors_clear[i].v[0],
				.clearValue.color.float32[1] = pass.colors_clear[i].v[1],
				.clearValue.color.float32[2] = pass.colors_clear[i].v[2],
				.clearValue.color.float32[3] = pass.colors_clear[i].v[3],
			};
			if(resolve_vkview) {
				info.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
				info.resolveImageView = resolve_vkview;
				info.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			color_view_count++;
		}
		render_info.colorAttachmentCount = color_view_count;
		render_info.pColorAttachments = color_infos;
		if(pass.depth_stencil.idx) {
			var vkview = pool_get(g.views, pass.depth_stencil).h;
			depth_info = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				.imageView = vkview,
				.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
				.loadOp = vk_load_op(pass.depth_load),
				.storeOp = vk_store_op(pass.depth_store),
				.clearValue.depthStencil.depth = pass.depth_clear,
			};
			render_info.pDepthAttachment = &depth_info;
			stencil_info = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				.imageView = vkview,
				.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
				.loadOp = vk_load_op(pass.stencil_load),
				.storeOp = vk_store_op(pass.stencil_store),
				.clearValue.depthStencil.stencil = pass.stencil_clear,
			};
			render_info.pStencilAttachment = &stencil_info;
		}
	} else {
		VK_Image img = {
			.h = g.swapchain.images[g.current_image_idx],
			.count = 1,
			.mip_count = 1,
			.format = g.swapchain.format,
		};
		vk_sync_image(b, img, VK_Sync_ColorAttachment);
		VkImageView vkview = g.swapchain.views[g.current_image_idx];
		color_infos[0] = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = vkview,
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp = vk_load_op(pass.colors_load[0]),
			.storeOp = vk_store_op(pass.colors_store[0]),
			.clearValue.color.float32[0] = pass.colors_clear[0].v[0],
			.clearValue.color.float32[1] = pass.colors_clear[0].v[1],
			.clearValue.color.float32[2] = pass.colors_clear[0].v[2],
			.clearValue.color.float32[3] = pass.colors_clear[0].v[3],
		};
		render_info.colorAttachmentCount = 1;
		render_info.pColorAttachments = color_infos;
		render_info.renderArea.extent = {os_win_width(), os_win_height()};
	}
	vk_sync_end(b);
	g.CmdBeginRendering(vk_cur_cmd(), &render_info);
}

void gfx_end_pass() {
	var& g = st->gfx;
	var cmd = vk_cur_cmd();
	g.CmdEndRendering(cmd);
}

u64 gfx_push_buffer(Gfx_Buffer& arena, u64 size) {
	u64 res = offset_push(arena.pos, size);
	Assert(arena.pos <= arena.size);
	return res;
}

b32 gfx_ready(u64 counter) {
	var& g = st->gfx;
	return counter <= g.stage_completed;
}

u8* gfx_buffer_base_ptr(Gfx_Buffer buf) { return vk_get_vkbuffer(buf).base + buf.base; }

void gfx_bind_pipeline(Gfx_Pipeline pip) {
	var& g = st->gfx;
	g.CmdBindPipeline(vk_cur_cmd(), VK_PIPELINE_BIND_POINT_GRAPHICS, pool_get(g.pipelines, pip).h);
}

void gfx_make_bind(Gfx_DescriptorDesc desc) {
	var& g = st->gfx;
	_DefSet(desc.count, 1);
	_DefSet(desc.type, Gfx_BindType_Storage);
	var& writer = g.descriptor_writer;
	VkDescriptorBindingFlags flags = {};
	if(desc.type == Gfx_BindType_Image) {
		flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	}
	writer.binding_flags[writer.binds_count] = flags;
	VkDescriptorType descriptor_type;
	switch(desc.type) {
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
	writer.binds_count++;
}

void gfx_bind_buffer(Gfx_Buffer buf, u32 binding) {
	var& g = st->gfx;
	var& writer = g.descriptor_writer;
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
	writer.writes_count++;
}

void gfx_make_binding_buffers(Slice<Gfx_BufferDesc> descs) {
	for(var desc : descs) {
		gfx_make_bind({.binding = desc.binding});
		Gfx_Buffer buf = gfx_make_buffer(desc.size, desc.gpu);
		if(desc.out_buffer) *desc.out_buffer = buf;
		if(desc.out_cpu_ptr) *desc.out_cpu_ptr = gfx_buffer_base_ptr(buf);
		gfx_bind_buffer(buf, desc.binding);
	}
}

void gfx_flush_descriptors() {
	var& g = st->gfx;

	///////////////////////////////////
	// Descriptors
	var& writer = g.descriptor_writer;
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
	Loop(i, writer.writes_count) {
		writer.writes[i].dstSet = g.descriptor_set;
	}
	g.UpdateDescriptorSets(vkdevice, writer.writes_count, writer.writes, 0, null);

	///////////////////////////////////
	// Pipeline layout
	VkPushConstantRange push_constant = {
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		.offset = 0,
		.size = sizeof(Gfx_DrawCtx),
	};
	VkPipelineLayoutCreateInfo pipeline_layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &g.descriptor_set_layout,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &push_constant,
	};
	VK_CHECK(g.CreatePipelineLayout(vkdevice, &pipeline_layout_info, g.allocator, &g.pipeline_layout));
}

void gfx_idle() { st->gfx.DeviceWaitIdle(vkdevice); }

void gfx_init() {
	ProfFunc;
	var& g = st->gfx;
	g.arena = arena_make(.name = "gfx arena");
	g.stage_next_value = 1;

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
		VkCommandPoolCreateInfo graphics_pool_create_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = g.device.graphics_queue_family_idx,
		};
		VK_CHECK(g.CreateCommandPool(vkdevice, &graphics_pool_create_info, g.allocator, &g.device.cmd_pool));
		VK_CHECK(g.CreateCommandPool(vkdevice, &graphics_pool_create_info, g.allocator, &g.device.upload_cmd_pool));
		g.images_in_flight = g.device.surface_capabilities.minImageCount;
		g.frames_in_flight = g.images_in_flight - 1;
		g.swapchain.surface_format = g.device.surface_formats[0];
		Loop(i, g.device.surface_format_count) {
			VkSurfaceFormatKHR format = g.device.surface_formats[i];
			if(format.format == VK_FORMAT_B8G8R8A8_UNORM &&
			// if(format.format == VK_FORMAT_B8G8R8A8_SRGB &&
							format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				if(format.format == VK_FORMAT_B8G8R8A8_UNORM) g.swapchain.format = Gfx_Format_BGRA8;
				if(format.format == VK_FORMAT_B8G8R8A8_SRGB) g.swapchain.format = Gfx_Format_BGRA8_SRGB;
				g.swapchain.surface_format = format;
				break;
			}
		}
		g.swapchain.present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		vk_swapchain_create();
		vk_make_cmd(g.device.cmd_pool, g.frames_in_flight, g.render_cmds);
		vk_make_cmd(g.device.upload_cmd_pool, 1, &g.upload_cmd);

		// Query pool
		VkQueryPoolCreateInfo pool_info = {
			.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
			.queryType = VK_QUERY_TYPE_TIMESTAMP,
			.queryCount = MaxTimeStamps,
		};
		Loop(i, Gfx_NumFramesInFlight) {
			VK_CHECK(g.CreateQueryPool(vkdevice, &pool_info, g.allocator, &g.query_pools[i]));
		}
#if BUILD_DEV
		VkQueryPoolCreateInfo stats_pool_info = {
			.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
			.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS,
			.queryCount = MaxTimeStamps,
			.pipelineStatistics =
				VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
				VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT,
		};
		Loop(i, Gfx_NumFramesInFlight) {
			VK_CHECK(g.CreateQueryPool(vkdevice, &stats_pool_info, g.allocator, &g.stats_query_pools[i]));
		}
		g.ns_per_tick = g.device.properties.limits.timestampPeriod;
#endif
	}

	// Sync
	{
		Loop(i, g.images_in_flight) {
			VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
			g.CreateSemaphore(vkdevice, &semaphore_create_info, g.allocator, &g.render_complete_semaphores[i]);
			g.CreateSemaphore(vkdevice, &semaphore_create_info, g.allocator, &g.image_available_semaphores[i]);
		}
		Loop(i, g.frames_in_flight) {
			VkFenceCreateInfo fence_create_info = {
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.flags = VK_FENCE_CREATE_SIGNALED_BIT,
			};
			VK_CHECK(g.CreateFence(vkdevice, &fence_create_info, g.allocator, &g.in_flight_fences[i]));
			// g.frames_upload_semaphores[i] = vk_semaphore_make(1);
		}
		g.stage_semaphore = vk_make_semaphore(0);
	}

	{
		VkDescriptorPoolSize pool_sizes[] = {
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, Gfx_MaxStorageBuffers},
			{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, Gfx_MaxImages},
			{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, Gfx_MaxCubeViews},
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

	g.cpu_mem = vk_make_mem(Gfx_CpuMemSize);
	g.gpu_mem = vk_make_mem(Gfx_GpuMemSize, true);
	g.cpu_buf = vk_make_buffer(Gfx_CpuMemSize);
	g.gpu_buf = vk_make_buffer(Gfx_GpuMemSize, true);

	// VkBufferDeviceAddressInfoKHR gpu_address_info = {
	// 	.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR,
	// 	.buffer = g.gpu_buf.h,
	// };
	// g.gpu_buf_address = g.GetBufferDeviceAddress(vkdevice, &gpu_address_info);
	// VkBufferDeviceAddressInfoKHR cpu_address_info = {
	// 	.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR,
	// 	.buffer = g.cpu_buf.h,
	// };
	// g.cpu_buf_address = g.GetBufferDeviceAddress(vkdevice, &cpu_address_info);
	g.stage_buffer = gfx_make_buffer(Gfx_StageBufferSize, 0);
	g.stage_ring = ring_make(gfx_buffer_base_ptr(g.stage_buffer), g.stage_buffer.size);
}

void gfx_shutdown() {
	var& g = st->gfx;
	gfx_idle();
	g.DestroySurfaceKHR(g.instance, g.surface, g.allocator);
#if BUILD_DEBUG
	PFN_vkDestroyDebugUtilsMessengerEXT func = null;
	Assign(func, g.GetInstanceProcAddr(g.instance, "vkDestroyDebugUtilsMessengerEXT"));
	func(g.instance, g.debug_messenger, g.allocator);
#endif
	g.DestroyInstance(g.instance, g.allocator);
}

void gfx_begin_timestamp(String name) {
	var& g = st->gfx;
	u32 idx = g.zone_count[g.cur_frame_idx]++;
	g.zone_names[g.cur_frame_idx*MaxTimeStamps + idx] = name;
	g.zone_depth[g.cur_frame_idx*MaxTimeStamps + idx] = g.stack_depth;
	g.open_zone_index[g.stack_depth] = idx;
	g.stack_depth++;
	g.CmdWriteTimestamp(vk_cur_cmd(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, g.query_pools[g.cur_frame_idx], idx*2);
	g.CmdBeginQuery(vk_cur_cmd(), g.stats_query_pools[g.cur_frame_idx], idx, 0);
}
void gfx_end_timestamp() {
	var& g = st->gfx;
	g.stack_depth--;
	u32 idx = g.open_zone_index[g.stack_depth];
	g.CmdWriteTimestamp(vk_cur_cmd(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, g.query_pools[g.cur_frame_idx], idx*2 + 1);
	g.CmdEndQuery(vk_cur_cmd(), g.stats_query_pools[g.cur_frame_idx], idx);
}

void gfx_begin() {
	ProfFunc;
	Scratch scratch;
	var& g = st->gfx;
	g.is_first_pass = true;
	{
		ProfBlock("rendering waiting");
		VK_CHECK(g.WaitForFences(vkdevice, 1, &g.in_flight_fences[g.cur_frame_idx], true, U64_MAX));
		VK_CHECK(g.ResetFences(vkdevice, 1, &g.in_flight_fences[g.cur_frame_idx]));
	}
	VkCommandBuffer cmd = vk_cur_cmd();
	vk_begin_cmd(cmd);

#if BUILD_DEV
	///////////////////////////////////
	// Timestamps
	Assert(g.stack_depth == 0);
	g.cur_prof_frame.start_ms = F64_MAX;
	g.cur_prof_frame.end_ms = 0;
	u32 query_count = g.zone_count[g.cur_frame_idx];
	array_clear(g.cur_prof_frame.zones);
	if(query_count) {
		u64 timestamps[MaxTimeStamps*2] = {};
		VK_CHECK(g.GetQueryPoolResults(vkdevice, g.query_pools[g.cur_frame_idx], 0, query_count*2, sizeof(u64)*query_count*2,
			timestamps, sizeof(u64), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));
		Gfx_PipelineStats stats[MaxTimeStamps] = {};
		VK_CHECK(g.GetQueryPoolResults(vkdevice, g.stats_query_pools[g.cur_frame_idx], 0, query_count, sizeof(Gfx_PipelineStats)*query_count,
			stats, sizeof(Gfx_PipelineStats), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));
		Loop(i, query_count) {
			u64 start = timestamps[i*2 + 0];
			u64 end = timestamps[i*2 + 1];
			String name = g.zone_names[i];
			f64 start_ms = start*g.ns_per_tick / Million(1);
			f64 end_ms = end*g.ns_per_tick / Million(1);
			g.cur_prof_frame.start_ms = Min(start_ms, g.cur_prof_frame.start_ms);
			g.cur_prof_frame.end_ms = Max(end_ms, g.cur_prof_frame.end_ms);
			array_push(g.cur_prof_frame.zones, {
				.start = start_ms,
				.end = end_ms,
				.name = name,
				.depth = g.zone_depth[g.cur_frame_idx*MaxTimeStamps + i],
				.vert_invocations = stats[i].vert_invocations,
				.frag_invocations = stats[i].frag_invocations,
			});
		}
	}
	g.CmdResetQueryPool(cmd, g.query_pools[g.cur_frame_idx], 0, MaxTimeStamps);
	g.CmdResetQueryPool(cmd, g.stats_query_pools[g.cur_frame_idx], 0, MaxTimeStamps);
	g.zone_count[g.cur_frame_idx] = 0;
#endif

	g.swapchain_resized = false;
	v2u win_size = os_win_size();
	if(win_size != g.size) {
		g.size = win_size;
		g.swapchain_resized = true;
		gfx_idle();
		vk_swapchain_create();
		Info("Swapchain recreated x: %i y: %i", win_size.x, win_size.y);
	}

	GfxBeginBlock("start");
	g.CmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipeline_layout, 0, 1, &g.descriptor_set, 0, null);
	GfxEndBlock();
	// Next image
	{
		ProfBlock("swapchain flip waiting");
#if GFX_X11 // NOTE: on x11 errors
		VkResult res = g.AcquireNextImageKHR(vkdevice, g.swapchain.h, U64_MAX, vk_get_cur_image_available_semaphore(), null, &g.current_image_idx);
		if(res != VK_SUCCESS) {
			// Warn("%s", vk_result_str(res));
		}
#else
		VK_CHECK(g_g.AcquireNextImageKHR(vkdevice, g_g.swapchain.handle, U64_MAX, image_available_semaphore, null, &image_index));
#endif
	}

	VK_CHECK(g.GetSemaphoreCounterValue(vkdevice, g.stage_semaphore, &g.stage_completed));
	if(g.stage_completed == g.stage_submitted) {
		os_mutex_lock(g.stage_mutex);
		ring_release_to(g.stage_ring, g.stage_in_flight_write);
		g.stage_in_flight_write = g.stage_ring.write;
		var batch = push_slice(scratch, Gfx_Stage, queue_count(g.stage_cmd_queue));
		for(var& v : batch) v = queue_pop(g.stage_cmd_queue);
		os_mutex_unlock(g.stage_mutex);
		if(batch.count) {
			g.stage_submitted = atomic_inc(&g.stage_next_value);
			vk_begin_cmd(g.upload_cmd);
			for(var stage_cmd : batch) {
				switch(stage_cmd.type) {
					InvalidDefaultCase;
					case Gfx_StageType_Texture: {
						var& img = pool_get(g.images, stage_cmd.img);
						vk_sync_image_now(g.upload_cmd, img, VK_Sync_TransferDst);
						vk_upload_image(g.upload_cmd, img, stage_cmd.stage_off);
						if(img.mip_count > 1) vk_make_mipmaps(g.upload_cmd, img);
						vk_sync_image_now(g.upload_cmd, img, VK_Sync_ShaderRead);
					}break;
					case Gfx_StageType_Buffer: {
						VkBufferCopy2 copy_region = {
							.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
							.srcOffset = g.stage_buffer.base + stage_cmd.stage_off,
							.dstOffset = stage_cmd.buf.base + stage_cmd.buf_off,
							.size = stage_cmd.buf_size,
						};
						VkCopyBufferInfo2 info = {
							.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
							.srcBuffer = g.cpu_buf.h,
							.dstBuffer = g.gpu_buf.h,
							.regionCount = 1,
							.pRegions = &copy_region,
						};
						g.CmdCopyBuffer2(g.upload_cmd, &info);
					}break;
					case Gfx_StageType_VertIndex: {
						VkBufferCopy2 regions[] = {
							{
								.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
								.srcOffset = g.stage_buffer.base + stage_cmd.stage_off,
								.dstOffset = stage_cmd.buf.base + stage_cmd.buf_off,
								.size = stage_cmd.buf_size,
							},
							{
								.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
								.srcOffset = g.stage_buffer.base + stage_cmd.stage_off + stage_cmd.buf_size,
								.dstOffset = stage_cmd.index_buf.base + stage_cmd.index_off,
								.size = stage_cmd.index_size,
							}
						};
						VkCopyBufferInfo2 info = {
							.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
							.srcBuffer = g.cpu_buf.h,
							.dstBuffer = g.gpu_buf.h,
							.regionCount = stage_cmd.index_size ? 2u : 1,
							.pRegions = regions,
						};
						g.CmdCopyBuffer2(g.upload_cmd, &info);
					}break;
				}
			}
			vk_end_cmd(g.upload_cmd);
			VkCommandBufferSubmitInfo command_info = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
				.commandBuffer = g.upload_cmd,
			};
			VkSemaphoreSubmitInfo signal_info = {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.semaphore = g.stage_semaphore,
				.value = g.stage_submitted,
			};
			VkSubmitInfo2 info = {
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
				.commandBufferInfoCount = 1,
				.pCommandBufferInfos = &command_info,
				.signalSemaphoreInfoCount = 1,
				.pSignalSemaphoreInfos = &signal_info,
			};
			VK_CHECK(g.QueueSubmit2(g.device.graphics_queue, 1, &info, null));
		}
	}
}

void gfx_end() {
	ProfFunc;
	var& g = st->gfx;
	VkCommandBuffer cmd = vk_cur_cmd();
	VK_Image img = {
		.h = g.swapchain.images[g.current_image_idx],
		.count = 1,
		.mip_count = 1,
		.format = g.swapchain.format,
		.sync = VK_Sync_ColorAttachment,
	};
	vk_sync_image_now(cmd, img, VK_Sync_Present);
	vk_end_cmd(cmd);

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
	VK_CHECK(g.QueueSubmit2(g.device.graphics_queue, 1, &info, g.in_flight_fences[g.cur_frame_idx]));

	///////////////////////////////////
	// Present
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
	if(res != VK_SUCCESS) {
		// Error("%s", vk_result_str(res));
	}
#else
	VK_CHECK(g_g.QueuePresentKHR(g_g.device.graphics_queue, &present_info));
#endif
	g.cur_frame_idx = (g.cur_frame_idx + 1) % g.frames_in_flight;
	g.current_frame_idx_plus_one = (g.current_frame_idx_plus_one + 1) % (g.frames_in_flight+1);
}

