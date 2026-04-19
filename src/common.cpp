#include "common.h"
#include "stb_image.h"
#include "json.cpp"
#include "game.cpp"
#include "vk.cpp"
#include "test.cpp"

Extern GlobalState* g_st;

f32 get_dt() { return g_st->dt; }
f32 get_time() { return g_st->time; }
f32 get_was_hotreload() { return g_st->should_hotreload; }

Transform& entity_transform(Handle<Entity> handle) { 
  Assert(handle.generation() == g_st->game.entity_id_pool.generations[handle.idx()]);
  return g_st->transforms[handle.idx()];
}
Transform& static_entity_transform(Handle<StaticEntity> handle) {
  Assert(handle.generation() == g_st->game.static_entity_id_pool.generations[handle.idx()]);
  return g_st->static_transforms[handle.idx()];
}

Transform& Handle<Entity>::trans() { return entity_transform((Handle<Entity>)handle); }
v3& Handle<Entity>::pos() { return trans().pos; }
v3& Handle<Entity>::rot() { return trans().rot; }
v3& Handle<Entity>::scale() { return trans().scale; }
Entity& Handle<Entity>::get() { return g_st->game.entities[handle & INDEX_MASK]; }
AABB& Handle<Entity>::aabb() { return get().aabb; }
v3& Handle<Entity>::vel() { return get().vel; }

Transform& Handle<StaticEntity>::trans() { return static_entity_transform((Handle<StaticEntity>)handle); }
v3& Handle<StaticEntity>::pos() { return static_entity_transform((Handle<StaticEntity>)handle).pos; }
v3& Handle<StaticEntity>::rot() { return static_entity_transform((Handle<StaticEntity>)handle).rot; }
v3& Handle<StaticEntity>::scale() { return static_entity_transform((Handle<StaticEntity>)handle).scale; }

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

