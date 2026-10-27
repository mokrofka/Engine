#pragma once
#include "base/base.h"

MakeId(Gfx_Image);
MakeId(Gfx_Sampler);
MakeId(Gfx_Shader);
MakeId(Gfx_Pipeline);
MakeId(Gfx_View);

enum {
	Gfx_NumFramesInFlight   = 2,
	Gfx_MaxImagesInFlight   = 4,
	Gfx_MaxColorAttachments = 4,
	Gfx_MaxShaders          = 16,
	Gfx_MaxPipelines        = 16,
	Gfx_MaxViews            = 32,
	Gfx_MaxImages           = 32,
	Gfx_MaxSamplers         = 8,
	Gfx_MaxCubeViews     			= 8,
	Gfx_MaxStorageBuffers   = 16,
	Gfx_MaxBindings         = 64,
	Gfx_DefaultSampleCount  = 4,
	Gfx_StageBufferSize     = MB(10),
	Gfx_CpuMemSize          = MB(20),
	Gfx_GpuMemSize          = MB(10),
};

//    - unsigned normalized
// S  - signed normalized
// UI - unsigned integer
// SI - signed integer
enum Gfx_Format {
	Gfx_Format_Default,
	Gfx_Format_R8_UI,
	Gfx_Format_R8,
	Gfx_Format_BGRA8,
	Gfx_Format_BGRA8_SRGB,
	Gfx_Format_RGBA8,
	Gfx_Format_RGBA8_SRGB,
	// Gfx_Format_Depth,
	Gfx_Format_DepthStencil,
};

enum Gfx_PrimitiveType {
	Gfx_PrimitiveType_Default,
	Gfx_PrimitiveType_Point,
	Gfx_PrimitiveType_Line,
	Gfx_PrimitiveType_Triangle,
	Gfx_PrimitiveType_TriangleStrip,
};

enum Gfx_Filter {
	Gfx_Filter_Default,
	Gfx_Filter_Nearest,
	Gfx_Filter_Linear,
};

enum Gfx_Wrap {
	Gfx_Wrap_Default,
	Gfx_Wrap_Repeat,
	Gfx_Wrap_CalmpToEdge,
	Gfx_Wrap_CalmpToBorder,
	Gfx_Wrap_MirroredRepeat,
};

enum Gfx_BorderColor {
	Gfx_BorderColor_Default,
	Gfx_BorderColor_TransparentBlack,
	Gfx_BorderColor_OpaqueBlack,
	Gfx_BorderColor_OpaqueWhite,
};

enum Gfx_CullMode {
	Gfx_CullMode_Default,
	Gfx_CullMode_None,
	Gfx_CullMode_Front,
	Gfx_CullMode_Back,
};

enum Gfx_FaceWinding {
	Gfx_FaceWinding_Default,
	Gfx_FaceWinding_CCW,
	Gfx_FaceWinding_CW,
};

enum Gfx_CompareOp {
	Gfx_CompareOp_Default,
	Gfx_CompareOp_Never,
	Gfx_CompareOp_Always,
	Gfx_CompareOp_Less,
	Gfx_CompareOp_LessEqual,
	Gfx_CompareOp_Greater,
	Gfx_CompareOp_GreaterEqual,
	Gfx_CompareOp_Equal,
	Gfx_CompareOp_NotEqual,
};

enum Gfx_StencilOp {
	Gfx_StencilOp_Default,
	Gfx_StencilOp_Keep,
	Gfx_StencilOp_Zero,
	Gfx_StencilOp_Replace,
	Gfx_StencilOp_IncrClamp,
	Gfx_StencilOp_DecrClamp,
	Gfx_StencilOp_Invert,
	Gfx_StencilOp_IncrWrap,
	Gfx_StencilOp_DecrWrap,
};

