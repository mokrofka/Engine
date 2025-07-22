#include "shader.h"

#include "asset_watch.h"
#include "res.h"
#include "render/r_frontend.h"

struct ShaderSysState {
  Arena* arena;
  HashMap hashmap;
  u32 shader_count;
  u32 screen_shader_count;
  u32 compute_shader_count;
};

global ShaderSysState st;

#define MaxShaderCount 256
void shader_init() {
  st.arena = mem_arena_alloc(KB(100));
  st.hashmap = hashmap_create(st.arena, sizeof(Shader), MaxShaderCount);
  Shader shader = {
    .id = INVALID_ID,
  };
  hashmap_fill(st.hashmap, &shader);

  asset_watch_directory_add("shaders", [](String name) {
    Scratch scratch;
    String shader_dir = push_str_cat(scratch, res_sys_base_path(), "/shaders");
    String shader_filepath = push_strf(scratch, "%s/%s", shader_dir, name);
    String shader_compiled_dir = push_str_cat(scratch, shader_dir, "/compiled");
    String shader_compiled_filepath = push_strf(scratch, "%s/%s%s", shader_compiled_dir, name, String(".spv"));

    String cmd = push_strf(scratch, "glslangValidator.exe -V \"%s\" -o \"%s\"", shader_filepath, shader_compiled_filepath);
    os_process_create(cmd);
  });
  
  asset_watch_directory_add("shaders/compiled", [](String name) {
    Scratch scratch;
    String choped = str_chop_last_dot(name);
    String choped_choped = str_chop_last_dot(choped);
    shader_reload(choped_choped, shader_get(choped_choped).id);
  });
}

void shader_create(Shader shader) {
  if (shader.type == ShaderType_Gfx) {
    shader.id = st.shader_count++;
    hashmap_set(st.hashmap, shader.name, &shader);
  } else if (shader.type == ShaderType_Screen) {
    shader.id = st.screen_shader_count++;
    hashmap_set(st.hashmap, shader.name, &shader);
  } else if (shader.type == ShaderType_Compute) {
    shader.id = st.compute_shader_count++;
    hashmap_set(st.hashmap, shader.name, &shader);
  }

  vk_r_shader_create(shader);
}

Shader& shader_get(String name) {
  Shader* shader; Assign(shader, hashmap_get(st.hashmap, name));
  Assert(shader->id != INVALID_ID);
  return *shader;
}
