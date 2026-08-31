#pragma once
#include "types.h"
#include "gfx.h"

const u32 R_MaxMaterials  = 32;
const u32 R_MaxLights     = 32;
const u32 R_MaxMeshes     = 32;
const u32 R_MaxTextures   = 32;
const u32 R_MaxCubemaps   = 32;
const u32 R_MaxFonts      = 32;
const u32 R_MaxDebugLines = KB(1);
#define _Matrix4x3 m4x3

#include "shader_header.h"

MakeId(R_TextureId)
MakeId(R_MeshId)
MakeId(R_MaterialId)
MakeId(R_FontId)
MakeId(R_LightId)

struct R_UI_Rect {
  v2 dst_p0;
  v2 dst_p1;
  v2 src_p0;
  v2 src_p1;
  v4 colors[4];
  u32 texture;
  u32 flags;
  f32 corner_radius;
  f32 edge_softness;
};

struct Image {
  u32 width;
  u32 height;
  u8* data;
  Gfx_PixelFormat format;
};

struct R_TextureDesc {
  u32 width;
  u32 height;
  u8* data;
  Gfx_PixelFormat pixel_format;
  b32 is_async;
};

struct R_Texture {
  u32 width;
  u32 height;
  Gfx_Image image;
  Gfx_View view;
};

struct R_CubeMapDesc {
  u32 width;
  u32 height;
  u8* cubes[6];
};

struct R_MeshDesc {
  Slice<R_Vertex> vertices;
  Slice<u32> indices;
  f32 bounds_min;
  f32 bounds_max;
  f32 bounds_rad;
};

struct R_MeshData {
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
  ShaderType type;
  Gfx_PipelineDesc pipeline_desc;
  R_MaterialProps props;
  String base_color;
};

struct R_Material {
  ShaderType type;
  u32 batch;
  R_MaterialProps props;
  R_TextureId base_color;
  u32 idx;
};

// struct R_Material {
//   R_MaterialDesc desc;
//   R_MaterialDesc2 desc2;
//   u32 batch;
//   R_TextureId tex;
//   u32 idx;
// };

struct R_Gradient {
  v4 color0;
  v4 color1;
  v4 color2;
  v4 color3;
};

struct R_Glyph {
  Rng2u rect;
  f32 xoff, yoff, xadvance;
};

struct R_FontData {
  u32 font_height;
  R_TextureId texture;
  R_Glyph glyphs[96];
};

struct R_DrawCall {
  v3 pos;
  v4 rot;
  v3 scale;
  v4 color;
  R_MeshId mesh;
  R_MaterialId mat;
  ShaderType type;
};

struct R_DrawBatch {
  Gfx_PipelineState state;
  Darray<R_DrawCall> draws;
  Darray<R_DrawCall> unindexed_draws;
};

struct R_DrawText {
  String str;
  R_FontId font;
  v2 pos;
  v4 color;
};

struct R_ShaderModuleWithPipelines {
  Gfx_Shader shd;
  Array<Gfx_Pipeline, 4> pipelines;
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
  R_MeshId mesh;
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
  PoolLinkList<R_Material, R_MaxMaterials, R_MaterialId> materials;
  Array<R_DrawBatch, 8> batches;

  Pool<Gfx_Mesh, R_MaxMeshes, R_MeshId> meshes;
  Pool<R_Texture, R_MaxTextures, R_TextureId> textures;
  Pool<R_MeshData, R_MaxMeshes, R_MeshId> new_meshes;
  Pool<R_FontData, R_MaxFonts, R_FontId> fonts;

  Gfx_Pipeline uber_pip;
  Gfx_Pipeline uber_pip_screen;
  Gfx_Pipeline ui_rect_pip;
  Gfx_Pipeline prev_pip;
  Gfx_PipelineState prev_pip_state;
  Gfx_Sampler com_sampler;

  R_FontId my_font;
  R_FontId raylib_font;

  R_RenderTarget world_rt;

  Gfx_Buffer cpu_vert_reg;
  R_Vertex* cpu_vertices;
  Gfx_Buffer vert_reg;
  Gfx_Buffer index_reg;

