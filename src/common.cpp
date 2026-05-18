#include "stb_image.h"
#include "stb_truetype.h"
#include "game.cpp"

#include "common.h"
#include "json.cpp"
#include "test.cpp"

u64 hash(Vertex x) { return hash_memory(&x, sizeof(x)); }
b32 equal(Vertex a, Vertex b) { return MemMatchStruct(&a, &b); }

Extern GlobalState* g_st;

Timer timer_init(f32 interval) {
  Timer timer = {
    .interval = interval,
  };
  return timer;
}

b32 timer_tick(Timer& t) {
  t.passed += g_st->dt;
  if (t.passed >= t.interval) {
    t.passed = 0;
    return true;
  }
  return false;
}

f64 tsc_to_ms(u64 tsc) {
  return (f64)tsc/cpu_frequency()*1000;
}

f32 get_dt() { return g_st->dt; }
f32 get_time() { return g_st->time; }

Transform& Handle<Entity>::trans() { 
  Assert(id_generation(handle) == g_st->game.entity_id_pool.generations[id_idx(handle)]);
  return g_st->transforms[id_idx(handle)];
}
v3& Handle<Entity>::pos() { return trans().pos; }
v3& Handle<Entity>::rot() { return trans().rot; }
v3& Handle<Entity>::scale() { return trans().scale; }
Entity& Handle<Entity>::get() { return g_st->game.entities[handle & INDEX_MASK]; }
Rng3& Handle<Entity>::aabb() { return get().aabb; }
v3& Handle<Entity>::vel() { return get().vel; }

Transform& Handle<StaticEntity>::trans() {
  Assert(id_generation(handle) == g_st->game.static_entity_id_pool.generations[id_idx(handle)]);
  return g_st->static_transforms[id_idx(handle)];
}
v3& Handle<StaticEntity>::pos() { return trans().pos; }
v3& Handle<StaticEntity>::rot() { return trans().rot; }
v3& Handle<StaticEntity>::scale() { return trans().scale; }

intern Mesh mesh_load_obj(Allocator arena, String name) {
  Scratch scratch(arena);
  var positions = darray_make<v3>(scratch);
  var normals = darray_make<v3>(scratch);
  var uvs = darray_make<v2>(scratch);
  var indexes = darray_make<v3u>(scratch);
  Slice buf = os_file_path_read_all(scratch, name);
  Lexer lexer = lexer_init({buf.data, buf.count});
  String word;
  while ((word = lexer_next_token(&lexer)).size) {
    // vert
    if (str_match(word, "v")) {
      v3 v = {};
      for (f32& e : v.v) {
        e = f32_from_str(lexer_next_token(&lexer));
      }
      // Info("v %f, %f, %f", v.x, v.y, v.z);
      darray_add(positions, v);
    }
    // norm
    else if (str_match(word, "vn")) {
      v3 v = {};
      for (f32& e : v.v) {
        e = f32_from_str(lexer_next_token(&lexer));
      }
      // Info("vn %f, %f, %f", v.x, v.y, v.z);
      darray_add(normals, v);
    }
    // uv
    else if (str_match(word, "vt")) {
      v2 v = {};
      for (f32& e : v.v) {
        e = f32_from_str(lexer_next_token(&lexer));
      }
      // Info("vt %f, %f", v.x, v.y);
      darray_add(uvs, v);
    }
    // indexes
    else if (str_match(word, "f")) {
      Loop (i, 3) {
        v3u raw = {};
        for (u32& e : raw.v) {
          e = u32_from_str(lexer_next_integer(&lexer)) - 1;
        }
        v3u v = {raw.x, raw.z, raw.y};
        // Info("%i, %i, %i", v.x, v.y, v.z);
        darray_add(indexes, v);
      }
    }
  }
  var vertices = darray_make<Vertex>(arena);
  var final_indices = darray_make<u32>(arena);
  Map<Vertex, u32> map(scratch);
  Loop (i, indexes.count) {
    v3u idx = indexes[i];
    Vertex vertex = {
      .pos = positions[idx.x],
      .norm = normals[idx.y],
      .uv = uvs[idx.z],
    };
    u32* found = map.get(vertex);
    if (found) {
      darray_add(final_indices, *found);
    } else {
      u32 new_index = vertices.count;
      darray_add(vertices, vertex);
      darray_add(final_indices, new_index);
      map.add(vertex, new_index);
    }
  }
  Mesh mesh = {
    .vertices = vertices.data,
    .indices = final_indices.data,
    .vert_count = vertices.count,
    .index_count = final_indices.count,
  };
  return mesh;
}

intern Mesh mesh_load_gltf(Allocator arena, String name) {
  Scratch scratch(arena);
  Slice buf = os_file_path_read_all(scratch, name);
  JsonReader r = json_reader_init({buf.data, buf.count});
  struct MeshInfo {
    // u32 pos_idx;
    // u32 norm_idx;
    // u32 uv_idx;
    // u32 indices_idx;
    // b32 arr[10];
    // u32 vert_count[10];
    u32 vert_count;
    u32 index_count;
    BufferRegion ranges[10];
    String file_name;
    u32 file_size;
  } info = {};
  // Parsing json
  JSON_OBJ(r, r.base_obj) {
  //   if (k.match("meshes")) {
  //     JSON_ARR(r, v) JSON_OBJ(r, obj) {
  //       if (k.match("primitives")) {
  //         u32 i = 0;
  //         JSON_ARR(r, v) JSON_OBJ(r, obj) {
  //           if (k.match("attributes")) {
  //             JSON_OBJ_(r, v) {
  //               if (key.match("POSITION")) {
  //                 info.pos_idx = i;
  //                 // info.arr[i] = true;
  //               }
  //               else if (key.match("NORMAL")) {
  //                 info.norm_idx = i;
  //                 // info.arr[i] = true;
  //               }
  //               else if (key.match("TEXCOORD_0")) {
  //                 info.uv_idx = i;
  //                 // info.arr[i] = true;
  //               }
  //               ++i;
  //             }
  //           }
  //           else if (k.match("indices")) {
  //             info.indices_idx = i;
  //             // info.arr[i] = true;
  //           }
  //         }
  //       }
  //     }
  //   }
    if (k.match("accessors")) {
      u32 i = 0;
      JSON_ARR(r, v) {
        JSON_OBJ(r, obj) {
          if (k.match("count")) {
            if (i == 0) {
              info.vert_count = u32_from_str(v.str);
            }
            else if (i == 3) {
              info.index_count = u32_from_str(v.str);
            }
          }
        }
        ++i;
      }
    }
    else if (k.match("bufferViews")) {
      u32 i = 0;
      JSON_ARR(r, v) {
        JSON_OBJ(r, obj) {
          if (k.match("byteLength")) {
            info.ranges[i].size = u32_from_str(v.str);
          }
          else if (k.match("byteOffset")) {
            info.ranges[i].offset = u32_from_str(v.str);
          }
        }
        ++i;
      }
    }
    else if (k.match("buffers")) {
      JSON_ARR(r, v) {
        JSON_OBJ(r, obj) {
          if (k.match("byteLength")) {
            info.file_size = u32_from_str(v.str);
          }
          else if (k.match("uri")) {
            info.file_name = v.str;
          }
        }
      }
    }
  }
  String model_dir = str_chop_last_slash(name);
  Slice buf1 = os_file_path_read_all(scratch, push_strf(scratch, "%s/%s", model_dir, info.file_name));
  v3* vertices_pos = (v3*)Offset(buf1.data, info.ranges[0].offset);
  v3* vertices_norm = (v3*)Offset(buf1.data, info.ranges[1].offset);
  v2* vertices_uv = (v2*)Offset(buf1.data, info.ranges[2].offset);
  u16* vertices_indices = (u16*)Offset(buf1.data, info.ranges[3].offset);
  Vertex* vertices = push_array(arena, Vertex, info.vert_count);
  u32* indices = push_array(arena, u32, info.index_count);
  Loop (i, info.index_count) {
    indices[i] = vertices_indices[i];
  }
  Loop (i, info.vert_count) {
    vertices[i] = {
      .pos = vertices_pos[i],
      .norm = vertices_norm[i],
      .uv = vertices_uv[i],
    };
  }
  Mesh mesh = {
    .vertices = vertices,
    .indices = (u32*)indices,
    .vert_count = info.vert_count,
    .index_count = info.index_count,
  };
  return mesh;
}

