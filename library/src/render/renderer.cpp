#include "renderer.h"

#include "asset_watch.h"

#include "entity.h"
#include "ui.h"

struct RendererSystemState {
  b8 is_render;
};

global RendererSystemState st;

void r_init() {
  shader_init();
  texture_init();
  mesh_init();

  st.is_render = true;
  
  vk_init();
  
  // ui_init();
}

void r_shutdown() {
  vk_shutdown();
}

void r_begin_draw_frame() {
  vk_begin_frame();

  // World
  // if (vk_is_viewport_render()) {
    {
      vk_begin_renderpass(Renderpass_World);
      vk_draw();
      vk_end_renderpass(Renderpass_World);
    }
  // }

  // Begin UI
  {
    // vk_r_begin_renderpass(Renderpass_UI);
    // ui_begin_frame();
  }

}

void r_end_draw_frame() {
  // End UI
  {
    // ui_end_frame();
    // vk_r_end_renderpass(Renderpass_UI);
  }

  {
    vk_begin_renderpass(Renderpass_Screen);
    vk_draw_screen();
    vk_end_renderpass(Renderpass_Screen);
  }

  vk_end_frame();
}

////////////////////////////////////////////////////////////////////////
// Geometry
struct MeshState {
  Arena* arena;
  u32 geom_count;
};

global MeshState mesh_st;

intern Mesh load_obj(String name) {
  Scratch scratch;

  DarrayArena<v3> vertices(scratch);
  DarrayArena<u32> indexes(scratch);
  u32 vert_count = 0;
  u32 index_count = 0;

  Buffer buff = res_binary_load(scratch, name);
  Range range = {.offset = (u64)buff.data, .size = buff.size + (u64)buff.data};

  String line;
  while ((line = str_read_line(&range))) {
    if (line.str[0] == 'v' && line.str[1] == ' ') { // vertex
      ++vert_count;
      u32 start = 2;
      v3 v = {
        f32_from_str(str_next_word(line, start)),
        f32_from_str(str_next_word(line, start)),
        f32_from_str(str_next_word(line, start)),
      };
      append(vertices, v);
    } else if (line.str[0] == 'f') {
      ++index_count;
      Loop (i, line.size) {
        if (line.str[i] == ' ') {
          String start_on_num = str_skip(line, i+1);
          i32 num_length = str_index_of(start_on_num, '/');
          String num = str_prefix(start_on_num, num_length);
          append(indexes, u32_from_str(num));
        }
      }
    }
  }

  Mesh mesh = {
    .vertices = push_array(mesh_st.arena, Vertex, vert_count),
    .indexes = push_array(mesh_st.arena, u32, index_count),
    .vert_count = vert_count,
    .index_count = index_count,
  };
  Loop (i, vert_count) {
    mesh.vertices[i] = {
      .pos = vertices[i],
    };
  }
  MemCopyTyped(mesh.indexes, indexes.data, index_count);

  push_buffer(scratch, 10, 4);

  return mesh;
}

void mesh_init() {
  mesh_st.arena = arena_alloc();
}

u32 mesh_create(String name) {
  Mesh mesh = load_obj(name);
  u32 id = vk_mesh_load(mesh);
  return id;
}

Mesh geometry_get(String name) {

}

////////////////////////////////////////////////////////////////////////
// Resource
struct ResSysState {
  Arena* arena;
  String asset_path;
};

global ResSysState res_st;

void res_sys_init(String asset_path) {
  res_st.arena = arena_alloc();
  res_st.asset_path = push_strf(res_st.arena, "%s/%s", os_get_current_directory(), asset_path);
}

String res_sys_base_path() {
  return res_st.asset_path;
}

Buffer res_binary_load(Arena* arena, String name) {
  Scratch scratch(&arena);
  String filepath = push_strf(scratch, "%s/%s", res_sys_base_path(), name);

  OS_Handle f = os_file_open(filepath, OS_AccessFlag_Read);
  if (!f) {
    Error("res_binary_load - unable to open file: '%s'", filepath);
    os_file_close(f);
    return {};
  }

  u64 file_size = os_file_size(f);
  u8* buffer = push_buffer(arena, file_size);
  u64 read_size = os_file_read(f, file_size, buffer);
  if (read_size == 0) {
    Error("Unable to binary read file: %s", filepath);
    os_file_close(f);
    return {};
  }

  Buffer result = {
    .data = buffer,
    .size = file_size,
  };

  os_file_close(f);
  return result;
}