enum Gfx_BlendFactor {
	Gfx_BlendFactor_Default,
	Gfx_BlendFactor_Zero,
	Gfx_BlendFactor_One,
	Gfx_BlendFactor_SrcColor,
	Gfx_BlendFactor_OneMinusSrcColor,
	Gfx_BlendFactor_SrcAlpha,
	Gfx_BlendFactor_OneMinusSrcAlpha,
	Gfx_BlendFactor_DstAlpha,
	Gfx_BlendFactor_OneMinusDstAlpha,
	Gfx_BlendFactor_SrcAlphaSaturated,
	Gfx_BlendFactor_BlendColor,
	Gfx_BlendFactor_OneMinusBlendColor,
	Gfx_BlendFactor_BlendAlpha,
	Gfx_BlendFactor_OneMinusBlendAlpha,
	Gfx_BlendFactor_Src1Color,
	Gfx_BlendFactor_OneMinusSrc1Color,
	Gfx_BlendFactor_Src1Alpha,
	Gfx_BlendFactor_OneMinusSrc1Alpha,
};

enum Gfx_BlendOp {
	Gfx_BlendOp_Default,
	Gfx_BlendOp_Add,
	Gfx_BlendOp_Subtract,
	Gfx_BlendOp_ReverseSubtract,
	Gfx_BlendOp_Min,
	Gfx_BlendOp_Max,
};

enum Gfx_ColorMask {
	Gfx_ColorMask_Default = 0,
	Gfx_ColorMask_None   = 0x10,
	Gfx_ColorMask_R      = 0x1,
	Gfx_ColorMask_G      = 0x2,
	Gfx_ColorMask_RG     = 0x3,
	Gfx_ColorMask_B      = 0x4,
	Gfx_ColorMask_RB     = 0x5,
	Gfx_ColorMask_GB     = 0x6,
	Gfx_ColorMask_RGB    = 0x7,
	Gfx_ColorMask_A      = 0x8,
	Gfx_ColorMask_RA     = 0x9,
	Gfx_ColorMask_GA     = 0xA,
	Gfx_ColorMask_RGA    = 0xB,
	Gfx_ColorMask_BA     = 0xC,
	Gfx_ColorMask_RBA    = 0xD,
	Gfx_ColorMask_GBA    = 0xE,
	Gfx_ColorMask_RGBA   = 0xF,
};

enum Gfx_LoadOp {
	Gfx_LoadOp_Default,
	Gfx_LoadOp_Clear,
	Gfx_LoadOp_Load,
	Gfx_LoadOp_DontCare,
};

enum Gfx_StoreOp {
	Gfx_StoreOp_Default,
	Gfx_StoreOp_Store,
	Gfx_StoreOp_DontCare,
};

struct Gfx_Pass {
	Gfx_View colors[Gfx_MaxColorAttachments];
	Gfx_View resolves[Gfx_MaxColorAttachments];
	Gfx_View depth_stencil;
	Gfx_LoadOp colors_load[Gfx_MaxColorAttachments];
	Gfx_StoreOp colors_store[Gfx_MaxColorAttachments];
	Gfx_LoadOp depth_load;
	Gfx_StoreOp depth_store;
	Gfx_LoadOp stencil_load;
	Gfx_StoreOp stencil_store;
	v4 colors_clear[Gfx_MaxColorAttachments];
	f32 depth_clear;
	u8 stencil_clear;
};

enum Gfx_ImageType {
	Gfx_ImageType_Default,
	Gfx_ImageType_2D,
	Gfx_ImageType_Cube,
	Gfx_ImageType_3D,
	Gfx_ImageType_Array,
};

struct Gfx_ImageDesc {
	Gfx_ImageType type;
	u32 width;
	u32 height;
	u32 slices_count;
	b32 mipmaps;
	Gfx_Format format;
	u32 samples;
};

struct Gfx_ViewDesc {
	Gfx_Image image;
	u32 mip_level;
	u32 mip_level_count;
	u32 slice;
	u32 slice_count;
	b32 stencil;
};

