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

  // shader_load("screen_shader", ShaderType_Screen);
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

String asset_base_path() {
  return st.asset_path;
}

struct Lexer {
  u8* cur;
  u8* end;
};

Lexer lexer_init(String buffer) {
  Lexer lexer = {
    .cur = buffer.str,
    .end = buffer.str + buffer.size,
  };
  return lexer;
}

String lexer_next_token(Lexer* l) {
  while (l->cur < l->end && char_is_space(*l->cur)) {
    l->cur++;
  }
  if (l->cur == l->end) return {};
  u8* current = l->cur;
  while (l->cur < l->end && !char_is_space(*l->cur)) {
    l->cur++;
  }
  String result = {current, (u64)l->cur - (u64)current};
  return result;
}

String lexer_next_integer(Lexer* l) {
  while (l->cur < l->end && char_is_space(*l->cur)) {
    l->cur++;
  }
  if (l->cur == l->end) return {};
  while (!char_is_digit(*l->cur)) {
    ++l->cur;
  }
  u8* current = l->cur;
  while (char_is_digit(*l->cur)) {
    ++l->cur;
  }
  String result = {current, (u64)l->cur - (u64)current};
  return result;
}

intern Mesh mesh_load_obj(String name) {
  Scratch scratch;
  Darray<v3> positions(scratch);
  Darray<v3> normals(scratch);
  Darray<v2> uvs(scratch);
  Darray<v3u> indexes(scratch);
  Buffer buff = os_file_read_all(scratch, name);
  Lexer lexer = lexer_init({buff.data, buff.size});
  String word;
  while ((word = lexer_next_token(&lexer)).size) {
    // vert
    if (str_match(word, "v")) {
      v3 v = {};
      for (f32& e : v.e) {
        e = f32_from_str(lexer_next_token(&lexer));
      }
      // Info("v %f, %f, %f", v.x, v.y, v.z);
      positions.append(v);
    }
    // norm
    else if (str_match(word, "vn")) {
      v3 v = {};
      for (f32& e : v.e) {
        e = f32_from_str(lexer_next_token(&lexer));
      }
      // Info("vn %f, %f, %f", v.x, v.y, v.z);
      normals.append(v);
    }
    // uv
    else if (str_match(word, "vt")) {
      v2 v = {};
      for (f32& e : v.e) {
        e = f32_from_str(lexer_next_token(&lexer));
      }
      // Info("vt %f, %f", v.x, v.y);
      uvs.append(v);
    }
    // indexes
    else if (str_match(word, "f")) {
      Loop (i, 3) {
        v3u raw = {};
        for (u32& e : raw.e) {
          e = u32_from_str(lexer_next_integer(&lexer)) - 1;
        }
        v3u v = {raw.x, raw.z, raw.y};
        // Info("%i, %i, %i", v.x, v.y, v.z);
        indexes.append(v);
      }
    }
  }

  Darray<Vertex> vertices(st.arena);
  Darray<u32> final_indices(st.arena);
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

intern Mesh mesh_load_gltf(String name) {
  Scratch scratch;
  Buffer buff = os_file_read_all(scratch, name);

  JsonReader r = json_reader_init({buff.data, buff.size});
  #define JSON_OBJ(r, obj) for (JsonValue key, val; r.iter_obj(obj, &key, &val);)
  #define JSON_ARR(r, arr, v) for (JsonValue v; r.iter_array(arr, &v);)

  JSON_OBJ(r, r.base_obj) {
    if (key.match("meshes")) {
      JSON_ARR(r, val, v) {
        JSON_OBJ(r, v) {
          if (key.match("attributes")) {
            JSON_OBJ(r, val) {
              if ("POSITION") {
                Info("pos: %s", val.str);
              }
              if ("norm: NORMAL") {
                Info("%s", val.str);
              }
              if ("texcoord: TEXCOORD_0") {
                Info("%s", val.str);
              }
            }
          }
          if (key.match("indices")) {
          
          }
          if (key.match("material")) {
          
          }
        }
      }
    }
  }

  // for (JsonValue key, val; r.iter_obj(r.base_obj, &key, &val);) {

  // }

  // while (r.base_obj) {
  // for (JsonValue val, key, ;;) {
  //   if (r.) {
    
  //   }
  //   JsonValue meshes = r.next_obj("meshes");
  // }

  // JsonValue val = r.next_obj(r.base_obj, "meshes");
  // while (val) {
  //   JsonValue obj = r.iter_next(val);
  //   JsonValue name = r.next_obj(obj);
  //   JsonValue primitives = r.next_obj(obj);
  //   while (primitives) {
  //     JsonValue obj = r.iter_next(primitives);
  //     JsonValue attributes = r.next_obj(obj);
  //     JsonValue POSITION = r.next_obj(attributes, )
  //     JsonValue NORMAL = r.next_obj(attributes, )
  //     JsonValue TEXCOORD_0 = r.next_obj(attributes, )
  //   }
  // }

  // iter_array(val) {
  //   JsonValue obj = iter_next(val);
  //   JsonValue name = r.iter_obj(obj, "name");
  //   JsonValue val =  r.iter_obj(obj, "primitives");


  // }

  // JsonValue key, val;
  // while (r.iter_obj(r.base_obj, &key, &val)) {
  //   if (key.match("meshes")) {
  //     JsonValue v;
  //     while (r.iter_array(val, &v)) {
  //       JsonValue key, val;
  //       while (r.iter_obj(v, &key, &val)) {
  //         if (key.match("name")) {
  //           Info("%s", val.str);
  //         } else if (key.match("primitives")) {
  //           JsonValue key, val;
  //           while (r.iter_obj()) {
            
  //           }
  //         }
          
  //       }
  //     }
  //   }
  // }

  // while (json_iter_object(&r, obj, &key, &val)) {
  //   if (str_match(key.str, "meshes")) {
  //     JsonValue val;
  //     while (json_iter_array(&r, val, &val)) {

  //     }
  //   }
  // }
  return {};
}

u32 mesh_load(String name) {
  Scratch scratch;
  String filepath = push_strf(scratch, "%s/%s/%s", asset_base_path(), String("models"), name);
  String format = str_skip_last_slash(name);
  Mesh mesh = {};
  if (str_match(format, "gltf")) {
    mesh = mesh_load_obj(filepath);
  } else if (str_match(format, "obj")) {
    mesh = mesh_load_gltf(filepath);
  } else {
    Assert(true);
  }

  u32 id = vk_mesh_load(mesh);
  return id;
}

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

struct TextureState {

};

global TextureState texture_st;

#include "vendor/stb_image.h"

intern Texture texture_image_load(String name) {
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
  Texture texture = texture_image_load(name);
  u32 id = vk_texture_load(texture);
  return id;
}

// https://github.com/rxi/sj.h.git
////////////////////////////////////////////////////////////////////////
// Json reader

intern b32 char_is_number_cont(u8 c) {
  return (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+';
}

JsonValue json_read(JsonReader* r) {
  JsonValue res = {};
  top:
  if (r->error.str) { return { .type = JsonType_Error, .str = str_range(r->cur, r->end)}; }
  u8* start = r->cur;
  switch (*r->cur) {
    case ' ': case '\n': case '\r': case '\t':
    case ':': case ',': {
      ++r->cur;
      goto top;
    }
    case '-': case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9': {
      res.type = JsonType_Number;
      while (r->cur != r->end && char_is_number_cont(*r->cur)) { ++r->cur; }
    } break;
    case '"': {
      res.type = JsonType_String;
      start = ++r->cur;
      while (true) {
        if (r->cur == r->end) { r->error = "unclosed string"; goto top; }
        if (*r->cur == '"')   { break; }
        if (*r->cur == '\\')  { r->cur++; }
        if (r->cur != r->end) { r->cur++; }
      }
      res.str = str_range(start, r->cur++);
      return res;
    }
    case '{': case '[': {
      res.type = (*r->cur == '{') ? JsonType_Object : JsonType_Array;
      res.depth = ++r->depth;
      r->cur++;
    } break;
    case '}': case ']': {
      res.type = JsonType_End;
      if (--r->depth < 0) {
        r->error = (*r->cur == '}') ? "stray '}'" : "stray ']'";
        goto top;
      }
      r->cur++;
    } break;
    case 'n': case 't': case 'f': {
      res.type = (*r->cur == 'n') ? JsonType_Null : JsonType_Bool;
      if (str_match(String(r->cur, 4),  "null")) { r->cur += 4; break; }
      if (str_match(String(r->cur, 4),  "true")) { r->cur += 4; break; }
      if (str_match(String(r->cur, 5), "false")) { r->cur += 5; break; }
    } // fallthrough
    default: {
      r->error = "unknown token";
      goto top;
    }
  }
  res.str = str_range(start, r->cur);
  return res;
}

JsonReader json_reader_init(String buffer) {
  JsonReader r = {
    .cur = buffer.str,
    .end = buffer.str + buffer.size,
  };
  r.base_obj = r.read();
  return r;
}


intern void json_discard_until(JsonReader* r, i32 depth) {
  JsonValue val;
  val.type = JsonType_Null;
  while (r->depth != depth && val.type != JsonType_Error) {
    val = json_read(r);
  }
}

b32 json_iter_object(JsonReader* r, JsonValue obj, JsonValue *key, JsonValue *val) {
  json_discard_until(r, obj.depth);
  *key = json_read(r);
  if (key->type == JsonType_Error || key->type == JsonType_End) { return false; }
  *val = json_read(r);
  if (val->type == JsonType_End)   { r->error = "unexpected object end"; return false; }
  if (val->type == JsonType_Error) { return false; }
  return true;
}

b32 json_iter_array(JsonReader* r, JsonValue arr, JsonValue* val) {
  json_discard_until(r, arr.depth);
  *val = json_read(r);
  if (val->type == JsonType_Error || val->type == JsonType_End) { return false; }
  return true;
}

JsonValue JsonReader::read() {
  return json_read(this);
}

b32 JsonReader::iter_obj(JsonValue obj, JsonValue *key, JsonValue *val)   {
  json_discard_until(this, obj.depth);
  *key = json_read(this);
  if (key->type == JsonType_Error || key->type == JsonType_End) { return false; }
  *val = json_read(this);
  if (val->type == JsonType_End)   { this->error = "unexpected object end"; return false; }
  if (val->type == JsonType_Error) { return false; }
  return true;
}

b32 JsonReader::iter_array(JsonValue arr, JsonValue* val) {
  json_discard_until(this, arr.depth);
  *val = json_read(this);
  if (val->type == JsonType_Error || val->type == JsonType_End) { return false; }
  return true;
}

////////////////////////////////////////////////////////////////////////
// Shaders

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

////////////////////////////////////////////////////////////////////////
// Meshes

String meshes_path_[Mesh_COUNT-1] = {
  [Mesh_Cube-1] = "cube.obj",
  // [Mesh_GltfCube-1] = "cube.gltf"
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

////////////////////////////////////////////////////////////////////////
// Textures

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


