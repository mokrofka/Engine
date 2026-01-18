#include "common.h"
#include "asset_watch.h"

Transform entities_transforms[MaxEntities];

f32 delta_time;

struct CommonState {
  Arena arena;
  String asset_path;
  String shader_dir;
  String shader_compiled_dir;
  Map<String, u32> shader_map;
};

global CommonState st;

void common_init() {
  Scratch scratch;
  st.arena = arena_init();
  st.asset_path = push_strf(st.arena, "%s/%s", os_get_current_directory(), String("../assets"));

  {
    st.shader_dir = push_str_cat(st.arena, asset_base_path(), "/shaders");
    st.shader_compiled_dir = push_str_cat(st.arena, st.shader_dir, "/compiled");
    asset_watch_directory_add(push_str_cat(scratch, asset_base_path(), "/shaders"), [](String name) {
      Scratch scratch;
      String shader_filepath = push_strf(scratch, "%s/%s", st.shader_dir, name);
      String shader_compiled_filepath = push_strf(scratch, "%s/%s%s", st.shader_compiled_dir, name, String(".spv"));
      String cmd = push_strf(scratch, "glslangValidator -V %s -o %s", shader_filepath, shader_compiled_filepath);
      os_process_launch(cmd);
    }, OS_WatchFlag_Modify);
    asset_watch_directory_add(push_str_cat(scratch, asset_base_path(), "/shaders/compiled"), [](String name) {
      Scratch scratch;
      String shader_name_with_format = str_chop_last_dot(name);
      String shader_name = str_chop_last_dot(shader_name_with_format);
      u32 id = st.shader_map.get(shader_name);
      vk_shader_reload(shader_name, id);
    }, OS_WatchFlag_Modify);
  }

  vk_init();

  shader_load("screen_shader", ShaderType_Screen);
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
    vk_begin_renderpass(RenderpassType_World);
    vk_draw();
    vk_end_renderpass(RenderpassType_World);
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
// Asset

String asset_base_path() {
  return st.asset_path;
}

////////////////////////////////////////////////////////////////////////
// Mesh

intern Mesh load_obj(String name) {
  Scratch scratch;

  Darray<v3> positions(scratch);
  Darray<v3> normals(scratch);
  Darray<v2> uvs(scratch);
  Darray<v3u> indexes(scratch);
  Buffer buff = os_file_all_read(scratch, name);

  Range range = {(u64)buff.data, buff.size + (u64)buff.data};
  String line;
  while ((line = str_read_line(&range)).size) {
    // Vert
    if (line.str[0] == 'v' && char_is_space(line.str[1])) {
      u32 start = 2;
      v3 v = {
        f32_from_str(str_next_word(line, start)),
        f32_from_str(str_next_word(line, start)),
        f32_from_str(str_next_word(line, start)),
      };

      // Info("v %f, %f, %f", v.x, v.y, v.z);
      positions.append(v);
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
      normals.append(v);
    }
    // UV
    else if (line.str[0] == 'v' && line.str[1] == 't') {
      u32 start = 3;
      v2 v = {
        f32_from_str(str_next_word(line, start)),
        f32_from_str(str_next_word(line, start)),
      };
      // Info("vt %f, %f", v.x, v.y);
      uvs.append(v);
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
        indexes.append(v);
      }
    }
  }

  Darray<Vertex> vertices(st.arena);
  Darray<u32> final_indices(st.arena);

  // TODO: use hash
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
    if (!vertices.exists_at(vertex, &vertex_index, EqualMem)) {
      vertex_index = vertices.count;
      vertices.append(vertex);
    }
    final_indices.append(vertex_index);
  }
  Mesh mesh = {
    .vertices = vertices.data,
    .indexes = (u32*)final_indices.data,
    .vert_count = vertices.count,
    .index_count = final_indices.count,
  };
  return mesh;
}

u32 mesh_load(String name) {
  Scratch scratch;
  String filepath = push_strf(scratch, "%s/%s/%s", asset_base_path(), String("models"), name);
  Mesh mesh = load_obj(filepath);
  u32 id = vk_mesh_load(mesh);
  return id;
}

////////////////////////////////////////////////////////////////////////
// Shader

struct ShaderState {
  Arena arena;
  String shader_dir;
  String shader_compiled_dir;
};

global ShaderState shader_st;

u32 shader_load(String name, ShaderType type) {
  Scratch scratch;
  u32 id = vk_shader_load(name, type);
  st.shader_map.insert(name, id);
  return id;
}

////////////////////////////////////////////////////////////////////////
// Texture

#include "vendor/stb_image.h"

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

u32 texture_load(String name) {
  Texture texture = image_load(name);
  u32 id = vk_texture_load(texture);
  return id;
}

////////////////////////////////////////////////////////////////////////
// Resources

ShaderDefinition shaders_definition_[Shader_COUNT-1] = {
  [Shader_Color-1] = "color_shader", ShaderType_Drawing,
};
ShaderDefinition shaders_definition(u32 idx) {
  Assert(IsInsideBounds(1, idx, Shader_COUNT));
  return shaders_definition_[idx-1];
}
u32 shaders_[Shader_COUNT-1];
u32& shaders(u32 idx) { 
  Assert(IsInsideBounds(1, idx, Shader_COUNT));
  return shaders_[idx-1];
}

String meshes_path_[Mesh_COUNT-1] = {
  [Mesh_Cube-1] = "cube.obj",
  // [Mesh_Room] = "room.obj",
};
String meshes_path(u32 idx) {
  Assert(IsInsideBounds(1, idx, Mesh_COUNT));
  return meshes_path_[idx-1];
}
u32 meshes_[Mesh_COUNT-1];
u32& meshes(u32 idx) {
  Assert(IsInsideBounds(1, idx, Mesh_COUNT));
  return meshes_[idx-1];
}

String textures_path_[Texture_COUNT-1] = {
  [Texture_OrangeLines-1] = "orange_lines_512.png",
  [Texture_Container-1] = "container.jpg",
  [Texture_Room-1] = "image.png",
};
String textures_path(u32 idx) { 
  Assert(IsInsideBounds(1, idx, Texture_COUNT));
  return textures_path_[idx-1];
}
u32 textures_[Texture_COUNT-1];
u32& textures(u32 idx) {
  Assert(IsInsideBounds(1, idx, Texture_COUNT));
  return textures_[idx-1];
}


