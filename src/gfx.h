#pragma once
#include "types.h"

MakeId(Gfx_Image)
MakeId(Gfx_Sampler)
MakeId(Gfx_Shader)
MakeId(Gfx_Pipeline)
MakeId(Gfx_View)
MakeId(Gfx_Buffer)

enum {
  Gfx_InvalidId,
  Gfx_NumFramesInFlight = 2,
  Gfx_MaxImagesInFlight = 4,
  Gfx_MaxColorAttachments = 8,
  Gfx_MaxMipmaps = 16,
  Gfx_MaxShaders = 32,
  Gfx_MaxPipelines = 32,
  Gfx_MaxViews = 32,
  Gfx_MaxImages = 32,
  Gfx_MaxSamplers = 32,
  Gfx_MaxBuffers = 32,
  Gfx_MaxCubeTextures = 16,
  Gfx_MaxStorageBuffers = 16,
  Gfx_MaxBindings = 64,
  Gfx_DefaultSampleCount = 4,
};

// unsigned normalized (no postfix)
// signed normalized (S postfix)
// unsigned integer (UI postfix)
// signed integer (SI postfix)
enum Gfx_PixelFormat {
  Gfx_PixelFormat_Default,
  Gfx_PixelFormat_None,

  Gfx_PixelFormat_R8_UI,
  Gfx_PixelFormat_R8,

  Gfx_PixelFormat_BGRA8,
  Gfx_PixelFormat_BGRA8_SRGB,

  Gfx_PixelFormat_RGBA8,
  Gfx_PixelFormat_RGBA8_SRGB,

  Gfx_PixelFormat_Depth,
  Gfx_PixelFormat_DepthStencil,

  Gfx_DefaultTextureTargetColorFormat = Gfx_PixelFormat_BGRA8,
  Gfx_DefaultDepthFormat = Gfx_PixelFormat_Depth,
};