intern Mesh mesh_load_glb(Allocator arena, String name) {
  Scratch scratch(arena);
  Slice buf = os_file_path_read_all(scratch, name);
  struct FileHeader {
    u32 magic;
    u32 version;
    u32 length;
  };
  struct Chunk {
    u32 chunk_length;
    u32 chunk_type;
    u32 chunk_data;
  };
  FileHeader* header = (FileHeader*)buf.data;
  Assert(str_match(String((u8*)&header->magic, 4), "glTF"));
  Chunk* json_chunk = (Chunk*)Offset(header, sizeof(FileHeader));
  Assert(str_match(String((u8*)&json_chunk->chunk_type, 4), "JSON"));
  Chunk* bin_chunk = (Chunk*)Offset(json_chunk, sizeof(Chunk)-4 + json_chunk->chunk_length);
  Assert(str_match(String((u8*)&bin_chunk->chunk_type, 3), "BIN"));
  struct Accessor{
    u32 count;
  };
  struct Primitives {
    u32 pos;
    u32 norm;
    u32 uv;
    u32 index;
  };
  struct MeshInfo {
    Accessor accessors[10];
    BufferRegion buffer_views[10];
    Primitives primitives;
    String file_name;
    u32 file_size;
  } info = {};
  JsonReader r = json_reader_init({(u8*)&json_chunk->chunk_data, buf.count});
  JSON_OBJ(r, r.base_obj) {
    if (k.match("meshes")) {
      JSON_ARR(r, v) JSON_OBJ(r, obj) {
        if (k.match("primitives")) {
          JSON_ARR(r, v) JSON_OBJ(r, obj) {
            if (k.match("attributes")) {
              JSON_OBJ_(r, v) {
                if (key.match("POSITION")) {
                  info.primitives.pos = u32_from_str(val.str);
                }
                else if (key.match("NORMAL")) {
                  info.primitives.norm = u32_from_str(val.str);
                }
                else if (key.match("TEXCOORD_0")) {
                  info.primitives.uv = u32_from_str(val.str);
                }
              }
            }
            else if (k.match("indices")) {
              info.primitives.index = u32_from_str(v.str);
            }
          }
        }
      }
    }
    if (k.match("accessors")) {
      u32 i = 0;
      JSON_ARR(r, v) {
        JSON_OBJ(r, obj) {
          if (k.match("count")) {
            info.accessors[i].count = u32_from_str(v.str);
          }
        }
        ++i;
      }
    }
    else if (k.match("bufferViews")) {
      u32 i = 0;
      JSON_ARR(r, v) {
        JSON_OBJ(r, obj) {
          if (k.match("byteLength")) {
            info.buffer_views[i].size = u32_from_str(v.str);
          }
          else if (k.match("byteOffset")) {
            info.buffer_views[i].offset = u32_from_str(v.str);
          }
        }
        ++i;
      }
    }
    else if (k.match("buffers")) {
      JSON_ARR(r, v) {
        JSON_OBJ(r, obj) {
          if (k.match("byteLength")) {
            info.file_size = u32_from_str(v.str);
          }
          else if (k.match("uri")) {
            info.file_name = v.str;
          }
        }
      }
    }
  }
  u8* data = (u8*)&bin_chunk->chunk_data;
  v3* vertices_pos = (v3*)Offset(data, info.buffer_views[info.primitives.pos].offset);
  v3* vertices_norm = (v3*)Offset(data, info.buffer_views[info.primitives.norm].offset);
  v2* vertices_uv = (v2*)Offset(data, info.buffer_views[info.primitives.uv].offset);
  u16* vertices_indices = (u16*)Offset(data, info.buffer_views[info.primitives.index].offset);
  u32 vertex_count = info.accessors[info.primitives.pos].count;
  u32 index_count = info.accessors[info.primitives.index].count;
  Vertex* vertices = push_array(arena, Vertex, vertex_count);
  u32* indices = push_array(arena, u32, index_count);
  Loop (i, index_count) {
    indices[i] = vertices_indices[i];
  }
  Loop (i, vertex_count) {
    vertices[i] = {
      .pos = vertices_pos[i],
      .norm = vertices_norm[i],
      .uv = vertices_uv[i],
    };
  }
  Mesh mesh = {
    .vertices = vertices,
    .indices = indices,
    .vert_count = vertex_count,
    .index_count = index_count,
  };
  return mesh;
}

intern Texture texture_image_load(String filepath) {
  Scratch scratch;
  Texture texture = {};
  u32 required_channel_count = 4;
  u32 channel_count;
  Slice buf = os_file_path_read_all(scratch, filepath);
  u8* data = stbi_load_from_memory(buf.data, buf.count, (i32*)&texture.width, (i32*)&texture.height, (i32*)&channel_count, required_channel_count);
  Assert(data);
  texture.data = data;
  return texture;
}

////////////////////////////////////////////////////////////////////////
// Assets

// TODO: allocate in hotreload build?
global String meshes_strs[Mesh_Load_COUNT] = {
#define X(enum_name, name) [enum_name] = Stringify(name),
  MESH_LIST
#undef X
};

global String textures_strs[Texture_COUNT] = {
#define X(enum_name, name) [enum_name] = Stringify(name),
  TEXTURE_LIST
#undef X
};

Handle<GpuMesh> mesh_get(MeshId id) { return g_st->meshes_handlers[id]; }
void mesh_set(MeshId id, Handle<GpuMesh> mesh_handle) { g_st->meshes_handlers[id] = mesh_handle; }
Handle<GpuMaterial> material_get(MaterialId id) { return g_st->materials_handlers[id]; }

constexpr ShaderState shader_default_info() {
  ShaderState info = {
    .type = ShaderType_Drawing,
    .topology = ShaderTopology_Triangle,
    .samples = 4,
    .is_transparent = false,
    .use_depth = true,
  };
  return info;
}

constexpr MaterialProps material_default_props() {
  MaterialProps props = {
    .ambient = v3_scale(1),
    .diffuse = v3_scale(1),
    .specular = v3_scale(1),
    .shininess = 1,
  };
  return props;
}

Handle<GpuMesh> mesh_load(String name) {
  GlobalState& g = *g_st;
  Scratch scratch;
  String filepath = push_strf(scratch, "%s/%s", g.models_dir, name);
  String format = str_skip_last_dot(name);
  Mesh mesh = {};
  if (str_match(format, "glb")) {
    mesh = mesh_load_glb(scratch, filepath);
  } else if (str_match(format, "gltf")) {
    mesh = mesh_load_gltf(scratch, filepath);
  } else if (str_match(format, "obj")) {
    mesh = mesh_load_obj(scratch, filepath);
  } else {
    InvalidPath;
  }
  Handle<GpuMesh> handle = vk_mesh_load(mesh);
  return handle;
}

Handle<GpuTexture> texture_load(String name) {
  GlobalState& g = *g_st;
  Scratch scratch;
  String filepath = push_strf(scratch, "%s/%s", g.textures_dir, name);
  Texture texture = texture_image_load(filepath);
  Handle<GpuTexture> handle = vk_texture_load(texture);
  return handle;
}

Handle<GpuCubemap> cubemap_load(String name) {
  GlobalState& g = *g_st;
  Scratch scratch;
  Texture textures[6];
  String sides[] = {
    "right", "left",
    "top", "bottom",
    "front", "back",
  };
  for EachElement(i, textures) {
    String texture_name = push_strf(scratch, "%s/%s%s", name, sides[i], String(".png"));
    String filepath = push_strf(scratch, "%s/%s", g.textures_dir, texture_name);
    textures[i] = texture_image_load(filepath);
  }
  vk_cubemap_load(textures);
  return {};
}

void asset_load() {
  GlobalState& g = *g_st;
#define X(enum_name, name) \
  g.meshes_handlers[enum_name] = mesh_load(meshes_strs[enum_name]); \
  g.str_to_mesh.add(Stringify(name), g.meshes_handlers[enum_name]);
  MESH_LIST
#undef X

#define X(enum_name, name) \
  g.textures_handlers[enum_name] = texture_load(textures_strs[enum_name]); \
  g.str_to_texture.add(Stringify(name), g.textures_handlers[enum_name]);
  TEXTURE_LIST
#undef X

  struct TakeMaterial { \
    String name = "e_texture";
    ShaderState state = shader_default_info();
    MaterialProps props = material_default_props();
    String texture = "container.jpg";
    Handle<GpuTexture> texture_handle;
    operator Material() {
      return {
        .shader = {
          .name = name,
          .state = state,
        },
        .props = props,
        .texture = texture,
        .texture_handle = texture_handle,
      };
    }
  };
#define X(enum_name, ...) \
  { \
    TakeMaterial mat = { \
      __VA_ARGS__ \
    }; \
    Handle<GpuTexture>* texture_hanle = g.str_to_texture.get(mat.texture); \
    if (texture_hanle) \
      mat.texture_handle = *texture_hanle; \
    g.materials_handlers[enum_name] = vk_material_load(mat); \
  }
  MATERIAL_LIST
#undef X
}

////////////////////////////////////////////////////////////////////////
// Lexer

Slice<Token> tokens_from_str(Allocator arena, String str) {
  // u32 off = 0;
//   u32 advance = 0;
//   for (u32 advance = 0; off <= str.size; off += advance) {
//     u8 byte      = (off+0 < str.size) ? str.str[off+0] : 0;
//     u8 next_byte = (off+1 < str.size) ? str.str[off+1] : 0;
//     b32 ender_found = false;
//   }
  return {};
}

////////////////////////////////////////////////////////////////////////
// @Input

