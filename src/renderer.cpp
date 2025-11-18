#include "renderer.h"

#include "asset_watch.h"

#include "ui.h"

struct RendererSystemState {
  b8 is_render;
};

f32 delta_time;

global RendererSystemState st;

void r_init() {
  shader_init();
  texture_init();
  mesh_init();

  st.is_render = true;
  
  vk_init();
  
  shader_create("screen_shader", ShaderType_Screen);
  // ui_init();
}

void r_shutdown() {
  vk_shutdown();
}

void r_begin_draw_frame() {

}

void r_end_draw_frame() {
  vk_begin_frame();

  // World
  {
    vk_begin_renderpass(Renderpass_World);
    vk_draw();
    vk_end_renderpass(Renderpass_World);
  }
  
  // {
  //   vk_begin_renderpass(Renderpass_UI);
  //   ui_begin_frame();
  //   ui_end_frame();
  //   vk_end_renderpass(Renderpass_UI);
  // }

  {
    // vk_begin_renderpass(Renderpass_Screen);
    // vk_draw_screen();
    // vk_end_renderpass(Renderpass_Screen);
  }

  vk_end_frame();
}

////////////////////////////////////////////////////////////////////////
// Geometry
struct MeshState {
  Arena* arena;
};

global MeshState mesh_st;

// intern Mesh load_obj(String name) {
//   Scratch scratch;
//   DarrayArena<v3> vertices(scratch);
//   DarrayArena<u32> indexes(scratch);
//   u32 vert_count = 0;
//   u32 index_count = 0;
//   Buffer buff = os_file_all_read(scratch, name);
//   Range range = {.offset = (u64)buff.data, .size = buff.size + (u64)buff.data};
//   String line;
//   while ((line = str_read_line(&range))) {
//     if (line.str[0] == 'v' && line.str[1] == ' ') { // vertex
//       ++vert_count;
//       u32 start = 2;
//       v3 v = {
//         f32_from_str(str_next_word(line, start)),
//         f32_from_str(str_next_word(line, start)),
//         f32_from_str(str_next_word(line, start)),
//       };
//       append(vertices, v);
//     } else if (line.str[0] == 'f') {
//       ++index_count;
//       Loop (i, line.size) {
//         if (line.str[i] == ' ') {
//           String start_on_num = str_skip(line, i+1);
//           i32 num_length = str_index_of(start_on_num, '/');
//           String num = str_prefix(start_on_num, num_length);
//           append(indexes, u32_from_str(num));
//         }
//       }
//     }
//   }
//   Mesh mesh = {
//     .vertices = push_array(mesh_st.arena, Vertex, vert_count),
//     .indexes = push_array(mesh_st.arena, u32, index_count),
//     .vert_count = vert_count,
//     .index_count = index_count,
//   };
//   Loop (i, vert_count) {
//     mesh.vertices[i] = {
//       .pos = vertices[i],
//     };
//   }
//   MemCopyTyped(mesh.indexes, indexes.data, index_count);
//   push_buffer(scratch, 10, 4);
//   return mesh;
// }

intern Mesh load_obj(String name) {
  Scratch scratch;
  DarrayArena<v3> positions(scratch);
  DarrayArena<v3> normals(scratch);
  DarrayArena<v2> uvs(scratch);
  DarrayArena<v3u> indexes(scratch);
  Buffer buff = os_file_all_read(scratch, name);

  Range range = {(u64)buff.data, buff.size + (u64)buff.data};
  String line;
  while ((line = str_read_line(&range))) {
    // Vert
    if (line.str[0] == 'v' && char_is_space(line.str[1])) {
      u32 start = 2;
      v3 v = {
        f32_from_str(str_next_word(line, start)),
        f32_from_str(str_next_word(line, start)),
        f32_from_str(str_next_word(line, start)),
      };

      // Info("v %f, %f, %f", v.x, v.y, v.z);
      append(positions, v);
    }
    // Norm
    else if (line.str[0] == 'v' && line.str[1] == 'n') {
      u32 start = 3;
      v3 v = {
        f32_from_str(str_next_word(line, start)),
        f32_from_str(str_next_word(line, start)),
        f32_from_str(str_next_word(line, start)),
      };
      // Info("vn %f, %f, %f", v.x, v.y, v.z);
      append(normals, v);
    }
    // UV
    else if (line.str[0] == 'v' && line.str[1] == 't') {
      u32 start = 3;
      v2 v = {
        f32_from_str(str_next_word(line, start)),
        f32_from_str(str_next_word(line, start)),
      };
      // Info("vt %f, %f", v.x, v.y);
      append(uvs, v);
    }
    // Indexes
    else if (line.str[0] == 'f') {
      u32 start = 2;
      Loop (i, 3) {
        String vert_index = str_next_word(line, start);

        u32 fist_slash_index = str_index_of(vert_index, '/');
        String first_num = str_prefix(vert_index, fist_slash_index);

        String second_part = str_skip(vert_index, fist_slash_index+1);
        u32 second_slash_index = str_index_of(second_part, '/');
        String second_num = str_prefix(second_part, second_slash_index);

        String third_num = str_skip(second_part, second_slash_index+1);

        v3u v = {
          u32_from_str(first_num) - 1,
          u32_from_str(third_num) - 1,
          u32_from_str(second_num) - 1,
        };
        // Info("%i, %i, %i", v.x, v.y, v.z);
        append(indexes, v);
      }
    }
  }

  DarrayArena<Vertex> vertices(mesh_st.arena);
  DarrayArena<u32> final_indices(mesh_st.arena);

  for (v3u idx : indexes) {
    u32 pos_idx = idx.x;
    u32 norm_idx = idx.y;
    u32 uv_idx = idx.z;
    Vertex vertex = {
      .pos = positions[pos_idx],
      .norm = normals[norm_idx],
      .uv = uvs[uv_idx],
    };

    u32 vertex_index;
    if (!exists_at(vertices, vertex, &vertex_index)) {
      vertex_index = len(vertices);
      append(vertices, vertex);
    }
    append(final_indices, vertex_index);
  }
  Mesh mesh = {
    .vertices = vertices.data,
    .indexes = (u32*)final_indices.data,
    .vert_count = len(vertices),
    .index_count = len(final_indices),
  };
  return mesh;
}