enum Gfx_PrimitiveType {
  Gfx_PrimitiveType_Default,
  Gfx_PrimitiveType_Point,
  Gfx_PrimitiveType_Line,
  Gfx_PrimitiveType_Triangle,
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

enum Gfx_LoadAction {
  Gfx_LoadAction_Default,
  Gfx_LoadAction_Clear,
  Gfx_LoadAction_Load,
  Gfx_LoadAction_DontCare,
};

enum Gfx_StoreAction {
  Gfx_StoreAction_Default,
  Gfx_StoreAction_Store,
  Gfx_StoreAction_DontCare,
};

struct Gfx_ColorAttachmentAction {
  Gfx_LoadAction load_action;
  Gfx_StoreAction store_action;
  v4 clear_value;
};

struct Gfx_DepthAttachmentAction {
  Gfx_LoadAction load_action;
  Gfx_StoreAction store_action;
  f32 clear_value;
};

struct Gfx_StencilAttachmentAction {
  Gfx_LoadAction load_action;
  Gfx_StoreAction store_action;
  u8 clear_value;
};

struct Gfx_PassAction {
  union {
    Gfx_ColorAttachmentAction colors[Gfx_MaxColorAttachments];
    Gfx_ColorAttachmentAction color;
  };
  Gfx_DepthAttachmentAction depth;
  Gfx_StencilAttachmentAction stencil;
};

struct Gfx_Attachments {
  union {
    Gfx_View colors[Gfx_MaxColorAttachments];
    Gfx_View color;
  };
  union {
    Gfx_View resolves[Gfx_MaxColorAttachments];
    Gfx_View resolve;
  };
  Gfx_View depth_stencil;
};

struct Gfx_Pass {
  b32 compute;
  Gfx_PassAction action;
  Gfx_Attachments attachments;
};

enum Gfx_ImageType {
  Gfx_ImageType_Default,
  Gfx_ImageType_2D,
  Gfx_ImageType_Cube,
  Gfx_ImageType_3D,
  Gfx_ImageType_Array,
};

typedef u32 Gfx_ImageUsage;
enum {
  Gfx_ImageUsage_StorageImage = Bit(0),
  Gfx_ImageUsage_ColorAttachment = Bit(1),
  Gfx_ImageUsage_ResolveAttachment = Bit(2),
  Gfx_ImageUsage_DepthStencilAttachment = Bit(3),
  Gfx_ImageUsage_Immutable = Bit(4),
};

struct Gfx_ImageDesc {
  Gfx_ImageType type;
  Gfx_ImageUsage usage;
  u32 width;
  u32 height;
  u32 slices_count;
  b32 mipmaps;
  Gfx_PixelFormat pixel_format;
  u32 sample_count;
  union {
    u8* data;
    u8* cube[6];
  };
};

enum Gfx_ViewType {
  Gfx_ViewType_Invalid,
  Gfx_ViewType_Texture,
  Gfx_ViewType_ColorAttachment,
  Gfx_ViewType_ResolveAttachment,
  Gfx_ViewType_DepthStencilAttachment,
};

struct Gfx_ImageViewDesc {
  Gfx_Image image;
  u32 mip_level;
  u32 slice;
  u32 mip_level_count;
  u32 slice_count;
};

struct Gfx_ViewDesc {
  Gfx_ImageViewDesc texture;
  Gfx_ImageViewDesc color_attachment;
  Gfx_ImageViewDesc resolve_attachment;
  Gfx_ImageViewDesc depth_stencil_attachment;
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

struct Gfx_ShaderDesc {
  String shader;
  b32 vert;
  b32 frag;
  b32 comp;
};

struct Gfx_StencilFaceState {
  Gfx_CompareOp compare;
  Gfx_StencilOp fail_op;
  Gfx_StencilOp depth_fail_op;
  Gfx_StencilOp pass_op;
};

struct Gfx_StencilState {
  b32 enabled;
  Gfx_StencilFaceState front;
  Gfx_StencilFaceState back;
  u8 read_mask;
  u8 write_mask;
  u8 ref;
};

struct Gfx_DepthState {
  Gfx_PixelFormat pixel_format;
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
};

struct Gfx_ColorTargetState {
  Gfx_PixelFormat pixel_format;
  Gfx_ColorMask write_mask;
  Gfx_BlendState blend;
};

struct Gfx_PipelineDesc {
  b32 compute;
  Gfx_Shader shader;
  Gfx_DepthState depth;
  Gfx_StencilState stencil;
  u32 color_count;
  union {
    Gfx_ColorTargetState colors[Gfx_MaxColorAttachments];
    Gfx_ColorTargetState color;
  };
  Gfx_PrimitiveType primitive_type;
  Gfx_CullMode cull_mode;
  Gfx_FaceWinding face_winding;
  u32 sample_count;
  v4 blend_color;
  b32 alpha_to_coverage_enabled;
};

enum Gfx_MemType {
  Gfx_MemType_Default,
  Gfx_MemType_Cpu,
  Gfx_MemType_Gpu,
};

struct Gfx_BufferDesc {
  Gfx_MemType type;
  u64 size;
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

struct Gfx_IndirectDrawcall {
  u32 base;
  u32 count;
};

struct Gfx_Mesh {
  u32 vert_count;
  u32 base_vert;
  u32 index_count;
  u32 base_index;
};

struct Gfx_Task {
  void (*on_complete)(void*);
  void* user_data;
};

////////////////////////////////////////////////////////////////////////
// @Vulkan

#include "vulkan/vulkan_core.h"

#if BUILD_DEBUG
  #define VK_CHECK(expr)                  \
    {                                     \
      if (expr != VK_SUCCESS) {           \
        Error("%s", vk_result_str(expr)); \
        InvalidPath;                      \
      }                                   \
    }
#else
  #define VK_CHECK(expr) expr
#endif
#define vkdevice st->gfx.device.logical_device

typedef u32 VK_Access;
enum {
  VK_Access_None = 0,

