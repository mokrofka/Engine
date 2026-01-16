#pragma once
#include "lib.h"
#include "r_types.h"

u32 vk_mesh_load(Mesh mesh);

void vk_init();
void vk_shutdown();

void vk_begin_frame();
void vk_end_frame();
void vk_begin_renderpass(u32 renderpass_id);
void vk_end_renderpass(u32 renderpass_id);

KAPI u32 vk_shader_load(String name, ShaderType type);
KAPI u32 vk_texture_load(Texture t);

void vk_draw();
void vk_draw_screen();
void vk_draw_compute();

KAPI void vk_update_transform(u32 entity_id, Transform trans);

// Entity
KAPI u32 vk_make_renderable(u32 entity_id, u32 mesh_id, u32 shader_id, u32 texture_id);
KAPI void vk_remove_renderable(u32 entity_id);
KAPI ShaderEntity& vk_get_entity(u32 entity_id);

// Point light
KAPI void vk_point_light_create(u32 entity_id);
KAPI void vk_point_light_remoove(u32 entity_id);
KAPI PointLight& vk_get_point_light_shader(u32 entity_id);

// Directional light
KAPI void vk_dir_light_make(u32 entity_id);
KAPI void vk_dir_light_remove(u32 entity_id);
KAPI DirLight& vk_dir_light_get(u32 entity_id);

// Spot light
KAPI void vk_spot_light_create(u32 entity_id);
KAPI void vk_spot_light_destroy(u32 entity_id);
KAPI SpotLight& vk_spot_light_get(u32 entity_id);

// Util
KAPI PushConstant& vk_get_push_constant(u32 id);
KAPI ShaderGlobalState* vk_get_shader_state();
KAPI void vk_shader_reload(String name, u32 id);