intern Texture texture_image_load(String filepath) {
  Scratch scratch;
  Texture texture = {};
  u32 required_channel_count = 4;
  u32 channel_count;
  Buffer buf = os_file_path_read_all(scratch, filepath);
  u8* data = stbi_load_from_memory(buf.data, buf.size, (i32*)&texture.width, (i32*)&texture.height, (i32*)&channel_count, required_channel_count);
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
// Profiler

ProfileBlock::ProfileBlock(String label_, String func, String str_to_hash) {
  ProfilerState& g = g_st->profiler;
  u32 idx = ModPow2(hash(str_to_hash, hash(func)), KB(4));
  anchor_idx = g.hash_to_indices[idx];
  if (anchor_idx == 0) {
    anchor_idx = g.anchors_count;
    ++g.anchors_count;
  }
  parent_idx = g.profiler_parent;
  ProfileAnchor* anchor = g.anchors + anchor_idx;
  anchor->label = label_;
  old_tsc_elapsed_inclusive = anchor->tsc_elapsed_inclusive;
  g.profiler_parent = anchor_idx;
  start_tsc = cpu_timer_now();

  anchor->depth = g.depth++;
  anchor->tsc_start = start_tsc;
}

ProfileBlock::~ProfileBlock() {
  ProfilerState& g = g_st->profiler;
  u64 elapsed = cpu_timer_now() - start_tsc;
  g.profiler_parent = parent_idx;
  ProfileAnchor* parent = g.anchors + parent_idx;
  ProfileAnchor* anchor = g.anchors + anchor_idx;
  parent->tsc_elapsed_exclusive -= elapsed;
  anchor->tsc_elapsed_exclusive += elapsed;
  anchor->tsc_elapsed_inclusive = old_tsc_elapsed_inclusive + elapsed;
  ++anchor->hit_count;

  --g.depth;
  anchor->tsc_end = cpu_timer_now();
}

void profiler_begin() {
  ProfilerState& g = g_st->profiler;
  if (g.anchors_count == 0) {
    g.anchors_count = 1;
    g.prev_anchors_count = 1;
  }
  g.tsc_start = cpu_timer_now();
  MemZeroArray(g.anchors, g.anchors_count);
  MemZeroArray(g.hash_to_indices, g.anchors_count);
  g.anchors_count = 1;
  // g.map.clear();
}

void profiler_end() {
  ProfilerState& g = g_st->profiler;
  g.tsc_end = cpu_timer_now();
  g.prev_tsc_elapsed = g.tsc_end - g.tsc_start;
  g.prev_tsc_start = g.tsc_start;
  g.prev_tsc_end = g.tsc_end;
  MemCopyArray(g.prev_anchors, g.anchors, g.anchors_count);
  g.prev_anchors_count = g.anchors_count;
}

Slice<ProfileAnchor> profiler_get_anchors() {
  ProfilerState& g = g_st->profiler;
  return Slice(g.prev_anchors+1, g.prev_anchors_count-1);
}

void profiler_view() {
  Scratch scratch;
  GameState& game = g_st->game;
  ProfilerState& profiler = g_st->profiler;
  var& window = profiler.window;
  if (os_is_key_pressed(Key_4)) {
    if (window.fullscreen) {
      ImGui::SetNextWindowPos(window.pos);
      ImGui::SetNextWindowSize(window.size);
    }
    window.fullscreen = !window.fullscreen;
  }
  ImGuiWindowFlags flags = {};
  if (window.fullscreen) {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
  }
  if (ImGui::Begin("Profiler", null, flags)) {
    if (!window.fullscreen) {
      window.pos = ImGui::GetWindowPos();
      window.size = ImGui::GetWindowSize();
    }

    Slice<ProfileAnchor> anchors = profiler_get_anchors();
    u64 cpu_freq = cpu_frequency();
    u64 tsc_total_elapsed = profiler.prev_tsc_elapsed;
    u64 tsc_start = profiler.prev_tsc_start;
    u64 tsc_end = profiler.prev_tsc_end;

    if (ImGui::IsWindowHovered()) {
      f32 wheel = os_get_scroll();
      if (wheel) {
        f32 zoom_factor = wheel > 0 ? 1.1 : 0.8;
        window.scale_x *= zoom_factor;
        window.scale_y *= zoom_factor;
      }
      f32 move_x = os_get_touchpad();
      if (move_x) {
        f32 offset_factor = move_x < 0 ? 1 : -1;
        window.offset_x += offset_factor * 100;
      }
    }

    ImGui::Text("%u moving cubes count", game.moving_cubes.count);
    ImGui::Text("%.1ffps %0.1fms CPU %.1fGhz", 1/get_dt(), 1000*(f64)tsc_total_elapsed/cpu_freq, (f64)cpu_freq/Billion(1));
    // ImGui::Text("%.1ffps %0.1fms CPU %.1fGhz", 1.0/(3.0/1000), 1000*(f64)tsc_total_elapsed/cpu_freq, (f64)cpu_freq/Billion(1));
    // v2 avail_size = ImGui::GetContentRegionAvail();
    // v2 avail_size = ImGui::GetWindowSize() * g.scale_x;
    v2 avail_size = ImGui::GetWindowSize() * window.scale_x;
    v2 draw_offset_min = ImGui::GetItemRectMin();
    v2 draw_offset_max = ImGui::GetItemRectMax();
    v2 mouse_pos = os_get_mouse_pos();
    Loop (i, anchors.count) {
      ImGui::PushID(i);
      ProfileAnchor anchor = anchors[i];
      f64 width_percent = (f64)anchor.tsc_elapsed_inclusive / tsc_total_elapsed;
      // f64 width_percent_offset = inverse_lerp_f64(tsc_start, anchor.tsc_start, tsc_end);
      f64 width_percent_offset = inverse_lerp_f64(tsc_start, anchor.tsc_start, tsc_end);
      f64 width_percent_with_children = 0;
      f32 width = avail_size.x;
      f32 height = 30;
      if (anchor.tsc_elapsed_inclusive != anchor.tsc_elapsed_exclusive) {
        width_percent_with_children = ((f64)anchor.tsc_elapsed_inclusive / (f64)tsc_total_elapsed);
        width *= width_percent_with_children;
      } else {
        width *= width_percent;
      }
      width *= window.scale_x;
      height *= window.scale_y;
      if (width_percent > 0.02 || width_percent_with_children > 0.02) {
        v2 offset = v2(avail_size.x * width_percent_offset, anchor.depth * height) + draw_offset_min;
        offset.x += window.offset_x;
        offset.y += draw_offset_max.y - draw_offset_min.y;
        v2 size = v2(width, height);
        Rect rect = Rect(offset, size + offset);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->AddRectFilled(rect.min, rect.max, IM_COL32(50, 50, 50, 255));
        draw->AddRect(rect.min, rect.max, IM_COL32(200, 200, 200, 255));
        String str = {};
        if (v2_in_rect(rect, mouse_pos)) {
          ImGui::BeginTooltip();
          ImGui::Text("Label: %s", anchor.label.str);
          ImGui::Text("Percent: %f%%", width_percent * 100);
          ImGui::Text("Hits: %lu", anchor.hit_count);
          ImGui::Text("Time: %fms", (f64)anchor.tsc_elapsed_inclusive / cpu_freq * 1000);
          ImGui::EndTooltip();
        }
        if (width_percent > 0.05 || width_percent_with_children > 0.05) {
          str = push_strf(scratch, "%s %.3f", anchor.label, (f64)anchor.tsc_elapsed_inclusive / cpu_freq * 1000);
          v2 text_size = ImGui::CalcTextSize((char*)str.str);
          v2 text_pos = {};
          if (text_size.x > size.x) {
            text_pos.x = rect.min.x;
            text_pos.y = rect.y + (size.y - text_size.y) * 0.5;
          } else {
            text_pos = v2(
              rect.x + (size.x - text_size.x) * 0.5f,
              rect.y + (size.y - text_size.y) * 0.5
            );
          }
          draw->PushClipRect(rect.min, rect.max); 
          draw->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), (char*)str.str);
          draw->PopClipRect();
        }
      }
      ImGui::PopID();
    }
  } ImGui::End();
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
  g.watches.add(file_watch);
}

