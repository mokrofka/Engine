
MakeId(Gfx_Image)
MakeId(Gfx_Sampler)
MakeId(Gfx_Shader)
MakeId(Gfx_Pipeline)
MakeId(Gfx_View)
MakeId(Gfx_Buffer)

// Configuration
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
// signed normalized (SN postfix)
// unsigned integer (UI postfix)
// signed integer (SI postfix)
enum Gfx_PixelFormat {
  Gfx_PixelFormat_Default,
  Gfx_PixelFormat_None,

  Gfx_PixelFormat_BGRA8,
  Gfx_PixelFormat_BGRA8_SRGB,

  Gfx_PixelFormat_RGBA8,
  Gfx_PixelFormat_RGBA8_SRGB,

  Gfx_PixelFormat_Depth,
  Gfx_PixelFormat_DepthStencil,

  Gfx_DefaultTextureTargetColorFormat = Gfx_PixelFormat_BGRA8,
  Gfx_DefaultDepthFormat = Gfx_PixelFormat_Depth,
};

// enum Gfx_ImageSamplerType {
//   Gfx_ImageSamplerType_Default,
//   Gfx_ImageSamplerType_Float,
//   Gfx_ImageSamplerType_Depth,
//   Gfx_ImageSamplerType_SInt,
//   Gfx_ImageSamplerType_UInt,
// };

// enum Gfx_SamplerType {
//   Gfx_SamplerType_Default,
//   Gfx_SamplerType_Filtering,
//   Gfx_SamplerType_NonFiltering,
//   Gfx_SamplerType_Comparison,
// };

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
  u8* data;
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

// enum Gfx_ShaderStage {
//   Gfx_ShaderStage_None,
//   Gfx_ShaderStage_Vertex,
//   Gfx_ShaderStage_Fragment,
//   Gfx_ShaderStage_Compute,
// };

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

////////////////////////////////////////////////////////////////////////
// Vulkan

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
#define vkdevice st->vk.device.logical_device

enum VK_MemType {
  VK_MemoryType_Cpu,
  VK_MemoryType_Gpu,
};

typedef u32 VK_Access;
enum {
  VK_Access_None = 0,
  VK_Access_Staging = Bit(0),
  VK_Access_VertBuffer = Bit(1),
  VK_Access_IndexBuffer = Bit(2),
  VK_Access_StorageBuffer_RO = Bit(3),
  VK_Access_StorageBuffer_RW = Bit(4),
  VK_Access_Texture = Bit(5),
  VK_Access_StorageImage = Bit(6),
  VK_Access_ColorAttachment = Bit(7),
  VK_Access_ResolveAttachment = Bit(8),
  VK_Access_DepthAttachment = Bit(9),
  VK_Access_StencilAttachment = Bit(10),
  VK_Access_Discard = Bit(11),
  VK_Access_Present = Bit(12),
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
  VK_ImageInfo info;

  VkDeviceMemory memory;
  VkImage h;
  VkImageView view;
  Gfx_ImageType type;
  u32 width;
  u32 height;
  u32 slices_count;
  u32 mipmaps_count;
  Gfx_ImageUsage usage;
  Gfx_PixelFormat pixel_format;
  u32 sample_count;
  VK_Access cur_access;

