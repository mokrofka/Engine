
const u32 MaxMaterials  = 32;
const u32 MaxLights     = 32;
const u32 MaxMeshes     = 32;
const u32 MaxTextures   = 32;
const u32 MaxDebugLines = KB(1);

struct R_KeyToShaderPipeline { String name; Gfx_PipelineDesc pipeline_desc; };
u64 hash(R_KeyToShaderPipeline x);
b32 equal(R_KeyToShaderPipeline a, R_KeyToShaderPipeline b);

#include "vk_bindings.h"

struct MaterialDesc {
  String shader_name;
  Gfx_PipelineDesc pipeline_desc;
  MaterialProps props;
  String texture;
};

struct R_Material {
  MaterialDesc desc;
  u32 entity_batch_idx;
  u32 texture_descriptor_idx;
};

struct R_Texture {
  u32 width;
  u32 height;
  union {
    u8* data;
    u8* cube[6];
  };
  Gfx_Image image;
  Gfx_View view;
};

struct R_Mesh {
  Slice<Vertex> vertices;
  Slice<u32> indices;
  f32 bounds_min;
  f32 bounds_max;
  f32 bounds_rad;
  Gfx_Mesh mesh;
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
  u32 entity_indices[MaxEntities];
};

struct R_MeshPush {
  EntityId id;
  MeshId mesh;
  MaterialId material;
};

struct R_EntityBatch {
  Gfx_Pipeline pip;
  Darray<R_MeshPush> pushed_meshes;
  Darray<R_MeshPush> pushed_meshes_unindexed;
};

R_EntityBatch r_entity_batch_make(Allocator alloc, Gfx_Pipeline pip);

struct R_ShaderModuleEntry {
  Gfx_Shader shd;
  Darray<Gfx_Pipeline> track_pipelines;
};

R_ShaderModuleEntry r_shader_module_entry_make(Allocator alloc);

struct R_ArenaBuffer {
  Gfx_Buffer buf;
  u64 pos;
  u64 cap;
};

R_ArenaBuffer r_arena_buffer_make(u64 size, Gfx_MemType type = Gfx_MemType_Gpu);
R_ArenaBuffer r_arena_buffer_make_round(u64 size, u64 round, Gfx_MemType type = Gfx_MemType_Gpu);
u64 r_arena_buffer_push(R_ArenaBuffer* buf, u64 size);

enum R_AttachmentType {
  R_AttachmentType_Default,
  R_AttachmentType_Color,
  R_AttachmentType_Resolve,
  R_AttachmentType_Depth,
};

struct R_Attachment {
  R_AttachmentType type;
  Gfx_Image images[Gfx_MaxImagesInFlight];
  Gfx_View views[Gfx_MaxImagesInFlight];
};

struct R_AttachmentDesc {
  R_AttachmentType type;
  v2u size;
};

R_Attachment r_attachment_make(R_AttachmentDesc desc);
void r_attachment_destroy(R_Attachment attachment);
void r_attachment_recreate(R_Attachment* attachment, v2u size);

typedef u32 R_RenderTargetUsage;
enum {
  R_RenderTargetUsage_Default = 0,
  R_RenderTargetUsage_Color = Bit(0),
  R_RenderTargetUsage_Resolve = Bit(1),
  R_RenderTargetUsage_Depth = Bit(2),
};

struct R_RenderTarget {
  R_RenderTargetUsage attachments;
  R_Attachment color;
  R_Attachment resolve;
  R_Attachment depth;
};

R_RenderTarget r_render_target_make(R_RenderTargetUsage usage, v2u size);
void r_render_target_destroy(R_RenderTarget rt);
void r_render_target_recreate(R_RenderTarget* rt, v2u size);
Gfx_Attachments r_render_target_to_attachments(R_RenderTarget rt);

struct R_State {
  Arena arena;
  AllocSegList gpa;
  
  f32 scale;
  f32 old_scale;
  
  Array<R_ShaderModuleEntry, Gfx_MaxShaders> modules;
  Map<String, u32, Gfx_MaxShaders> shader_to_module_idx;
  Map<R_KeyToShaderPipeline, Gfx_Pipeline, Gfx_MaxPipelines> shader_to_pipeline;
  Pool<R_Material, MaxMaterials, MaterialId> materials;
  Array<R_EntityBatch, Gfx_MaxShaders> entity_batches;
  Map<u32, u32, Gfx_MaxPipelines> pip_idx_to_entity_batch_idx;

  Pool<Gfx_Mesh, MaxMeshes, MeshId> meshes;
  Pool<R_Texture, MaxTextures, TextureId> textures;

  Gfx_Pipeline triangle_pip;
  Gfx_Pipeline screen_pip;
  Gfx_Pipeline cubemap_pip;
  Gfx_Pipeline debug_line_pip;
  Gfx_Pipeline ui_pip;
  Gfx_Sampler com_sampler;

  R_RenderTarget world_rt;

  R_ArenaBuffer vert_arena;
  R_ArenaBuffer index_arena;

  Gfx_Buffer vert_buffer_each_frame;
  RingBuffer vert_ring_buffer;
  Array<DebugDrawLine, MaxDebugLines> draw_lines;
  Array<DebugDrawLine, MaxDebugLines> draw_lines_consistent;
  Array<DebugDrawRect, MaxDebugLines> draw_rects;
  u64 draw_base_lines;
  u64 draw_base_consistent_lines;
  u64 draw_base_rects;

  Gfx_Buffer gpu_global_buf;
  Gfx_Buffer gpu_entities_buf;
  Gfx_Buffer gpu_entities_indices_buf;
  Gfx_Buffer gpu_materials_buf;
  R_GlobalStateGPU* gpu_global;
  R_EntityGPU* gpu_entities;
  u32* gpu_entities_indices;
  R_MaterialGPU* gpu_materials;

  Slice<OS_Handle> shader_module_compilation_pids;
  Slice<String> shaders_to_compile;
};

v4& get_pos();
mat4& get_mat();
void r_init();

MeshId r_mesh_load(MeshDesc mesh);
TextureId r_texture_load(TextureDesc texture);
MaterialId r_material_make(MaterialDesc material);
void r_cubemap_load(TextureDesc texture);

void r_shader_reload(String name);

void r_begin();
void r_end();

void r_init();
void r_shutdown();

void r_set_entity_color(EntityId entity_handle, v4 color);

void r_push_mesh(EntityId id, MeshId mesh, MaterialId material);

void r_debug_line(v3 a, v3 b, v4 color);
void r_debug_line_persistent(v3 a, v3 b, v4 color);
void r_debug_cuboid(Rng3 rng, v4 color);
void r_draw_rect(Rng2 rect, v4 color);

void imgui_init();
void imgui_begin_frame();
void imgui_end_frame();

void r_shaders_compile(Allocator arena);

#define IM_VEC2_CLASS_EXTRA                               \
        constexpr ImVec2(const v2& f) : x(f.x), y(f.y) {} \
        operator v2() const { return v2(x,y); }
#include "imgui/imgui.h"


