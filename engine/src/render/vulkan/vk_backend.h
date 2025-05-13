#pragma once
#include "render/r_types.h"

void vk_r_backend_init(R_Backend* backend);

void vk_r_backend_shutdown();

void vk_r_backend_on_resize(u32 width, u32 height);

b32 vk_r_backend_begin_frame(f32 delta_time);
void vk_r_update_global_world_state(mat4 projection, mat4 view, v3 view_position, v4 ambient_color, i32 mode);
void vk_r_update_global_ui_state(mat4 projection, mat4 view, i32 mode);
b32 vk_r_backend_end_frame(f32 delta_time);

b32 vk_r_begin_renderpass(u32 renderpass_id);
b32 vk_r_end_renderpass(u32 renderpass_id);

void vk_r_draw_geometry(GeometryRenderData data);

void* vk_r_create_texture(u8* pixels, u32 width, u32 height, u32 channel_count);
void vk_r_destroy_texture(Texture* texture);

void vk_r_create_material(Material* material);
void vk_r_destroy_material(Material* material);

void vk_r_create_geometry(Geometry* geometry);
void vk_r_destroy_geometry(Geometry* geometry);

// Shader
void vk_r_shader_create(struct Shader* s, void* data, void* data_new, u64 size);
void vk_draw();
void vk_make_renderable(u32 id, u32 geom_id, u32 shader_id);
// void vk_r_shader_destroy(struct Shader* shader);

// void vk_r_shader_initialize(struct Shader* shader);
// void vk_r_shader_use(struct Shader* shader);
// void vk_r_shader_bind_globals(struct Shader* s);
// void vk_r_shader_bind_instance(struct Shader* s, u32 instance_id);
// void vk_r_shader_apply_globals(struct Shader* s);
// void vk_r_shader_apply_instance(struct Shader* s);
// void vk_r_shader_acquire_instance_resources(struct Shader* s, u32* out_instance_id);
// void vk_r_shader_release_instance_resources(struct Shader* s, u32 instance_id);
// void vk_r_set_uniform(struct Shader* frontend_shader, struct ShaderUniform* uniform, const void* value);