b32 key_pressed(Key key) {
  if (os_is_key_pressed(key)) {
    if (!g_st->input.consumed[key]) return true;
  }
  return false;
}

b32 key_pressed_consume(Key key) {
  if (key_pressed(key)) {
    key_consume(key);
    return true;
  }
  return false;
}

b32 key_down(Key key) {
  if (os_is_key_down(key)) {
    if (!g_st->input.consumed[key]) return true;
  }
  return false;
}

b32 key_down_consume(Key key) {
  if (key_down(key)) {
    key_consume(key);
    return true;
  }
  return false;
}

void key_consume(Key key) {
  g_st->input.consumed[key] = true;
}

void input_update() {
  MemZeroArray(g_st->input.consumed, ArrayCount(g_st->input.consumed));
}

////////////////////////////////////////////////////////////////////////
// some ui

ScrollState scroll_state_make(f32 scale) {
  ScrollState res = {
    .scale_level = scale,
    .scale = v2_scale(scale),
  };
  return res;
}

void ui_handle_scroll(ScrollState& s, ScrollType type) {
  f32 wheel = os_get_scroll();
  if (wheel) {
    if (os_is_key_down(Key_Ctrl)) {
      v2 mouse = os_get_mouse_pos();
      f32 sensity = 1.3;
      f32 zoom = (wheel > 0) ? sensity : 1.0f/sensity;
      switch (type) {
        case ScrollType_Default: {
          // we have: mouse == world * scale + offset;
          v2 world = (mouse - s.offset) / s.scale_level;
          s.scale_level *= zoom;
          s.offset = mouse - world * s.scale_level;
          s.scale = v2_scale(s.scale_level);
        } break;
        case ScrollType_PowClamp: {
          v2 world = {
            (mouse.x - s.offset.x) / s.scale.x,
            (mouse.y - s.offset.y) / s.scale.y
          };
          s.scale_level *= zoom;
          s.scale_level = ClampBot(s.scale_level, 0.02);
          s.scale.y = Clamp(0.01, s.scale_level, 3);
          f32 t = Pow(s.scale_level + 1, 2);
          f32 ratio = t * 0.3;
          s.scale.x = s.scale.y * ratio;

          if (s.scale.y > 1) {
            f32 inv = 1.0f / s.scale_level;
            f32 target = inv / (1.0f + inv);
            s.scale.y = Lerp(1.0f, 0.3f, target);
          }

          s.offset.x = mouse.x - world.x * s.scale.x;
          s.offset.y = mouse.y - world.y * s.scale.y;

        } break;
      }
    }
    else {
      s.offset.y += wheel * 100.0f;
    }
  }

  f32 scroll_h = os_get_scroll_h();
  if (scroll_h) {
    f32 sensity = 100;
    if (os_is_key_down(Key_Shift)) {
      sensity *= 3;
    }
    s.offset.x += scroll_h * sensity;
  }
}

void imgui_window_toggle_fullscreen(ImguiWindow& window) {
  window.toggle_fullscreen = 1;
}

void imgui_window_apply_state(ImguiWindow& win) {
  if (win.toggle_fullscreen) {
    if (!win.fullscreen) {
      win.fullscreen = true;
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
      win.flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    } else {
      win.fullscreen = false;
      ImGui::SetNextWindowPos(win.pos);
      ImGui::SetNextWindowSize(win.size);
      win.flags = NoFlags;
    }
    win.toggle_fullscreen = false;
  }
}

void imgui_window_track_state(ImguiWindow& window) {
  if (!window.fullscreen) {
    window.pos = ImGui::GetWindowPos();
    window.size = ImGui::GetWindowSize();
  }
}

////////////////////////////////////////////////////////////////////////
// UI

// void ui_begin() {
//   UI_State& g = g_st->ui;
//   g.last_hot = g.hot;
//   g.hot = 0;
// }

// void ui_end() {
//   UI_State g = g_st->ui;
//   if (os_is_key_released(MouseKey_Left)) {
//     g.active = 0;
//   }
// }

// void ui_push_box(String str) {
//   UI_State& g = g_st->ui;

//   UI_Box& parent = g.boxes[g.boxes_count];
//   ++g.boxes_count;

//   u64 hash_idx = hash(str);
//   UI_Box box = {
//     .pos = v2(parent.pos + parent.size),
//     .size = {100 + parent.size.x, 100 + parent.pos.y},
//     .hash = (hash(hash_idx, parent.hash))
//   };
//   g.boxes[g.boxes_count] = box;

//   ui_button(box.hash, box.pos, box.pos+box.size);
// }

// void ui_pop_box() {
//   UI_State& g = g_st->ui;
//   if (g.boxes_count > 0) {
//     --g.boxes_count;
//   }
// }

// b32 ui_begin_window(u32 id, v2 size) {
//   UI_State& g = g_st->ui;
//   v2& pos = g.windows[id].pos;
//   v2 mouse = os_get_mouse_pos();
//   Rng2 title_rect(pos, v2(pos.x + size.x, pos.y + 20));

//   b32 hovered = contains_2f32(title_rect, mouse);
//   if (hovered) {
//     g.hot = id;
//   }

//   // PRESS → start dragging
//   if (g.last_hot == id && os_is_key_pressed(MouseKey_Left)) {
//     g.active = id;
//     g.active_window = id;

//     // store offset
//     g.drag_offset = v2(mouse.x - pos.x, mouse.y - pos.y);
//   }

//   // DRAG
//   if (g.active == id && os_is_key_down(MouseKey_Left)) {
//     pos.x = mouse.x - g.drag_offset.x;
//     pos.y = mouse.y - g.drag_offset.y;
//   }

//   // RELEASE
//   if (g.active == id && os_is_key_released(MouseKey_Left)) {
//     g.active = 0;
//   }

//   //  Draw window body
//   vk_draw_quad(pos, v2(pos.x + size.x, pos.y + size.y), v3(0.2f,0.2f,0.2f));

//   //  Draw title bar
//   v3 title_color = v3(0.3f,0.3f,0.3f);
//   if (g.hot == id) title_color = v3(0.4f,0.4f,0.4f);
//   if (g.active == id) title_color = v3(0.2f,0.2f,0.2f);

//   title_rect = {pos, v2(pos.x + size.x, pos.y + 20)};
//   vk_draw_quad(title_rect.min, title_rect.max, title_color);

//   return true;
// }

// b32 ui_button(u32 id, v2 min, v2 max) {
//   UI_State& g = g_st->ui;
//   b32 hovered = contains_2f32({min, max}, os_get_mouse_pos());
//   if (hovered) {
//     g.hot = id;
//   }

//   if (g.last_hot == id && os_is_key_pressed(MouseKey_Left)) {
//     g.active = id;
//   }

//   b32 clicked = 0;
//   if (g.active == id && os_is_key_released(MouseKey_Left)) {
//     if (g.hot == id) {
//       clicked = true;
//     }
//     g.active = 0;
//   }

//   v3 color = {0.6f, 0.6f, 0.6f};
//   if (g.hot == id) color = v3(0.8f, 0.8f, 0.8f);
//   if (g.active == id) color = v3(0.4f, 0.4f, 0.4f);

//   vk_draw_quad(min, max, color);
//   return clicked;
// }

