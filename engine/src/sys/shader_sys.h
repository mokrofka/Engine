#pragma once
#include "lib.h"

#include "res/res_types.h"

struct ShaderSysConfig {
  u64 mem_reserve;
  u16 shader_count_max;
  u8 uniform_count_max;
  u8 global_textures_max;
  u8 instance_textures_max;
};

enum ShaderState {
  ShaderState_NotCreated,
  ShaderState_Uninitialized,
  ShaderState_Initialized,
};

struct ShaderUniform {
  u64 offset;
  u16 location;
  u16 index;
  u16 size;
  u8 set_index;
  ShaderScope scope;
  ShaderUniformType type;
};

struct ShaderAttribute {
  char* name;
  ShaderAttributeType type;
  u32 size;
};

// struct Shader {
//   u32 id;

//   String64 name64;
//   b8 use_instances;
//   b8 use_locals;

//   u64 required_ubo_alignment;

//   u64 global_ubo_size;
//   u64 global_ubo_stride;
//   u64 global_ubo_offset;

//   u64 ubo_size;
//   u64 ubo_stride;

//   u64 push_constant_size;
//   u64 push_constant_stride;

//   Texture** global_textures;

//   u8 instance_texture_count;

//   ShaderScope bound_scope;

//   u32 bound_instance_id;
//   u32 bound_ubo_offset;

//   HashMap uniform_lookup;

//   ShaderUniform* uniforms;
//   ShaderAttribute* attributes;
//   ShaderState state;

//   u8 push_constant_range_count;
//   MemRange push_constant_ranges[32];
//   u16 attribute_stride;

//   void* internal_data;
// };

struct Shader {
  String name;
  u32 id;
  b8 has_position;
  b8 has_color;
  u8 stages;
};

void shader_sys_init(Arena* arena, ShaderSysConfig config);
void shader_sys_shutdown();

KAPI Shader* shader_sys_create(ShaderConfig* config);
KAPI Shader* shader_create(ShaderConfig config, void* data, void* data_new, u64 data_size);

u32 shader_sys_get_id(char* shader_name);
Shader* shader_sys_get_by_id(u32 shader_id);
Shader* shader_sys_get(const char* shader_name);

void shader_sys_use(const char* shader_name);
void shader_sys_use_by_id(u32 shader_id);

u16 shader_sys_uniform_index(Shader* s, const char* uniform_name);

void shader_sys_uniform_set(const char* uniform_name, const void* value);
void shader_sys_sampler_set(const char* sampler_name, const Texture* t);
void shader_sys_uniform_set_by_index(u16 index, const void* value);
void shader_sys_sampler_set_by_index(u16 index, const struct texture* t);

void shader_sys_apply_global();
void shader_sys_apply_instance();

void shader_sys_bind_instance(u32 instance_id);
