#pragma once
#include "types.h"
#include "gfx.h"

///////////////////////////////////
// Gpu memory layout

const u32 R_MaxMaterials  = 32;
const u32 R_MaxLights     = 32;
const u32 R_MaxMeshes     = 32;
const u32 R_MaxTextures   = 32;
const u32 R_MaxCubemaps   = 32;
const u32 R_MaxFonts      = 32;
const u32 R_MaxDebugLines = KB(1);
const u32 R_MaxTextDraws  = 32;
const u32 R_MaxGlyphCharacters = 96;

struct R_ShaderDesc { String name; Gfx_PipelineDesc pipeline_desc; };
u64 hash(R_ShaderDesc x);
b32 equal(R_ShaderDesc a, R_ShaderDesc b);

#include "shader_header.h"

MakeId(R_Texture)
MakeId(R_Mesh)
MakeId(R_Material)
MakeId(R_Font)
MakeId(R_Light)

struct R_Vertex {
  v3 pos;
  v3 norm;
  v2 uv;
  v4 color;
};

struct R_UI_Rect {
  v2 dst_p0;
  v2 dst_p1;
  v2 src_p0;
  v2 src_p1;
  u32 texture;
  v4 color;
  u32 flags;
};

struct R_TextureDesc {
  u32 width;
  u32 height;
  u8* data;
  Gfx_PixelFormat pixel_format;
  b32 is_async;
};

struct R_TextureData {
  u32 width;
  u32 height;
  u8* data;
  Gfx_Image image;
  Gfx_View view;
};

struct R_CubeMapDesc {
  u32 width;
  u32 height;
  u8* cubes[6];
};

struct R_CubeMapData {
  u32 width;
  u32 height;
  u8* cube[6];
  Gfx_Image image;
  Gfx_View view;
};

struct R_MeshDesc {
  Slice<R_Vertex> vertices;
  Slice<u32> indices;
  f32 bounds_min;
  f32 bounds_max;
  f32 bounds_rad;
};

struct R_MeshData {
  Slice<R_Vertex> vertices;
  Slice<u32> indices;
  f32 bounds_min;
  f32 bounds_max;
  f32 bounds_rad;
  Gfx_Mesh mesh;
};

struct R_MaterialProps {
  v3 ambient;
  v3 diffuse;
  v3 specular;
  f32 shininess;
};

struct R_MaterialDesc {
  String shader;
  Gfx_PipelineDesc pipeline_desc;
  R_MaterialProps props;
  String base_color;
};

struct R_MaterialData {
  R_MaterialDesc desc;
  u32 entity_batch_idx;
  R_Texture tex;
  u32 idx;
};

struct R_Glyph {
  Rng2u rect;
  f32 xoff, yoff, xadvance;
};

struct R_FontData {
  u32 font_height;
  R_Texture texture;
  R_Glyph glyphs[R_MaxGlyphCharacters];
};

struct R_DrawCall {
  v3 pos;
  v4 rot;
  v3 scale;
  v4 color;
  R_Mesh mesh;
  R_Material mat;
};

struct R_DrawBatch {
  Gfx_Pipeline pip;
  Darray<R_DrawCall> draws;
  Darray<R_DrawCall> unindexed_draws;
};

struct R_DrawText {
  String str;
  R_Font font;
  v2 pos;
  v4 color;
};

struct R_ShaderModuleWithPipelines {
  Gfx_Shader shd;
  Darray<Gfx_Pipeline> pipelines;
};

struct R_ArenaBuffer {
  Gfx_Buffer buf;
  u64 pos;
  u64 cap;
};

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

typedef u32 R_RenderTargetUsage;
enum {
  R_RenderTargetUsage_Default,
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

struct R_AsyncMesh {
  R_Mesh mesh;
  R_MeshData data;
};

struct R_Camera {
  v3 pos;
  v4 rot;
  f32 fov;
  f32 near_plane;
  f32 far_plane;
  b32 orthographic;
  f32 ortho_size;
};

struct R_State {
  Arena arena;
  Alloc gpa;
  
  f32 scale;
  f32 old_scale;
  
  Array<R_ShaderModuleWithPipelines, Gfx_MaxShaders> shader_modules;
  Map<String, u32, Gfx_MaxShaders> shader_to_module_idx;
  Map<R_ShaderDesc, Gfx_Pipeline, Gfx_MaxPipelines> shader_desc_to_pipeline;
  Map<u32, u32, Gfx_MaxPipelines> pip_idx_to_entity_batch_idx;
  PoolLinkList<R_MaterialData, R_MaxMaterials, R_Material> materials;
  Array<R_DrawBatch, Gfx_MaxShaders> entity_batches;

  Pool<Gfx_Mesh, R_MaxMeshes, R_Mesh> meshes;
  Pool<R_TextureData, R_MaxTextures, R_Texture> textures;
  Pool<R_MeshData, R_MaxMeshes, R_Mesh> new_meshes;
  Pool<R_FontData, R_MaxFonts, R_Font> fonts;

  Gfx_Pipeline triangle_pip;
  Gfx_Pipeline screen_pip;
  Gfx_Pipeline software_render_pip;
  Gfx_Pipeline cubemap_pip;
  Gfx_Pipeline debug_line_pip;
  Gfx_Pipeline ui_pip;
  Gfx_Pipeline ui_rect_pip;
  Gfx_Pipeline font_pip;
  Gfx_Sampler com_sampler;