////////////////////////////////////////////////////////////////////////
// Shader
struct ShaderSysState {
  Arena* arena;
  // HashMap hashmap;
  u32 shader_count;
  u32 screen_shader_count;
  u32 compute_shader_count;
};

global ShaderSysState shader_st;

#define MaxShaderCount 256
void shader_init() {
  shader_st.arena = arena_alloc();
  u8* buff = push_buffer(shader_st.arena, MB(4));
  MemSet(buff, 0, 1);
  // shader_st.hashmap = hashmap_create(shader_st.arena, sizeof(Shader), MaxShaderCount);
  Shader shader = {
    .id = INVALID_ID,
  };
  // hashmap_fill(shader_st.hashmap, &shader);

  asset_watch_directory_add("shaders", [](String name) {
    Scratch scratch;
    String shader_dir = push_str_cat(scratch, res_sys_base_path(), "/shaders");
    String shader_filepath = push_strf(scratch, "%s/%s", shader_dir, name);
    String shader_compiled_dir = push_str_cat(scratch, shader_dir, "/compiled");
    String shader_compiled_filepath = push_strf(scratch, "%s/%s%s", shader_compiled_dir, name, String(".spv"));

    String cmd = push_strf(scratch, "glslangValidator.exe -V \"%s\" -o \"%s\"", shader_filepath, shader_compiled_filepath);
    // os_process_create(cmd);
  });
  
  asset_watch_directory_add("shaders/compiled", [](String name) {
    Scratch scratch;
    String choped = str_chop_last_dot(name);
    String choped_choped = str_chop_last_dot(choped);
    vk_shader_reload(choped_choped, shader_get(choped_choped).id);
  });
}

void shader_create(Shader shader) {
  if (shader.type == ShaderType_Gfx) {
    shader.id = shader_st.shader_count++;
    // hashmap_set(shader_st.hashmap, shader.name, &shader);
  } else if (shader.type == ShaderType_Screen) {
    shader.id = shader_st.screen_shader_count++;
    // hashmap_set(shader_st.hashmap, shader.name, &shader);
  } else if (shader.type == ShaderType_Compute) {
    shader.id = shader_st.compute_shader_count++;
    // hashmap_set(shader_st.hashmap, shader.name, &shader);
  }

  // vk_r_shader_create(shader);
}

Shader& shader_get(String name) {
  // Shader* shader; Assign(shader, hashmap_get(shader_st.hashmap, name));
  // Assert(shader->id != INVALID_ID);
  // return *shader;
}


////////////////////////////////////////////////////////////////////////
// Texture
#include "vendor/stb_image.h"

struct TextureSystemState {
  Arena* arena;
  // HashMap hashmap;
  u32 texture_count;
};

global TextureSystemState texture_st;

#define MaxTextureCount 256
void texture_init() {
  texture_st.arena = arena_alloc();
  // texture_st.hashmap = hashmap_create(texture_st.arena, sizeof(u32), MaxTextureCount);
  u32 invalid_id = INVALID_ID;
  // hashmap_fill(texture_st.hashmap, &invalid_id);
}

Texture res_texture_load(String name) {
  Scratch scratch;
  Texture texture = {};
  
  u32 required_channel_count = 4;
  // stbi_set_flip_vertically_on_load(true);

  String filepath = push_strf(scratch, "%s/%s/%s", res_sys_base_path(), String("textures"), name);

  String filepath_c = push_str_copy(scratch, filepath);

  u8* data = stbi_load(
    (char*)filepath_c.str,
    (i32*)&texture.width,
    (i32*)&texture.height,
    (i32*)&texture.channel_count,
    required_channel_count);
  if (!data) {
    AssertMsg(false, "Image resource loader failed to load file '%s'", filepath);
    return {};
  }

  texture.filepath = push_str_copy(texture_st.arena, filepath);

  texture.data = data;
  texture.channel_count = required_channel_count;
  
  return texture;
}

void res_texture_unload(void* data) {
  stbi_image_free(data);
}

intern void destroy_texture(Texture* t) {
  res_texture_unload(t->data);
}

void texture_load(String name) {
  Texture texture = res_texture_load(name);
  vk_texture_load(texture);
}

KAPI Texture& texture_get(String name) {
  // Texture* t; Assign(t, hashmap_get(texture_st.hashmap, name));
  // Assert(t->id != INVALID_ID);
  // return *t;
}