  // Transfer
  VK_Access_TransferDst = Bit(0),
  VK_Access_TransferSrc = Bit(1),

  // Vertex
  VK_Access_VertBuffer = Bit(2),
  VK_Access_IndexBuffer = Bit(3),

  // Shader reads
  VK_Access_StorageBuffer_RO = Bit(4),
  VK_Access_StorageBuffer_RW = Bit(5),
  VK_Access_Texture = Bit(6),
  VK_Access_StorageImage_RO = Bit(7),
  VK_Access_StorageImage_RW = Bit(8),

  // Shader stage
  VK_Access_VertexShader = Bit(9),
  VK_Access_FragmentShader = Bit(10),
  VK_Access_ComputeShader = Bit(11),

  // Attachments
  VK_Access_ColorAttachment = Bit(12),
  VK_Access_ResolveAttachment = Bit(13),
  VK_Access_DepthAttachment = Bit(14),
  VK_Access_StencilAttachment = Bit(15),
  VK_Access_DepthRead = Bit(16),

  VK_Access_IndirectBuffer = Bit(18),

  VK_Access_Discard = Bit(19),
  VK_Access_Present = Bit(20),
};

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

struct VK_BufferRegion {
  VkBuffer h;
  u64 base;
  u64 pos;
  u64 cap;
  VK_Access cur_access;
};

struct Buffer {
  Gfx_MemType type;
  u64 base;
  u64 size;
};

struct VK_Image {
  VkDeviceMemory memory;
  VkImage h;
  Gfx_ImageType type;
  u32 width;
  u32 height;
  u32 slices_count;
  u32 mipmaps_count;
  Gfx_ImageUsage usage;
  Gfx_PixelFormat pixel_format;
  u32 sample_count;
  VK_Access cur_access;
};

struct VK_View {
  VkImageView h;
  Gfx_Image ref;
  Gfx_ViewType type;
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
  b32 vert;
  b32 frag;
  b32 comp;
};

struct VK_Pipeline {
  VkPipeline h;
  Gfx_Shader shd_ref;
  b32 compute;
  Gfx_DepthState depth;
  Gfx_StencilState stencil;
  u32 color_count;
  Gfx_ColorTargetState colors[Gfx_MaxColorAttachments];
  Gfx_PrimitiveType primitive_type;
  Gfx_CullMode cull_mode;
  Gfx_FaceWinding face_winding;
  u32 sample_count;
  v4 blend_color;
  b32 alpha_to_coverage_enabled;
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
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memory;
  u32 gpu_type_idx;
  u32 cpu_type_idx;
  VkFormat depth_format;
};

struct VK_Swapchain {
  VkSwapchainKHR h;
  VkSurfaceFormatKHR format;  
  VkPresentModeKHR present_mode;
  VkImage images[4];
  VkImageView views[4];
  VK_Image depth_attachment;
  VkSwapchainKHR h_old;
  VkImageView old_view[4];
};

struct VK_Semaphore {
  VkSemaphore h;
  u64 counter;
};

union VK_PushConstant {
  u32 drawcall_base;
  u32 image_index;
};

struct VK_DescriptorWriter {
  VkDescriptorBindingFlags binding_flags[Gfx_MaxBindings];
  VkDescriptorSetLayoutBinding bindings[Gfx_MaxBindings];
  u32 binds_count;
  VkDescriptorBufferInfo buffers[Gfx_MaxBindings];
  VkWriteDescriptorSet writes[Gfx_MaxBindings];
  u32 writes_count;
};

struct VK_IndirectDrawCall {
  union {
    VkDrawIndirectCommand draw_command;
    VkDrawIndexedIndirectCommand index_draw_command;
  };
  u32 base_instance;
};

String vk_result_str(VkResult result);
void vk_surface_create();
void vk_swapchain_create();

VkSemaphore vk_get_cur_image_available_semaphore();
VkSemaphore vk_get_cur_render_complete_semaphore();
VkCommandBuffer vk_get_cur_cmd();

void vk_bind_pipeline(VkPipeline pipeline);
u32 vk_find_memory_idx(u32 type_filter, u32 property_flags);
void vk_push_constants(VK_PushConstant constants);
// VK_Semaphore vk_semaphore_make(u64 initial_counter = 0);
// void vk_semaphore_wait(VK_Semaphore semaphore, u64 counter);

VkCommandBuffer vk_cmd_alloc(VkCommandPool pool);
void vk_cmd_free(VkCommandPool pool, VkCommandBuffer cmd);
void vk_cmd_begin(VkCommandBuffer cmd);
void vk_cmd_end(VkCommandBuffer cmd);
void vk_cmd_submit(VkCommandBuffer cmd);
void vk_cmd_end_submit(VkCommandBuffer cmd);
VkCommandBuffer vk_cmd_alloc_begin();
void vk_cmd_end_free(VkCommandBuffer cmd);

VK_Memory vk_mem_make(Gfx_MemType type, u64 size);
VK_Buffer vk_buffer_make(u64 size, Gfx_MemType type = Gfx_MemType_Gpu);
void vk_bind_vert_buffer(VK_Buffer buffer);
void vk_bind_index_buffer(VK_Buffer buffer);
VK_Buffer* vk_choose_buffer(Gfx_Buffer buf);

////////////////////////////////////////////////////////////////////////
// @Gfx

struct Gfx_Environment {
  u64 gpu_mem_size;
  u64 cpu_mem_size;
  u64 image_mem_size;
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