  R_Font my_font;
  R_Font raylib_font;

  R_RenderTarget world_rt;

  R_ArenaBuffer vert_arena;
  R_ArenaBuffer index_arena;

  Gfx_Buffer vert_buffer_each_frame;
  RingBuffer vert_ring_buffer;
  Array<R_Vertex, R_MaxDebugLines> draw_lines;
  Array<R_Vertex, R_MaxDebugLines> draw_lines_persistent;
  Array<R_UI_Rect, R_MaxDebugLines> draw_rects;
  u64 draw_base_lines;
  u64 draw_base_persistent_lines;

  Queue<R_AsyncMesh, 12> async_mesh;
  Mutex async_stage_mutex;
  R_Texture dummy_texture;
  R_Texture dummy_cubemap;
  R_Texture dummy_mesh;
  R_Texture cur_cubemap;

  v2 point;
  v2 point_dir;

  Gfx_Buffer gpu_state_buf;
  Gfx_Buffer gpu_entities_buf;
  Gfx_Buffer gpu_entities_indices_buf;
  Gfx_Buffer gpu_materials_buf;
  Gfx_Buffer gpu_drawcall_buf;
  Gfx_Buffer gpu_software_render_buf;
  Gfx_Buffer gpu_ui_rect_buf;

  GpuState* gpu_state;
  GpuEntity* gpu_entities;
  u32* gpu_entities_indices;
  GpuMaterial* gpu_materials;
  GpuDrawCall* gpu_drawcall;
  u32* gpu_software_render;
  GpuUI_Rect* gpu_ui_rect;

  Slice<OS_Handle> shader_module_compilation_pids;
  Slice<String> shaders_to_compile;
};

b32 r_texture_is_null(R_Texture tex);
b32 r_mesh_is_null(R_Mesh mesh);
b32 r_material_is_null(R_Mesh m);
b32 r_font_is_null(R_Mesh f);
b32 r_shader_is_null(R_Mesh shd);

R_DrawBatch r_make_draw_batch(Allocator alloc, Gfx_Pipeline pip);
R_ShaderModuleWithPipelines r_make_shader_module_with_pipelines(Allocator alloc);
R_ArenaBuffer r_make_arena_buffer(u64 size, Gfx_MemType type = Gfx_MemType_Gpu);
R_ArenaBuffer r_make_round_arena_buffer(u64 size, u64 round, Gfx_MemType type = Gfx_MemType_Gpu);
u64 r_push_arena_buffer(R_ArenaBuffer* buf, u64 size);
R_Attachment r_make_attachment(R_AttachmentDesc desc);
void r_destroy_attachment(R_Attachment attachment);
void r_recreate_attachment(R_Attachment* attachment, v2u size);
R_RenderTarget r_make_render_target(R_RenderTargetUsage usage, v2u size);
void r_destroy_render_target(R_RenderTarget rt);
void r_recreate_render_target(R_RenderTarget* rt, v2u size);
Gfx_Attachments r_render_target_to_attachments(R_RenderTarget rt);

u32 r_texture_get_descriptor_idx(R_Texture id);
R_TextureDesc r_load_image(String name);
R_Texture r_load_texture(String name);
R_Texture r_load_async_texture(String name);
R_Texture r_make_texture(R_TextureDesc tex);
R_Texture r_make_cubemap(R_CubeMapDesc desc);
R_Texture r_load_cubemap(String dir);
R_Texture r_load_async_cubemap(String dir);
void r_set_cubemap(R_Texture cubemap);
void r_texture_update(R_Texture t, u8* data);
void r_texture_readback(R_Texture t, u8* dst);
void r_texture_destroy(R_Texture t);

R_Mesh r_load_mesh(String name);
R_Mesh r_load_async_mesh(String name);
R_Mesh r_make_mesh(R_MeshDesc desc);
void r_update_mesh(R_Mesh mesh, R_MeshDesc desc);
void r_destroy_mesh(R_Mesh mesh);

// TODO: primitives gen, drawing

R_Material r_material_make(R_MaterialDesc desc);
void r_material_destroy(R_Material mat);

void r_shader_reload(String name);
Gfx_Pipeline r_pipeline_make(String name, Gfx_PipelineDesc desc);
void r_shaders_compile(Allocator arena);
void r_shaders_compile_join();

R_Font r_load_font(String name, u32 size);

void r_init();
void r_shutdown();
void r_begin();
void r_end();

void r_draw_mesh(R_Mesh mesh, R_Material mat, v3 pos);
void r_draw_mesh_trs(R_Mesh mesh, R_Material mat, v3 pos, v4 rot, v3 scale);
void r_draw_entity(ThingId id);

void r_draw_line(v3 a, v3 b, v4 color);
void r_draw_line_persistent(v3 a, v3 b, v4 color);
void r_draw_grid(v3 center, u32 slices, f32 spacing, v4 color);
void r_draw_cuboid(Rng3 rng, v4 color);
void r_draw_rect(Rng2 rect, v4 color);
void r_draw_texture(Rng2 rect, R_Texture tex);
void r_draw_rect_outline(Rng2 rect, u32 thickness, v4 color);
void r_draw_text_ext(R_Font font, v2 pos, String str, v4 color, u32 font_height);

void imgui_init();
void imgui_begin_frame();
void imgui_end_frame();