struct Gfx_SamplerDesc {
	Gfx_Filter min_filter;
	Gfx_Filter mag_filter;
	Gfx_Filter mipmap_filter;
	Gfx_Wrap wrap_u;
	Gfx_Wrap wrap_v;
	Gfx_Wrap wrap_w;
	f32 min_lod;
	f32 max_lod;
	Gfx_BorderColor border_color;
	Gfx_CompareOp compare;
	u32 max_anisotropy;
};

struct Gfx_PipelineDesc {
	b32 compute;
	Gfx_Shader shader;
	Gfx_Format pixel_format;
	Gfx_Format depth_pixel_format;
	v4 blend_color;
};

struct Gfx_DepthState {
	Gfx_Format pixel_format;
	Gfx_CompareOp compare;
	b32 write_enabled;
	b32 test_disable;
	f32 bias;
	f32 bias_slope_scale;
	f32 bias_clamp;
};

struct Gfx_BlendState {
	b32 enabled;
	Gfx_BlendFactor src_factor_rgb;
	Gfx_BlendFactor dst_factor_rgb;
	Gfx_BlendOp op_rgb;
	Gfx_BlendFactor src_factor_alpha;
	Gfx_BlendFactor dst_factor_alpha;
	Gfx_BlendOp op_alpha;
	Gfx_ColorMask write_mask;
};

struct Gfx_PipelineState {
	Gfx_DepthState depth;
	Gfx_BlendState blend;
	Gfx_PrimitiveType primitive_type;
	Gfx_CullMode cull_mode;
	Gfx_FaceWinding face_winding;
	u32 sample_count;
};

struct Gfx_Mesh {
	u32 base_vert;
	u32 vert_count;
	u32 base_index;
	u32 index_count;
};

struct Gfx_Buffer {
	b32 gpu;
	u64 base;
	u64 size;
	u64 pos;
};

enum Gfx_BindType {
	Gfx_BindType_Default,
	Gfx_BindType_Storage,
	Gfx_BindType_Image,
	Gfx_BindType_Sampler,
};

struct Gfx_DescriptorDesc {
	Gfx_BindType type;
	u32 binding;
	u32 count;
};

struct Gfx_BufferDesc {
	u32 binding;
	u64 size;
	b32 gpu;
	void** out_cpu_ptr;
	Gfx_Buffer* out_buffer;
};

enum Gfx_StageType {
	Gfx_StageType_Texture = 1,
	Gfx_StageType_Buffer,
	Gfx_StageType_VertIndex,
};

struct Gfx_Stage {
	Gfx_StageType type;
	Gfx_Image img;
	Gfx_Buffer buf;
	Gfx_Buffer index_buf;
	u64 buf_off;
	u64 index_off;
	u64 stage_off;
	u64 index_stage_off;
	u64 buf_size;
	u64 index_size;
};

struct Gfx_DrawCtx {
	u32 idx;
};

////////////////////////////////////////////////////////////////////////
// @Vulkan
#include "vulkan/vulkan_core.h"

#if BUILD_DEBUG
	#define VK_CHECK(expr)                 \
		{                                     \
			if(expr != VK_SUCCESS) {           \
				Error("%s", vk_result_str(expr)); \
				InvalidPath;                      \
			}                                   \
		}
#else
	#define VK_CHECK(expr) expr
#endif
#define vkdevice st->gfx.device.logical_device

#define VK_SyncMaxBatch 16
#define VK_SyncShaderStages (VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT   | \
                            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | \
                            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT)
enum VK_Sync {
	VK_Sync_Undefined,
	VK_Sync_TransferSrc,
	VK_Sync_TransferDst,
	VK_Sync_VertexIndexBuffer,
	VK_Sync_IndirectBuffer,
	VK_Sync_ShaderRead,
	VK_Sync_Storage,
	VK_Sync_ColorAttachment,
	VK_Sync_DepthAttachment,
	VK_Sync_Present,
	VK_Sync_HostRead,
	VK_Sync_General,
	VK_Sync_COUNT,
};

