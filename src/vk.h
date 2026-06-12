#pragma once
#include "vk_bindings.h"

const u32 MaxMaterials  = 32;
const u32 MaxLights     = 32;
const u32 MaxMeshes     = 128;
const u32 MaxTextures   = 32;
const u32 MaxLines      = KB(1);
const u32 MaxDrawCalls  = KB(1);
const u32 MaxDebugLines = KB(1);

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

struct VK_KeyToShaderPipeline { String name; ShaderState state; };
u64 hash(VK_KeyToShaderPipeline x);
b32 equal(VK_KeyToShaderPipeline a, VK_KeyToShaderPipeline b);

struct GpuMaterial {
  u32 pipeline_idx;
  u32 texture_idx;
};

enum VK_DescriptorType {
  VK_DescriptorType_Storage,
  VK_DescriptorType_Image,
  VK_DescriptorType_Sampler,
};

enum VK_RenderpassType {
  VK_RenderpassType_World,
  VK_RenderpassType_UI,
  VK_RenderpassType_Screen,
};

struct GPUJob {
  VkFence fence;
  void (*on_complete)(void*);
  void* user_data;
};

///////////////////////////////////
// Gpu memory layout

struct EntityGPU {
  alignas(16) mat4 model;
  alignas(16) v4 color;
  u32 material_idx;
};

struct MaterialGPU {
  alignas(16) v3 ambient;
  alignas(16) v3 diffuse;
  alignas(16) v3 specular;
  f32 shininess;
  u32 texture_idx;
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
  mat4 mat;
  alignas(16) v4 ambient_color;
  f32 time;
  u32 point_light_count;
  u32 dir_light_count;
  u32 spot_light_count;
  u32 entity_indices[MaxEntities+MaxStaticEntities];
};

struct VK_Pipeline0 {
  VkPipeline h;
  u32 batch_idx;
};

struct VK_MeshBatch {
  GpuMeshId mesh_id;
  Darray<OpaqueId> entities;
};

struct VK_MeshesBatches {
  Darray<VK_MeshBatch> mesh_batches;
  Dmap<u32, u32> mesh_to_batch;
};

typedef u32 VK_BatchType;
enum {
  VK_BatchType_Indexed,
  VK_BatchType_Unindexed,
  VK_BatchType_StaticIndexed,
  VK_BatchType_StaticUnindexed,
};

struct VK_PipelineBatch {
  VK_MeshesBatches batches[4];
};

struct VK_RenderEntity {
  u32 entity_idx_in_mesh_batch;
#if BUILD_DEBUG
  b32 is_init;
#endif
};

struct VK_Mesh {
  u32 vert_count;
  u32 base_vert;
  u32 index_count;
  u32 base_index;
};

struct VK_ShaderModuleEntry {
  VK_Shader module;
  Darray<u32> track_pipelines;
  Darray<u32> track_shader_states;
};

struct VK_DrawCallInfo {
  union {
    VkDrawIndirectCommand draw_command;
    VkDrawIndexedIndirectCommand index_draw_command;
  };
  u32 base_instance;
};

struct VK_IndirectDrawCall {
  u32 drawcall_base; // push constant and mem offset * sizeof()
  u32 drawcall_count;
};

struct VK_State {
  Arena arena;
  AllocSegList gpa;
  
  VkAllocationCallbacks allocator_;
  VkAllocationCallbacks* allocator;
  VkInstance instance;
  VkSurfaceKHR surface;
  VkDebugUtilsMessengerEXT debug_messenger;
  
  VK_Device device;
  VK_Swapchain swapchain;

  VkSemaphore* image_available_semaphores;
  VkSemaphore* render_complete_semaphores;
  VkSemaphore* compute_complete_semaphores;
  VkFence* in_flight_fences;

  // VK_Semaphore* frames_upload_semaphores;
  VkCommandBuffer* cmds_frames_upload;
  VkFence* fences_frames_upload;
  VkCommandBuffer* cmds_render;
  VkCommandBuffer* cmds_upload;
  VkFence* fences_async_upload;

  u32 images_in_flight;
  u32 frames_in_flight;
  u32 current_image_idx;
  u32 current_frame_idx;
  u32 current_frame_idx_plus_one;
  u32 width;
  u32 height;
  f32 scale;
  f32 old_scale;

  
  VkDescriptorPool descriptor_pool;
  VkDescriptorSetLayout descriptor_set_layout;
  VkDescriptorSet descriptor_sets;

  VkSampler sampler;
  VkPipelineLayout pipeline_layout;

  VK_Image msaa_texture0;
  VK_Image offscreen_depth_buffer0;
  VK_Image texture_targets0[16];

  Darray<VK_Pipeline0> pipelines0;
  Darray<u32> entity_pipelines;
  Darray<VK_ShaderModuleEntry> modules;
  Map<VK_KeyToShaderPipeline, u32, Gfx_MaxPipelines> shader_to_pipeline;
  Map<String, u32, Gfx_MaxShaders> shader_to_module;
  Array<GpuMaterial, MaxMaterials> materials;
  Darray<ShaderState> shader_states;
  Darray<VK_PipelineBatch> batches;

