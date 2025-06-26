#include "shader.h"

#include "asset_watch.h"
#include "render/r_frontend.h"
#include "res.h"

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
    String c = push_str_copy(scratch, choped);
    shader_reload(choped_choped, shader_get(choped_choped));
  });
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
