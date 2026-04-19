#include "common.h"

void* vk_init();
void vk_shutdown();
void vk_hotreload(void* ctx);

Handle<GpuTexture> vk_texture_load(Texture texture);
Handle<GpuMaterial> vk_material_load(Material material);
Handle<GpuCubemap> vk_cubemap_load(Texture* textures);
Handle<GpuMesh> vk_mesh_load(Mesh mesh);

void vk_shader_reload(String name);

void vk_begin_draw_frame();
void vk_end_draw_frame();

void vk_make_renderable(Handle<Entity> entity_handle, Handle<GpuMesh> mesh_handle, Handle<GpuMaterial> material_handle);
void vk_make_renderable_static(Handle<StaticEntity> entity_handle, Handle<GpuMesh> mesh_handle, Handle<GpuMaterial> material_handle);
void vk_remove_renderable(Handle<Entity> entity_handle);
void vk_set_entity_color(Handle<Entity> entity_handle, v4 color);

mat4& vk_get_view();
mat4& vk_get_projection();
void vk_draw_line(v3 a, v3 b, v3 color);
void vk_draw_aabb(v3 min, v3 max, v3 color);
void vk_draw_quad(v2 min, v2 max, v3 color);

void vk_imgui_init();
void vk_imgui_begin_frame();
void vk_imgui_end_frame();