void watch_directory_add(String watch_name, WatchOp op, OS_WatchFlags flags) {
  WatchState& g = g_st->watch;
  String dir_path = push_strf(g.alloc, "%s", watch_name);
  OS_Watch watch = os_watch_open(flags);
  os_watch_attach(watch, dir_path);
  WatchDirectory dir_watch = {
    .path = dir_path,
    .watch = watch,
    .op = op,
  };
  g.directories.add(dir_watch);
}

void watch_update() {
  WatchState& g = g_st->watch;
  Scratch scratch;
  for (WatchFile& x : g.watches) {
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
  for (WatchDirectory x : g.directories) {
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

////////////////////////////////////////////////////////////////////////
// UI

void ui_begin() {
  UI_State& g = g_st->ui;
  g.last_hot = g.hot;
  g.hot = 0;
}

void ui_end() {
  UI_State g = g_st->ui;
  if (os_is_key_released(MouseKey_Left)) {
    g.active = 0;
  }
}

void ui_push_box(String str) {
  UI_State& g = g_st->ui;

  UI_Box& parent = g.boxes[g.boxes_count];
  ++g.boxes_count;

  u64 hash_idx = hash(str);
  UI_Box box = {
    .pos = v2(parent.pos + parent.size),
    .size = {100 + parent.size.x, 100 + parent.pos.y},
    .hash = (hash(hash_idx, parent.hash))
  };
  g.boxes[g.boxes_count] = box;

  ui_button(box.hash, box.pos, box.pos+box.size);
}

void ui_pop_box() {
  UI_State& g = g_st->ui;
  if (g.boxes_count > 0) {
    --g.boxes_count;
  }
}

b32 ui_begin_window(u32 id, v2 size) {
  UI_State& g = g_st->ui;
  v2& pos = g.windows[id].pos;
  v2 mouse = os_get_mouse_pos();
  Rect title_rect(pos, v2(pos.x + size.x, pos.y + 20));

  b32 hovered = v2_in_rect(title_rect, mouse);
  if (hovered) {
    g.hot = id;
  }

  // PRESS → start dragging
  if (g.last_hot == id && os_is_key_pressed(MouseKey_Left)) {
    g.active = id;
    g.active_window = id;

    // store offset
    g.drag_offset = v2(mouse.x - pos.x, mouse.y - pos.y);
  }

  // DRAG
  if (g.active == id && os_is_key_down(MouseKey_Left)) {
    pos.x = mouse.x - g.drag_offset.x;
    pos.y = mouse.y - g.drag_offset.y;
  }

  // RELEASE
  if (g.active == id && os_is_key_released(MouseKey_Left)) {
    g.active = 0;
  }

  //  Draw window body
  vk_draw_quad(pos, v2(pos.x + size.x, pos.y + size.y), v3(0.2f,0.2f,0.2f));

  //  Draw title bar
  v3 title_color = v3(0.3f,0.3f,0.3f);
  if (g.hot == id) title_color = v3(0.4f,0.4f,0.4f);
  if (g.active == id) title_color = v3(0.2f,0.2f,0.2f);

  title_rect = {pos, v2(pos.x + size.x, pos.y + 20)};
  vk_draw_quad(title_rect.min, title_rect.max, title_color);

  return true;
}

b32 ui_button(u32 id, v2 min, v2 max) {
  UI_State& g = g_st->ui;
  b32 hovered = v2_in_rect({min, max}, os_get_mouse_pos());
  if (hovered) {
    g.hot = id;
  }

  if (g.last_hot == id && os_is_key_pressed(MouseKey_Left)) {
    g.active = id;
  }

  b32 clicked = 0;
  if (g.active == id && os_is_key_released(MouseKey_Left)) {
    if (g.hot == id) {
      clicked = true;
    }
    g.active = 0;
  }

  v3 color = {0.6f, 0.6f, 0.6f};
  if (g.hot == id) color = v3(0.8f, 0.8f, 0.8f);
  if (g.active == id) color = v3(0.4f, 0.4f, 0.4f);

  vk_draw_quad(min, max, color);
  return clicked;
}

void common_init() {
  GlobalState& g = *g_st;
  global_allocator_init();
  test();
  os_gfx_init();
  estimate_cpu_frequency();
  g.arena = arena_init();
  g.asset_path = push_strf(g.arena, "%s/%s", os_get_current_directory(), String("../assets"));
  g.shader_dir = push_str_cat(g.arena, g.asset_path, "/shaders");
  g.shader_compiled_dir = push_str_cat(g.arena, g.shader_dir, "/compiled");
  g.models_dir = push_str_cat(g.arena, g.asset_path, "/models");
  g.textures_dir = push_str_cat(g.arena, g.asset_path, "/textures");
  watch_directory_add(g.shader_dir, WatchOp_RecompileShader);
  watch_directory_add(g.shader_compiled_dir, WatchOp_ShaderReload);
  g.transforms = push_array(g.arena, Transform, MaxEntities);
  g.static_transforms = push_array(g.arena, Transform, MaxStaticEntities);
  g.vk_st = vk_init();
  g.profiler.window = {
    .scale_x = 1,
    .scale_y = 1,
  };
#if DEAR_IMGUI
  vk_imgui_init();
#endif
  game_init();
}

void common_update() {
  profiler_view();
}

void foo_m() {
  g_st->should_hotreload = true;
}

shared_function void common_main(HotReloadData* data) {
  Scratch scratch;
if (data->ctx == null) {
    Arena arena = arena_init();
    data->ctx = push_struct_zero(arena, GlobalState);
    g_st = (GlobalState*)data->ctx;
    g_st->arena = arena;
    common_init();
#if HOTRELOAD_BUILD
    watch_add(data->lib, WatchOp_NotifyHotreload);
#endif
  }
  if (!g_st) {
    g_st = (GlobalState*)data->ctx;
    // g_st->foo_m = foo_m;
    vk_hotreload(g_st->vk_st);
    g_st->should_hotreload = false;
  }

  GlobalState& g = *g_st;

  u64 target_fps = Billion(1) / 60;
  u64 last_time = os_now_ns();
  Timer timer = timer_init(1);

  while (!os_window_should_close()) {
    if (g_st->should_hotreload) {
      goto hotreload;
    }

    profiler_begin();
    {TimeBlock("frame");
    os_pump_messages();
    u64 start_time = os_now_ns();
    g.dt = f64(start_time - last_time) / Billion(1);
    g.time += g.dt;
    last_time = start_time;
    vk_begin_draw_frame();
    ui_begin();
    common_update();
    game_update();
    ui_end();
    vk_end_draw_frame();
    os_input_update();
    watch_update();

    u64 frame_duration = os_now_ns() - start_time;
    if (frame_duration < target_fps) {
      u64 sleep_time = target_fps - frame_duration;
      TimeBlock("Sleep");
      os_sleep_ms(sleep_time / Million(1));
    }
    if (timer_tick(timer)) {
      Info("Frame rate: %f, %f fps", g.dt, 1/g.dt);
    }
    }
    profiler_end();
  }

  // deinit
  // vk_shutdown();
  // os_gfx_shutdown();
  os_exit(0);

  hotreload:
}

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

void insert_sort(i32* arr, i32 size) {
  for (i32 i = 1; i < size; ++i) {
    i32 key = arr[i];
    i32 j = i - 1;
    while (j >= 0 && arr[j] > key) {
      arr[j + 1] = arr[j];
      j--;
    }
    arr[j + 1] = key;
  }
}

static i32 partition(i32 *arr, i32 low, i32 high) {
  i32 pivot = arr[high]; // choose last element as pivot
  i32 i = low;           // place for next smaller element
  for (i32 j = low; j < high; ++j) {
    if (arr[j] < pivot) {
      Swap(arr[i], arr[j]);
      i++;
    }
  }
  Swap(arr[i], arr[high]); // place pivot in correct position
  return i;
}

void quick_sort(i32* arr, i32 low, i32 high) {
  if (low < high) {
    i32 p = partition(arr, low, high);
    quick_sort(arr, low, p - 1);
    quick_sort(arr, p + 1, high);
  }
}
