#pragma once
#include "common.h"

v4& get_pos();
mat4& get_mat();

mat4& vk_get_view();
mat4& vk_get_projection();
void vk_set_state(void* vk);

Handle<GpuTexture> vk_texture_load(Texture texture);
Handle<GpuMaterial> vk_material_load(Material material);
Handle<GpuCubemap> vk_cubemap_load(Texture* textures);
Handle<GpuMesh> vk_mesh_load(Mesh mesh);

void vk_shader_reload(String name);
void vk_compile_and_load_shaders();

void vk_begin_draw_frame();
void vk_end_draw_frame();

void* vk_init();
void vk_shutdown();

void vk_make_renderable(Handle<Entity> entity_handle, Handle<GpuMesh> mesh_handle, Handle<GpuMaterial> material_handle);
void vk_make_renderable_static(Handle<StaticEntity> entity_handle, Handle<GpuMesh> mesh_handle, Handle<GpuMaterial> material_handle);
void vk_remove_renderable(Handle<Entity> entity_handle);
void vk_set_entity_color(Handle<Entity> entity_handle, v4 color);

void vk_draw_line(v3 a, v3 b, v3 color);
void vk_draw_line_consistent(v3 a, v3 b, v3 color);
void vk_draw_cuboid(Rng3f32 rng, v3 color);
void vk_draw_rect(Rng2f32 rect, v3 color);

void vk_imgui_init();
void vk_imgui_begin_frame();
void vk_imgui_end_frame();