void profiler_view() {
  Scratch scratch;
  ProfState& g = prof_get();
  ProfWindow& win = g_st->profile_win;

  // Avg, min, max
  ProfFrame prev_frame = prof_get_prev_frame(g_st->current_frame);
  var anchors = prev_frame.anchors;
  u64 cpu_freq = cpu_frequency();
  u64 tsc_start = prev_frame.frame_time.tsc_start;
  u64 tsc_end = prev_frame.frame_time.tsc_end;
  u64 tsc_elapsed = tsc_end - tsc_start;
  u64 tsc_elapsed_sum = 0;
  u64 tsc_elapsed_max = g.frames_times[0].tsc_end - g.frames_times[0].tsc_start;
  u64 tsc_elapsed_min = g.frames_times[0].tsc_end - g.frames_times[0].tsc_start;
  for EachElement(i, g.frames_times) {
    ProfFrameTime frame = g.frames_times[i];
    u64 elapsed = frame.tsc_end - frame.tsc_start;
    tsc_elapsed_sum += elapsed;
    tsc_elapsed_max = Max(tsc_elapsed_max, elapsed);
    tsc_elapsed_min = Min(tsc_elapsed_min, elapsed);
  }
  g.frame_avg_time = tsc_to_ms(tsc_elapsed_sum / 120);
  g.frame_max_time = tsc_to_ms(tsc_elapsed_max);
  g.frame_min_time = tsc_to_ms(tsc_elapsed_min);

  if (key_pressed(Key_H)) {
    ImGui::SetNextWindowFocus(); 
  }

  if (win.open) {
    imgui_window_apply_state(win);

    if (ImGui::Begin("Profiler", null, win.flags)) {
      imgui_window_track_state(win);
      if (key_pressed(Key_1)) g.active_tab = ProfileTabActive_Root;
      if (key_pressed(Key_2)) g.active_tab = ProfileTabActive_Frames;
      if (key_pressed(Key_3)) g.active_tab = ProfileTabActive_Time;
      if (key_pressed(Key_4)) g.active_tab = ProfileTabActive_LaunchTime;
      if (key_pressed(Key_5)) g.active_tab = ProfileTabActive_Memory;
      if (key_pressed(Key_P)) g.paused = !g.paused;
      if (ImGui::IsWindowHovered()) {
        if (key_pressed(Key_V)) {
          imgui_window_toggle_fullscreen(win);
        }
      }

      ImGui::SetWindowFontScale(0.8f);
      if (ImGui::BeginTabBar("MyTabBar")) {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        v2 cursor_pos = ImGui::GetCursorScreenPos();
        v2 mouse_pos = os_get_mouse_pos();
        v2 win_pos = ImGui::GetWindowPos();
        v2 avail_size = ImGui::GetWindowSize();
        avail_size.x -= (cursor_pos - win_pos).x * 2;

        enum UI_ItemType {
          UI_ItemType_Bar,
          UI_ItemType_NextThread,
        };
        struct UI_Item {
          UI_ItemType type;
          Rng2 rect;
          ProfAnchor anchor;
        };

        var draw_frame_graph = [&](Slice<Slice<ProfAnchor>> slices, ProfFrameTime time, f32 width_off, ScrollState scroll_state, b32 wrap = false) {
          Scratch scratch;
          var items = darray_make<UI_Item>(scratch);
          Loop (i, slices.count) {
            var anchors = slices[i];

            ///////////////////////////////////
            // Build rect layout
            {
              // ProfFrameTime time = g.frames_times[anchors_idx];
              u64 tsc_start = time.tsc_start;
              u64 tsc_end = time.tsc_end;
              u64 tsc_elapsed = tsc_end - tsc_start;
              Loop(i, anchors.count) {
                ProfAnchor anchor = anchors[i];
                u64 var_tsc_elapsed_incl = anchor.tsc_elapsed_incl;
                u64 var_tsc_start = anchor.tsc_start;
                
                // Handle async anchors
                if (wrap) {
                  if (!anchor.was_poped) {
                    var_tsc_elapsed_incl = tsc_end - anchor.tsc_start;
                    anchor.tsc_elapsed_incl = var_tsc_elapsed_incl;
                  }
                  if (anchor.tsc_start < tsc_start) {
                    var_tsc_start = tsc_start;
                    anchor.tsc_start = var_tsc_start;
                  }
                }
                else {
                  if (!anchor.was_poped) {
                    break;
                  }
                }

                f64 width_t = (f64)var_tsc_elapsed_incl / tsc_elapsed;
                f64 width_t_off = normalize((f64)tsc_start, var_tsc_start, tsc_end);
                if (anchor.tsc_elapsed_incl != anchor.tsc_elapsed_excl) {
                  width_t = (f64)anchor.tsc_elapsed_incl / tsc_elapsed;
                }
                f32 height = 30;
                f32 height_off = anchor.depth * height;
                f32 width = width_t * avail_size.x;
                f32 width_off = width_t_off * avail_size.x;
                Rng2 rect = pos_size_2f32(v2(width_off, height_off), v2(width, height));
                UI_Item item = {
                  .type = UI_ItemType_Bar,
                  .rect = rect,
                  .anchor = anchor,
                };
                darray_add(items, item);
              }
            }
            darray_add(items, {.type = UI_ItemType_NextThread});
          }

          ///////////////////////////////////
          // Anchors and thread offsets
          f32 height_off = 0;
          f32 thread_height_off = 200;
          Loop (i, items.count) {
            UI_Item& item = items[i];
            switch (item.type) {
              case UI_ItemType_Bar: {
                item.rect = shift_2f32(item.rect, v2(width_off, height_off));
              } break;
              case UI_ItemType_NextThread: {
                height_off += thread_height_off;
              } break;
            }
          }

          // Scroll
          Loop (i, items.count) {
            UI_Item& item = items[i];
            item.rect = shift_2f32(item.rect, cursor_pos);
            item.rect.min.x *= scroll_state.scale.x;
            item.rect.min.y *= scroll_state.scale.y;
            item.rect.max.x *= scroll_state.scale.x;
            item.rect.max.y *= scroll_state.scale.y;
            item.rect = shift_2f32(item.rect, scroll_state.offset);
          }

          ///////////////////////////////////
          // Drawing
          Loop (i, items.count) {
            UI_Item item = items[i];
            switch (item.type) {
              default:{}break;
              case UI_ItemType_Bar: {
                ImU32 color = {};
                ImString str = {};
                ProfAnchor anchor = item.anchor;
                Rng2 rect = item.rect;
                switch (anchor.type) {
                  case ProfType_Work: {
                    color = IM_COL32(50, 50, 50, 255);
                    str = String("work");
                  } break;
                  case ProfType_Sleep: {
                    color = IM_COL32(50, 70, 80, 255);
                    str = String("sleep");
                  } break;
                  case ProfType_Worker: {
                    color = IM_COL32(50, 100, 80, 255);
                    str = String("job");
                  } break;
                }
                draw->AddRectFilled(IM_RECT(rect), color);
                draw->AddRect(IM_RECT(rect), IM_COL32(200, 200, 200, 255));
                if (contains_2f32(rect, mouse_pos)) {
                  ImGui::BeginTooltip();
                  ImGui::Text("Label: %s", anchor.label.str);
                  ImGui::Text("Percent: %f%%", dim_2f32(rect).x / avail_size.x * 100);
                  // ImGui::Text("Hits: %lu", anchor.hit_count);
                  ImGui::Text("Time: %fms", tsc_to_ms(anchor.tsc_elapsed_incl));
                  ImGui::Text("Time exclusive: %fms", tsc_to_ms(anchor.tsc_elapsed_excl));
                  ImGui::Text("Type: %s", str.str);
                  ImGui::EndTooltip();
                }

                // Text
                {
                  ImString str = push_strf(scratch, "%s %.3f", anchor.label, tsc_to_ms(anchor.tsc_elapsed_incl));
                  v2 text_size = ImGui::CalcTextSize(str);
                  if (dim_2f32(rect).x < 30.1 || scroll_state.scale.y < 0.3) {
                    continue;
                  }
                  v2 text_pos = {};
                  if (text_size.x > dim_2f32(rect).x) {
                    text_pos.x = rect.min.x;
                    text_pos.y = rect.min.y + (dim_2f32(rect).y - text_size.y) * 0.5;
                  } else {
                    text_pos = align_center_2f32(rect, text_size).min;
                  }
                  draw->PushClipRect(rect.min, rect.max);
                  draw->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), str);
                  draw->PopClipRect();
                }
              } break;
            }
          }
        };

        ImGui::Text("%.1ffps %.1fms CPU %.1fGhz, Recording: %s", 1000 / tsc_to_ms(tsc_elapsed), tsc_to_ms(tsc_elapsed), (f64)cpu_freq / Billion(1), g.paused ? "off" : "on");
        ImGui::Text("avg %.1fms, max %.1f, min %.1f", g.frame_avg_time, g.frame_max_time, g.frame_min_time);
        f32 info_height = 60;
        cursor_pos.y += info_height;

        ///////////////////////////////////
        // Draw thread names
        var draw_threads = [&](ScrollState scroll_state) {
          f32 thread_height = 200;
          f32 thread_height_offset = 0;
          f32 text_off_above = -40;
          {
            ImString str = push_strf(scratch, "Main thread");
            v2 text_pos = (v2(0, text_off_above) + cursor_pos);
            text_pos.y *= scroll_state.scale.y;
            text_pos.y += scroll_state.offset.y;
            draw->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), str);
          }
          {
            Loop (i, THREAD_COUNT) {
              thread_height_offset += thread_height;
              ImString str = push_strf(scratch, "Worker %i", i);
              v2 text_pos = v2(0, thread_height_offset + text_off_above) + cursor_pos;
              text_pos.y *= scroll_state.scale.y;
              text_pos.y += scroll_state.offset.y;
              draw->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), str);
            }
            thread_height_offset = 0;
          }
        };

        ///////////////////////////////////
        // Root
        if (ImGui::BeginTabItem("root"), g.active_tab == ProfileTabActive_Root) {
          ScrollState& scroll_state = win.root_scroll_state;
          if (ImGui::IsWindowHovered()) {
            ui_handle_scroll(scroll_state, ScrollType_PowClamp);
          }
          cursor_pos.y += 30;
          draw_threads(scroll_state);
          u32 idx = (g_st->current_frame-1) % ArrayCount(g.frames_times);
          Slice<ProfAnchor> slices[ArrayCount(g.prof_threads)] = {};
          for EachElement(i, g.prof_threads) {
            slices[i] = slice(g.prof_threads[i].recorded_anchors[idx]);
          }
          ProfFrameTime time = g.frames_times[idx];
          draw_frame_graph(ArraySlice(slices), time, 0, scroll_state);
          ImGui::EndTabItem();
        };

        ///////////////////////////////////
        // Frames
        if (ImGui::BeginTabItem("frames"), g.active_tab == ProfileTabActive_Frames) {
          ScrollState& scroll_state = win.frames_scroll_state;
          if (ImGui::IsWindowHovered()) {
            ui_handle_scroll(scroll_state, ScrollType_PowClamp);
          }
          f32 width_size = avail_size.x;

          ///////////////////////////////////
          // Little bars
          Loop (i, ArrayCount(g.frames_times)) {
            ProfFrameTime frame_time = g.frames_times[i];
            f32 max_height = 40;
            f32 max_ms = 30;
            f64 frame_ms = tsc_to_ms(frame_time.tsc_end - frame_time.tsc_start);
            f64 height = max_height / (max_ms / frame_ms);
            v2 size = v2(avail_size.x / ArrayCount(g.frames_times), height);
            v2 min = cursor_pos + v2(i*size.x, -height + max_height);
            Rng2 rect = pos_size_2f32(min, size);
            if (contains_2f32(rect, mouse_pos)) {
              ImGui::BeginTooltip();
              ImGui::Text("frame: %i", i);
              ImGui::EndTooltip();
              if (os_is_key_pressed(MouseKey_Left)) {
                win.frames_scroll_state.offset.x = -width_size * i;
                win.frames_scroll_state.scale = v2_scale(1);
              }
            }
            ImU32 color = IM_COL32(50, 200, 50, 255);
            if (contains_1f32(Rng1f32(17, 20), frame_ms)) {
              color = IM_COL32(200, 200, 50, 255);
            } else if (frame_ms > 20) {
              color = IM_COL32(255, 100, 50, 255);
            }
            if (i == g_st->current_frame % ArrayCount(g.frames_times)) {
              color = IM_COL32(230,230,230,255);
            }
            if (i == g_st->current_frame % ArrayCount(g.frames_times)) {
              draw->AddRectFilled(IM_RECT(rect), color);
              draw->AddRect(IM_RECT(rect), IM_COL32(10, 10, 10, 100));
            } else {
              draw->AddRectFilled(IM_RECT(rect), color);
              draw->AddRect(IM_RECT(rect), IM_COL32(10, 10, 10, 100));
            }
          }
          cursor_pos.y += 80;

          ///////////////////////////////////
          // Draw lines and current rect
          {
            f32 width_offset = 0;
            Loop (i, ArrayCount(g.frames_times)) {
              f32 line_height = 1000;
              f32 thick = 1;
              v2 base = cursor_pos + v2(width_offset, 0);
              v2 p0 = base + v2(0, -line_height / 2);
              v2 p1 = base + v2(0, line_height);
              v2 p2 = base + v2(width_size, 0);
              v2 p3 = base + v2(width_size, 0) + v2(0, line_height);
              p0 = p0 * scroll_state.scale.x + scroll_state.offset;
              p1 = p1 * scroll_state.scale.x + scroll_state.offset;
              p2 = p2 * scroll_state.scale.x + scroll_state.offset;
              p3 = p3 * scroll_state.scale.x + scroll_state.offset;
              draw->AddLine(p0, p1, IM_COL32(200, 200, 200, 255), thick);
              if (i == g_st->current_frame % ArrayCount(g.frames_times)) {
                draw->AddRectFilled(p0, p3, IM_COL32(100, 100, 100, 100));
              }
              width_offset += width_size;
            }
          }

          draw_threads(scroll_state);

          ///////////////////////////////////
          // Draw graph per thread
          for EachElement(j, g.frames_times) {
            Slice<ProfAnchor> slices[ArrayCount(g.prof_threads)] = {};
            for EachElement(i, g.prof_threads) {
              slices[i] = slice(g.prof_threads[i].recorded_anchors[j]);
            }
            ProfFrameTime time = g.frames_times[j];
            draw_frame_graph(ArraySlice(slices), time, j * width_size, scroll_state);
          }
          ImGui::EndTabItem();
        }

        ///////////////////////////////////
        // Time
        if (ImGui::BeginTabItem("time"), g.active_tab == ProfileTabActive_Time) {
          var sorted_anchors = slice_clone(scratch, anchors);
          sort_insert(sorted_anchors, [](ProfAnchor a, ProfAnchor b) { return a.tsc_elapsed_excl > b.tsc_elapsed_excl; });

          Loop (i, anchors.count) {
            ImGui::PushID(i);
            ProfAnchor anchor = sorted_anchors[i];
            f64 width_exclusive_percent = (f64)anchor.tsc_elapsed_excl / tsc_elapsed;
            f32 width_exclusive = avail_size.x * 0.8;
            f32 height = 30;
            width_exclusive *= width_exclusive_percent;

            v2 offset = v2(0,  i * height) + cursor_pos;
            v2 size = v2(width_exclusive, height);
            Rng2 rect = Rng2(offset, size + offset);

            ImDrawList* draw = ImGui::GetWindowDrawList();
            draw->AddRectFilled(IM_RECT(rect), IM_COL32(50, 50, 50, 255));
            draw->AddRect(IM_RECT(rect), IM_COL32(200, 200, 200, 255));

            ImString name_str = push_strf(scratch, "%s", anchor.label);
            ImString ms_str = push_strf(scratch, "%.3fms", (f64)anchor.tsc_elapsed_excl / cpu_freq * 1000);
            v2 name_offset = v2(0, height * i) + cursor_pos;
            v2 ms_offset = v2(avail_size.x * 0.82, height * i) + cursor_pos;

            draw->AddText(name_offset, ImGui::GetColorU32(ImGuiCol_Text), name_str);
            draw->AddText(ms_offset, ImGui::GetColorU32(ImGuiCol_Text), ms_str);

            ImGui::PopID();
          }
          ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("launch"), g.active_tab == ProfileTabActive_LaunchTime) {
          ScrollState& scroll_state = win.launch_time_scroll_state;
          if (ImGui::IsWindowHovered()) {
            ui_handle_scroll(scroll_state, ScrollType_PowClamp);
          }

          draw_threads(scroll_state);

          Slice<ProfAnchor> slices[ArrayCount(g.prof_threads)] = {};
          for EachElement(i, g.prof_threads) {
            slices[i] = slice(g.prof_threads[i].launch_anchors);
          }
          ProfFrameTime time = g.launch_time;
          draw_frame_graph(ArraySlice(slices), time, 0, scroll_state, true);
          ImGui::EndTabItem();
        }

        ///////////////////////////////////
        // Memory
        if (ImGui::BeginTabItem("memory"),  g.active_tab == ProfileTabActive_Memory) {
          ScrollState& scroll_state = win.mem_scroll_state;
          if (ImGui::IsWindowHovered()) {
            ui_handle_scroll(scroll_state);
          }
          enum UI_ItemType {
            UI_ItemType_MemUsage,
            UI_ItemType_MemLevel,
            UI_ItemType_Arena,
            UI_ItemType_Child,
          };
          struct UI_Item {
            UI_ItemType type;
            Rng2 rect;
            u32 depth;
            AllocatorInfo* info;
            u32 mem_level;
          };
          var items = darray_make<UI_Item>(scratch);
          AllocatorInfoList infos = get_allocators_info();
          var infos_sorted = sort_list_insert(scratch, infos.first, [](var a, var b) { return a->pos > b->pos; });
          f64 mem_usage = 0;
          for (var x : infos_sorted) {
            mem_usage += x->cmt;
          }
          f32 mem_levels[] = {KB(1), KB(10), KB(100), MB(1), MB(10), MB(100), GB(1)};

          ///////////////////////////////////
          // Layout
          {
            f32 row_h = 30;
            Rng2Cursor curs = {};
            // mem usage
            {
              UI_Item item = {
                .type = UI_ItemType_MemUsage,
                .rect = layout_row(curs, Rng1(0, avail_size.x), row_h),
              };
              darray_add(items, item);
            }

            b32 level_drawn[ArrayCount(mem_levels)] = {};
            Loop (i, infos_sorted.count) {
              var& info = *infos_sorted[i];
  
              //  Mem level
              u32 mem_level = 0;
              for EachElement(i, mem_levels) {
                if (info.pos < mem_levels[i]) {
                  mem_level = i;
                  break;
                }
              }
              if (!level_drawn[mem_level]) {
                level_drawn[mem_level] = true;
                UI_Item item = {
                  .type = UI_ItemType_MemLevel,
                  .rect = layout_row(curs, Rng1(0, avail_size.x), row_h),
                  .mem_level = mem_level,
                };
                darray_add(items, item);
              }
  
              // Arena
              {
                UI_Item item = {
                  .type = UI_ItemType_Arena,
                  .rect = layout_row(curs, Rng1(0, avail_size.x), row_h),
                  .info = &info,
                  .mem_level = mem_level,
                };
                darray_add(items, item);
              }
  
              // Children
              u32 depth = 1;
              struct StackEntry {
                AllocatorInfo* node;
                u32 depth;
              };
              var stack = darray_make<StackEntry>(scratch);
              Slice sorted_children = sort_list_insert(scratch, info.first, [](var a, var b) { return a->pos > b->pos; });
              ReverseLoop (i, sorted_children.count) {
                darray_add(stack, {sorted_children[i], 1});
              }
              while (stack.count) {
                StackEntry entry = darray_pop(stack);
                var child = entry.node;
                UI_Item item = {
                  .type = UI_ItemType_Child,
                  .depth = entry.depth,
                  .info = child,
                  .mem_level = mem_level,
                };
                item.rect = Rng2(
                  v2(depth * 10, curs.pos.y),
                  v2(avail_size.x, curs.pos.y + row_h * 0.6)
                );
                darray_add(items, item);
                layout_next(curs, row_h * 0.6);
                if (child->first) {
                  ++depth;
                  Slice sorted_children = sort_list_insert(scratch, child->first, [](var a, var b) { return a->pos > b->pos; });
                  ReverseLoop (i, sorted_children.count) {
                    darray_add(stack, {sorted_children[i], entry.depth + 1});
                  }
                }
              }
            }
          }

          Loop (i, items.count) {
            UI_Item& item = items[i];
            item.rect = shift_2f32(item.rect, cursor_pos);
            item.rect = scale_2f32(item.rect, scroll_state.scale.x);
            item.rect = shift_2f32(item.rect, scroll_state.offset);
          }

          Rng2 rounding_edge = shift_2f32(scale_2f32(pos_size_2f32(cursor_pos, avail_size), scroll_state.scale.x), scroll_state.offset);
          rounding_edge = pad_2f32(rounding_edge, 10);
          draw->AddRect(IM_RECT(rounding_edge), IM_COL32(200, 200, 200, 255));

          ///////////////////////////////////
          // Drawing
          Loop (i, items.count) {
            UI_Item item = items[i];
            AllocatorInfo& info = *item.info;

            switch (item.type) {
              case UI_ItemType_MemUsage: {
                MemFormatSize mem_fmt = mem_format_size(mem_usage);
                ImString mem_usage_str = push_strf(scratch, "mem usage: %.2f%s", mem_fmt.size, mem_fmt.format);
                draw->AddText(cursor_pos, IM_COL32(255,255,255,255), mem_usage_str);
              } break;
              case UI_ItemType_MemLevel: {
                MemFormatSize mem_fmt = mem_format_size(mem_levels[item.mem_level]);
                ImString str = push_strf(scratch, "%.0f%s", mem_fmt.size, mem_fmt.format);
                v2 text_size = ImGui::CalcTextSize(str);
                Rng2 rect = item.rect;
                Rng2 text_rect = align_center_2f32(rect, text_size);
                Rng2 pad_text_rect = pad_2f32(text_rect, 5);
                draw->AddRectFilled(IM_RECT(pad_text_rect), IM_COL32(80, 90, 130, 120));
                draw->AddText(text_rect.min, IM_COL32(255,255,255,255), str);
              } break;
              case UI_ItemType_Arena: {
                f32 t_w = dim_2f32(item.rect).x;
                f32 t_pos = info.pos / mem_levels[item.mem_level];
                // f32 t_cap = info.cap / mem_levels[item.mem_level];
                f32 t_excl = info.exclusive_pos / mem_levels[item.mem_level];
                f32 w_pos = t_w * t_pos;
                // f32 w_cap = t_w * t_cap;
                f32 w_excl = t_w * t_excl;
                Rng2 excl_rect = slice_x_2f32(item.rect, Rng1(0, w_excl));
                Rng2 incl_rect = slice_x_2f32(item.rect, Rng1(w_excl, w_pos));

                draw->AddRectFilled(IM_RECT(excl_rect), IM_COL32(70, 80, 50, 255));
                draw->AddRect(IM_RECT(excl_rect), IM_COL32(200, 200, 200, 255));
                draw->AddRectFilled(IM_RECT(incl_rect), IM_COL32(40, 70, 120, 255));
                draw->AddRect(IM_RECT(incl_rect), IM_COL32(200, 200, 200, 255));

                MemFormatSize pos = mem_format_size(info.pos);
                MemFormatSize pos_exclusive = mem_format_size(info.exclusive_pos);
                MemFormatSize cmt = mem_format_size(info.cmt);

                ImString name_str = push_strf(scratch, "%s", info.name);
                ImString mem_str = push_strf(scratch, "%.2f%s pos, %.2f%s cmt", pos.size, pos.format, cmt.size, cmt.format);

                draw->AddText(excl_rect.min, ImGui::GetColorU32(ImGuiCol_Text), name_str);
                draw->AddText(slice_x_2f32(item.rect, 0.2, 1).min, ImGui::GetColorU32(ImGuiCol_Text), mem_str);
                if (contains_2f32(union_2f32(incl_rect, excl_rect), mouse_pos)) {
                  ImGui::BeginTooltip();
                  ImString inclusive = push_strf(scratch, "inclusive: %.2f%s", pos.size, pos.format);
                  ImGui::Text("%s", inclusive.str);
                  ImString exclusive = push_strf(scratch, "exclusive: %.2f%s", pos_exclusive.size, pos_exclusive.format);
                  ImGui::Text("%s", exclusive.str);
                  ImGui::EndTooltip();
                }
              } break;
              case UI_ItemType_Child: {
                f32 t_w = dim_2f32(item.rect).x;
                f32 t_pos = info.pos / mem_levels[item.mem_level];
                f32 t_cap = info.cap / mem_levels[item.mem_level];
                // f32 t_excl = info.exclusive_pos / mem_levels[item.mem_level];
                f32 w_pos = t_w * t_pos;
                f32 w_cap = t_w * t_cap;
                // f32 w_excl = t_w * t_excl;
                Rng2 child_rect = slice_x_2f32(item.rect, Rng1(0, w_pos));
                Rng2 child_rect_cap = slice_x_2f32(item.rect, Rng1(w_pos, w_cap));

                // pos
                draw->AddRectFilled(IM_RECT(child_rect), IM_COL32(70, 70, 70, 255));
                draw->AddRect(IM_RECT(child_rect), IM_COL32(200, 200, 200, 255));
                // cap
                draw->AddRectFilled(IM_RECT(child_rect_cap), IM_COL32(80, 40, 40, 255));
                draw->AddRect(IM_RECT(child_rect_cap), IM_COL32(100, 100, 100, 255));

                MemFormatSize pos = mem_format_size(info.pos);
                MemFormatSize cap = mem_format_size(info.cap);
                ImString child_name_str = push_strf(scratch, "%s", info.name);
                ImString child_meta_str = push_strf(scratch, "%.2f%s pos, %.2f%s cap, alloc count: %u, free count: %u, current alloc count: %u", pos.size, pos.format, cap.size, cap.format, info.allocs, info.frees, info.current_allocs);
                draw->AddText(child_rect.min, ImGui::GetColorU32(ImGuiCol_Text), child_name_str);
                draw->AddText(slice_x_2f32(child_rect, 0.3, 1).min, ImGui::GetColorU32(ImGuiCol_Text), child_meta_str);
              } break;
            }
          }

          ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
      }
    } ImGui::End();
  }
}

