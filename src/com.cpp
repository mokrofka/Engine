#include "com.h"
#include "generated.h"

#include "vk.cpp"
#include "test.cpp"
#include "tokenizer.cpp"

#include "stb_image.h"
#include "stb_truetype.h"

////////////////////////////////////////////////////////////////////////
// @Common

ImGui_DrawList imgui_get_window_drawlist() {
  ImGui_DrawList res = {
    .draw = ImGui::GetWindowDrawList(),
  };
  return res;
}
void imgui_draw_rect(ImGui_DrawList draw, Rng2 rect, v4 col, f32 rounding, ImDrawFlags flags, f32 thickness) {
  draw.draw->AddRect(rect.min, rect.max, u32_from_rgba(col), rounding, flags, thickness);
}
void imgui_draw_rect_filled(ImGui_DrawList draw, Rng2 rect, v4 col, f32 rounding, ImDrawFlags flags) {
  draw.draw->AddRectFilled(rect.min, rect.max, u32_from_rgba(col));
}
void imgui_draw_push_clip_rect(ImGui_DrawList draw, Rng2 rect) {
  draw.draw->PushClipRect(rect.min, rect.max);
}
void imgui_draw_pop_clip_rect(ImGui_DrawList draw) {
  draw.draw->PopClipRect();
}
void imgui_draw_line(ImGui_DrawList draw, v2 p0, v2 p1, v4 col, f32 thickness) {
  draw.draw->AddLine(p0, p1, u32_from_rgba(col));
}
void imgui_draw_text(ImGui_DrawList draw, v2 pos, v4 col, String fmt, ...) {
  Scratch scratch;
  VaList args;
  va_start(args, fmt);
  String formateted = push_strfv(scratch, fmt, args);
  va_end(args);
  draw.draw->AddText(pos, u32_from_rgba(col), (char*)formateted.str, (char*)(formateted.str + formateted.size));
}
void imgui_text(String fmt, ...) {
  Scratch scratch;
  VaList args;
  va_start(args, fmt);
  String formateted = push_strfv(scratch, fmt, args);
  va_end(args);
  ImGui::TextUnformatted((char*)formateted.str);
}
v2 imgui_calc_text_size(String str) {
  return ImGui::CalcTextSize((char*)str.str, (char*)str.str+str.size);
}
void imgui_begin_tab_item(String str) {
  Scratch scratch;
  String str_c = push_str_copy(scratch, str);
  ImGui::BeginTabItem((char*)str_c.str);
}

