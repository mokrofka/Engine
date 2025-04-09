#pragma once
#include "render/r_types.h"

void vk_r_backend_init(R_Backend* backend);

void vk_r_backend_shutdown();

void vk_r_backend_on_resize(u16 width, u16 height);

b8 vk_r_backend_begin_frame(f32 delta_time);
void vk_r_update_global_state(mat4 projeection, mat4 view, v3 view_position, v4 ambient_color, i32 mode);
b8 vk_r_backend_end_frame(f32 delta_time);

void vk_r_update_object(GeometryRenderData data);

void vk_r_create_texture(u8* pixels, Texture* texture);
void vk_r_destroy_texture(Texture* texture);

void vk_r_create_material(Material* material);
void vk_r_destroy_material(Material* material);
