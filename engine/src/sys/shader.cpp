#include "shader.h"

#include "render/r_frontend.h"

struct ShaderSysState {
  HashMap hashmap;
  u32 shader_count;
};

global ShaderSysState st;

#define MaxShaderCount KB(1)
void shader_init(Arena* arena) {
  st.hashmap = hashmap_create(arena, sizeof(u32), MaxShaderCount);
  u32 invalid_id = INVALID_ID;
  hashmap_fill(st.hashmap, &invalid_id);
}

u32 shader_create(Shader shader) {
  hashmap_set(st.hashmap, shader.name, &st.shader_count);

  vk_r_shader_create(shader);
  
  return st.shader_count++;
}

u32 shader_get(String name) {
  u32 id;
  hashmap_get(st.hashmap, name, &id);
  Assert(id != INVALID_ID);
  return id;
}
