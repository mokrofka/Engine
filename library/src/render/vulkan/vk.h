#pragma once
#include "lib.h"
#include "render/r_types.h"

u32 vk_mesh_load(Mesh mesh);

void vk_init();
void vk_shutdown();

void vk_begin_frame();
void vk_end_frame();
void vk_begin_renderpass(u32 renderpass_id);
void vk_end_renderpass(u32 renderpass_id);

KAPI u32 vk_shader_load(String name);
KAPI void vk_texture_load(Texture t);

void vk_draw();
void vk_draw_compute();

// Entity
KAPI u32 entity_make_renderable(u32 mesh_id, u32 shader_id);
KAPI void entity_remove_renderable(u32 entity_id);
KAPI ShaderEntity& shader_get_entity(u32 entity_id);

// Point light
KAPI void entity_make_point_light(u32 entity_id);
KAPI void entity_remove_point_light(u32 entity_id);
KAPI PointLight& shader_get_point_light(u32 entity_id);

// Directional light
KAPI void entity_make_dir_light(u32 entity_id);
KAPI void entity_remove_dir_light(u32 entity_id);
KAPI DirLight& shader_get_dir_light(u32 entity_id);

// Spot light
KAPI void entity_make_spot_light(u32 entity_id);
KAPI void entity_remove_spot_light(u32 entity_id);
KAPI SpotLight& shader_get_spot_light(u32 entity_id);

// Util
KAPI PushConstant& get_push_constant(u32 id);
KAPI ShaderGlobalState* shader_get_global_state();
KAPI void vk_shader_reload(String name, u32 id);

void vk_draw_screen();

KAPI void yes_render();