  VkSemaphore image_available_semaphores[Gfx_MaxImagesInFlight];
  VkSemaphore render_complete_semaphores[Gfx_MaxImagesInFlight];
  VkSemaphore compute_complete_semaphores[Gfx_MaxImagesInFlight];
  VkFence in_flight_fences[Gfx_NumFramesInFlight];

  VkCommandBuffer cmds_frames_upload[Gfx_NumFramesInFlight];
  VkFence fences_frames_upload[Gfx_NumFramesInFlight];
  VkCommandBuffer cmds_render[Gfx_NumFramesInFlight];
  VkCommandBuffer cmds_upload[Gfx_NumFramesInFlight];
  VkFence fences_async_upload[Gfx_NumFramesInFlight];

  VkDescriptorPool descriptor_pool;
  VkDescriptorSetLayout descriptor_set_layout;
  VkDescriptorSet descriptor_sets;
  VkPipelineLayout pipeline_layout;

  Gfx_Environment environment;
  VK_Memory gpu_mem;
  VK_Memory cpu_mem;
  VK_DescriptorWriter descriptor_writer;

  VK_Buffer gpu_buf;
  VK_Buffer cpu_buf;

  v2u size;
  b32 swapchain_resized;
  u32 images_in_flight;
  u32 frames_in_flight;
  u32 current_image_idx;
  u32 current_frame_idx;
  u32 current_frame_idx_plus_one;

  Pool<Buffer, Gfx_MaxBuffers, Gfx_Buffer> buffers;
  Pool<VK_Shader, Gfx_MaxShaders, Gfx_Shader> shaders;
  Pool<VK_Pipeline, Gfx_MaxPipelines, Gfx_Pipeline> pipelines;
  Pool<VK_Image, Gfx_MaxImages, Gfx_Image> images;
  Pool<VK_View, Gfx_MaxViews, Gfx_View> views;
  Pool<VK_View, Gfx_MaxViews, Gfx_View> cubemap_views;
  Pool<VK_Sampler, Gfx_MaxSamplers, Gfx_Sampler> samplers;

  u32 cube_idx;
  struct {
    v2u size;
    b32 compute;
    Gfx_PassAction action;
    Gfx_Attachments attachments;
  } cur_pass;