struct VK_SyncBatch {
	VkCommandBuffer cmd;
	Array<VkImageMemoryBarrier2, VK_SyncMaxBatch> images;
	Array<VkBufferMemoryBarrier2, VK_SyncMaxBatch> buffers;
};

struct VK_SyncInfo {
	VkPipelineStageFlags2 stage;
	VkAccessFlags2 access;
	VkImageLayout layout;
	b32 writes;
};

// struct VK_SyncBuffer {
// 	VkBuffer buffer;
// 	VkDeviceSize offset;
// 	VkDeviceSize size;
// 	VK_Sync state;
// };

struct VK_Memory {
	VkDeviceMemory h;
	u8* mapped_mem;
	u64 pos;
	u64 cap;
};

struct VK_Buffer {
	VkBuffer h;
	u8* base;
	u64 pos;
	u64 cap;
};

struct VK_Image {
	VkImage h;
	VkDeviceMemory memory;
	u64 mem_size;
	Gfx_ImageType type;
	u32 width;
	u32 height;
	u32 count;
	u32 mip_count;
	Gfx_Format format;
	u32 sample_count;
	VK_Sync sync;
};

struct VK_View {
	VkImageView h;
	Gfx_Image img;
	u32 mip_level;
	u32 slice;
	u32 mip_level_count;
	u32 slice_count;
};

struct VK_Sampler {
	VkSampler h;
	Gfx_Filter min_filter;
	Gfx_Filter mag_filter;
	Gfx_Filter mipmap_filter;
	Gfx_Wrap wrap_u;
	Gfx_Wrap wrap_v;
	Gfx_Wrap wrap_w;
	f32 min_lod;
	f32 max_lod;
	Gfx_BorderColor border_color;
	Gfx_CompareOp compare;
	u32 max_anisotropy;
};

struct VK_Shader {
	VkShaderModule h;
};

struct VK_Pipeline {
	VkPipeline h;
	Gfx_Shader shader;
	b32 compute;
	Gfx_Format pixel_format;
	Gfx_Format depth_pixel_format;
	v4 blend_color;
};

struct VK_Device {
	VkPhysicalDevice physical_device;
	VkDevice logical_device;
	VkSurfaceCapabilitiesKHR surface_capabilities;
	u32 surface_format_count;
	VkSurfaceFormatKHR* surface_formats;
	u32 present_mode_count;
	VkPresentModeKHR* present_modes;
	u32 graphics_queue_family_idx;
	u32 transfer_queue_family_idx;
	u32 compute_queue_family_idx;
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkQueue transfer_queue;
	VkQueue compute_queue;
	VkCommandPool cmd_pool;
	VkCommandPool upload_cmd_pool;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties memory;
	u32 gpu_type_idx;
	u32 cpu_type_idx;
	VkFormat depth_format;
};

struct VK_Swapchain {
	VkSwapchainKHR h;
	VkSurfaceFormatKHR surface_format;  
	Gfx_Format format;
	VkPresentModeKHR present_mode;
	VkImage images[Gfx_MaxImagesInFlight];
	VkImageView views[Gfx_MaxImagesInFlight];
	VK_Image depth_attachment;
	VkSwapchainKHR h_old;
	VkImageView old_view[Gfx_MaxImagesInFlight];
};

struct VK_DescriptorWriter {
	VkDescriptorBindingFlags binding_flags[Gfx_MaxBindings];
	VkDescriptorSetLayoutBinding bindings[Gfx_MaxBindings];
	u32 binds_count;
	VkDescriptorBufferInfo buffers[Gfx_MaxBindings];
	VkWriteDescriptorSet writes[Gfx_MaxBindings];
	u32 writes_count;
};

VkBool32 vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_types, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data);
void vk_surface_create();
String vk_result_str(VkResult result);
VkImageCreateFlags vk_image_create_flags(Gfx_ImageType t);
VkImageType vk_image_type(Gfx_ImageType t);
VkImageUsageFlags vk_image_usage(Gfx_Format fmt);
VkFormat vk_format(Gfx_Format fmt);
VkImageAspectFlagBits vk_aspect(Gfx_Format fmt);