void game_view() {
  GameState& g = g_st->game;
  ImguiWindow& win = g_st->game_win;
  Camera& cam = g.cam;
  if (win.open) {
    imgui_window_apply_state(win);
    ImGui::Begin("Game");

    if (ImGui::IsWindowHovered()) {
      if (key_pressed(Key_V)) {
        imgui_window_toggle_fullscreen(win);
      }
    }

    v3 pos = cam.pos;
    ImGui::Text("entities: %u, static entities: %u", g.entities_count, g.static_entities_count);
    ImGui::Text("Camera: x: %.2f y: %.2f z: %.2f", pos.x, pos.y, pos.z);
    // ImGui::SliderFloat("", &cam.speed, 0, 10000);
    ImGui::DragFloat("speed", &cam.speed, 1);
    ImGui::End();
  }
}

////////////////////////////////////////////////////////////////////////
// Watch

void watch_add(String watch_name, WatchOp op) {
  WatchState& g = g_st->watch;
  FileProperties props = os_file_path_properties(watch_name);
  WatchFile file_watch = {
    .path = watch_name,
    .modified = props.modified,
    .op = op,
  };
  array_add(g.watches, file_watch);
}

void watch_directory_add(String watch_name, WatchOp op, OS_WatchFlags flags) {
  WatchState& g = g_st->watch;
  String dir_path = push_strf(g.arena, "%s", watch_name);
  OS_Watch watch = os_watch_open(flags);
  os_watch_attach(watch, dir_path);
  WatchDirectory dir_watch = {
    .path = dir_path,
    .watch = watch,
    .op = op,
  };
  array_add(g.directories, dir_watch);
}

