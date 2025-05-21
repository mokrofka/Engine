#include "shader_sys.h"

#include "render/r_frontend.h"

struct ShaderSysState {
  Arena* arena;
  ShaderSysConfig config;
  HashMap hashmap;
  u32 shader_count;
};

global ShaderSysState st;

void shader_sys_init(Arena* arena, ShaderSysConfig config) {
  st.config = config; 
  st.hashmap = hashmap_create(arena, sizeof(u32), config.shader_count_max);
}

u32 shader_create(ShaderConfig config, void* data, u64 data_size, u64 push_size) {
  hashmap_set(st.hashmap, config.name, &st.shader_count);

  Shader shader;
  shader.name = config.name;
  shader.has_position = config.has_position;
  shader.has_color = config.has_color;
  shader.has_tex_coord = config.has_tex_coord;
  shader.stages = config.stages;
  vk_r_shader_create(&shader, data, data_size, push_size);
  
  return st.shader_count++;
}

u32 shader_get(String name) {
  u32 id;
  hashmap_get(st.hashmap, name, &id);
  return id;
}