VkPrimitiveTopology vk_primitive_topology(Gfx_PrimitiveType t);
VkCullModeFlags vk_cullmode(Gfx_CullMode cm);
VkFrontFace vk_frontface(Gfx_FaceWinding fw);
VkCompareOp vk_compare_op(Gfx_CompareOp op);
VkStencilOp vk_stencil_op(Gfx_StencilOp op);
VkBlendOp vk_blend_op(Gfx_BlendOp op);
VkBlendFactor vk_blend_factor(Gfx_BlendFactor f);
VkColorComponentFlags vk_color_write_mask(Gfx_ColorMask m);
VkAttachmentLoadOp vk_load_op(Gfx_LoadOp a);
VkAttachmentStoreOp vk_store_op(Gfx_StoreOp a);
VkImageViewType vk_image_view_type(Gfx_ImageType t);
VkFilter vk_sampler_minmag_filter(Gfx_Filter f);
VkSamplerMipmapMode vk_sampler_mipmap_mode(Gfx_Filter f);
VkSamplerAddressMode vk_sampler_address_mode(Gfx_Wrap f);
VkBorderColor vk_sampler_border_color(Gfx_BorderColor c);

b32 vk_sync_needed(VK_Sync prev, VK_Sync next);
VkImageMemoryBarrier2 vk_sync_make_image_barrier(VkImage image, VkImageSubresourceRange range, VK_Sync prev, VK_Sync next);
// VkBufferMemoryBarrier2 vk_sync_make_buffer_barrier(VK_SyncBuffer buf, VK_Sync prev, VK_Sync next);
VK_SyncBatch vk_sync_begin(VkCommandBuffer cmd);
void vk_sync_end(VK_SyncBatch& b);
void vk_sync_image(VK_SyncBatch& b, VK_Image& img, VK_Sync next);
// void vk_sync_buffer(VK_SyncBatch& b, VK_SyncBuffer& buf, VK_Sync next);
void vk_sync_image_now(VkCommandBuffer cmd, VK_Image& img, VK_Sync next);
// void vk_sync_buffer_now(VkCommandBuffer cmd, VK_SyncBuffer& buf, VK_Sync next);
void vk_sync_everything(VkCommandBuffer cmd);

VkSemaphore vk_get_cur_image_available_semaphore();
VkSemaphore vk_get_cur_render_complete_semaphore();
VkCommandBuffer vk_cur_cmd();
u32 vk_find_memory_idx(u32 type_filter, u32 property_flags);
VkSemaphore vk_make_semaphore(u64 initial_counter);

void vk_make_cmd(VkCommandPool pool, u32 count, VkCommandBuffer* out);
void vk_begin_cmd(VkCommandBuffer cmd);
void vk_end_cmd(VkCommandBuffer cmd);
void vk_submit_cmd(VkCommandBuffer cmd);

VK_Memory vk_make_mem(u64 size, b32 gpu = 0);
VK_Buffer vk_make_buffer(u64 size, b32 gpu = 0);
VK_Buffer vk_get_vkbuffer(Gfx_Buffer buf);
VkShaderModule vk_make_shader(Slice<u8> code);
VkPipeline vk_make_pipeline(Gfx_PipelineDesc desc);
void vk_loader_load_core();
void vk_loader_load_instance();
void vk_loader_load_device();
void vk_instance_init();
void vk_device_init();
void vk_swapchain_create();
void vk_upload_image(VkCommandBuffer cmd, VK_Image image, u64 stage_offset);
void vk_make_mipmaps(VkCommandBuffer cmd, VK_Image& image);

////////////////////////////////////////////////////////////////////////
// @Gfx
struct Gfx_ProfileZone {
	String name;
	f64 start;
	f64 end;
	u32 depth;
	u64 vert_invocations;
	u64 frag_invocations;
};

struct Gfx_ProfileFrame {
	Array<Gfx_ProfileZone, 128> zones;
	f64 start_ms;
	f64 end_ms;
};