void watch_update() {
  WatchState& g = g_st->watch;
  Scratch scratch;
  Loop (i, g.watches.count) {
    WatchFile& x = g.watches[i];
    FileProperties props = os_file_path_properties(x.path);
    if (props.modified > x.modified) {
      switch (x.op) {
        case WatchOp_NotifyHotreload: {
          g_st->should_hotreload = true;
        } break;
        InvalidDefaultCase break;
      }
      x.modified = props.modified;
    }
  }
  Loop (i, g.directories.count) {
  WatchDirectory x = g.directories[i];
    StringList list = os_watch_check(scratch, x.watch);
    for EachNode(it, StringNode, list.first) {
      String name = it->string;
      switch (x.op) {
        case WatchOp_RecompileShader: {
          GlobalState& g = *g_st;
          Scratch scratch;
          String shader_filepath = push_strf(scratch, "%s/%s", g.shader_dir, name);
          String shader_compiled_filepath = push_strf(scratch, "%s/%s%s", g.shader_compiled_dir, name, String(".spv"));
          StringList list = {};
          str_list_push(scratch, &list, "glslangValidator");
          str_list_push(scratch, &list, "-V");
          str_list_push(scratch, &list, shader_filepath);
          str_list_push(scratch, &list, "-o");
          str_list_push(scratch, &list, shader_compiled_filepath);
          os_process_launch(list);
        } break;
        case WatchOp_ShaderReload: {
          String shader_name_with_format = str_chop_last_dot(name);
          String shader_name = str_chop_last_dot(shader_name_with_format);
          vk_shader_reload(shader_name);
        } break;
        InvalidDefaultCase break;
      }
    }
  }
}