  u64 mem_offset;
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

struct VK_Shader {
  VkShaderModule h;
  b32 vert;
  b32 frag;
  b32 comp;
};

struct VK_Pipeline {
  VkPipeline h;
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

String vk_result_str(VkResult result);
void vk_surface_create();

////////////////////////////////////////////////////////////////////////
// @Misc

VkSemaphore vk_get_current_image_available_semaphore();
VkSemaphore vk_get_current_render_complete_semaphore();
VkCommandBuffer vk_get_cur_cmd();

void vk_bind_pipeline(VkPipeline pipeline);
u32 vk_find_memory_idx(u32 type_filter, u32 property_flags);
void vk_push_constants(VK_PushConstant constants);
// VK_Semaphore vk_semaphore_make(u64 initial_counter = 0);
// void vk_semaphore_wait(VK_Semaphore semaphore, u64 counter);

////////////////////////////////////////////////////////////////////////
// @Cmd

VkCommandBuffer vk_cmd_alloc(VkCommandPool pool);
void vk_cmd_free(VkCommandPool pool, VkCommandBuffer cmd);
void vk_cmd_begin(VkCommandBuffer cmd);
void vk_cmd_end(VkCommandBuffer cmd);
void vk_cmd_submit(VkCommandBuffer cmd);
void vk_cmd_end_submit(VkCommandBuffer cmd);
VkCommandBuffer vk_cmd_alloc_begin();
void vk_cmd_end_free(VkCommandBuffer cmd);

////////////////////////////////////////////////////////////////////////
// @Buffer

VK_Memory vk_mem_alloc(VK_MemType type, u64 size);
VK_Buffer vk_buffer_alloc(u64 size, VK_MemType mem_type = VK_MemoryType_Gpu);
void vk_buffer_upload(VK_Buffer buffer, Region region, void* data);
void vk_bind_vert_buffer(VK_Buffer buffer);
void vk_bind_index_buffer(VK_Buffer buffer);

////////////////////////////////////////////////////////////////////////
// Gfx

struct Gfx_Environment {
  u64 gpu_mem_size;
  u64 cpu_mem_size;
};

struct Gfx_State {
  Gfx_Environment environment;
  VK_Memory gpu_memory;
  VK_Memory cpu_memory;
  VK_DescriptorWriter descriptor_writer;
  Pool<VK_Buffer, Gfx_MaxBuffers, Gfx_Buffer> buffers;

  // Indirect drawing
  u32* base_index;
  u32 entity_cursor;
  u32 drawcall_cursor;
};

void gfx_init(Gfx_Environment environment);

Gfx_Shader gfx_shader_make(Gfx_ShaderDesc desc);
Gfx_Pipeline gfx_pipeline_make(Gfx_PipelineDesc desc);
Gfx_Image gfx_image_make(Gfx_ImageDesc desc);
Gfx_View gfx_view_make(Gfx_ViewDesc desc);
Gfx_Sampler gfx_sampler_make(Gfx_SamplerDesc desc);
Gfx_Buffer gfx_buffer_make(Gfx_BufferDesc desc);
void gfx_binding_make(Gfx_DescriptorDesc desc);
#define gfx_push_data(buf, bind, T, c) (T*)_gfx_push_data((buf), (bind), sizeof(T) * (c), alignof(T))
u8* _gfx_push_data(Gfx_Buffer buf, u32 binding, u64 size, u64 align);
void gfx_flush();

void gfx_image_destroy(Gfx_Image img);
void gfx_view_destroy(Gfx_View view);

void gfx_pass_begin(Gfx_Pass pass);
void gfx_pass_end();
void gfx_apply_viewport(Rng2 rect);
void gfx_apply_scissor(Rng2 rect);
void gfx_pipeline_bind(Gfx_Pipeline pip);
void gfx_draw(u32 base_vert, u32 vert_count, u32 instance_count = 1, u32 base_instance = 0);
void gfx_draw_indexed(u32 base_index, u32 index_count, u32 base_vert, u32 instance_count = 1, u32 base_instance = 0);
void gfx_draw_indirect(Gfx_IndirectDrawcall drawcall);
void gfx_draw_indexed_indirect(Gfx_IndirectDrawcall drawcall);

void gfx_push_indirect(Gfx_Mesh mesh, u32 id, u32 instance_count = 1);
u32  gfx_indirect_begin();
Gfx_IndirectDrawcall gfx_indirect_end(u32 base);

void gfx_push_indirect_instanced(Gfx_Mesh mesh, u32 count);
void gfx_instance_set_indices(u32* indices);
u32* gfx_indirect_indices();

void gfx_end();



