#pragma once
#include "common.h"

v4& get_pos();
mat4& get_mat();

mat4& vk_get_view();
mat4& vk_get_projection();
void vk_set_state(void* ctx);

GpuTextureH vk_texture_load(Texture texture);
GpuMaterialId vk_material_load(Material material);
GpuCubemapId vk_cubemap_load(Texture* textures);
GpuMeshId vk_mesh_load(Mesh mesh);

void vk_shader_reload(String name);
void vk_shader_compile_and_load_modules();

void vk_begin_draw_frame();
void vk_end_draw_frame();

void* vk_init();
void vk_shutdown();

void vk_make_renderable(EntityId entity_handle, GpuMeshId mesh_handle, GpuMaterialId material_handle);
void vk_make_renderable_static(StaticEntityId entity_handle, GpuMeshId mesh_handle, GpuMaterialId material_handle);
void vk_remove_renderable(EntityId entity_handle);
void vk_set_entity_color(EntityId entity_handle, v4 color);

void vk_draw_line(v3 a, v3 b, v3 color);
void vk_draw_line_consistent(v3 a, v3 b, v3 color);
void vk_draw_cuboid(Rng3f32 rng, v3 color);
void vk_draw_rect(Rng2f32 rect, v3 color);

void vk_imgui_init();
void vk_imgui_begin_frame();
void vk_imgui_end_frame();

Slice<String> vk_shader_compile(Allocator arena);