void common_init() {
  GlobalState& g = *g_st;
  estimate_cpu_frequency();
  global_allocator_init();
  os_gfx_init();
  prof_init(g.arena);
  prof_launch_begin();
  {
    ProfBlock("init");
    thread_pool_init(THREAD_COUNT);
    test();

    g.gpa.init(g.arena);
    g.transforms = push_array(g.arena, Transform, MaxEntities);
    g.static_transforms = push_array(g.arena, Transform, MaxStaticEntities);
    g.asset_path = push_strf(g.arena, "%s/%s", os_get_current_directory(), String("../assets"));
    g.shader_dir = push_str_cat(g.arena, g.asset_path, "/shaders");
    g.shader_compiled_dir = push_str_cat(g.arena, g.shader_dir, "/compiled");
    g.models_dir = push_str_cat(g.arena, g.asset_path, "/models");
    g.textures_dir = push_str_cat(g.arena, g.asset_path, "/textures");
    g.str_to_texture.init(g.gpa);
    g.str_to_mesh.init(g.gpa);
    g.str_to_material.init(g.gpa);

    g.watch.arena = g.arena;
    watch_directory_add(g.shader_dir, WatchOp_RecompileShader);
    watch_directory_add(g.shader_compiled_dir, WatchOp_ShaderReload);

    var& win = g.profile_win;
    win.root_scroll_state = scroll_state_make(1);
    win.frames_scroll_state = scroll_state_make(1);
    win.launch_time_scroll_state = scroll_state_make(1);
    win.mem_scroll_state = scroll_state_make(1);
    win.open = false;

    g.vk = vk_init();
#if DEAR_IMGUI
    vk_imgui_init();
#endif
    game_init();
    
  }
  prof_launch_end();
}

void common_update() {
  GlobalState& g = *g_st;
  input_update();
  profiler_view();
  game_view();
  
  if (key_pressed(Key_F1)) g.profile_win.open = !g.profile_win.open;
  if (key_pressed(Key_F2)) g.imgui_demo_open = !g.imgui_demo_open;
  if (key_pressed(Key_F3)) g.game_win.open = !g.game_win.open;

  if (g_st->imgui_demo_open) {
    ImGui::ShowDemoWindow();
  }

  // {
  //   ProfBlock("block0");
  //   os_sleep_ms(1);
  // }
  // {
  //   ProfBlock("block1");
  //   os_sleep_ms(1);
  // }
  // Loop (i, 2) {
  //   foo(2);
  // }
  Task task = {
    .func = [](void* ptr) {
      // ProfBlock("job1");
      // ProfBlock("sleep job");
      // ProfBlock("sleep job");
      {
        // ProfBlock("job2");
        // os_sleep_ms(rand_u32()%10);
      }
      // os_sleep_ms(4);
      os_sleep_ms(rand_u32()%2);
      // os_sleep_ms(rand_u32()%10);
      // os_sleep_ms(32);
    }
  };
  {
    ProfBlock("push jobs");
    Loop (i, 16) {
      task_queue_push(task);
    }
  }
  // thread_wait_for();
}

shared_function void common_main(HotReloadData* data) {
  Scratch scratch;
if (data->ctx == null) {
    Arena arena = arena_make_named("common arena");
    data->ctx = push_struct_zero(arena, GlobalState);
    g_st = (GlobalState*)data->ctx;
    g_st->arena = arena;
    {
      TimeScope scope;
      common_init();
    }
#if HOTRELOAD_BUILD
    watch_add(data->lib, WatchOp_NotifyHotreload);
#endif
  }
  if (!g_st) {
    g_st = (GlobalState*)data->ctx;
    vk_set_state(g_st->vk);
    g_st->should_hotreload = false;
  }

  GlobalState& g = *g_st;

  u64 target_fps = Billion(1) / 60;
  u64 last_time = os_now_ns();

  while (!os_window_should_close()) {
    if (g_st->should_hotreload) {
      goto hotreload;
    }

    prof_begin(g.current_frame);
    {
      ProfBlock("frame");
      os_pump_messages();
      u64 start_time = os_now_ns();
      g.dt = f64(start_time - last_time) / Billion(1);
      g.time += g.dt;
      last_time = start_time;
      vk_begin_draw_frame();
      // ui_begin();
      common_update();
      game_update();
      // ui_end();
      vk_end_draw_frame();
      os_input_update();
      watch_update();

      u64 frame_duration = os_now_ns() - start_time;
      if (frame_duration < target_fps) {
        u64 sleep_time = target_fps - frame_duration;
        ProfBlock("main sleep", ProfType_Sleep);
        os_sleep_ms(sleep_time / Million(1));
      }
    }
    prof_end(g.current_frame);
    ++g.current_frame;
  }

  // deinit
  // vk_shutdown();
  // os_gfx_shutdown();
  os_exit(0);

  hotreload:
  thread_wait_for();
}
// void vk_draw() {
//   ProfFunc;

//   VkCommandBuffer cmd = vk_get_current_cmd();
//   vk->CmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->pipeline_layout, 0, 1, &vk->descriptor_sets, 0, null);
//   VkDeviceSize size = 0;
//   vk->CmdBindVertexBuffers(cmd, 0, 1, &vk->vert_buffer.h, &size);
//   vk->CmdBindIndexBuffer(cmd, vk->index_buffer.h, 0, VK_INDEX_TYPE_UINT32);

//   VK_State& g = *vk;
//   GlobalStateGPU& shader_st = *vk->gpu_global_shader_st;
//   shader_st.projection_view = vk->projection * vk->view;
//   shader_st.projection = vk->projection;
//   shader_st.view = vk->view;
  
//   u32 draw_call_count = 0;
//   u32 draw_call_mem_offset = 0;
//   u32 entities_draw_count = 0;
//   u32 entity_inst_offset = 0;

//   Loop (i, g.pipelines.count) {
//     vk_bind_pipeline(cmd, g.pipelines[i]);
//     VK_RenderBatch& batch = vk->batches[i];

//     var fill_buffer = []() {
      
//     };

//     // Indexed Entities per shader
//     u32 per_shader_indexed_draw_count = 0;
//     for (VK_MeshBatch mesh_batch : batch.batch_indexed.mesh_batches) {
//       if (mesh_batch.entities.count == 0) continue;
//       for (Handle<Entity> entity_handle : mesh_batch.entities) {
//         u32 entity_idx = entity_handle.idx();
//         vk->gpu_entities[entity_idx].model = mat4_transform(entity_handle.trans());
//         vk->gpu_entities_indices[entities_draw_count++] = entity_idx;
//       }
//       u32 mesh_idx = mesh_batch.mesh_handle.handle;
//       VK_Mesh mesh = vk->meshes[mesh_idx];
//       VK_DrawCallInfo info = {
//         .index_draw_command = {
//           .indexCount = (u32)mesh.index_count,
//           .instanceCount = mesh_batch.entities.count,
//           .firstIndex = (u32)(mesh.index_offset/sizeof(u32)),
//           .vertexOffset = (i32)(mesh.vert_offset/sizeof(Vertex)),
//           .firstInstance = 0,
//         },
//         .entity_inst_offset = entity_inst_offset,
//       };
//       vk->gpu_draw_call_infos[draw_call_count++] = info;
//       entity_inst_offset += mesh_batch.entities.count;
//       ++per_shader_indexed_draw_count;
//     }
//     if (per_shader_indexed_draw_count > 0) {
//       VK_PushConstant push = {.drawcall_offset = draw_call_mem_offset / (u32)sizeof(VK_DrawCallInfo)};
//       vk->CmdPushConstants(cmd, vk->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(VK_PushConstant), &push);
//       vk->CmdDrawIndexedIndirect(cmd, vk->indirect_draw_buffer.h, draw_call_mem_offset, per_shader_indexed_draw_count, sizeof(VK_DrawCallInfo));
//     }

