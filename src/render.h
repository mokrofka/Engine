
const u32 R_MaxMaterials  = 32;
const u32 R_MaxLights     = 32;
const u32 R_MaxMeshes     = 32;
const u32 R_MaxTextures   = 32;
const u32 R_MaxFonts      = 32;
const u32 R_MaxDebugLines = KB(1);

struct R_KeyToShaderPipeline { String name; Gfx_PipelineDesc pipeline_desc; };
u64 hash(R_KeyToShaderPipeline x);
b32 equal(R_KeyToShaderPipeline a, R_KeyToShaderPipeline b);

#include "vk_bindings.h"

MakeId(R_Texture)
MakeId(R_Mesh)
MakeId(R_Material)
MakeId(R_Font)
MakeId(R_Shader)
MakeId(R_Light)

b32 r_texture_is_null(R_Texture tex);
b32 r_mesh_is_null(R_Mesh mesh);
b32 r_material_is_null(R_Mesh m);
b32 r_font_is_null(R_Mesh f);
b32 r_shader_is_null(R_Mesh shd);

struct MaterialProps {
  v3 ambient;
  v3 diffuse;
  v3 specular;
  f32 shininess;
};

struct Vertex {
  v3 pos;
  v3 norm;
  v2 uv;
  v4 color;
};

struct Texture {
  u32 width;
  u32 height;
  u8* data;
};

struct MeshDesc {
  Slice<Vertex> vertices;
  Slice<u32> indices;
  f32 bounds_min;
  f32 bounds_max;
  f32 bounds_rad;
};

struct MaterialDesc {
  String shader_name;
  Gfx_PipelineDesc pipeline_desc;
  MaterialProps props;
  String texture;
};

struct R_MaterialData {
  MaterialDesc desc;
  u32 entity_batch_idx;
  u32 texture_descriptor_idx;
};

struct R_TextureData {
  u32 width;
  u32 height;
  union {
    u8* data;
    u8* cube[6];
  };
  Gfx_Image image;
  Gfx_View view;
};

struct R_MeshData {
  Slice<Vertex> vertices;
  Slice<u32> indices;
  f32 bounds_min;
  f32 bounds_max;
  f32 bounds_rad;
  Gfx_Mesh mesh;
};

const u32 MaxGlyphCharacters = 96;

struct R_Glyph {
  i32 x0,y0,x1,y1;
  f32 xoff, yoff,xadvance;
};

struct R_FontData {
  R_Texture texture;
  R_Glyph glyphs[MaxGlyphCharacters];
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
  R_Mesh mesh;
  R_Material material;
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

struct R_AsyncMesh {
  R_Mesh mesh;
  R_MeshData data;
};

struct R_State {
  Arena arena;
  AllocSegList gpa;
  
  f32 scale;
  f32 old_scale;
  
  Array<R_ShaderModuleEntry, Gfx_MaxShaders> modules;
  Map<String, u32, Gfx_MaxShaders> shader_to_module_idx;
  Map<R_KeyToShaderPipeline, Gfx_Pipeline, Gfx_MaxPipelines> shader_to_pipeline;
  Pool<R_MaterialData, R_MaxMaterials, R_Material> materials;
  Array<R_EntityBatch, Gfx_MaxShaders> entity_batches;
  Map<u32, u32, Gfx_MaxPipelines> pip_idx_to_entity_batch_idx;

  Pool<Gfx_Mesh, R_MaxMeshes, R_Mesh> meshes;
  Pool<R_TextureData, R_MaxTextures, R_Texture> textures;
  Pool<R_MeshData, R_MaxMeshes, R_Mesh> new_meshes;
  Pool<R_FontData, R_MaxFonts, R_Font> fonts;

  

  Gfx_Pipeline triangle_pip;
  Gfx_Pipeline screen_pip;
  Gfx_Pipeline cubemap_pip;
  Gfx_Pipeline debug_line_pip;
  Gfx_Pipeline ui_pip;
  Gfx_Pipeline font_pip;
  Gfx_Sampler com_sampler;

  R_Font my_font;

  R_RenderTarget world_rt;

  R_ArenaBuffer vert_arena;
  R_ArenaBuffer index_arena;

  Gfx_Buffer vert_buffer_each_frame;
  RingBuffer vert_ring_buffer;
  Array<Vertex, R_MaxDebugLines> draw_lines;
  Array<Vertex, R_MaxDebugLines> draw_lines_persistent;
  Array<Vertex, R_MaxDebugLines> draw_rects;
  Array<Vertex, R_MaxDebugLines> draw_rects_texture;
  u64 draw_base_lines;
  u64 draw_base_persistent_lines;
  u64 draw_base_rects;
  u64 draw_base_rects_texture;

  Queue<R_AsyncMesh, 12> async_mesh;

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

u32 r_texture_get_descriptor_idx(R_Texture id);
Texture r_image_load(String name);
R_Texture r_texture_load(String name);
R_Texture r_texture_make(Texture tex);
R_Texture r_texture_cube_load(String dir);
void r_texture_update(R_Texture t, u8* data);
void r_texture_readback(R_Texture t, u8* dst);
void r_texture_destroy(R_Texture t);

R_Mesh r_mesh_load(String name);
R_Mesh r_mesh_load_async(String name);
R_Mesh r_mesh_make(MeshDesc desc);
void r_mesh_update(R_Mesh mesh, MeshDesc desc);
void r_mesh_destroy(R_Mesh mesh);

// TODO: primitives gen, drawing

R_Material r_material_make(MaterialDesc desc);
void r_material_destroy(R_Material mat);

void r_shader_reload(String name);
Gfx_Pipeline r_pipeline_make(String name, Gfx_PipelineDesc desc);
void r_shaders_compile(Allocator arena);
void r_shaders_compile_join();

R_Font r_font_load(String path, f32 size);

void r_init();
void r_shutdown();
void r_begin();
void r_end();

void r_set_entity_color(EntityId entity_handle, v4 color);
void r_push_mesh(EntityId id, R_Mesh mesh, R_Material material);

void r_debug_line(v3 a, v3 b, v4 color);
void r_debug_line_persistent(v3 a, v3 b, v4 color);
void r_debug_grid(v3 center, u32 slices, f32 spacing, v4 color);
void r_debug_cuboid(Rng3 rng, v4 color);
void r_draw_rect(Rng2 rect, v4 color);

void imgui_init();
void imgui_begin_frame();
void imgui_end_frame();


#define IM_VEC2_CLASS_EXTRA                               \
        constexpr ImVec2(const v2& f) : x(f.x), y(f.y) {} \
        operator v2() const { return v2(x,y); }
#include "imgui/imgui.h"