Vertex cube_vertices[] = {
  // Front face (0, 0, 1)
  {.pos = v3(-1.00, -1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3( 1.00, -1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3( 1.00,  1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3( 1.00,  1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3(-1.00, -1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(0.0f, 0.0f)},

  // Back face (0, 0, -1)
  {.pos = v3( 1.00, -1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3(-1.00, -1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3(-1.00,  1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3( 1.00,  1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3( 1.00, -1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(0.0f, 0.0f)},

  // Left face (-1, 0, 0)
  {.pos = v3(-1.00, -1.00, -1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3(-1.00, -1.00,  1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3(-1.00,  1.00,  1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00,  1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00, -1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3(-1.00, -1.00, -1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},

  // Right face (1, 0, 0)
  {.pos = v3(1.00, -1.00,  1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3(1.00, -1.00, -1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3(1.00,  1.00, -1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(1.00,  1.00, -1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(1.00,  1.00,  1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3(1.00, -1.00,  1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},

  // Bottom face (0, -1, 0)
  {.pos = v3(-1.00, -1.00, -1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3( 1.00, -1.00, -1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3( 1.00, -1.00,  1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3( 1.00, -1.00,  1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3(-1.00, -1.00,  1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3(-1.00, -1.00, -1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(0.0f, 1.0f)},

  // Top face (0, 1, 0)
  {.pos = v3(-1.00,  1.00,  1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3( 1.00,  1.00,  1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3( 1.00,  1.00, -1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3( 1.00,  1.00, -1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00, -1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00,  1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},
};

Vertex triangle_vertices[] = {
  {.pos = v3( 0.0,   0.5, 0), .uv = v2(0.5, 1), .color = v3(1,0,0)},
  {.pos = v3(-0.5,  -0.5, 0), .uv = v2(0.0, 0), .color = v3(0,1,0)},
  {.pos = v3( 0.5,  -0.5, 0), .uv = v2(1.0, 0), .color = v3(0,0,1)},
};

Vertex axis_vertices[] = {
  {.pos = v3_zero(), .color = v3(1,0,0)},
  {.pos = v3(1,0,0), .color = v3(1,0,0)},
  {.pos = v3_zero(), .color = v3(0,1,0)},
  {.pos = v3(0,1,0), .color = v3(0,1,0)},
  {.pos = v3_zero(), .color = v3(0,0,1)},
  {.pos = v3(0,0,1), .color = v3(0,0,1)},
};

u64 hash(Vertex x) { return hash_memory(&x, sizeof(x)); }
b32 equal(Vertex a, Vertex b) { return MemMatchStruct(&a, &b); }

Extern GlobalState* st;

Timer timer_make(f32 interval) {
  Timer timer = {
    .interval = interval,
  };
  return timer;
}

void timer_tick(Timer& t) {
  t.passed += st->dt;
  if (t.passed >= t.interval) {
    t.passed = 0;
  }
}

b32 timer_passed(Timer& t) {
  if (t.passed >= t.interval) {
    return true;
  }
  return false;
}

f64 tsc_to_ms(u64 tsc) {
  return (f64)tsc/cpu_frequency()*1000;
}

f32 get_dt() { return st->dt; }
f32 get_time() { return st->time; }

Entity& get_entity(EntityId id) {
  Assert(id_generation(id) == st->game.entity_id_pool.generations[id_idx(id)])
  return st->game.entities[id_idx(id)];
}

StaticEntity& get_static_entity(StaticEntityId id) {
  Assert(id_generation(id) == st->game.static_entity_id_pool.generations[id_idx(id)]);
  return st->game.static_entities[id_idx(id)];
}

Mesh load_obj(Allocator arena, String name) {
  Scratch scratch(arena);
  var positions = darray_make<v3>(scratch);
  var normals = darray_make<v3>(scratch);
  var uvs = darray_make<v2>(scratch);
  var indexes = darray_make<v3u>(scratch);
  Slice buf = os_file_path_read_all(scratch, name);
  Lexer lexer = lexer_init(str_make(buf));
  String word;
  while ((word = lexer_next_token(&lexer)).size) {
    // vert
    if (str_match(word, "v")) {
      v3 v = {};
      for (f32& e : v.v) {
        e = f32_from_str(lexer_next_token(&lexer));
      }
      // Info("v %f, %f, %f", v.x, v.y, v.z);
      array_push(positions, v);
    }
    // norm
    else if (str_match(word, "vn")) {
      v3 v = {};
      for (f32& e : v.v) {
        e = f32_from_str(lexer_next_token(&lexer));
      }
      // Info("vn %f, %f, %f", v.x, v.y, v.z);
      array_push(normals, v);
    }
    // uv
    else if (str_match(word, "vt")) {
      v2 v = {};
      for (f32& e : v.v) {
        e = f32_from_str(lexer_next_token(&lexer));
      }
      // Info("vt %f, %f", v.x, v.y);
      array_push(uvs, v);
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
        array_push(indexes, v);
      }
    }
  }
  var vertices = darray_make<Vertex>(arena);
  var final_indices = darray_make<u32>(arena);
  var map = map_make<Vertex, u32>(scratch);
  Loop (i, indexes.count) {
    v3u idx = indexes[i];
    Vertex vertex = {
      .pos = positions[idx.x],
      .norm = normals[idx.y],
      .uv = uvs[idx.z],
    };
    Result res = map_get(map, vertex);
    if (res.err) {
      u32 new_index = vertices.count;
      array_push(vertices, vertex);
      array_push(final_indices, new_index);
      map_set(map, vertex, new_index);
    } else {
      array_push(final_indices, res.v);
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

Mesh load_gltf(Allocator arena, String name) {
  Scratch scratch(arena);
  Slice buf = os_file_path_read_all(scratch, name);
  JsonReader r = json_reader_init(str_make(buf));
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

Mesh load_glb(Allocator arena, String name) {
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
  Assert(str_match(str_make((u8*)&header->magic, 4), "glTF"));
  Chunk* json_chunk = (Chunk*)Offset(header, sizeof(FileHeader));
  Assert(str_match(str_make((u8*)&json_chunk->chunk_type, 4), "JSON"));
  Chunk* bin_chunk = (Chunk*)Offset(json_chunk, sizeof(Chunk)-4 + json_chunk->chunk_length);
  Assert(str_match(str_make((u8*)&bin_chunk->chunk_type, 3), "BIN"));
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
  JsonReader r = json_reader_init(str_make((u8*)&json_chunk->chunk_data, buf.count));
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

Texture load_image(String filepath) {
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
// @Assets

global String meshes_strs[] = {
#define X(enum_name, name) [enum_name] = Stringify(name),
  MESH_LIST
#undef X
#define X(enum_name) [enum_name] = Stringify(enum_name),
  MESH_0_LIST
#undef X
};

global String textures_strs[] = {
#define X(enum_name, name) [enum_name] = Stringify(name),
  TEXTURE_LIST
#undef X
};

global String materials_strs[] = {
#define X(enum_name, ...) [enum_name] = Stringify(enum_name),
  MATERIAL_LIST
#undef X
};

GpuMeshId mesh_get(MeshEnum mesh_enum) { return st->meshes_ids[mesh_enum]; }
void mesh_set(MeshEnum mesh_enum, GpuMeshId mesh_id) { 
  st->meshes_ids[mesh_enum] = mesh_id;
  String str = push_str_copy(st->arena, meshes_strs[mesh_enum]);
  st->mesh_id_to_str[mesh_id.v] = str;
  map_set(st->str_to_mesh_id, str, mesh_id);
}
GpuMaterialId material_get(MaterialEnum id) { return st->materials_ids[id]; }

constexpr ShaderState shader_default_state() {
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
    .ambient = v3_splat(1),
    .diffuse = v3_splat(1),
    .specular = v3_splat(1),
    .shininess = 1,
  };
  return props;
}

GpuMeshId mesh_load(String name) {
  GlobalState& g = *st;
  Scratch scratch;
  String filepath = push_strf(scratch, "%s/%s", g.models_dir, name);
  String format = str_skip_last_dot(name);
  Mesh mesh = {};
  if (str_match(format, "glb")) {
    mesh = load_glb(scratch, filepath);
  } else if (str_match(format, "gltf")) {
    mesh = load_gltf(scratch, filepath);
  } else if (str_match(format, "obj")) {
    mesh = load_obj(scratch, filepath);
  } else {
    InvalidPath;
  }
  GpuMeshId handle = vk_mesh_load(mesh);
  return handle;
}

GpuTextureId texture_load(String name) {
  GlobalState& g = *st;
  Scratch scratch;
  String filepath = push_strf(scratch, "%s/%s", g.textures_dir, name);
  Texture texture = load_image(filepath);
  GpuTextureId handle = vk_texture_load(texture);
  return handle;
}

GpuCubemapId cubemap_load(String name) {
  GlobalState& g = *st;
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
    textures[i] = load_image(filepath);
  }
  vk_cubemap_load(textures);
  return {};
}

void assets_load() {
  GlobalState& g = *st;
  var m_load = [&](MeshEnum enum_name, String name) {
    g.meshes_ids[enum_name] = mesh_load(meshes_strs[enum_name]);
    String str = push_str_copy(g.arena, meshes_strs[enum_name]);
    map_set(g.str_to_mesh_id, str, g.meshes_ids[enum_name]);
    g.mesh_id_to_str[id_idx(g.meshes_ids[enum_name])] = str;
  };
#define X(enum_name, name) m_load(enum_name, Stringify(name));
  MESH_LIST
#undef X

  var t_load = [&](TextureEnum enum_name, String name) {
    g.textures_ids[enum_name] = texture_load(textures_strs[enum_name]); \
    String str = push_str_copy(g.arena, textures_strs[enum_name]);
    map_set(g.str_to_texture_id, str, g.textures_ids[enum_name]);
    g.texture_id_to_str[id_idx(g.textures_ids[enum_name])] = str;
  };
#define X(enum_name, name) t_load(enum_name, Stringify(name));
  TEXTURE_LIST
#undef X

  var mat_load = [&](MaterialEnum enum_name, MaterialDesc desc) {
    g.materials_ids[enum_name] = vk_material_load(desc);
    String str = push_str_copy(g.arena, materials_strs[enum_name]);
    map_set(g.str_to_material_id, str, g.materials_ids[enum_name]);
    g.material_id_to_str[id_idx(g.materials_ids[enum_name])] = str;
  };
  mat_load(Material_Orange, {
    .shader = {
      .name = "e_texture",
      .state = shader_default_state(),
    },
    .props = material_default_props(),
    .texture = "orange_lines_512.png",
  });
  mat_load(Material_Container, {
    .shader = {
      .name = "e_texture",
      .state = shader_default_state(),
    },
    .props = material_default_props(),
    .texture = "container.jpg",
  });
  mat_load(Material_Axis, {
    .shader = {
      .name = "e_vert_color",
      .state = {
        .topology = ShaderTopology_Line,
        .samples = 4,
        .use_depth = true,
      },
    },
  });
  mat_load(Material_Line, {
    .shader = {
      .name = "e_color",
      .state = {
        .topology = ShaderTopology_Line,
        .samples = 4,
        .use_depth = true,
      },
    },
  });
  mat_load(Material_Line, {
    .shader = {
      .name = "e_color",
      .state = {
        .topology = ShaderTopology_Line,
        .samples = 4,
        .use_depth = true,
      },
    },
  });
}

////////////////////////////////////////////////////////////////////////
// @Json

// https://github.com/rxi/sj.h.git
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
      if (str_match(str_make(r->cur, 4),  "null")) { r->cur += 4; break; }
      if (str_match(str_make(r->cur, 4),  "true")) { r->cur += 4; break; }
      if (str_match(str_make(r->cur, 5), "false")) { r->cur += 5; break; }
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
  r.base_obj = json_read(&r);
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

////////////////////////////////////////////////////////////////////////
// @Serialization

////////////////////////////////////////////////////////////////////////
// @Input

b32 key_pressed(Key key) {
  if (os_is_key_pressed(key)) {
    if (!st->input.consumed[key]) return true;
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
    if (!st->input.consumed[key]) return true;
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
  st->input.consumed[key] = true;
}

void input_update() {
  ArrayZero(st->input.consumed);
}

////////////////////////////////////////////////////////////////////////
// @UI

ScrollState scroll_state_make(f32 scale) {
  ScrollState res = {
    .scale_level = scale,
    .scale = v2_splat(scale),
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
          s.scale = v2_splat(s.scale_level);
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
  ProfWindow& win = st->profile_win;

  // Avg, min, max
  ProfFrame prev_frame = prof_get_prev_frame(st->current_frame);
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
      if (key_pressed(Key_1)) g.future_active_tab = ProfileTabActive_Root;
      if (key_pressed(Key_2)) g.future_active_tab = ProfileTabActive_Frames;
      if (key_pressed(Key_3)) g.future_active_tab = ProfileTabActive_Time;
      if (key_pressed(Key_4)) g.future_active_tab = ProfileTabActive_LaunchTime;
      if (key_pressed(Key_5)) g.future_active_tab = ProfileTabActive_Memory;
      if (key_pressed(Key_P)) g.paused = !g.paused;
      if (ImGui::IsWindowHovered()) {
        if (key_pressed(Key_V)) {
          imgui_window_toggle_fullscreen(win);
        }
      }

      ImGui::SetWindowFontScale(0.8f);
      if (ImGui::BeginTabBar("MyTabBar")) {
        ImGui_DrawList draw = imgui_get_window_drawlist();
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
                f64 width_t_off = Unlerp((f64)tsc_start, var_tsc_start, tsc_end);
                if (anchor.tsc_elapsed_incl != anchor.tsc_elapsed_excl) {
                  width_t = (f64)anchor.tsc_elapsed_incl / tsc_elapsed;
                }
                f32 height = 30;
                f32 height_off = anchor.depth * height;
                f32 width = width_t * avail_size.x;
                f32 width_off = width_t_off * avail_size.x;
                Rng2 rect = rng2_make(v2(width_off, height_off), v2(width, height));
                UI_Item item = {
                  .type = UI_ItemType_Bar,
                  .rect = rect,
                  .anchor = anchor,
                };
                array_push(items, item);
              }
            }
            array_push(items, {.type = UI_ItemType_NextThread});
          }

          ///////////////////////////////////
          // Anchors and thread offsets
          f32 height_off = 0;
          f32 thread_height_off = 200;
          Loop (i, items.count) {
            UI_Item& item = items[i];
            switch (item.type) {
              case UI_ItemType_Bar: {
                item.rect = rng2_shift(item.rect, v2(width_off, height_off));
              } break;
              case UI_ItemType_NextThread: {
                height_off += thread_height_off;
              } break;
            }
          }

          // Scroll
          Loop (i, items.count) {
            Rng2& rect = items[i].rect;
            rect = rng2_shift(rect, cursor_pos);
            rect = rng2_scale(rect, scroll_state.scale);
            rect = rng2_shift(rect, scroll_state.offset);
          }

          ///////////////////////////////////
          // Drawing
          Loop (i, items.count) {
            UI_Item item = items[i];
            switch (item.type) {
              default:{}break;
              case UI_ItemType_Bar: {
                v4 color = {};
                String str = {};
                ProfAnchor anchor = item.anchor;
                Rng2 rect = item.rect;
                switch (anchor.type) {
                  case ProfType_Default: {
                    color = ColorGreyDark;
                    str = "work";
                  } break;
                  case ProfType_Sleep: {
                    // color = ColorGreen0;
                    color = ColorGreenUi;
                    str = "sleep";
                  } break;
                  case ProfType_Worker: {
                    color = ColorOrangeUi;
                    str = "job";
                  } break;
                }
                imgui_draw_rect_filled(draw, rect, color);
                imgui_draw_rect(draw, rect, ColorGreyLight);
                if (rng2_contains(rect, mouse_pos)) {
                  ImGui::BeginTooltip();
                  imgui_text("Label: %s", anchor.label.str);
                  imgui_text("Percent: %f%%", rng2_dim(rect).x / avail_size.x * 100);
                  imgui_text("Time: %fms", tsc_to_ms(anchor.tsc_elapsed_incl));
                  imgui_text("Time exclusive: %fms", tsc_to_ms(anchor.tsc_elapsed_excl));
                  imgui_text("Type: %s", str.str);
                  ImGui::EndTooltip();
                }

                // Text
                {
                  String str = push_strf(scratch, "%s %.3f", anchor.label, tsc_to_ms(anchor.tsc_elapsed_incl));
                  v2 text_size = imgui_calc_text_size(str);
                  if (rng2_dim(rect).x < 30.1 || scroll_state.scale.y < 0.3) {
                    continue;
                  }
                  v2 text_pos = {};
                  if (text_size.x > rng2_dim(rect).x) {
                    text_pos.x = rect.min.x;
                    text_pos.y = rect.min.y + (rng2_dim(rect).y - text_size.y) * 0.5;
                  } else {
                    text_pos = rng2_align_dim_at_center(rect, text_size).min;
                  }
                  imgui_draw_push_clip_rect(draw, rect);
                  imgui_draw_text(draw, text_pos, ColorWhite, str);
                  imgui_draw_pop_clip_rect(draw);
                }
              } break;
            }
          }
        };

        imgui_text("%.1ffps %.1fms CPU %.1fGhz, Recording: %s", 1000 / tsc_to_ms(tsc_elapsed), tsc_to_ms(tsc_elapsed), (f64)cpu_freq / Billion(1), g.paused ? "off" : "on");
        imgui_text("avg %.1fms, max %.1f, min %.1f", g.frame_avg_time, g.frame_max_time, g.frame_min_time);
        f32 info_height = 60;
        cursor_pos.y += info_height;

        ///////////////////////////////////
        // Draw thread names
        var draw_threads = [&](ScrollState scroll_state) {
          f32 thread_height = 200;
          f32 thread_height_offset = 0;
          f32 text_off_above = -40;
          {
            String str = push_strf(scratch, "Main thread");
            v2 text_pos = (v2(0, text_off_above) + cursor_pos);
            text_pos.y *= scroll_state.scale.y;
            text_pos.y += scroll_state.offset.y;
            imgui_draw_text(draw, text_pos, ColorWhite, str);
          }
          {
            Loop (i, THREAD_COUNT) {
              thread_height_offset += thread_height;
              String str = push_strf(scratch, "Worker %i", i);
              v2 text_pos = v2(0, thread_height_offset + text_off_above) + cursor_pos;
              text_pos.y *= scroll_state.scale.y;
              text_pos.y += scroll_state.offset.y;
              imgui_draw_text(draw, text_pos, ColorWhite, str);
            }
            thread_height_offset = 0;
          }
        };

        ///////////////////////////////////
        // Tab mouse click
        var tab_mouse_click_handle = [&](String name, ProfTabActive tab) {
          Scratch scratch;
          String str = push_strf(scratch, name);
          imgui_begin_tab_item(str);
          Rng2 tab_rect = Rng2(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
          // if (str_match(name, "root")) {
          //   Debug("root");
          //   Info("min: %f %f", tab_rect.min.x, tab_rect.min.y);
          //   Info("max: %f %f", tab_rect.max.x, tab_rect.max.y);
          // }
          // if (str_match(name, "frames")) {
          //   Debug("frames");
          //   Info("min: %f %f", tab_rect.min.x, tab_rect.min.y);
          //   Info("max: %f %f", tab_rect.max.x, tab_rect.max.y);
          // }
          // if (str_match(name, "time")) {
          //   Debug("time");
          //   Info("min: %f %f", tab_rect.min.x, tab_rect.min.y);
          //   Info("max: %f %f", tab_rect.max.x, tab_rect.max.y);
          // }
          // if (str_match(name, "launch")) {
          //   Debug("launch");
          //   Info("min: %f %f", tab_rect.min.x, tab_rect.min.y);
          //   Info("max: %f %f", tab_rect.max.x, tab_rect.max.y);
          // }
          // if (str_match(name, "memory")) {
          //   Debug("memory");
          //   Info("min: %f %f", tab_rect.min.x, tab_rect.min.y);
          //   Info("max: %f %f", tab_rect.max.x, tab_rect.max.y);
          // }
          if (key_pressed(MouseKey_Left)) {
            if (rng2_contains(tab_rect, mouse_pos)) {
              switch (tab) {
                case ProfileTabActive_Root: g.future_active_tab = ProfileTabActive_Root; break;
                case ProfileTabActive_Frames: g.future_active_tab = ProfileTabActive_Frames; break;
                case ProfileTabActive_Time: g.future_active_tab = ProfileTabActive_Time; break;
                case ProfileTabActive_LaunchTime: g.future_active_tab = ProfileTabActive_LaunchTime; break;
                case ProfileTabActive_Memory: g.future_active_tab = ProfileTabActive_Memory; break;
              }
            }
          }
        };

        ///////////////////////////////////
        // Tabs
        tab_mouse_click_handle("root", ProfileTabActive_Root);
        tab_mouse_click_handle("frames", ProfileTabActive_Frames);
        tab_mouse_click_handle("time", ProfileTabActive_Time);
        tab_mouse_click_handle("launch", ProfileTabActive_LaunchTime);
        tab_mouse_click_handle("memory", ProfileTabActive_Memory);
        switch (g.active_tab) {
          case ProfileTabActive_Root: {
            ScrollState& scroll_state = win.root_scroll_state;
            if (ImGui::IsWindowHovered()) {
              ui_handle_scroll(scroll_state, ScrollType_PowClamp);
            }
            cursor_pos.y += 30;
            draw_threads(scroll_state);
            u32 idx = (st->current_frame-1) % ArrayCount(g.frames_times);
            Slice<ProfAnchor> slices[ArrayCount(g.prof_threads)] = {};
            for EachElement(i, g.prof_threads) {
              slices[i] = slice(g.prof_threads[i].recorded_anchors[idx]);
            }
            ProfFrameTime time = g.frames_times[idx];
            draw_frame_graph(ArraySlice(slices), time, 0, scroll_state);
            ImGui::EndTabItem();
          } break;
          case ProfileTabActive_Frames: {
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
              Rng2 rect = rng2_make(min, size);
              if (rng2_contains(rect, mouse_pos)) {
                ImGui::BeginTooltip();
                ImGui::Text("frame: %i", i);
                ImGui::EndTooltip();
                if (os_is_key_pressed(MouseKey_Left)) {
                  win.frames_scroll_state.offset.x = -width_size * i;
                  win.frames_scroll_state.scale = v2_splat(1);
                }
              }
              v4 color = ColorGreen;
              if (rng1_contains(Rng1(17, 21), frame_ms)) {
                color = ColorYellow;
              } else if (frame_ms > 20) {
                color = ColorRed;
              }
              if (i == st->current_frame % ArrayCount(g.frames_times)) {
                color = ColorGrey3;
              }

              if (i == st->current_frame % ArrayCount(g.frames_times)) {
                imgui_draw_rect_filled(draw, rect, color);
              } else {
                imgui_draw_rect_filled(draw, rect, color);
                imgui_draw_rect(draw, rect, v4_set_w(ColorGrey0, 0.3));
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
                imgui_draw_line(draw, p0, p1, ColorGrey3, thick);
                if (i == st->current_frame % ArrayCount(g.frames_times)) {
                  imgui_draw_rect_filled(draw, Rng2(p0, p3), v4(0.4,0.4,0.4,0.4));
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
          } break;
          case ProfileTabActive_Time: {
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

              imgui_draw_rect_filled(draw, rect, ColorGreyDark);
              imgui_draw_rect(draw, rect, ColorGreyLight);

              String name_str = push_strf(scratch, "%s", anchor.label);
              String ms_str = push_strf(scratch, "%.3fms", (f64)anchor.tsc_elapsed_excl / cpu_freq * 1000);
              v2 name_offset = v2(0, height * i) + cursor_pos;
              v2 ms_offset = v2(avail_size.x * 0.82, height * i) + cursor_pos;

              imgui_draw_text(draw, name_offset, ColorWhite, name_str);
              imgui_draw_text(draw, ms_offset, ColorWhite, ms_str);

              ImGui::PopID();
            }
            ImGui::EndTabItem();
            } break;
          case ProfileTabActive_LaunchTime: {
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
          } break;
          case ProfileTabActive_Memory: {
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
            Loop (i, infos_sorted.count) {
              AllocatorInfo* x = infos_sorted[i];
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
                array_push(items, item);
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
                  array_push(items, item);
                }
    
                // Arena
                {
                  UI_Item item = {
                    .type = UI_ItemType_Arena,
                    .rect = layout_row(curs, Rng1(0, avail_size.x), row_h),
                    .info = &info,
                    .mem_level = mem_level,
                  };
                  array_push(items, item);
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
                  array_push(stack, {sorted_children[i], 1});
                }
                while (stack.count) {
                  StackEntry entry = array_pop(stack);
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
                  array_push(items, item);
                  layout_next(curs, row_h * 0.6);
                  if (child->first) {
                    ++depth;
                    Slice sorted_children = sort_list_insert(scratch, child->first, [](var a, var b) { return a->pos > b->pos; });
                    ReverseLoop (i, sorted_children.count) {
                      array_push(stack, {sorted_children[i], entry.depth + 1});
                    }
                  }
                }
              }
            }

            Loop (i, items.count) {
              UI_Item& item = items[i];
              item.rect = rng2_shift(item.rect, cursor_pos);
              item.rect = rng2_scale(item.rect, scroll_state.scale);
              item.rect = rng2_shift(item.rect, scroll_state.offset);
            }

            Rng2 rounding_edge = rng2_shift(rng2_scale(rng2_make(cursor_pos, avail_size), scroll_state.scale.x), scroll_state.offset);
            rounding_edge = rng2_pad(rounding_edge, 10);
            imgui_draw_rect(draw, rounding_edge, ColorGreyLight);

            ///////////////////////////////////
            // Drawing
            Loop (i, items.count) {
              UI_Item item = items[i];
              AllocatorInfo& info = *item.info;

              switch (item.type) {
                case UI_ItemType_MemUsage: {
                  MemFormatSize mem_fmt = mem_format_size(mem_usage);
                  String mem_usage_str = push_strf(scratch, "mem usage: %.2f%s", mem_fmt.size, mem_fmt.format);
                  imgui_draw_text(draw, cursor_pos, ColorWhite, mem_usage_str);
                } break;
                case UI_ItemType_MemLevel: {
                  MemFormatSize mem_fmt = mem_format_size(mem_levels[item.mem_level]);
                  String str = push_strf(scratch, "%.0f%s", mem_fmt.size, mem_fmt.format);
                  v2 text_size = imgui_calc_text_size(str);
                  Rng2 rect = item.rect;
                  Rng2 text_rect = rng2_align_dim_at_center(rect, text_size);
                  Rng2 pad_text_rect = rng2_pad(text_rect, 5);
                  imgui_draw_rect_filled(draw, pad_text_rect, v4(0.3, 3.5, 0.5, 0.5));
                  imgui_draw_text(draw, text_rect.min, ColorWhite, str);
                } break;
                case UI_ItemType_Arena: {
                  f32 t_w = rng2_dim(item.rect).x;
                  f32 t_pos = info.pos / mem_levels[item.mem_level];
                  // f32 t_cap = info.cap / mem_levels[item.mem_level];
                  f32 t_excl = info.exclusive_pos / mem_levels[item.mem_level];
                  f32 w_pos = t_w * t_pos;
                  // f32 w_cap = t_w * t_cap;
                  f32 w_excl = t_w * t_excl;
                  Rng2 excl_rect = rng2_subrng_x(item.rect, Rng1(0, w_excl));
                  Rng2 incl_rect = rng2_subrng_x(item.rect, Rng1(w_excl, w_pos));

                  imgui_draw_rect_filled(draw, excl_rect, ColorGreenUi);
                  imgui_draw_rect(draw, excl_rect, ColorGreyLight);
                  imgui_draw_rect_filled(draw, incl_rect, ColorBlueUi);
                  imgui_draw_rect(draw, incl_rect, ColorGreyLight);

                  MemFormatSize pos = mem_format_size(info.pos);
                  MemFormatSize pos_exclusive = mem_format_size(info.exclusive_pos);
                  MemFormatSize cmt = mem_format_size(info.cmt);

                  String name_str = push_strf(scratch, "%s", info.name);
                  String mem_str = push_strf(scratch, "%.2f%s pos, %.2f%s cmt", pos.size, pos.format, cmt.size, cmt.format);

                  imgui_draw_text(draw, excl_rect.min, ColorWhite, name_str);
                  imgui_draw_text(draw, rng2_subrng_x01(item.rect, Rng1(0.2, 1)).min, ColorWhite, mem_str);
                  
                  if (rng2_contains(rng2_union(incl_rect, excl_rect), mouse_pos)) {
                    ImGui::BeginTooltip();
                    imgui_text(push_strf(scratch, "inclusive: %.2f%s", pos.size, pos.format));
                    imgui_text(push_strf(scratch, "exclusive: %.2f%s", pos_exclusive.size, pos_exclusive.format));
                    ImGui::EndTooltip();
                  }
                } break;
                case UI_ItemType_Child: {
                  f32 t_w = rng2_dim(item.rect).x;
                  f32 t_pos = info.pos / mem_levels[item.mem_level];
                  f32 t_cap = info.cap / mem_levels[item.mem_level];
                  // f32 t_excl = info.exclusive_pos / mem_levels[item.mem_level];
                  f32 w_pos = t_w * t_pos;
                  f32 w_cap = t_w * t_cap;
                  // f32 w_excl = t_w * t_excl;
                  Rng2 child_rect = rng2_subrng_x(item.rect, Rng1(0, w_pos));
                  Rng2 child_rect_cap = rng2_subrng_x(item.rect, Rng1(w_pos, w_cap));

                  // pos
                  imgui_draw_rect_filled(draw, child_rect, ColorGreyDark);
                  imgui_draw_rect(draw, child_rect, ColorGreyLight);
                  
                  // cap
                  imgui_draw_rect_filled(draw, child_rect_cap, ColorRedUi);
                  imgui_draw_rect(draw, child_rect_cap, ColorGrey);

                  MemFormatSize pos = mem_format_size(info.pos);
                  MemFormatSize cap = mem_format_size(info.cap);
                  String child_name_str = push_strf(scratch, "%s", info.name);
                  String child_meta_str = push_strf(scratch, "%.2f%s pos, %.2f%s cap, alloc count: %u, free count: %u, current alloc count: %u", pos.size, pos.format, cap.size, cap.format, info.allocs, info.frees, info.current_allocs);
                  imgui_draw_text(draw, child_rect.min, ColorWhite, child_name_str);
                  imgui_draw_text(draw, rng2_subrng_x01(child_rect, Rng1(0.3, 1)).min, ColorWhite, child_meta_str);
                } break;
              }
            }
            ImGui::EndTabItem();
          } break;
        }
        ImGui::EndTabBar();
      }
    } ImGui::End();
  }

  g.active_tab = g.future_active_tab;
}

String dumb_struct(Allocator arena, Slice<MemberDefinition> members, void* ptr, EntityFlags flags = {}) {
  Scratch scratch(arena);
  var string = dstr_make(arena);
  Loop (i, members.count) {
    MemberDefinition member = members[i];
    u8* member_ptr = Offset(ptr, member.offset);
    switch (members[i].type) {
      default:{} break;
      case MetaType_u32: {
        dstr_push(string, push_strf(scratch, "%s %u\n", member.name, *(u32*)member_ptr));
      } break;
      case MetaType_i32: {
        dstr_push(string, push_strf(scratch, "%s %u\n", member.name, *(i32*)member_ptr));
      } break;
      case MetaType_b32: {
        dstr_push(string, push_strf(scratch, "%s %u\n", member.name, *(b32*)member_ptr));
      } break;
      case MetaType_f32: {
        dstr_push(string, push_strf(scratch, "%s %f\n", member.name, *(f32*)member_ptr));
      } break;
      case MetaType_v2: {
        v2 v = *((v2*)member_ptr);
        dstr_push(string, push_strf(scratch, "%s %f %f\n", member.name, v.x, v.y));
      } break;
      case MetaType_v3: {
        v3 v = *((v3*)member_ptr);
        dstr_push(string, push_strf(scratch, "%s %f %f %f\n", member.name, v.x, v.y, v.z));
      } break;
      case MetaType_Rng3: {
        Rng3 v = *((Rng3*)member_ptr);
        dstr_push(string, push_strf(scratch, "%s %f %f %f %f %f %f\n", member.name, v.min.x,v.min.y,v.min.z, v.max.x,v.max.y,v.max.z));
      } break;
      case MetaType_GpuMeshId: {
        GpuMeshId v = *((GpuMeshId*)member_ptr);
        dstr_push(string, push_strf(scratch, "%s \"%s\"\n", member.name, st->mesh_id_to_str[v.v]));
      } break;
      case MetaType_GpuMaterialId: {
        GpuMaterialId v = *((GpuMaterialId*)member_ptr);
        dstr_push(string, push_strf(scratch, "%s \"%s\"\n", member.name, st->material_id_to_str[v.v]));
      } break;
      case MetaType_String: {
        if (FlagHas(flags, EntityFlag_Referenced)) {
          String v = *((String*)member_ptr);
          dstr_push(string, push_strf(scratch, "%s \"%s\"\n", member.name, v));
        }
      } break;
      case MetaType_EntityFlags: {
        EntityFlags v = *((EntityFlags*)member_ptr);
        dstr_push(string, push_strf(scratch, "%s %u\n", member.name, v));
      } break;
    }
  }
  return string;
}

void dumb_struct_load(Slice<MemberDefinition> members, void* ptr, Parser* parser) {
  Parser& p = *parser;
  while (!tok_match(p, TokenType_CloseBrace)) {
    MemberDefinition member = {};
    Token ident_token = tok_advance(p);
    if (ident_token.type == TokenType_Identifier) {
      Loop (i, members.count) {
        if (str_match(members[i].name, ident_token.str)) {
          member = members[i];
          break;
        }
      }
    }
    switch (member.type) {
      default:{} break;
      case MetaType_u32: {
        Token tok = tok_advance(p);
        *(u32*)Offset(ptr, member.offset) = u32_from_str(tok.str);
      } break;
      case MetaType_i32: {
        *(i32*)Offset(ptr, member.offset) = parse_i32(p);
      } break;
      case MetaType_b32: {
        Token tok = tok_advance(p);
        *(u32*)Offset(ptr, member.offset) = u32_from_str(tok.str);
      } break;
      case MetaType_f32: {
        *(f32*)Offset(ptr, member.offset) = parse_f32(p);
      } break;
      case MetaType_v2: {
        *(v2*)Offset(ptr, member.offset) = v2(parse_f32(p), parse_f32(p));
      } break;
      case MetaType_v3: {
        *(v3*)Offset(ptr, member.offset) = v3(parse_f32(p), parse_f32(p), parse_f32(p));
      } break;
      case MetaType_Rng3: {
        *(Rng3*)Offset(ptr, member.offset) = Rng3(v3(parse_f32(p), parse_f32(p), parse_f32(p)), v3(parse_f32(p), parse_f32(p), parse_f32(p)));
      } break;
      case MetaType_GpuMeshId: {
        Token tok = tok_require(p, TokenType_String);
        Result mesh = map_get(st->str_to_mesh_id, tok.str);
        if (mesh.err) {
          InvalidPath;
        }
        *(GpuMeshId*)Offset(ptr, member.offset) = mesh.v;
      } break;
      case MetaType_GpuMaterialId: {
        Token tok = tok_require(p, TokenType_String);
        Result material = map_get(st->str_to_material_id, tok.str);
        if (material.err) {
          InvalidPath;
        }
        *(GpuMaterialId*)Offset(ptr, member.offset) = material.v;
      } break;
      case MetaType_String: {
        Token tok = tok_require(p, TokenType_String);
        *(String*)Offset(ptr, member.offset) = push_str_copy(st->arena, tok.str);
      } break;
      case MetaType_EntityFlags: {
        *(EntityFlags*)Offset(ptr, member.offset) = (EntityFlags)parse_u32(p);
      } break;
    }
  }
}

////////////////////////////////////////////////////////////////////////
// @Watch

void watch_add(String watch_name, WatchOp op) {
  WatchState& g = st->watch;
  FileProperties props = os_file_path_properties(watch_name);
  WatchFile file_watch = {
    .path = watch_name,
    .modified = props.modified,
    .op = op,
  };
  array_push(g.watches, file_watch);
}

void watch_directory_add(String watch_name, WatchOp op, OS_WatchFlags flags) {
  WatchState& g = st->watch;
  String dir_path = push_strf(g.arena, "%s", watch_name);
  OS_Watch watch = os_watch_open(flags);
  os_watch_attach(watch, dir_path);
  WatchDirectory dir_watch = {
    .path = dir_path,
    .watch = watch,
    .op = op,
  };
  array_push(g.directories, dir_watch);
}

void watch_update() {
  WatchState& g = st->watch;
  Scratch scratch;
  Loop (i, g.watches.count) {
    WatchFile& x = g.watches[i];
    FileProperties props = os_file_path_properties(x.path);
    if (props.modified > x.modified) {
      switch (x.op) {
        case WatchOp_NotifyHotreload: {
          st->should_hotreload = true;
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
          GlobalState& g = *st;
          Scratch scratch;
          String shader_filepath = push_strf(scratch, "%s/%s", g.shader_dir, name);
          String shader_compiled_filepath = push_strf(scratch, "%s/%s%s", g.shader_compiled_dir, str_chop_last_dot(name), String(".spv"));
          StringList list = {};
          str_list_push(scratch, &list, "slangc");
          str_list_push(scratch, &list, shader_filepath);
          str_list_push(scratch, &list, "-target");
          str_list_push(scratch, &list, "spirv");
          str_list_push(scratch, &list, "-g");
          str_list_push(scratch, &list, "-o");
          str_list_push(scratch, &list, shader_compiled_filepath);
          os_process_launch(list);
        } break;
        case WatchOp_ShaderReload: {
          GlobalState& g = *st;
          String shader_name = str_chop_last_dot(name);
          String shader_name_slang = push_strf(scratch, "%s.slang", shader_name);
          String shader_filepath = push_strf(scratch, "%s/%s", g.shader_dir, shader_name_slang);
          String shader_compiled_filepath = push_strf(scratch, "%s/%s", g.shader_compiled_dir, name);
          os_file_path_copy_mtime(shader_filepath, shader_compiled_filepath);
          vk_shader_reload(shader_name);
        } break;
        InvalidDefaultCase break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////
// @State

void com_init() {
  Scratch scratch;

  GlobalState& g = *st;
  estimate_cpu_frequency();
  global_allocator_init();
  os_gfx_init();
  prof_init(g.arena);
  prof_launch_begin();

  {
    ProfBlock("init");
    thread_pool_init(THREAD_COUNT);
    test();

    g.gpa = alloc_seglist_make(g.arena);
    g.asset_path = push_strf(g.arena, "%s/%s", os_get_current_directory(), String("../assets"));
    g.shader_dir = push_str_cat(g.arena, g.asset_path, "/shaders");
    g.shader_compiled_dir = push_str_cat(g.arena, g.shader_dir, "/compiled");
    g.models_dir = push_str_cat(g.arena, g.asset_path, "/models");
    g.textures_dir = push_str_cat(g.arena, g.asset_path, "/textures");
    g.str_to_texture_id = map_make<String, GpuTextureId>(g.gpa);
    g.str_to_mesh_id = map_make<String, GpuMeshId>(g.gpa);
    g.str_to_material_id = map_make<String, GpuMaterialId>(g.gpa);
    g.watch.arena = g.arena;
    watch_directory_add(g.shader_dir, WatchOp_RecompileShader);
    watch_directory_add(g.shader_compiled_dir, WatchOp_ShaderReload);
    g.shader_module_compilation_pids = darray_make<OS_Handle>(g.gpa);
    g.shader_module_compiled_names = vk_shader_compile(scratch);

    ProfWindow& win = g.profile_win;
    win.root_scroll_state = scroll_state_make(1);
    win.frames_scroll_state = scroll_state_make(1);
    win.launch_time_scroll_state = scroll_state_make(1);
    win.mem_scroll_state = scroll_state_make(1);
    win.open = false;

    vk_init();
#if DEAR_IMGUI
    vk_imgui_init();
#endif
    game_init();
    
  }
  prof_launch_end();
}

void com_update() {
  GlobalState& g = *st;
  input_update();
  profiler_view();
  game_view();
  
  if (key_pressed(Key_F1)) g.profile_win.open = !g.profile_win.open;
  if (key_pressed(Key_F2)) g.imgui_demo_open = !g.imgui_demo_open;
  if (key_pressed(Key_F3)) g.game_win.open = !g.game_win.open;

  if (st->imgui_demo_open) {
    ImGui::ShowDemoWindow();
  }

  {
    ProfBlock("push jobs");
    Loop (i, 2) {
      thread_task_push({.func = [](void* ctx) {os_sleep_ms(rand_u32()%2);}}, true);
    }
    TaskId id0 = thread_task_push({.func = [](void* ctx) { os_sleep_ms(2); }});
    TaskId id1 = thread_task_push({.func = [](void* ctx) { os_sleep_ms(2); }});
    TaskId id2 = thread_task_push({.func = [](void* ctx) { os_sleep_ms(2); }});
    TaskId id3 = thread_task_push({.func = [](void* ctx) { os_sleep_ms(2); }});
    thread_wait_task(id0);
    thread_wait_task(id1);
    thread_wait_task(id2);
    thread_wait_task(id3);
  }
}

shared_function void common_main(HotReloadData* data) {
  Scratch scratch;
if (data->ctx == null) {
    Arena arena = arena_make_named("common arena");
    data->ctx = push_struct_zero(arena, GlobalState);
    st = (GlobalState*)data->ctx;
    st->arena = arena;
    {
      u64 start = cpu_timer_now();
      com_init();
      Info("init time: %fms", tsc_to_ms(cpu_timer_now() - start));
    }
#if HOTRELOAD_BUILD
    watch_add(data->lib, WatchOp_NotifyHotreload);
#endif
  }
  if (!st) {
    st = (GlobalState*)data->ctx;
    st->should_hotreload = false;
  }

  GlobalState& g = *st;

  u64 target_fps = Billion(1) / 60;
  u64 last_time = os_now_ns();


  while (!os_window_should_close()) {
    if (st->should_hotreload) {
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
      com_update();
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

////////////////////////////////////////////////////////////////////////
// @Game

Mesh sphere_generate(Allocator arena) {
  u32 lat_steps = 10;
  u32 lon_steps = 10;
  u32 vert_count = lat_steps*lon_steps;
  u32 index_count = (lat_steps - 1) * lon_steps * 6;
  Vertex* vertices = push_array(arena, Vertex, vert_count);
  u32* indices = push_array(arena, u32, index_count);
  f32 lat_step_angle = PI / lat_steps;
  f32 lon_step_angle = 2*PI / lon_steps;
  for (u32 i = 0; i < lat_steps; ++i) {
    f32 lat_angle = -PI/2 + i*lat_step_angle;
    for (u32 j = 0; j < lon_steps; ++j) {
      f32 lon_angle = j * lon_step_angle;
      Vertex vert = {
        .pos.x = Cos(lat_angle) * Cos(lon_angle),
        .pos.y = Sin(lat_angle),
        .pos.z = Cos(lat_angle) * Sin(lon_angle),
        .uv.x = (f32)j / (lon_steps - 1),  // 0 → 1 across longitude
        .uv.y = (f32)i / (lat_steps - 1),  // 0 → 1 from bottom to top
      };
      vertices[i*lon_steps + j] = vert;
    }
  }
  var idx = [&](u32 i, u32 j) {
    return i * lon_steps + j;
  };
  u32 k = 0;
  for (u32 i = 0; i < lat_steps - 1; ++i) {
    for (u32 j = 0; j < lon_steps; ++j) {
      u32 next_j = (j + 1) % lon_steps; // wrap around
      u32 v0 = idx(i, j);
      u32 v1 = idx(i, next_j);
      u32 v2 = idx(i + 1, j);
      u32 v3 = idx(i + 1, next_j);
      indices[k++] = v0;
      indices[k++] = v2;
      indices[k++] = v1;
      indices[k++] = v1;
      indices[k++] = v2;
      indices[k++] = v3;
    }
  }
  u32 north_pole_index = vert_count - 1; // last vertex
  for (u32 j = 0; j < lon_steps; ++j) {
    u32 next_j = (j + 1) % lon_steps;
    indices[k++] = idx(lat_steps - 2, j); // last row before pole
    indices[k++] = north_pole_index;      // pole
    indices[k++] = idx(lat_steps - 2, next_j);
  }
  Mesh mesh = {
    .vert_count = lat_steps * lon_steps,
    .vertices = vertices,
    .index_count = index_count,
    .indices = indices,
  };
  return mesh;
}

Mesh grid_generate(Allocator arena, u32 size, f32 step) {
  Vertex* vertices = push_array(arena, Vertex, size*4);
  v3 pos_offset = v3(-(i32)size/2, 0, -(i32)size/2);
  for (i32 i = 0; i < size; ++i) {
    vertices[i*2].pos = pos_offset + v3(0, 0, i*step);
    vertices[i*2+1].pos = pos_offset + v3(size*step, 0, i*step);
  }
  Vertex* vertical_vertices = vertices + size*2;
  for (i32 i = 0; i < size; ++i) {
    vertical_vertices[i*2].pos = pos_offset + v3(i*step, 0, 0);
    vertical_vertices[i*2+1].pos = pos_offset + v3(i*step, 0, size*step);
  }
  Mesh mesh = {
    .vertices = vertices,
    .vert_count = size*4,
  };
  return mesh;
}

v3 ray_from_camera() {
  v2 mouse_pos = os_get_mouse_pos();
  v2u win_size = os_get_window_size();
  v2 norm_coords = v2(2 * (mouse_pos.x/win_size.x) - 1, 2 * -(mouse_pos.y/win_size.y) + 1);
  v4 clip_coords = v4(norm_coords.x, norm_coords.y, -1, 1);
  v4 eye_coord = mat4_inverse(st->projection) * clip_coords;
  eye_coord = v4(eye_coord.x, eye_coord.y, -1, 0);
  v3 world_coord = v3_of_v4(st->view * eye_coord);
  world_coord = v3_norm(world_coord);
  return world_coord;
}

// b32 ray_intersect_AABB(Ray ray, AABB aabb) {
//   v3 tMin = v3_hadamard_div(aabb.min - ray.origin, ray.dir);
//   v3 tMax = v3_hadamard_div(aabb.max - ray.origin, ray.dir);
//   v3 t1 = v3_less(tMin, tMax);
//   v3 t2 = v3_greater(tMin, tMax);
//   f32 tNear = Max3(t1.x, t1.y, t1.z);
//   f32 tFar = Min3(t2.x, t2.y, t2.z);
//   if (tNear > tFar) {
//     return false;
//   }
//   return true;
// };

EntityId e_alloc_bare() {
  GameState& g = st->game;
  EntityId e_id = {id_pool_push(g.entity_id_pool)};
  Entity& e = get_entity(e_id);
  e = {};
  ++g.entities_count;
  g.id_track_entities[id_idx(e_id.v)] = obj_pool_push(g.all_dynamic_entities, e_id);
  return e_id;
}
EntityId e_alloc(GpuMeshId mesh_id, GpuMaterialId material_id, EntityThing thing = {}) {
  GameState& g = st->game;
  EntityId e_id = {id_pool_push(g.entity_id_pool)};
  Entity& e = get_entity(e_id);
  e = {
    .name = thing.name,
    .flags = thing.flags,
    .scale = v3_one(),
    .mesh_id = mesh_id,
    .material_id = material_id,
  };
  vk_make_renderable(e_id, mesh_id, material_id);
  ++g.entities_count;
  g.id_track_entities[id_idx(e_id.v)] = obj_pool_push(g.all_dynamic_entities, e_id);
  return e_id;
}
EntityId e_alloc(MeshEnum mesh_id, MaterialEnum material_id, EntityThing thing = {}) {
  return e_alloc(mesh_get(mesh_id), material_get(material_id), thing);
}

StaticEntityId e_static_alloc(GpuMeshId mesh_id, GpuMaterialId material_id) {
  GameState& g = st->game;
  StaticEntityId e_id = {id_pool_push(g.static_entity_id_pool)};
  StaticEntity& e = get_static_entity(e_id);
  e = {
    .scale = v3_one(),
    .mesh_id = mesh_id,
    .material_id = material_id,
  };
  vk_make_renderable_static(e_id, mesh_id, material_id);
  ++g.static_entities_count;
  g.id_track_static_entities[id_idx(e_id.v)] = obj_pool_push(g.all_static_entities, e_id);
  return e_id;
}
StaticEntityId e_static_alloc(MeshEnum mesh_id, MaterialEnum material_id) {
  return e_static_alloc(mesh_get(mesh_id), material_get(material_id));
}

void e_free(EntityId e_id) {
  GameState& g = st->game;
  obj_pool_remove(g.all_dynamic_entities, g.id_track_entities[id_idx(e_id.v)]);
  vk_remove_renderable(e_id);
  id_pool_remove(g.entity_id_pool, e_id.v);
  --g.entities_count;
}

void e_static_free(StaticEntityId e_id) {
  GameState& g = st->game;
  obj_pool_remove(g.all_static_entities, g.id_track_static_entities[id_idx(e_id.v)]);
  vk_remove_static_renderable(e_id);
  id_pool_remove(g.static_entity_id_pool, e_id.v);
  --g.static_entities_count;
}

void select_obj() {
  GameState& g = st->game;
  v3 dir = ray_from_camera();
  EntityId e_id = e_alloc(Mesh_Cube, Material_Orange);
  Entity& e = get_entity(e_id);
  // e.pos() = st->cam.pos + v3_norm(mat4_forward(st->cam.view));
  e.pos = g.cam.pos;
  e.scale = v3_splat(0.3);
  e.vel = dir * 4;
}

void camera_init() {
  GameState& g = st->game;
  Camera& cam = g.cam;
  cam = {
    .pos = v3(0,0,5),
    .yaw = -90,
    .fov = 45,
    .speed = 10,
  };
  cam.dir = {
    CosD(cam.yaw) * CosD(cam.pitch),
    SinD(cam.pitch),
    SinD(cam.yaw) * CosD(cam.pitch)
  };
  st->view = mat4_look_at(cam.pos, cam.dir, v3_up());
}

void camera_update() {
  GameState& g = st->game;
  Camera& cam = g.cam;
  v2 win_size = v2_of_v2u(os_get_window_size());
  mat4& projection = st->projection;
  mat4& view = st->view;
  projection = mat4_perspective(degtorad(cam.fov), win_size.x / win_size.y, 0.1f, 10000.0f);

  // Camera rotation
  {
    f32 rotation_speed = 180.0f * get_dt();
    if (os_is_key_down(Key_A)) {
      cam.yaw += -rotation_speed;
    }
    if (os_is_key_down(Key_D)) {
      cam.yaw += rotation_speed;
    }
    if (os_is_key_down(Key_R)) {
      cam.pitch += rotation_speed;
    }
    if (os_is_key_down(Key_F)) {
      cam.pitch += -rotation_speed;
    }
  }

  // Camera movement
  {
    f32 speed = cam.speed*1;
    v3 velocity = {};
    if (os_is_key_down(Key_W)) {
      v3 forward = mat4_forward(view);
      velocity += forward;
    }
    if (os_is_key_down(Key_S)) {
      v3 backward = mat4_backward(view);
      velocity += backward;
    }
    if (os_is_key_down(Key_Q)) {
      v3 left = mat4_left(view);
      velocity += left;
    }
    if (os_is_key_down(Key_E)) {
      v3 right = mat4_right(view);
      velocity += right;
    }
    if (os_is_key_down(Key_Space)) {
      velocity.y += 1.0f;
    }
    if (os_is_key_down(Key_X)) {
      velocity.y -= 1.0f;
    }
    if (os_is_key_down(Key_Shift)) {
      speed *= 20;
    }
    if (os_is_key_down(Key_LAlt)) {
      speed *= 0.1;
    }
    if (velocity != v3_zero()) {
      velocity = v3_norm(velocity);
      cam.pos += velocity * speed * get_dt();
    }
  }
  
  // Camera update
  cam.pitch = Clamp(-89.0f, cam.pitch, 89.0f);
  cam.dir = {
    CosD(cam.yaw) * CosD(cam.pitch),
    SinD(cam.pitch),
    SinD(cam.yaw) * CosD(cam.pitch)
  };
  view = mat4_look_at(cam.pos, cam.dir, v3_up());
}

void scene_init() {
  Scratch scratch;
  GameState& g = st->game;
  camera_init();
  g.rotating_cube_id = e_alloc(Mesh_Cube, Material_Orange, {"rotating_cube", EntityFlag_Referenced});
  g.monkey_id = e_alloc(Mesh_MonkeyGlb, Material_Container, {"monkey", EntityFlag_Referenced});
  Entity& monkey = get_entity(g.monkey_id);
  monkey.aabb = {v3_splat(-1.2), v3_splat(1.2)};
  {
    EntityId triangle_id = e_alloc(Mesh_Triangle, Material_Orange);
    Entity& triangle = get_entity(triangle_id);
    triangle.pos = v3_splat(3);
  }
  {
    EntityId grid_id = e_alloc(Mesh_Grid, Material_Line);
    vk_set_entity_color(grid_id, v4_splat(0.6));
    Entity& grid = get_entity(grid_id);
    grid.pos = v3(0,0,-5);
  }
  {
    g.axis_attached_to_cam_id = e_alloc(Mesh_Axis, Material_Axis, {"axis_attached_to_cam", EntityFlag_Referenced});
  }
  Loop (i, 3) {
    EntityId cube_id = e_alloc(Mesh_Cube, Material_Orange);
    u32 range = 10;
    Entity& cube = get_entity(cube_id);
    cube.pos = v3_rand_rng(-v3_splat(range), v3_splat(range));
  }
#if 1
  // Loop (i, MB(1)-KB(1)) {
  // Loop (i, KB(100)) {
  Loop (i, KB(1)) {
    // Handle<StaticEntity> e = entity_static_create(Mesh_Cube, Material_Orange);
    // u32 range = KB(1);
    // e.pos() = v3_rand_range(-v3_scale(range), v3_scale(range));
    MeshEnum meshes[] = {
      // Mesh_MonkeyGlb,
      // Mesh_Triangle,
      Mesh_Cube,
    };
    MaterialEnum materials[] = {
      Material_Orange,
      // Material_Container,
      // Material_Screen,
    };
    StaticEntityId e_id = e_static_alloc(meshes[rand_rng_u32(0, ArrayCount(meshes)-1)], materials[rand_rng_u32(0, ArrayCount(materials)-1)]);
    array_push(g.static_cubes, e_id);
    u32 range = KB(1);
    StaticEntity& e = get_static_entity(e_id);
    e.pos = v3_rand_rng(-v3_splat(range), v3_splat(range));
    // v3 dir = v3_rand_rng(v3_scale(-1), v3_scale(1));
    // e.pos() = v3_norm(dir) * range;
  }
#endif

  {
    EntityId sphere_id = e_alloc(Mesh_Sphere, Material_Container);
    Entity& sphere = get_entity(sphere_id);
    sphere.pos = v3(0,0,-10);
  }
  {
    // Handle<Entity> e = entity_create(Mesh_Castle, Shader_E_Texture, Material_Castle);
    // e.pos().z = -100;
  }
  {
    // Loop (i, KB(1)) {
    //   Handle<Entity> e = entity_create(Mesh_Cube, Material_Screen);
    //   u32 range = 100;
    //   e.pos() = v3_rand_range(-v3_scale(range), v3_scale(range));
    // }
  }
  {
    // Loop (i, KB(400)) {
    // Loop (i, MB(1)-KB(1)) {
    Loop (i, 0) {
      EntityId e_id = e_alloc(Mesh_Cube, Material_Container);
      u32 range = KB(1);
      Entity& e = get_entity(e_id);
      e.pos = v3_rand_rng(-v3_splat(range), v3_splat(range));
      array_push(g.moving_cubes, e_id);
    }
  }

  Loop (i, 0) {
    MeshEnum meshes[] = {
      Mesh_MonkeyGlb,
      Mesh_Triangle,
      Mesh_Cube,
    };
    MaterialEnum materials[] = {
      // Material_Orange,
      Material_Container,
      // Material_Screen,
    };
    EntityId e_id = e_alloc(meshes[rand_rng_u32(0, ArrayCount(meshes)-1)], materials[rand_rng_u32(0, ArrayCount(materials)-1)]);
    u32 range = 100;
    Entity& e = get_entity(e_id);
    e.pos = v3_rand_rng(-v3_splat(range), v3_splat(range));
    array_push(g.moving_cubes, e_id);
  }
}

void scene_deinit() {
  // st->entity_pool = {};
  // st->entity_pool.clear();
  // arena_clear(&st->arena);
}

void scene_update() {
  Scratch scratch;
  GameState& g = st->game;
  if (key_pressed(MouseKey_Left)) {
    // select_obj();
    // v3 dir = ray_from_camera();
    // vk_draw_line_consistent(g.cam.pos - v3(0,0.1,0), g.cam.pos + dir*100, ColorWhite);
    // v3 max = st->cam.pos + v3_one();
    // v3 min = st->cam.pos - v3_one();
  }
  // moving cube and monkey
  {
    EntityId cube_id = g.rotating_cube_id;
    EntityId monkey_id = g.monkey_id;
    Entity& cube = get_entity(cube_id);
    Entity& monkey = get_entity(monkey_id);
    monkey.pos.x += 0.1 * get_dt();
    cube.pos.x = monkey.pos.x + Sin(get_time()) * 4;
    cube.pos.z = monkey.pos.z + Cos(get_time()) * 4;
    cube.pos.y = monkey.pos.z + Cos(get_time()) * 4;
  }
  {
    Entity& e = get_entity(g.monkey_id);
    vk_draw_cuboid(rng3_shift(e.aabb, e.pos), ColorWhite);
  }
  {
    mat4& view = st->view;
    v3 forward = mat4_forward(view);
    v3 right   = mat4_right(view);
    v3 up      = mat4_up(view);
    f32 dist = 1.0f;
    f32 xoff = 0.3f;
    f32 yoff = 0.3f;
    Entity& axis = get_entity(g.axis_attached_to_cam_id);
    axis.pos = g.cam.pos + forward*dist + right*xoff + up*yoff;
    axis.scale = v3_splat(0.1);
    // axis.scale = v3_scale(1.1);
  }

  ///////////////////////////////////
  // Random creating and moving stuff
  Loop (i, 0) {
    MeshEnum meshes[] = {
      // Mesh_MonkeyGlb,
      // Mesh_Triangle,
      Mesh_Cube,
    };
    MaterialEnum materials[] = {
      Material_Orange,
      // Material_Container,
      // Material_Screen,
    };
    EntityId e_id = e_alloc(meshes[rand_rng_u32(0, ArrayCount(meshes)-1)], materials[rand_rng_u32(0, ArrayCount(materials)-1)]);
    Entity& e = get_entity(e_id);
    u32 range = 100;
    e.pos = v3_rand_rng(-v3_splat(range), v3_splat(range));
    array_push(g.moving_cubes, e_id);
  }
  Loop (i, g.moving_cubes.count) {
    Entity& e = get_entity(g.moving_cubes[i]);
    e.pos += e.vel * get_dt();
    v3 center = {0, 0, 0};
    v3 dir = e.pos - center;
    v3 tangent = v3_norm(v3{-dir.z, 0, dir.x});
    e.vel += tangent * 2.0f * get_dt();
    e.vel += -dir * 0.5f * get_dt();
  }

  // vk_draw_rect(Rng2(v2(100,100), v2(200,200)), v3(1,1,1));
  v4& pos = get_pos();
  f32 speed = 1 * get_dt();
  if (key_down(Key_A)) {
    pos.x -= speed;
  }
  if (key_down(Key_D)) {
    pos.x += speed;
  }

  mat4& mat = get_mat();
  mat = mat4_translate(v3_of_v4(pos));
}

void game_save_state() {
  Scratch scratch;
  GameState& g = st->game;

  Dstring data = dstr_make(scratch);
  dstr_push(data, "Camera {\n");
  dstr_push(data, dumb_struct(scratch, ArraySlice(members_of_Camera), &g.cam));
  dstr_push(data, "}\n");

  {
    var& p = g.all_dynamic_entities;
    for (IndexId node = p.first; node.v != U32_MAX; node = p.data[id_idx(node)].next) {
      Entity& e = get_entity(p.data[id_idx(node)].elem);
      dstr_push(data, "Entity {\n");
      dstr_push(data, dumb_struct(scratch, ArraySlice(members_of_Entity), &e, e.flags));
      dstr_push(data, "}\n");
    }
  }
  {
    var& p = g.all_static_entities;
    for (IndexId node = p.first; node.v != U32_MAX; node = p.data[id_idx(node)].next) {
      StaticEntity& e = get_static_entity(p.data[id_idx(node)].elem);
      dstr_push(data, "StaticEntity {\n");
      dstr_push(data, dumb_struct(scratch, ArraySlice(members_of_StaticEntity), &e));
      dstr_push(data, "}\n");
    }
  }

  OS_Handle file = os_file_open(push_strf(scratch, "%s/saved", os_get_current_directory(), String("saved")), OS_AccessFlag_Write | OS_AccessFlag_Trunc);
  os_file_write(file, data.size, data.str);
  os_file_close(file);
}

void game_load_state() {
  GameState& g = st->game;
  Scratch scratch;
  Slice data = os_file_path_read_all(scratch, push_strf(scratch, "%s/saved", os_get_current_directory(), String("saved")));
  Slice tokens = tokens_from_str(scratch, str_make(data.data, data.size));
  Parser p = parser_make(tokens);

  {
    var& p = g.all_dynamic_entities;
    for (IndexId node = p.first; node.v != U32_MAX; node = p.data[id_idx(node)].next) {
      EntityId e_id = p.data[id_idx(node)].elem;
      e_free(e_id);
    }
  }
  {
    var& p = g.all_static_entities;
    for (IndexId node = p.first; node.v != U32_MAX; node = p.data[id_idx(node)].next) {
      StaticEntityId e_id = p.data[id_idx(node)].elem;
      e_static_free(e_id);
    }
  }

  while (!tok_is_end(p)) {
    Token tok = tok_advance(p);
    switch (tok.type) {
      default:{} break;
      case TokenType_Identifier: {
        if (str_match(tok.str, "Camera")) {
          dumb_struct_load(ArraySlice(members_of_Camera), &g.cam, &p);
        } else if (str_match(tok.str, "Entity")) {
          EntityId e_id = e_alloc_bare();
          Entity& e = get_entity(e_id);
          dumb_struct_load(ArraySlice(members_of_Entity), &e, &p);
          vk_make_renderable(e_id, e.mesh_id, e.material_id);
          if (FlagHas(e.flags, EntityFlag_Referenced)) {
            if (str_match("monkey", e.name)) {
              g.monkey_id = e_id;
            } else if (str_match("axis_attached_to_cam", e.name)) {
              g.axis_attached_to_cam_id = e_id;
            } else if (str_match("rotating_cube", e.name)) {
              g.rotating_cube_id = e_id;
            } 
          }
        } else if (str_match(tok.str, "StaticEntity")) {
          StaticEntity entity = {};
          dumb_struct_load(ArraySlice(members_of_StaticEntity), &entity, &p);
          StaticEntityId e_id = e_static_alloc(entity.mesh_id, entity.material_id);
          StaticEntity&e = get_static_entity(e_id);
          e = entity;
        }
      }
    }
  }
}

void game_view() {
  Scratch scratch;
  GameState& g = st->game;
  ImguiWindow& win = st->game_win;
  Camera& cam = g.cam;
  if (win.open) {
    imgui_window_apply_state(win);
    ImGui::Begin("Game");

    ImGui::ColorEdit4("color picker", g.color.v, ImGuiColorEditFlags_Float);

    if (ImGui::IsWindowHovered()) {
      if (key_pressed(Key_V)) {
        imgui_window_toggle_fullscreen(win);
      }
    }

    v3 pos = cam.pos;
    ImGui::Text("entities: %u, static entities: %u", g.entities_count, g.static_entities_count);
    ImGui::Text("Camera: x: %.2f y: %.2f z: %.2f", pos.x, pos.y, pos.z);
    ImGui::DragFloat("speed", &cam.speed, 1);
    {
      imgui_text(push_str_copy(scratch, dumb_struct(scratch, ArraySlice(members_of_Camera), &g.cam)));
    }
    {
      Entity& e = get_entity(g.axis_attached_to_cam_id);
      imgui_text(push_str_copy(scratch, dumb_struct(scratch, ArraySlice(members_of_Entity), &e, e.flags)));
    }

    if (ImGui::Button("save state")) {
      game_save_state();
    }
    if (ImGui::Button("load state")) {
      game_load_state();
    }
    if (ImGui::Button("clear moving cubes")) {
      Loop (i, g.moving_cubes.count) {
        EntityId e =  g.moving_cubes[i];
        e_free(e);
      }
      array_clear(g.moving_cubes);
    }

    ImGui::End();
  }
}

void game_init() {
  ProfFunc;
  v4& pos = get_pos();
  pos = {};
  GameState& g = st->game;
  Scratch scratch;
  g.arena = arena_make_named("game arena");
  g.gpa = alloc_seglist_make(g.arena, "game gpa");
  g.timer = timer_make(1);

  id_pool_init(g.entity_id_pool);
  id_pool_init(g.static_entity_id_pool);
  g.all_dynamic_entities = obj_pool_linklist_make<EntityId, IndexId>(g.gpa);
  g.all_static_entities = obj_pool_linklist_make<StaticEntityId, IndexId>(g.gpa);
  g.entities = push_array(g.arena, Entity, MaxEntities);
  g.static_entities = push_array(g.arena, StaticEntity, MaxStaticEntities);
  g.moving_cubes = darray_make<EntityId>(g.gpa);
  g.static_cubes = darray_make<StaticEntityId>(g.gpa);
  g.find_entity = map_make<String, EntityId>(g.gpa);

  // Mesh cube_mesh = {.vertices = cube_vertices, .vert_count = ArrayCount(cube_vertices)};
  // mesh_set(Mesh_Cube, vk_mesh_load(cube_mesh));
  Mesh triangle_mesh =  {.vertices = triangle_vertices, .vert_count = ArrayCount(triangle_vertices)};
  mesh_set(Mesh_Triangle, vk_mesh_load(triangle_mesh));
  Mesh grid_mesh = grid_generate(scratch, 100, 1);
  mesh_set(Mesh_Grid, vk_mesh_load(grid_mesh));
  Mesh axis_mesh = {.vertices = axis_vertices, .vert_count = ArrayCount(axis_vertices)};
  mesh_set(Mesh_Axis, vk_mesh_load(axis_mesh));
  Mesh sphere = sphere_generate(scratch);
  mesh_set(Mesh_Sphere, vk_mesh_load(sphere));
  cubemap_load("night_cubemap");
  assets_load();
  scene_init();

  // camera_init();
  // g.cube0 = e_alloc(Mesh_Cube, Material_Container);
  // g.cube0.pos() = v3(1,0,0);
  // g.cube1 = e_alloc(Mesh_Cube, Material_Orange);
  // g.cube1.pos() = v3(-1,0,0);
  // g.monkey = e_alloc(Mesh_MonkeyGlb, Material_Container);
  // g.monkey.pos() = v3(2,0,0);

  // var grid = e_alloc(Mesh_Grid, Material_Line);
  // g.grid = grid;
  // vk_set_entity_color(grid, v4_scale(0.6));
  // grid.pos() = v3(0,0,-5);

  // Loop (i, KB(10)) {
  //   MeshId meshes[] = {
  //     // Mesh_MonkeyGlb,
  //     // Mesh_Triangle,
  //     Mesh_Cube,
  //   };
  //   MaterialId materials[] = {
  //     Material_Orange,
  //     // Material_Container,
  //     // Material_Screen,
  //   };
  //   var e = e_static_alloc(ArrayRand(meshes), ArrayRand(materials));
  //   u32 range = KB(1);
  //   e.pos() = v3_rand_rng(-v3_scale(range), v3_scale(range));
  // }
}

void game_update() {
  ProfFunc;
  GameState& g = st->game;
  timer_tick(g.timer);
  {
    Entity& e = get_entity(g.monkey_id);
    e.pos.x += get_dt() * 1;
    e.pos.y += get_dt() * 0.5;
  }

  // push_array(g.arena, u32, 100);
  if (timer_passed(g.timer)) {
    // push_array(g.gpa, u32, 1000);
    // push_array(g.gpa_arena0, u32, 200);
    // push_array(g.gpa_arena1, u32, 500);
    // push_array(g.gpa_gpa0, u32, 100);
    // push_array(g.gpa_gpa1, u32, 150);
  }
  
  Scratch scratch;
  camera_update();
  if (os_is_key_down(Key_T)) {
    scene_deinit();
    scene_init();
  }
  if (os_is_key_down(Key_Escape)) {
    os_close_window();
  }
  scene_update();
}



