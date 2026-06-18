
const u32 MaxMaterials  = 32;
const u32 MaxLights     = 32;
const u32 MaxMeshes     = 32;
const u32 MaxTextures   = 32;
const u32 MaxDrawCalls  = KB(1);
const u32 MaxDebugLines = KB(1);

struct R_KeyToShaderPipeline { String name; Gfx_PipelineDesc pipeline_desc; };
u64 hash(R_KeyToShaderPipeline x);
b32 equal(R_KeyToShaderPipeline a, R_KeyToShaderPipeline b);

#include "vk_bindings.h"

struct MaterialDesc {
  String shader_name;
  Gfx_PipelineDesc pipeline_desc;
  // ShaderDesc shader;
  MaterialProps props;
  String texture;
};

struct R_GpuMaterial {
  u32 entity_pipeline_idx;
  u32 texture_idx;
};

///////////////////////////////////
// Gpu memory layout

struct R_EntityGPU {
  alignas(16) mat4 model;
  alignas(16) v4 color;
  u32 material_idx;
};

struct R_MaterialGPU {
  alignas(16) v3 ambient;
  alignas(16) v3 diffuse;
  alignas(16) v3 specular;
  f32 shininess;
  u32 texture_idx;
};

struct R_PointLightGPU {
  alignas(16) v3 color;
  alignas(16) v3 pos;
  f32 intensity;
  f32 rad;
};

struct R_DirLightGPU {
  alignas(16) v3 color;
  alignas(16) v3 dir;
  f32 intensity;
};

struct R_SpotLightGPU {
  alignas(16) v3 color;
  alignas(16) v3 pos;
  alignas(16) v3 dir;
  f32 intensity;
  f32 inner_cutoff;
  f32 outer_cutoff;
};

struct R_GlobalStateGPU {
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

struct R_EntityPipeline {
  Gfx_Pipeline pip;
  u32 batch_idx;
};

struct R_MeshBatch {
  GpuMeshId mesh_id;
  Darray<OpaqueId> entities;
};

struct R_MeshesBatches {
  Darray<R_MeshBatch> mesh_batches;
  Dmap<u32, u32> mesh_to_batch;
};

struct R_EntityPipelineBatch {
  R_MeshesBatches batches[4];
};

typedef u32 R_BatchType;
enum {
  VK_BatchType_Indexed,
  VK_BatchType_Unindexed,
  VK_BatchType_StaticIndexed,
  VK_BatchType_StaticUnindexed,
};

struct R_RenderEntity {
  u32 entity_idx_in_mesh_batch;
#if BUILD_DEBUG
  b32 is_init;
#endif
};

struct R_ShaderModuleEntry {
  Gfx_Shader shd;
  Darray<Gfx_Pipeline> track_pipelines;
};

struct R_State {
  Arena arena;
  AllocSegList gpa;
  
  f32 scale;
  f32 old_scale;
  
  Darray<R_EntityPipeline> entity_pipelines;
  Darray<R_ShaderModuleEntry> modules;
  Map<R_KeyToShaderPipeline, Gfx_Pipeline, Gfx_MaxPipelines> shader_to_pipeline;
  Map<u32, u32, Gfx_MaxPipelines> pip_idx_to_entity_batch_idx;
  Map<String, u32, Gfx_MaxShaders> shader_to_module_idx;
  Array<R_GpuMaterial, MaxMaterials> materials;
  Darray<R_EntityPipelineBatch> batches;
  Array<R_RenderEntity, MaxEntities+MaxStaticEntities> entities;
  // Array<Gfx_Mesh, MaxMeshes> meshes;
  Array<VK_Image, MaxTextures> textures;
  u32 static_entities_count;
  u32 static_entities_count_old;

  Pool<Gfx_Mesh, MaxMeshes, GpuMeshId> meshes;

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
  // VK_Buffer vert_buffer;
  // VK_Buffer index_buffer;
  Gfx_Buffer vert_buffer;
  Gfx_Buffer index_buffer;

  Gfx_Buffer vert_buffer_each_frame;
  RingBuffer vert_ring_buffer;
  Array<DebugDrawLine, MaxDebugLines> draw_lines;
  Array<DebugDrawLine, MaxDebugLines> draw_lines_consistent;
  Array<DebugDrawRect, MaxDebugLines> draw_rects;
  u64 draw_lines_offset;
  u64 draw_lines_consistent_offset;
  u64 draw_rects_offset;

  Gfx_Buffer storage_buf;
  R_GlobalStateGPU* gpu_global;
  R_EntityGPU* gpu_entities;
  u32* gpu_entities_indices;
  R_MaterialGPU* gpu_materials;
  VK_Drawcall* gpu_drawcalls;

  Slice<OS_Handle> shader_module_compilation_pids;
  Slice<String> shaders_to_compile;
};

void vk_image_layout_transition(VkCommandBuffer cmd, VK_Image image, VkImageLayout old_layout, VkImageLayout new_layout, VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT);
void vk_image_layout_transition_swapchain(VkCommandBuffer cmd, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout, VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT);
void vk_image_upload_to_gpu(VkCommandBuffer cmd, VK_Image image);
void vk_texture_generate_mipmaps(VK_Image image);

v4& get_pos();
mat4& get_mat();
void r_init();

GpuMeshId r_mesh_load(Mesh mesh);
GpuTextureId r_texture_load(Texture texture);
GpuMaterialId r_material_load(MaterialDesc material);
GpuCubemapId r_cubemap_load(Texture* textures);

void r_shader_reload(String name);

void r_begin();
void r_end();

void r_init();
void r_shutdown();

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

void r_shaders_compile(Allocator arena);

#define IM_VEC2_CLASS_EXTRA                               \
        constexpr ImVec2(const v2& f) : x(f.x), y(f.y) {} \
        operator v2() const { return v2(x,y); }
#include "imgui/imgui.h"