struct Gfx_PipelineStats {
	u64 vert_invocations;
	u64 frag_invocations;
};

struct Gfx_State {
	Arena arena;
	VkAllocationCallbacks allocator_;
	VkAllocationCallbacks* allocator;
	VkInstance instance;
	VkSurfaceKHR surface;
	VkDebugUtilsMessengerEXT debug_messenger;
	VK_Device device;
	VK_Swapchain swapchain;

	#define MaxTimeStamps 128
	VkQueryPool query_pools[Gfx_NumFramesInFlight];
	VkQueryPool stats_query_pools[Gfx_NumFramesInFlight];
	u32 zone_count[Gfx_NumFramesInFlight];
	String zone_names[Gfx_NumFramesInFlight * MaxTimeStamps];
	u32 zone_depth[Gfx_NumFramesInFlight * MaxTimeStamps];
	u32 open_zone_index[MaxTimeStamps];
	u32 stack_depth;
	Gfx_ProfileFrame cur_prof_frame;
	f64 ns_per_tick;

	VkSemaphore image_available_semaphores[Gfx_MaxImagesInFlight];
	VkSemaphore render_complete_semaphores[Gfx_MaxImagesInFlight];
	VkSemaphore compute_complete_semaphores[Gfx_MaxImagesInFlight];
	VkFence in_flight_fences[Gfx_NumFramesInFlight];

	VkCommandBuffer render_cmds[Gfx_NumFramesInFlight];
	VkCommandBuffer upload_cmd;

	VkDescriptorPool descriptor_pool;
	VkDescriptorSetLayout descriptor_set_layout;
	VkDescriptorSet descriptor_set;
	VkPipelineLayout pipeline_layout;

	VK_Memory gpu_mem;
	VK_Memory cpu_mem;
	u64 images_mem_usage;
	u64 target_images_mem_usage;
	VK_DescriptorWriter descriptor_writer;

	VK_Buffer gpu_buf;
	VK_Buffer cpu_buf;
	// u64 gpu_buf_address;
	// u64 cpu_buf_address;
	Gfx_Buffer stage_buffer;

	v2u size;
	b32 swapchain_resized;
	u32 images_in_flight;
	u32 frames_in_flight;
	u32 current_image_idx;
	u32 cur_frame_idx;
	u32 current_frame_idx_plus_one;

	Pool<VK_Shader, Gfx_MaxShaders, Gfx_Shader> shaders;
	Pool<VK_Pipeline, Gfx_MaxPipelines, Gfx_Pipeline> pipelines;
	Pool<VK_Image, Gfx_MaxImages, Gfx_Image> images;
	Pool<VK_View, Gfx_MaxViews, Gfx_View> views;
	Pool<VK_View, Gfx_MaxCubeViews, Gfx_View> cubemap_views;
	Pool<VK_Sampler, Gfx_MaxSamplers, Gfx_Sampler> samplers;

	RingBuffer stage_ring;
	u64 stage_in_flight_write;
	VkSemaphore stage_semaphore;
	u64 stage_completed;
	u64 stage_submitted;
	u64 stage_next_value;