//     // Entities per shader
//     u32 per_shader_draw_count = 0;
//     for (VK_MeshBatch mesh_batch : batch.batch.mesh_batches) {
//       if (mesh_batch.entities.count == 0) continue;
//       for (Handle<Entity> entity_handle : mesh_batch.entities) {
//         u32 entity_idx = entity_handle.idx();
//         vk->gpu_entities[entity_idx].model = mat4_transform(entity_handle.trans());
//         vk->gpu_entities_indices[entities_draw_count++] = entity_idx;
//       }
//       u32 mesh_idx = mesh_batch.mesh_handle.handle;
//       VK_Mesh mesh = vk->meshes[mesh_idx];
//       VK_DrawCallInfo info = {
//         .draw_command = {
//           .vertexCount = (u32)mesh.vert_count,
//           .instanceCount = mesh_batch.entities.count,
//           .firstVertex = (u32)(mesh.vert_offset/sizeof(Vertex)),
//           .firstInstance = 0,
//         },
//         .entity_inst_offset = entity_inst_offset,
//       };
//       vk->gpu_draw_call_infos[draw_call_count++] = info;
//       entity_inst_offset += mesh_batch.entities.count;
//       ++per_shader_draw_count;
//     }
//     if (per_shader_draw_count > 0) {
//       u32 draw_call_indexed_mem_offset = draw_call_mem_offset + (u32)sizeof(VK_DrawCallInfo)*per_shader_indexed_draw_count;
//       VK_PushConstant push = {.drawcall_offset = draw_call_indexed_mem_offset / (u32)sizeof(VK_DrawCallInfo)};
//       vk->CmdPushConstants(cmd, vk->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(VK_PushConstant), &push);
//       vk->CmdDrawIndirect(cmd, vk->indirect_draw_buffer.h, draw_call_indexed_mem_offset , per_shader_draw_count, sizeof(VK_DrawCallInfo));
//     }
//     draw_call_mem_offset = draw_call_count * sizeof(VK_DrawCallInfo);

//     /////////////////////////////////
//     // Rebuilding static buffer
//     #define MaxDrawCalls (MaxDrawCalls/2)
//     if ((batch.static_entities_count_old != batch.static_entities_count) || (batch.static_entities_indexed_count_old != batch.static_entities_indexed_count)) {
//       batch.static_entities_indexed_count_old = batch.static_entities_indexed_count;
//       batch.static_entities_count_old = batch.static_entities_count;

//       u32 static_draw_call_count = 0;
//       u32 static_entities_draw_count = 0;
//       u32 static_entities_draw_id_offset = 0;

//       // Indexed Entities per shader
//       u32 per_shader_static_indexed_draw_count = 0;
//       for (VK_MeshBatch mesh_batch : batch.static_batch_indexed.mesh_batches) {
//         if (mesh_batch.entities.count == 0) continue;
//         for (Handle<StaticEntity> entity_handle : mesh_batch.entities) {
//           u32 entity_idx = entity_handle.idx();
//           vk->gpu_entities[MaxEntities+entity_idx].model = mat4_transform(entity_handle.trans());
//           vk->gpu_entities_indices[MaxEntities+static_entities_draw_count++] = MaxEntities+entity_idx;
//         }
//         u32 mesh_idx = mesh_batch.mesh_handle.handle;
//         VK_Mesh mesh = vk->meshes[mesh_idx];
//         VK_DrawCallInfo info = {
//           .index_draw_command = {
//             .indexCount = (u32)mesh.index_count,
//             .instanceCount = mesh_batch.entities.count,
//             .firstIndex = (u32)(mesh.index_offset/sizeof(u32)),
//             .vertexOffset = (i32)(mesh.vert_offset/sizeof(Vertex)),
//             .firstInstance = 0,
//           },
//           .entity_inst_offset = MaxEntities+static_entities_draw_id_offset,
//         };
//         vk->gpu_draw_call_infos[MaxDrawCalls+static_draw_call_count++] = info;
//         static_entities_draw_id_offset += mesh_batch.entities.count;
//         ++per_shader_static_indexed_draw_count;
//       }
//       vk->static_draw_indexed_offset = MaxDrawCalls*sizeof(VK_DrawCallInfo);
//       vk->static_draw_offset = vk->static_draw_indexed_offset + per_shader_static_indexed_draw_count*sizeof(VK_DrawCallInfo);

//       // Entities per shader
//       u32 per_shader_static_draw_count = 0;
//       for (VK_MeshBatch mesh_batch : batch.static_batch.mesh_batches) {
//         if (mesh_batch.entities.count == 0) continue;
//         for (Handle<StaticEntity> entity_handle : mesh_batch.entities) {
//           u32 entity_idx = entity_handle.idx();
//           vk->gpu_entities[MaxEntities+entity_idx].model = mat4_transform(entity_handle.trans());
//           vk->gpu_entities_indices[MaxEntities+static_draw_call_count++] = MaxEntities+entity_idx;
//         }
//         u32 mesh_idx = mesh_batch.mesh_handle.handle;
//         VK_Mesh mesh = vk->meshes[mesh_idx];
//         VK_DrawCallInfo info = {
//           .draw_command = {
//             .vertexCount = (u32)mesh.vert_count,
//             .instanceCount = mesh_batch.entities.count,
//             .firstVertex = (u32)(mesh.vert_offset/sizeof(Vertex)),
//             .firstInstance = 0,
//           },
//           .entity_inst_offset = MaxEntities+static_entities_draw_id_offset,
//         };
//         vk->gpu_draw_call_infos[MaxDrawCalls+static_draw_call_count++] = info;
//         static_entities_draw_id_offset += mesh_batch.entities.count;
//         ++per_shader_static_draw_count;
//       }

//       vk->static_indexed_draw_count = per_shader_static_indexed_draw_count;
//       vk->static_draw_count = per_shader_static_draw_count;
//     }
//     if (batch.static_entities_indexed_count) {
//       VK_PushConstant push = {MaxDrawCalls};
//       vk->CmdPushConstants(cmd, vk->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(VK_PushConstant), &push);
//       vk->CmdDrawIndexedIndirect(cmd, vk->indirect_draw_buffer.h, vk->static_draw_indexed_offset, vk->static_indexed_draw_count, sizeof(VK_DrawCallInfo));
//     }
//     if (batch.static_entities_count) {
//       VK_PushConstant push = {MaxDrawCalls+vk->static_indexed_draw_count};
//       vk->CmdPushConstants(cmd, vk->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(VK_PushConstant), &push);
//       vk->CmdDrawIndirect(cmd, vk->indirect_draw_buffer.h, vk->static_draw_offset, vk->static_draw_count, sizeof(VK_DrawCallInfo));
//     }
//     #undef MaxDrawCalls
//   }

//   // Debug drawing
//   if (vk->draw_lines.count > 0) {
//     g.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.debug_line_shader);
//     g.CmdBindVertexBuffers(cmd, 0, 1, &g.vert_buffer.h, (VkDeviceSize*)&g.draw_lines_offset);
//     g.CmdDraw(cmd, g.draw_lines.count*2, 1, 0, 0);
//     g.draw_lines.clear();
//   }
//   if (g.draw_lines_consistent.count > 0) {
//     g.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.debug_line_shader);
//     g.CmdBindVertexBuffers(cmd, 0, 1, &g.vert_buffer.h, (VkDeviceSize*)&g.draw_lines_consistent_offset);
//     g.CmdDraw(cmd, g.draw_lines_consistent.count*2, 1, 0, 0);
//   }

//   // Cube map
//   vk->CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->cubemap_shader);
//   Handle<GpuMesh> h = mesh_get(Mesh_Cube);
//   VK_Mesh mesh = vk->meshes[h.handle];
//   vk->CmdBindVertexBuffers(cmd, 0, 1, &vk->vert_buffer.h, &mesh.vert_offset);
//   if (mesh.index_count) {
//     vk->CmdBindIndexBuffer(cmd, vk->index_buffer.h, mesh.index_offset, VK_INDEX_TYPE_UINT32);
//     vk->CmdDrawIndexed(cmd, mesh.index_count, 1, 0, 0, 0);
//   } else {
//     vk->CmdDraw(cmd, mesh.vert_count, 1, 0, 0);
//   }

//   vk->CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->ui_shader);
//   if (vk->draw_rects.count > 0) {
//     vk->CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->ui_shader);
//     vk->CmdBindVertexBuffers(cmd, 0, 1, &vk->vert_buffer.h, (VkDeviceSize*)&vk->draw_rects_offset);
//     vk->CmdDraw(cmd, vk->draw_rects.count*6, 1, 0, 0);
//     vk->draw_rects.clear();
//   }

//   g.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.triangle);
//   g.CmdDraw(cmd, 3, 1, 0, 0);

// }