  Array<R_Vertex, R_MaxDebugLines> draw_lines;
  Array<R_Vertex, R_MaxDebugLines> draw_lines_persistent;
  Array<R_UI_Rect, R_MaxDebugLines> draw_rects;

  Queue<R_AsyncMesh, 12> async_mesh;
  Mutex async_stage_mutex;
  R_TextureId dummy_texture;
  R_TextureId dummy_cubemap;
  R_TextureId dummy_mesh;
  R_TextureId cur_cubemap;

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
  GpuDrawCall* gpu_drawcalls;
  u32* gpu_software_render;
  GpuUI_Rect* gpu_ui_rects;

  Slice<OS_Handle> shader_module_compilation_pids;
  Slice<String> shaders_to_compile;
};

R_DrawBatch r_make_draw_batch(Allocator alloc, Gfx_Pipeline pip);
R_Attachment r_make_attachment(R_AttachmentDesc desc);
void r_destroy_attachment(R_Attachment attachment);
void r_recreate_attachment(R_Attachment* attachment, v2u size);
R_RenderTarget r_make_render_target(R_RenderTargetUsage usage, v2u size);
void r_destroy_render_target(R_RenderTarget rt);
void r_recreate_render_target(R_RenderTarget* rt, v2u size);
Gfx_Attachments r_render_target_to_attachments(R_RenderTarget rt);

u32 r_texture_get_descriptor_idx(R_TextureId id);
R_TextureDesc r_load_image(String name);
R_TextureId r_load_texture(String name);
R_TextureId r_load_async_texture(String name);
R_TextureId r_make_texture(R_TextureDesc tex);
R_TextureId r_make_cubemap(R_CubeMapDesc desc);
R_TextureId r_load_cubemap(String dir);
R_TextureId r_load_async_cubemap(String dir);
void r_set_cubemap(R_TextureId cubemap);
void r_texture_update(R_TextureId t, u8* data);
void r_texture_readback(R_TextureId t, u8* dst);
void r_texture_destroy(R_TextureId t);

R_MeshId r_load_mesh(String name);
R_MeshId r_load_async_mesh(String name);
R_MeshId r_make_mesh(R_MeshDesc desc);
void r_update_mesh(R_MeshId mesh, R_MeshDesc desc);
void r_destroy_mesh(R_MeshId mesh);

// TODO: primitives gen, drawing

u32 r_make_pipeline_state(Gfx_PipelineState s);
R_MaterialId r_material_make(R_Material mat);

void r_material_destroy(R_MaterialId mat);

void r_shader_reload(String name);
Gfx_Pipeline r_make_pipeline(String name, Gfx_PipelineDesc desc);
Gfx_Pipeline r_make_pipeline2(String name, Gfx_PipelineDesc2 desc);
void r_shaders_compile(Allocator arena);
void r_shaders_compile_join();

R_FontId r_load_font(String name, u32 size);
void r_apply_state(Gfx_PipelineState s);

void r_init();
void r_shutdown();
void r_begin();
void r_end();

void r_draw_mesh(R_MeshId mesh, R_MaterialId mat, v3 pos);
void r_draw_mesh_trs(R_MeshId mesh, R_MaterialId mat, v3 pos, v4 rot, v3 scale);
void r_draw_entity(ThingId id);

void r_draw_line(v3 a, v3 b, v4 color);
void r_draw_line_persistent(v3 a, v3 b, v4 color);
void r_draw_grid(v3 center, u32 slices, f32 spacing, v4 color);
void r_draw_cuboid(Rng3 rng, v4 color);
void r_draw_rect(Rng2 rect, v4 color);
void r_draw_rect_rounded(Rng2 rect, v4 color, f32 corner_radius, f32 edge_softness);
void r_draw_rect_gradient(Rng2 rect, R_Gradient grad);
void r_draw_texture(Rng2 rect, R_TextureId tex);
void r_draw_rect_outline(Rng2 rect, u32 thickness, v4 color);
void r_draw_text_ext(R_FontId font, v2 pos, String str, v4 color, u32 font_height);

void imgui_init();
void imgui_begin_frame();
void imgui_end_frame();



