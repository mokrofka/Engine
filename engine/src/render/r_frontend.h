#pragma once
#include "lib.h"

#include "r_types.h"

struct R_Config {
  u32 mem_reserve;
};

void r_init(Arena* arena, R_Config config);
void r_shutdown();

void r_on_resized(u32 width, u32 height);

void r_draw_frame(R_Packet* packet);
void r_begin_draw_frame(R_Packet* packet);
void r_end_draw_frame(R_Packet* packet);

// HACK this should not be exposed out the engine
KAPI void r_set_view(mat4 view);

void* r_create_texture(u8* pixels, u32 width, u32 height, u32 channel_count);
void r_destroy_texture(Texture* texture);

void r_create_material(Material* material);
void r_destroy_material(Material* material);

void r_create_geometry(Geometry* geometry);
void r_destroy_geometry(Geometry* geometry);

u32 r_renderpass_id(String name);
void r_shader_create(struct Shader* s, u32 renderpass_id, u32 stage_count, String* stage_filenames, ShaderStage* stages);
void r_shader_destroy(Shader* s);

void r_shader_initialize(struct Shader* shader);
void r_shader_use(struct Shader* shader);
void r_shader_bind_globals(struct Shader* s);
void r_shader_bind_instance(struct Shader* s, u32 instance_id);
void r_shader_apply_globals(struct Shader* s);
void r_shader_apply_instance(struct Shader* s);
void r_shader_acquire_instance_resources(struct Shader* s, u32* out_instance_id);
void r_shader_release_instance_resources(struct Shader* s, u32 instance_id);
void r_set_uniform(struct Shader* frontend_shader, struct shader_uniform* uniform, const void* value);