  Array<VK_RenderEntity, MaxEntities+MaxStaticEntities> entities;
  Array<VK_Mesh, MaxMeshes> meshes;
  Array<VK_Image, MaxTextures> textures;

  Darray<VK_IndirectDrawCall> static_draw_calls;
  u32 static_entities_count;
  u32 static_entities_count_old;


  GlobalStateGPU* gpu_global_shader_st;
  EntityGPU* gpu_entities;
  MaterialGPU* gpu_materials;
  VK_DrawCallInfo* gpu_draw_call_infos;
  u32* gpu_entities_indices;

  Pool<VK_Shader, Gfx_MaxShaders, Gfx_Shader> shaders;
  Pool<VK_Pipeline, Gfx_MaxPipelines, Gfx_Pipeline> pipelines;
  Pool<VK_Image, Gfx_MaxImages, Gfx_Image> images;
  Pool<VK_View, Gfx_MaxViews, Gfx_View> views;
  Pool<VK_Sampler, Gfx_MaxSamplers, Gfx_Sampler> samplers;
  struct {
    v2u size;
    b32 compute;
    Gfx_PassAction action;
    Gfx_Attachments attachments;
  } cur_pass;

  Gfx_Pipeline triangle_pip;
  Gfx_Pipeline screen_pip;
  Gfx_Pipeline cubemap_pip;
  Gfx_Pipeline debug_line_pip;
  Gfx_Pipeline ui_pip;
  Gfx_Sampler com_sampler;

  Gfx_Image image_color[4];
  Gfx_Image image_resolve[4];
  Gfx_Image image_depth[4];
  Gfx_View views_color[4];
  Gfx_View views_resolve[4];
  Gfx_View views_depth[4];

  VK_Memory gpu_mem;
  VK_Memory cpu_mem;
  VK_Buffer vert_buffer;
  VK_Buffer index_buffer;
  VK_Buffer stage_buffer;
  VK_Buffer indirect_draw_buffer;
  VK_Buffer storage_buffer;

  RingBuffer vert_ring_buffer;
  VK_Buffer vert_buffer_each_frame;
  Array<DebugDrawLine, MaxDebugLines> draw_lines;
  Array<DebugDrawLine, MaxDebugLines> draw_lines_consistent;
  Array<DebugDrawRect, MaxDebugLines> draw_rects;
  u64 draw_lines_offset;
  u64 draw_lines_consistent_offset;
  u64 draw_rects_offset;

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
    X(WaitSemaphores) \
    X(CreateFence) \
    X(DestroyFence) \
    X(WaitForFences) \
    X(ResetFences) \
    X(GetFenceStatus) \
    X(AcquireNextImageKHR) \
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
    X(CreateComputePipelines) \
    X(DestroyPipeline) \
    X(AllocateDescriptorSets) \
    X(FreeDescriptorSets) \
    X(UpdateDescriptorSets) \
    X(CmdBindPipeline) \
    X(CmdPipelineBarrier) \
    X(CmdPipelineBarrier2) \
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

void vk_image_layout_transition(VkCommandBuffer cmd, VK_Image image, VkImageLayout old_layout, VkImageLayout new_layout, VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT);
void vk_image_layout_transition_swapchain(VkCommandBuffer cmd, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout, VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT);
void vk_image_upload_to_gpu(VkCommandBuffer cmd, VK_Image image);
void vk_texture_generate_mipmaps(VK_Image image);

v4& get_pos();
mat4& get_mat();
void vk_init();

GpuTextureId vk_texture_load(Texture texture);
GpuMaterialId vk_material_load(MaterialDesc material);
GpuCubemapId vk_cubemap_load(Texture* textures);
GpuMeshId vk_mesh_load(Mesh mesh);

void vk_shader_reload(String name);

void vk_begin_draw_frame();
void vk_end_draw_frame();

void vk_init();
void vk_shutdown();

void vk_make_renderable(EntityId entity, GpuMeshId mesh_id, GpuMaterialId material_id);
void vk_make_renderable_static(StaticEntityId entity, GpuMeshId mesh_id, GpuMaterialId material_id);
void vk_remove_renderable(EntityId entity_id);
void vk_remove_static_renderable(StaticEntityId entity_id);
void vk_set_entity_color(EntityId entity_handle, v4 color);

void vk_draw_line(v3 a, v3 b, v4 color);
void vk_draw_line_consistent(v3 a, v3 b, v4 color);
void vk_draw_cuboid(Rng3 rng, v4 color);
void vk_draw_rect(Rng2 rect, v4 color);

void vk_imgui_init();
void vk_imgui_begin_frame();
void vk_imgui_end_frame();

Slice<String> vk_shader_compile(Allocator arena);

#define IM_VEC2_CLASS_EXTRA                               \
        constexpr ImVec2(const v2& f) : x(f.x), y(f.y) {} \
        operator v2() const { return v2(x,y); }
#include "imgui/imgui.h"