	Mutex stage_mutex;
	QueueSPSC<Gfx_Stage, 32> stage_cmd_queue;
	Mutex stage_reserve_mutex;
	Gfx_Pass prev_pass;
	b32 is_first_pass;

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
		X(DestroyDevice) \
		X(GetDeviceQueue) \
		X(DeviceWaitIdle) \
		X(CreateCommandPool) \
		X(DestroyCommandPool) \
		X(CreateSwapchainKHR) \
		X(DestroySwapchainKHR) \
		X(GetSwapchainImagesKHR) \
		X(AcquireNextImageKHR) \
		X(CreateQueryPool) \
		X(GetQueryPoolResults) \
		X(CmdResetQueryPool) \
		X(CmdWriteTimestamp) \
		X(CmdBeginQuery) \
		X(CmdEndQuery) \
		X(CreateImage) \
		X(CreateImageView) \
		X(DestroyImage) \
		X(DestroyImageView) \
		X(GetImageMemoryRequirements) \
		X(GetBufferMemoryRequirements) \
		X(GetBufferDeviceAddress) \
		X(AllocateMemory) \
		X(FreeMemory) \
		X(AllocateCommandBuffers) \
		X(FreeCommandBuffers) \
		X(BeginCommandBuffer) \
		X(EndCommandBuffer) \
		X(BindImageMemory) \
		X(CreateSemaphore) \
		X(DestroySemaphore) \
		X(WaitSemaphores) \
		X(GetSemaphoreCounterValue) \
		X(CreateFence) \
		X(DestroyFence) \
		X(WaitForFences) \
		X(ResetFences) \
		X(GetFenceStatus) \
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
		X(BindBufferMemory) \
		X(MapMemory) \
		X(UnmapMemory) \
		X(FlushMappedMemoryRanges) \
		X(CreatePipelineLayout) \
		X(DestroyPipelineLayout) \
		X(CreateGraphicsPipelines) \
		X(CreateComputePipelines) \
		X(DestroyPipeline) \
		X(AllocateDescriptorSets) \
		X(FreeDescriptorSets) \
		X(UpdateDescriptorSets) \
		X(CmdBindPipeline) \
		X(CmdPipelineBarrier) \
		X(CmdPipelineBarrier2) \
		X(CmdBlitImage2) \
		X(CmdCopyBuffer2) \
		X(CmdCopyBufferToImage2) \
		X(CmdCopyImageToBuffer2) \
		X(CmdExecuteCommands) \
		X(CmdSetViewport) \
		X(CmdSetScissor) \
		X(CmdSetFrontFace) \
		X(CmdSetCullMode) \
		X(CmdSetStencilTestEnable) \
		X(CmdSetDepthTestEnable) \
		X(CmdSetDepthWriteEnable) \
		X(CmdSetDepthCompareOp) \
		X(CmdSetColorBlendEnableEXT) \
		X(CmdSetColorBlendEquationEXT) \
		X(CmdSetColorWriteMaskEXT) \
		X(CmdSetStencilReference) \
		X(CmdSetStencilOp) \
		X(CmdSetStencilCompareMask) \
		X(CmdSetStencilWriteMask) \
		X(CmdSetPrimitiveTopology) \
		X(CmdSetPolygonModeEXT) \
		X(CmdSetRasterizationSamplesEXT) \
		X(CmdSetSampleMaskEXT) \
		X(CmdSetAlphaToCoverageEnableEXT) \
		X(CmdBeginRendering) \
		X(CmdEndRendering) \
		X(CmdClearColorImage) \
		X(CmdClearDepthStencilImage) \
		X(CmdPushConstants) \
		X(CmdBindVertexBuffers) \
		X(CmdBindIndexBuffer) \
		X(CmdDraw) \
		X(CmdDrawIndexed) \
		X(CmdDrawIndirect) \
		X(CmdDrawIndexedIndirect) \
		X(CmdBindDescriptorSets) \
		X(QueueSubmit2) \
		X(QueueWaitIdle) \
		X(QueuePresentKHR) \

	#define VK_DECL(name) Glue(PFN_, vk##name) name;
	#define VK_GET_PROC(name) g.name = (Glue(PFN_, vk##name))os_lib_get_proc(g.lib, Stringify(vk##name));
	#define VK_INSTANCE_GET_PROC(name) g.name = (Glue(PFN_, vk##name))g.GetInstanceProcAddr(g.instance, Stringify(vk##name));
	#define VK_DEVICE_GET_PROC(name) g.name = (Glue(PFN_, vk##name))g.GetDeviceProcAddr(vkdevice, Stringify(vk##name));

	OS_Handle lib;

#define X(name) VK_DECL(name)
	VK_GET_PROC_LIST;
	VK_INSTANCE_GET_PROC_LIST;
	VK_DEVICE_GET_PROC_LIST;
#undef X
};

