#pragma once
#include "render/r_types.h"

void vk_r_backend_init(Arena* arena);
void vk_r_backend_shutdown();

void vk_r_backend_on_resize(u32 width, u32 height);

void vk_r_backend_begin_frame();
void vk_r_backend_end_frame();
void vk_r_begin_renderpass(u32 renderpass_id);
void vk_r_end_renderpass(u32 renderpass_id);

void vk_r_geometry_create(Geometry& geometry);
void vk_r_shader_create(Shader s);
void vk_texture_load(Texture& t);

void vk_draw();
void vk_compute_draw();

v2 vk_get_viewport_size();
b32 vk_is_viewport_render();

////////////////////////////////////////////////////////////////////////
// Entity
KAPI void entity_make_renderable(u32 entity_id, u32 geom_id, u32 shader_id);
KAPI void entity_remove_renderable(u32 entity_id);
KAPI ShaderEntity* shader_get_entity(u32 entity_id);

////////////////////////////////////////////////////////////////////////
// Point light
KAPI void entity_make_point_light(u32 entity_id);
KAPI void entity_remove_point_light(u32 entity_id);
KAPI PointLight* shader_get_point_light(u32 entity_id);

////////////////////////////////////////////////////////////////////////
// Directional light
KAPI void entity_make_dir_light(u32 entity_id);
KAPI void entity_remove_dir_light(u32 entity_id);
KAPI DirLight* shader_get_dir_light(u32 entity_id);

////////////////////////////////////////////////////////////////////////
// Spot light
KAPI void entity_make_spot_light(u32 entity_id);
KAPI void entity_remove_spot_light(u32 entity_id);
KAPI SpotLight* shader_get_spot_light(u32 entity_id);

////////////////////////////////////////////////////////////////////////
// Util
KAPI PushConstant* get_push_constant(u32 id);
KAPI ShaderGlobalState* shader_get_global_state();
KAPI void shader_reload(String name, u32 id);

