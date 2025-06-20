#pragma once
#include "render/r_types.h"

void vk_r_backend_init(Arena* arena);
void vk_r_backend_shutdown();

void vk_r_backend_on_resize(u32 width, u32 height);

void vk_r_backend_begin_frame();
void vk_r_backend_end_frame();
void vk_r_begin_renderpass(u32 renderpass_id);
void vk_r_end_renderpass(u32 renderpass_id);

void vk_r_geometry_create(Geometry* geometry);
void vk_r_destroy_geometry(Geometry* geometry);
void vk_r_shader_create(struct Shader* s);
void vk_texture_load(Texture* texture);

void vk_draw();
void vk_compute_draw();

KAPI void vk_make_renderable(u32 id, u32 geom_id, u32 shader_id);
KAPI void vk_remove_renderable(u32 entity_id);
KAPI void vk_make_light(u32 entity_id);
KAPI void vk_remove_light(u32 entity_id);

v2 vk_get_viewport_size();
b32 vk_is_viewport_render();

KAPI PushConstant* vk_get_push_constant(u32 id);
KAPI ShaderGlobalState* shader_get_global_state();
KAPI ShaderEntity* shader_get_entity_data(u32 entity_id);
KAPI DirectionalLight* shader_get_light_data(u32 entity_id);