  // Indirect drawing
  Gfx_Buffer drawcall_buf;
  u32* base_index;
  u32 entity_cursor;
  u32 drawcall_cursor;

  Gfx_Buffer stage_buffer;

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
    X(CreateImage) \
    X(CreateImageView) \
    X(DestroyImage) \
    X(DestroyImageView) \
    X(GetImageMemoryRequirements) \
    X(GetBufferMemoryRequirements) \
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

Gfx_Shader gfx_shader_make(Gfx_ShaderDesc desc);
Gfx_Pipeline gfx_pipeline_make(Gfx_PipelineDesc desc);
Gfx_Image gfx_image_make(Gfx_ImageDesc desc);
Gfx_View gfx_view_make(Gfx_ViewDesc desc);
Gfx_Sampler gfx_sampler_make(Gfx_SamplerDesc desc);
Gfx_Buffer gfx_buffer_make(u64 size, Gfx_MemType type = Gfx_MemType_Gpu, u64 align = 16);
Gfx_Buffer gfx_buffer_make_round_base(u64 size, u64 round, Gfx_MemType type = Gfx_MemType_Gpu, u64 align = 16);
void gfx_bind_make(Gfx_DescriptorDesc desc);

Gfx_PipelineDesc gfx_query_pipeline_desc(Gfx_Pipeline pip);
Gfx_ImageDesc gfx_query_image_desc(Gfx_Image img);

void gfx_shader_update(Gfx_Shader shd, Gfx_ShaderDesc desc);
void gfx_pipeline_update(Gfx_Pipeline pip, Gfx_PipelineDesc desc);
void gfx_image_update(Gfx_Image img, u8* data);
void gfx_image_update(Gfx_Image img, Gfx_ImageDesc desc);
void gfx_view_update(Gfx_View view, Gfx_ViewDesc desc);
void gfx_buffer_update(Gfx_Buffer buf, u64 offset, Slice<u8> data);

void gfx_image_destroy(Gfx_Image img);
void gfx_view_destroy(Gfx_View view);
void gfx_shader_destroy(Gfx_Shader shd);
void gfx_pipeline_destroy(Gfx_Pipeline pip);

void gfx_image_readback(Gfx_Image img, u8* dst);
void gfx_pass_begin(Gfx_Pass pass);
void gfx_pass_end();
void gfx_apply_viewport(Rng2 rect, b32 y_origin_at_bottom = false);
void gfx_apply_scissor(Rng2 rect);
void gfx_draw(u32 base_vert, u32 vert_count, u32 instance_count = 1, u32 base_instance = 0);
void gfx_draw_indexed(u32 base_index, u32 index_count, u32 base_vert, u32 instance_count = 1, u32 base_instance = 0);
void gfx_draw_indirect(Gfx_IndirectDrawcall drawcall);
void gfx_draw_indexed_indirect(Gfx_IndirectDrawcall drawcall);
void gfx_draw_mesh(Gfx_Mesh mesh);

void gfx_draw_mesh_indirect(Gfx_Mesh mesh, u32 id, u32 instance_count = 1);
u32  gfx_indirect_begin();
Gfx_IndirectDrawcall gfx_indirect_end(u32 base);

void gfx_push_indirect_instanced(Gfx_Mesh mesh, u32 count);
void gfx_instance_set_indices(u32* indices);
u32* gfx_indirect_indices();

u8* gfx_buffer_base_ptr(Gfx_Buffer buf);
u64 gfx_buffer_base(Gfx_Buffer buf);

void gfx_pipeline_bind(Gfx_Pipeline pip);
void gfx_bind_vert(Gfx_MemType type = Gfx_MemType_Gpu);
void gfx_bind_index(Gfx_MemType type = Gfx_MemType_Gpu);
void gfx_bind_buffer(Gfx_Buffer buf, u32 binding);
void gfx_flush();

void gfx_idle();
void gfx_init(Gfx_Environment environment);
void gfx_shutdown();
void gfx_begin();
void gfx_end();