void mesh_init() {
  mesh_st.arena = arena_alloc();
}

u32 mesh_create(String name) {
  Scratch scratch;
  String filepath = push_strf(scratch, "%s/%s/%s", asset_base_path(), String("models"), name);
  Mesh mesh = load_obj(filepath);
  u32 id = vk_mesh_load(mesh);
  return id;
}

Mesh mesh_get(String name) {
  return {};
}

////////////////////////////////////////////////////////////////////////
// Asset

struct AssetState {
  Arena* arena;
  String asset_path;
};

global AssetState asset_st;

void asset_init(String asset_path) {
  asset_st.arena = arena_alloc();
  asset_st.asset_path = push_strf(asset_st.arena, "%s/%s", os_get_current_directory(), asset_path);
}

String asset_base_path() {
  return asset_st.asset_path;
}

////////////////////////////////////////////////////////////////////////
// Shader

struct ShaderState {
  Arena* arena;
  String shader_dir;
  String shader_compiled_dir;
  Map<String, u32> map;
};

global ShaderState shader_st;

u32 shader_create(String name, ShaderType type) {
  u32 id = vk_shader_load(name, type);
  shader_st.map.insert(name, id);
  return id;
}

void shader_init() {
  shader_st.arena = arena_alloc();
  shader_st.shader_dir = push_str_cat(shader_st.arena, asset_base_path(), "/shaders");
  shader_st.shader_compiled_dir = push_str_cat(shader_st.arena, shader_st.shader_dir, "/compiled");
  asset_watch_directory_add("shaders", [](String name) {
    Scratch scratch;
    String shader_filepath = push_strf(scratch, "%s/%s", shader_st.shader_dir, name);
    String shader_compiled_filepath = push_strf(scratch, "%s/%s%s", shader_st.shader_compiled_dir, name, String(".spv"));
    String cmd = push_strf(scratch, "glslangValidator -V %s -o %s", shader_filepath, shader_compiled_filepath);
    os_process_launch(cmd);
  }, OS_WatchFlag_Modify);
  asset_watch_directory_add("shaders/compiled", [](String name) {
    Scratch scratch;
    String shader_name_with_format = str_chop_last_dot(name);
    String shader_name = str_chop_last_dot(shader_name_with_format);
    u32 id = shader_st.map.get(shader_name);
    vk_shader_reload(shader_name, id);
  }, OS_WatchFlag_Modify);
}

Shader& shader_get(String name) {
  // Shader* shader; Assign(shader, hashmap_get(shader_st.hashmap, name));
  // Assert(shader->id != INVALID_ID);
  // return *shader;
  Shader a;
  return a;
}

////////////////////////////////////////////////////////////////////////
// Texture

#include "vendor/stb_image.h"

struct TextureState {
  Arena* arena;
  // HashMap hashmap;
  u32 texture_count;
};

global TextureState texture_st;

intern Texture image_load(String name) {
  Scratch scratch;
  Texture texture = {};
  u32 required_channel_count = 4;
  u32 channel_count;
  String filepath = push_strf(scratch, "%s/%s/%s", asset_base_path(), String("textures"), name);
  u8* data = stbi_load(
    (char*)filepath.str,
    (i32*)&texture.width,
    (i32*)&texture.height,
    (i32*)&channel_count,
    required_channel_count);

  Assert(data);

  texture.channel_count = required_channel_count;
  texture.data = data;
  return texture;
}

void texture_init() {
  texture_st.arena = arena_alloc();
}

u32 texture_load(String name) {
  Texture texture = image_load(name);
  u32 id = vk_texture_load(texture);
  return id;
}

KAPI Texture& texture_get(String name) {
  // Texture* t; Assign(t, hashmap_get(texture_st.hashmap, name));
  // Assert(t->id != INVALID_ID);
  // return *t;
  Texture a;
  return a;
}
