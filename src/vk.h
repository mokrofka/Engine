#pragma once
#include "vk_bindings.h"

#include "vulkan/vulkan_core.h"

const u32 MaxMaterials  = KB(1);
const u32 MaxLights     = KB(1);
const u32 MaxLines      = KB(1);
const u32 MaxMeshes     = KB(1);
const u32 MaxTextures   = KB(1);
const u32 MaxDrawCalls  = KB(1);
const u32 MaxDebugLines = KB(1);

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
#define vkdevice st->vk.device.logical_device

struct VK_KeyToShaderPipeline { String name; ShaderState state; };
u64 hash(VK_KeyToShaderPipeline x);
b32 equal(VK_KeyToShaderPipeline a, VK_KeyToShaderPipeline b);

struct GpuMaterial {
  u32 pipeline_idx;
  u32 texture_idx;
};

typedef u32 VK_BufferUsageFlags;
enum {
  VK_BufferUsageFlag_Vert = Bit(0),
  VK_BufferUsageFlag_Index = Bit(1),
  VK_BufferUsageFlag_Dst = Bit(2),
  VK_BufferUsageFlag_Src = Bit(3),
  VK_BufferUsageFlag_Storage = Bit(4),
  VK_BufferUsageFlag_Indirect = VK_BufferUsageFlag_Storage | Bit(5),
};

enum VK_DescriptorType {
  VK_DescriptorType_Storage,
  VK_DescriptorType_Image,
  VK_DescriptorType_Sampler,
};

enum VK_MemType {
  VK_MemType_Gpu,
  VK_MemType_Cpu,
};

enum VK_RenderpassType {
  VK_RenderpassType_World,
  VK_RenderpassType_UI,
  VK_RenderpassType_Screen,
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

struct VK_Pipeline {
  VkPipeline h;
  u32 batch_idx;
};

struct VK_MeshBatch {
  GpuMeshId mesh_id;
  Darray<u32> entities;
};

struct VK_MeshesBatches {
  Darray<VK_MeshBatch> mesh_batches;
  Map<u32, u32> mesh_to_batch;
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

struct VK_PushConstant {
  u32 drawcall_offset;
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
  VkImage h;
  VkImageView view;
  VK_ImageInfo info;
  u64 offset;
};

struct VK_Mesh {
  u32 vert_count;
  u64 vert_mem_offset;
  u32 index_count;
  u64 index_mem_offset;
};

struct VK_Swapchain {
  VkSwapchainKHR h;
  VkSwapchainKHR h_old;
  VkSurfaceFormatKHR format;  
  VkPresentModeKHR present_mode;
  VK_Image images[4];
  VK_Image old_images[4];
  VK_Image depth_attachment;
  u64 depth_image_mem_offset;
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

struct VK_ShaderModule {
  VkPipelineShaderStageCreateInfo stages[2];
};

struct VK_ShaderModuleEntry {
  VK_ShaderModule module;
  Darray<u32> track_pipelines;
  Darray<u32> track_shader_states;
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
  u32 entity_inst_offset;
};

struct VK_IndirectDrawCall {
  u32 draw_call_offset; // push constant and mem offset * sizeof()
  u32 draw_call_count;
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
  VK_SyncObj sync;
  VK_Swapchain swapchain;

  u32 images_in_flight;
  u32 frames_in_flight;
  u32 current_image_idx;
  u32 current_frame_idx;
  u32 width;
  u32 height;
  f32 scale;
  f32 old_scale;

  VK_Memory gpu_mem;
  VK_Memory cpu_mem;
  VK_Buffer vert_buffer;
  VK_Buffer index_buffer;
  VK_Buffer stage_buffer;
  VK_Buffer indirect_draw_buffer;
  VK_Buffer storage_buffer;
  
  VkDescriptorPool descriptor_pool;
  VkDescriptorSetLayout descriptor_set_layout;
  VkDescriptorSet descriptor_sets;

  VkCommandBuffer* cmds;
  VkCommandBuffer upload_cmd;
  VkSampler sampler;
  VkPipelineLayout pipeline_layout;

  VK_Image msaa_texture;
  VK_Image offscreen_depth_buffer;
  VK_Image* texture_targets;

  Darray<VK_Pipeline> pipelines;
  Darray<u32> entity_pipelines;
  u32 screen_pipeline;
  u32 cubemap_pipeline;
  u32 debug_line_pipeline;
  u32 ui_pipeline;
  u32 triangle_pipeline;
  Darray<VK_ShaderModuleEntry> modules;
  Map<VK_KeyToShaderPipeline, u32> shader_to_pipeline;
  Map<String, u32> shader_to_module;
  Array<GpuMaterial, MaxMaterials> materials;
  Darray<ShaderState> shader_states;
  Darray<VK_PipelineBatch> batches;

  Array<VK_RenderEntity, MaxEntities+MaxStaticEntities> entities;
  Array<VK_Mesh, MaxMeshes> meshes;
  Array<VK_Image, MaxTextures> textures;

  Darray<VK_IndirectDrawCall> static_draw_calls;
  u32 static_entities_count;
  u32 static_entities_count_old;

  u64 draw_lines_offset;
  u64 draw_lines_consistent_offset;
  u64 draw_rects_offset;
  Array<DebugDrawLine, MaxDebugLines> draw_lines;
  Array<DebugDrawLine, MaxDebugLines> draw_lines_consistent;
  Array<DebugDrawRect, MaxDebugLines> draw_rects;

  GlobalStateGPU* gpu_global_shader_st;
  EntityGPU* gpu_entities;
  MaterialGPU* gpu_materials;
  VK_DrawCallInfo* gpu_draw_call_infos;
  u32* gpu_entities_indices;

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

v4& get_pos();
mat4& get_mat();
void vk_init();

GpuTextureId vk_texture_load(Texture texture);
GpuMaterialId vk_material_load(MaterialDesc material);
GpuCubemapId vk_cubemap_load(Texture* textures);
GpuMeshId vk_mesh_load(Mesh mesh);

void vk_shader_reload(String name);
void vk_shader_compile_and_load_modules();

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

struct ImGui_DrawList {
  ImDrawList* draw;
};

ImGui_DrawList imgui_get_window_drawlist();
void imgui_draw_rect(ImGui_DrawList draw, Rng2 rect, v4 col, f32 rounding = 0, ImDrawFlags flags = 0, f32 thickness = 1);
void imgui_draw_rect_filled(ImGui_DrawList draw, Rng2 rect, v4 col, f32 rounding = 0, ImDrawFlags flags = 0);
void imgui_draw_push_clip_rect(ImGui_DrawList draw, Rng2 rect);
void imgui_draw_pop_clip_rect(ImGui_DrawList draw);
void imgui_draw_line(ImGui_DrawList draw, v2 p0, v2 p1, v4 col, f32 thickness = 1);
void imgui_draw_text(ImGui_DrawList draw, v2 pos, v4 col, String fmt, ...);
void imgui_text(String fmt, ...);
v2 imgui_calc_text_size(String str);
void imgui_begin_tab_item(String str);