u32 gfx_format_bytesize(Gfx_Format fmt);

Gfx_Shader gfx_make_shader(Slice<u8> shd);
Gfx_Pipeline gfx_make_pipeline(Gfx_PipelineDesc desc);
Gfx_Image gfx_make_image(Gfx_ImageDesc desc);
Gfx_View gfx_make_view(Gfx_ViewDesc desc);
Gfx_Sampler gfx_make_sampler(Gfx_SamplerDesc desc);
Gfx_Buffer gfx_make_buffer(u64 size, b32 gpu, u64 align = 16);

Gfx_PipelineDesc gfx_query_pipeline_desc(Gfx_Pipeline pip);
Gfx_ImageDesc gfx_query_image_desc(Gfx_Image img);

void gfx_update_shader(Gfx_Shader shd, Slice<u8> code);
void gfx_update_pipeline(Gfx_Pipeline pip, Gfx_PipelineDesc desc);
u64 gfx_update_image(Gfx_Image img, Slice<Slice<u8>> data, b32 sync = false);
u64 gfx_update_buffer(Gfx_Buffer dst, u64 off, Slice<u8> data, b32 sync = false);
u64 gfx_update_vert_index(Gfx_Buffer vert_dst, u64 vert_off, Gfx_Buffer index_dst, u64 index_off, Slice<u8> vert_data, Slice<u8> index_data);

void gfx_destroy_image(Gfx_Image img);
void gfx_destroy_view(Gfx_View view);
void gfx_destroy_shader(Gfx_Shader shd);
void gfx_destroy_pipeline(Gfx_Pipeline pip);

void gfx_apply_viewport(Rng2 rect, b32 y_origin_at_bottom = false);
void gfx_apply_scissor(Rng2 rect);
void gfx_apply_primitive_type(Gfx_PrimitiveType t);
void gfx_apply_cull_mode(Gfx_CullMode cm);
void gfx_apply_face_winding(Gfx_FaceWinding fw);
void gfx_apply_depth_test(b32 disable);
void gfx_apply_depth_write(b32 enable);
void gfx_apply_depth_compare(Gfx_CompareOp op);
void gfx_apply_color_blend_enable(b32 enable);
void gfx_apply_color_blend_equation(Gfx_BlendState bs);
void gfx_apply_color_blend_mask(Gfx_BlendState bs);
void gfx_apply_sample(u32 sample_count);
void gfx_apply_sample_mask(u32 sample_count, u32 mask);
void gfx_apply_alpha_to_coverage(b32 enable);

void gfx_push_ctx(Gfx_DrawCtx constants);
void gfx_draw(u32 base_vert, u32 vert_count, u32 instance_count = 1, u32 base_instance = 0);
void gfx_draw_indirect(Region region, Gfx_Buffer buf, u32 offset, u32 stride);
void gfx_draw_mesh(Gfx_Mesh mesh);

void gfx_begin_pass(Gfx_Pass pass);
void gfx_end_pass();

u64 gfx_push_buffer(Gfx_Buffer& arena, u64 size);
b32 gfx_ready(u64 counter);

u8* gfx_buffer_base_ptr(Gfx_Buffer buf);

void gfx_bind_pipeline(Gfx_Pipeline pip);
void gfx_make_bind(Gfx_DescriptorDesc desc);
void gfx_bind_buffer(Gfx_Buffer buf, u32 binding);
void gfx_make_binding_buffers(Slice<Gfx_BufferDesc> descs);
void gfx_flush_descriptors();
void gfx_idle();

void gfx_begin_timestamp(String name);
void gfx_end_timestamp();

void gfx_init();
void gfx_shutdown();
void gfx_begin();
void gfx_end();

#if BUILD_DEV
 #define GfxBeginBlock(name) gfx_begin_timestamp(name)			
 #define GfxEndBlock() gfx_end_timestamp()		
#else
	#define GfxBeginBlock(name)
	#define GfxEndBlock()
#endif

