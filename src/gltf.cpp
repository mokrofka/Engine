#include "lib.h"
#include "common.h"

intern Mesh mesh_load_obj(Allocator arena, String name) {
  Scratch scratch(arena);
  Darray<v3> positions(scratch);
  Darray<v3> normals(scratch);
  Darray<v2> uvs(scratch);
  Darray<v3u> indexes(scratch);
  Buffer buf = os_file_path_read_all(scratch, name);
  Lexer lexer = lexer_init({buf.data, buf.size});
  String word;
  while ((word = lexer_next_token(&lexer)).size) {
    // vert
    if (str_match(word, "v")) {
      v3 v = {};
      for (f32& e : v.e) {
        e = f32_from_str(lexer_next_token(&lexer));
      }
      // Info("v %f, %f, %f", v.x, v.y, v.z);
      positions.add(v);
    }
    // norm
    else if (str_match(word, "vn")) {
      v3 v = {};
      for (f32& e : v.e) {
        e = f32_from_str(lexer_next_token(&lexer));
      }
      // Info("vn %f, %f, %f", v.x, v.y, v.z);
      normals.add(v);
    }
    // uv
    else if (str_match(word, "vt")) {
      v2 v = {};
      for (f32& e : v.e) {
        e = f32_from_str(lexer_next_token(&lexer));
      }
      // Info("vt %f, %f", v.x, v.y);
      uvs.add(v);
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
        indexes.add(v);
      }
    }
  }
  Darray<Vertex> vertices(arena);
  Darray<u32> final_indices(arena);
  Map<Vertex, u32> map(scratch);
  for (v3u idx : indexes) {
    Vertex vertex = {
      .pos = positions[idx.x],
      .norm = normals[idx.y],
      .uv = uvs[idx.z],
    };
    u32* found = map.get(vertex);
    if (found) {
      final_indices.add(*found);
    } else {
      u32 new_index = vertices.count;
      vertices.add(vertex);
      final_indices.add(new_index);
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
  Buffer buf = os_file_path_read_all(scratch, name);
  JsonReader r = json_reader_init({buf.data, buf.size});
  struct MeshInfo {
    // u32 pos_idx;
    // u32 norm_idx;
    // u32 uv_idx;
    // u32 indices_idx;
    // b32 arr[10];
    // u32 vert_count[10];
    u32 vert_count;
    u32 index_count;
    Range ranges[10];
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
  Buffer buff1 = os_file_path_read_all(scratch, push_strf(scratch, "%s/%s", model_dir, info.file_name));
  v3* vertices_pos = (v3*)Offset(buff1.data, info.ranges[0].offset);
  v3* vertices_norm = (v3*)Offset(buff1.data, info.ranges[1].offset);
  v2* vertices_uv = (v2*)Offset(buff1.data, info.ranges[2].offset);
  u16* vertices_indices = (u16*)Offset(buff1.data, info.ranges[3].offset);
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
  Buffer buf = os_file_path_read_all(scratch, name);
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
    Range buffer_views[10];
    Primitives primitives;
    String file_name;
    u32 file_size;
  } info = {};
  JsonReader r = json_reader_init({(u8*)&json_chunk->chunk_data, buf.size});
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
