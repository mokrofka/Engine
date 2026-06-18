#include "com.h"

#include "gfx.cpp"
#include "render.cpp"
#include "test.cpp"
#include "tokenizer.cpp"
#include "debug.cpp"
#include "generated.cpp"

#include "stb_image.h"
#include "stb_truetype.h"

////////////////////////////////////////////////////////////////////////
// @Common

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

b32 timer_passed(Timer& t) {
  t.passed += get_dt();
  if (t.passed >= t.interval) {
    t.passed = 0;
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
  return pool_get(st->game.entities, id);
}

Transform get_entity_transform(EntityId id) {
  Entity& e = get_entity(id);
  Transform res = {
    .pos = e.pos,
    .rot = e.rot,
    .scale = e.scale,
  };
  return res;
}

StaticEntity& get_static_entity(StaticEntityId id) {
  return pool_get(st->game.static_entities, id);
}

Transform get_static_entity_transform(StaticEntityId id) {
  StaticEntity& e = get_static_entity(id);
  Transform res = {
    .pos = e.pos,
    .rot = e.rot,
    .scale = e.scale,
  };
  return res;
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
    Result vert_index_r = map_get(map, vertex);
    if (vert_index_r.err) {
      u32 new_index = vertices.count;
      array_push(vertices, vertex);
      array_push(final_indices, new_index);
      map_set(map, vertex, new_index);
    }
    u32 vert_index = vert_index_r;
    if (!vert_index_r.err) {
      array_push(final_indices, vert_index);
    }
  }
  Mesh mesh = {
    .vertices = slice(vertices),
    .indices = slice(final_indices),
  };
  return mesh;
}

#define Gltf_i8  5120
#define Gltf_u8  5121
#define Gltf_i16 5122
#define Gltf_u16 5123
#define Gltf_u32 5125
#define Gltf_f32 5126

Mesh load_gltf(Allocator arena, String path, b32 is_glb) {
  Scratch scratch(arena);

  String json = {};
  Slice<u8> cursor = {};
  if (is_glb) {
    struct FileHeader {
      u32 magic;
      u32 version;
      u32 size;
    };
    struct ChunkHeader {
      u32 chunk_length;
      u32 chunk_type;
    };
    cursor = os_file_path_read_all(scratch, path);

    FileHeader* header = (FileHeader*)cursor.data;
    Assert(str_match(str_make((u8*)&header->magic, 4), "glTF"));
    cursor = slice_skip(cursor, sizeof(FileHeader));

    ChunkHeader* json_chunk = (ChunkHeader*)cursor.data;
    Assert(str_match(str_make((u8*)&json_chunk->chunk_type, 4), "JSON"));
    cursor = slice_skip(cursor, sizeof(ChunkHeader));
    json = str_make(slice_prefix(cursor, json_chunk->chunk_length));
    cursor = slice_skip(cursor, json_chunk->chunk_length);

    ChunkHeader* bin_chunk = (ChunkHeader*)cursor.data;
    Assert(str_match(str_make((u8*)&bin_chunk->chunk_type, 3), "BIN"));
    cursor = slice_skip(cursor, sizeof(ChunkHeader));

  } else {
    json = os_file_path_read_all_str(scratch, path);
  }

  struct Accessor {
    u32 buffer_view;
    u32 count;
  };

  struct {
    u32 pos_attribute;
    u32 norm_attribute;
    u32 texcoord_attribute;
    u32 indices_attribute;
    Accessor pos_accessor;
    Accessor norm_accessor;
    Accessor texcoord_accessor;
    Accessor indices_accessor;
    Region pos_buffer_view;
    Region norm_buffer_view;
    Region texcoord_buffer_view;
    Region indices_buffer_view;
  } gltf;

  JsParser p = js_parse_make(scratch, json);
  JsObj root = js_parse(&p).obj;
  {
    Slice meshes = js_get_array(root, "meshes");
    JsObj mesh = meshes[0]->obj;
    {
      Slice primitives = js_get_array(mesh, "primitives");
      JsObj first = primitives[0]->obj;
      {
        JsObj attributes = js_get_obj(first, "attributes");
        gltf.pos_attribute = js_get_number(attributes, "POSITION");
        gltf.norm_attribute = js_get_number(attributes, "NORMAL");
        gltf.texcoord_attribute = js_get_number(attributes, "TEXCOORD_0");
      }
      gltf.indices_attribute = js_get_number(first, "indices");
    }
  }
  {
    Slice accessors = js_get_array(root, "accessors");
    JsObj pos_accessor = accessors[gltf.pos_attribute]->obj;
    gltf.pos_accessor.buffer_view = js_get_number(pos_accessor, "bufferView");
    gltf.pos_accessor.count = js_get_number(pos_accessor, "count");
    JsObj norm_accessor = accessors[gltf.norm_attribute]->obj;
    gltf.norm_accessor.buffer_view = js_get_number(norm_accessor, "bufferView");
    gltf.norm_accessor.count = js_get_number(norm_accessor, "count");
    JsObj texcoord_accessor = accessors[gltf.texcoord_attribute]->obj;
    gltf.texcoord_accessor.buffer_view = js_get_number(texcoord_accessor, "bufferView");
    gltf.texcoord_accessor.count = js_get_number(texcoord_accessor, "count");
    JsObj indices_accessor = accessors[gltf.indices_attribute]->obj;
    gltf.indices_accessor.buffer_view = js_get_number(indices_accessor, "bufferView");
    gltf.indices_accessor.count = js_get_number(indices_accessor, "count");
  }
  {
    Slice buffer_views = js_get_array(root, "bufferViews");
    JsObj pos_buffer_view = buffer_views[gltf.pos_accessor.buffer_view]->obj;
    gltf.pos_buffer_view.size = js_get_number(pos_buffer_view, "byteLength");
    gltf.pos_buffer_view.offset = js_get_number(pos_buffer_view, "byteOffset");
    JsObj norm_buffer_view = buffer_views[gltf.norm_accessor.buffer_view]->obj;
    gltf.norm_buffer_view.size = js_get_number(norm_buffer_view, "byteLength");
    gltf.norm_buffer_view.offset = js_get_number(norm_buffer_view, "byteOffset");
    JsObj texcoord_buffer_view = buffer_views[gltf.texcoord_accessor.buffer_view]->obj;
    gltf.texcoord_buffer_view.size = js_get_number(texcoord_buffer_view, "byteLength");
    gltf.texcoord_buffer_view.offset = js_get_number(texcoord_buffer_view, "byteOffset");
    JsObj indices_buffer_view = buffer_views[gltf.indices_accessor.buffer_view]->obj;
    gltf.indices_buffer_view.size = js_get_number(indices_buffer_view, "byteLength");
    gltf.indices_buffer_view.offset = js_get_number(indices_buffer_view, "byteOffset");
  }
  String file_path = str_chop_last_dot(path);
  String file_path_bin = push_strf(scratch, "%s.bin", file_path);
  Slice data = is_glb ? cursor : os_file_path_read_all(scratch, file_path_bin);
  Slice pos = slice_reinterpret<v3>(slice(data, gltf.pos_buffer_view.offset, gltf.pos_buffer_view.offset + gltf.pos_buffer_view.size));
  Slice norm = slice_reinterpret<v3>(slice(data, gltf.norm_buffer_view.offset, gltf.norm_buffer_view.offset + gltf.norm_buffer_view.size));
  Slice texcoord = slice_reinterpret<v2>(slice(data, gltf.texcoord_buffer_view.offset, gltf.texcoord_buffer_view.offset + gltf.texcoord_buffer_view.size));
  Slice indices_u16 = slice_reinterpret<u16>(slice(data, gltf.indices_buffer_view.offset, gltf.indices_buffer_view.offset + gltf.indices_buffer_view.size));

  Slice vertices = push_slice(arena, Vertex, gltf.pos_accessor.count);
  Slice indices = push_slice(arena, u32, gltf.indices_accessor.count);
  Loop (i, indices.count) {
    indices[i] = indices_u16[i];
  }
  Loop (i, vertices.count) {
    vertices[i] = {
      .pos = pos[i],
      .norm = norm[i],
      .uv = texcoord[i],
    };
  }
  Mesh mesh = {
    .vertices = vertices,
    .indices = indices,
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
  st->mesh_id_to_str[mesh_id.idx] = str;
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
    mesh = load_gltf(scratch, filepath, true);
  } else if (str_match(format, "gltf")) {
    mesh = load_gltf(scratch, filepath, false);
  } else if (str_match(format, "obj")) {
    mesh = load_obj(scratch, filepath);
  } else {
    InvalidPath;
  }
  GpuMeshId handle = r_mesh_load(mesh);
  return handle;
}

GpuTextureId texture_load(String name) {
  GlobalState& g = *st;
  Scratch scratch;
  String filepath = push_strf(scratch, "%s/%s", g.textures_dir, name);
  Texture texture = load_image(filepath);
  GpuTextureId handle = r_texture_load(texture);
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
  r_cubemap_load(textures);
  return {};
}

void assets_load() {
  GlobalState& g = *st;
  var m_load = [&](MeshEnum enum_name, String name) {
    g.meshes_ids[enum_name] = mesh_load(meshes_strs[enum_name]);
    String str = push_str_copy(g.arena, meshes_strs[enum_name]);
    map_set(g.str_to_mesh_id, str, g.meshes_ids[enum_name]);
    g.mesh_id_to_str[g.meshes_ids[enum_name].idx] = str;
  };
#define X(enum_name, name) m_load(enum_name, Stringify(name));
  MESH_LIST
#undef X

  var t_load = [&](TextureEnum enum_name, String name) {
    g.textures_ids[enum_name] = texture_load(textures_strs[enum_name]); \
    String str = push_str_copy(g.arena, textures_strs[enum_name]);
    map_set(g.str_to_texture_id, str, g.textures_ids[enum_name]);
    g.texture_id_to_str[g.textures_ids[enum_name].idx] = str;
  };
#define X(enum_name, name) t_load(enum_name, Stringify(name));
  TEXTURE_LIST
#undef X

  var mat_load = [&](MaterialEnum enum_name, MaterialDesc desc) {
    g.materials_ids[enum_name] = r_material_load(desc);
    String str = push_str_copy(g.arena, materials_strs[enum_name]);
    map_set(g.str_to_material_id, str, g.materials_ids[enum_name]);
    g.material_id_to_str[g.materials_ids[enum_name].idx] = str;
  };
  mat_load(Material_Orange, {
    .shader_name = "e_texture",
    .pipeline_desc = {
      .depth = {
        .compare = Gfx_CompareOp_Less,
        .write_enabled = true,
      }
    },
    .props = material_default_props(),
    .texture = "orange_lines_512.png",
  });
  mat_load(Material_Container, {
    .shader_name = "e_texture",
    .pipeline_desc = {
      .depth = {
        .compare = Gfx_CompareOp_Less,
        .write_enabled = true,
      }
    },
    .props = material_default_props(),
    .texture = "container.jpg",
  });
  mat_load(Material_Axis, {
    .shader_name = "e_vert_color",
    .pipeline_desc = {
      .primitive_type = Gfx_PrimitiveType_Line,
      .depth = {
        .compare = Gfx_CompareOp_Less,
        .write_enabled = true,
      }
    },
    .props = material_default_props(),
  });
  mat_load(Material_Line, {
    .shader_name = "e_color",
    .pipeline_desc = {
      .primitive_type = Gfx_PrimitiveType_Line,
      .depth = {
        .compare = Gfx_CompareOp_Less,
        .write_enabled = true,
      }
    },
    .props = material_default_props(),
  });

  // mat_load(Material_Orange, {
  //   .shader = {
  //     .name = "e_texture",
  //     .state = shader_default_state(),
  //   },
  //   .props = material_default_props(),
  //   .texture = "orange_lines_512.png",
  // });
  // mat_load(Material_Container, {
  //   .shader = {
  //     .name = "e_texture",
  //     .state = shader_default_state(),
  //   },
  //   .props = material_default_props(),
  //   .texture = "container.jpg",
  // });
  // mat_load(Material_Axis, {
  //   .shader = {
  //     .name = "e_vert_color",
  //     .state = {
  //       .topology = ShaderTopology_Line,
  //       .samples = 4,
  //       .use_depth = true,
  //     },
  //   },
  // });
  // mat_load(Material_Line, {
  //   .shader = {
  //     .name = "e_color",
  //     .state = {
  //       .topology = ShaderTopology_Line,
  //       .samples = 4,
  //       .use_depth = true,
  //     },
  //   },
  // });
  // mat_load(Material_Line, {
  //   .shader = {
  //     .name = "e_color",
  //     .state = {
  //       .topology = ShaderTopology_Line,
  //       .samples = 4,
  //       .use_depth = true,
  //     },
  //   },
  // });
}

////////////////////////////////////////////////////////////////////////
// @Serialization

////////////////////////////////////////////////////////////////////////
// @Input

b32 key_pressed(Key key) {
  if (os_key_is_pressed(key)) {
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
  if (os_key_is_down(key)) {
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

////////////////////////////////////////////////////////////////////////
// @UI

ScrollState scroll_state_make(f32 scale) {
  ScrollState res = {
    .scale_level = scale,
    .scale = v2_splat(scale),
  };
  return res;
}

void scroll_state_update(ScrollState& s, ScrollType type) {
  f32 wheel = os_mouse_get_wheel();
  if (wheel) {
    if (os_key_is_down(Key_Ctrl)) {
      v2 mouse = os_mouse_get_pos();
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

  f32 scroll_h = os_mouse_get_wheel_horizontal();
  if (scroll_h) {
    f32 sensity = 100;
    if (os_key_is_down(Key_Shift)) {
      sensity *= 3;
    }
    s.offset.x += scroll_h * sensity;
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


String dumb_struct(Allocator arena, Slice<MemberDefinition> members, void* ptr, EntityFlags flags) {
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
        dstr_push(string, push_strf(scratch, "%s \"%s\"\n", member.name, st->mesh_id_to_str[v.idx]));
      } break;
      case MetaType_GpuMaterialId: {
        GpuMaterialId v = *((GpuMaterialId*)member_ptr);
        dstr_push(string, push_strf(scratch, "%s \"%s\"\n", member.name, st->material_id_to_str[v.idx]));
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
        // *(u32*)Offset(ptr, member.offset) 
        *OffsetAs(ptr, u32, member.offset) = u32_from_str(tok.str);
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
        Assert(!mesh.err);
        *(GpuMeshId*)Offset(ptr, member.offset) = mesh;
      } break;
      case MetaType_GpuMaterialId: {
        Token tok = tok_require(p, TokenType_String);
        Result material = map_get(st->str_to_material_id, tok.str);
        Assert(!material.err);
        *(GpuMaterialId*)Offset(ptr, member.offset) = material;
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
    Slice strs = os_watch_check(scratch, x.watch);
    Loop (i, strs.count) {
      String name = strs[i];
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
          r_shader_reload(shader_name);
        } break;
        InvalidDefaultCase break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////
// @State

void foo() {
  String d = R"(
  {
    "name": "cube",
    "visible": true,
    "pos": [1.0, 2.0, 3.0],
    "material": {
      "color": [1.0, 0.0, 0.0],
      "roughness": 0.5
    }
  }
  )";

  Scratch scratch;

  {
    struct {
      String name;
      b32 visible;
      v3 pos;
      struct {
        v3 color;
        f32 roughness;
      } material;
    } res = {};

    JsParser p = js_parse_make(scratch, d);
    JsObj root = js_parse(&p).obj;
    Loop (i, root.fields.count) {
      var [k, v] = root.fields[i];
      if (str_match(k, "name")) {
        res.name = push_str_copy(scratch, v->str);
      }
      else if (str_match(k, "visible")) {
        res.visible = v->boolean;
      }
      else if (str_match(k, "pos")) {
        Loop (i, v->array.count) {
          res.pos.v[i] = v->array[i]->number;
        }
      }
      else if (str_match(k, "material")) {
        JsObj material = v->obj;
        Loop (i, material.fields.count) {
          var [k, v] = material.fields[i];
          if (str_match(k, "color")) {
            Loop (i, v->array.count) {
              res.material.color.v[i] = v->array[i]->number;
            }
          } else if (str_match(k, "roughness")) {
            res.material.roughness = v->number;
          }
        }
      }
    }
  }

  {
    JsParser p = js_parse_make(scratch, d);
    JsObj root = js_parse(&p).obj;
    String name = js_get_str(root, "name");
    Info("%s", name);
    b32 visible = js_get_bool(root, "visible");
    Info("%u", visible);
    Slice pos = js_get_array(root, "pos");
    Loop (i, pos.count) {
      Info("%f", pos[i]->number);
    }
    {
      JsObj material = js_get_obj(root, "material");
      Slice color = js_get_array(material, "color");
      Loop (i, color.count) {
        Info("%f", color[i]->number);
      }
      f64 roughness = js_get_number(material, "roughness");
      Info("%f", roughness);
    }
  }

  {
    JsParser p = js_parse_make(scratch, d);
    JsVal root = js_parse(&p);
    JsVal name = js_get_val(root, "name");
    Info("%s", name.str);
    JsVal visible = js_get_val(root, "visible");
    Info("%u", visible.boolean);
    JsVal pos = js_get_val(root, "pos");
    Loop (i, pos.array.count) {
      Info("%f", pos.array[i]->number);
    }
    JsVal material = js_get_val(root, "material");
    JsVal color = js_get_val(material, "color");
    Loop (i, color.array.count) {
      Info("%f", color.array[i]->number);
    }
    JsVal roughness = js_get_val(material, "roughness");
    Info("%f", roughness.number);
  }
}

void com_init() {
  Scratch scratch;
  foo();

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
    g.watch.arena = g.arena;
    watch_directory_add(g.shader_dir, WatchOp_RecompileShader);
    watch_directory_add(g.shader_compiled_dir, WatchOp_ShaderReload);
    r_shaders_compile(scratch);

    r_init();
    debug_init();
    game_init();
  }
  prof_launch_end();
}

void com_update() {
  // GlobalState& g = *st;
  ArrayZero(st->input.consumed);
  debug_update();

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
      r_begin();
      // ui_begin();
      com_update();
      game_update();
      // ui_end();
      r_end();
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

void gfx_api_design_foo() {
  // {
  //   bind_pipeline(shader0);
  //   draw_thing(mesh0, pos0);
  
  //   bind_pipeline(shader1);
  //   draw_thing(mesh1, pos1);
  // }

  // {
  //   push_pipeline(shader0);
  //   draw_thing(mesh, pos);
  //   pop_pipeline();
  
  //   push_pipeline(shader1);
  //   draw_thing(mesh1, pos1);
  //   pop_pipeline();
  // }

  // {
  //   init:
  //   make_renderable(e_id0, mesh0, pipeline0);
  //   make_renderable(e_id1, mesh1, pipeline1);
  // }
}

////////////////////////////////////////////////////////////////////////
// @Game

Mesh sphere_generate(Allocator arena) {
  u32 lat_steps = 10;
  u32 lon_steps = 10;
  u32 vert_count = lat_steps*lon_steps;
  u32 index_count = (lat_steps - 1) * lon_steps * 6;
  var vertices = push_slice(arena, Vertex, vert_count);
  var indices = push_slice(arena, u32, index_count);
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
  // u32 north_pole_index = vert_count - 1; // last vertex
  // for (u32 j = 0; j < lon_steps; ++j) {
  //   u32 next_j = (j + 1) % lon_steps;
  //   indices[k++] = idx(lat_steps - 2, j); // last row before pole
  //   indices[k++] = north_pole_index;      // pole
  //   indices[k++] = idx(lat_steps - 2, next_j);
  // }
  Mesh mesh = {
    .vertices = vertices,
    .indices = indices,
  };
  return mesh;
}

Mesh grid_generate(Allocator arena, u32 size, f32 step) {
  var vertices = push_slice(arena, Vertex, size*4);
  v3 pos_offset = v3(-(i32)size/2, 0, -(i32)size/2);
  for (i32 i = 0; i < size; ++i) {
    vertices[i*2].pos = pos_offset + v3(0, 0, i*step);
    vertices[i*2+1].pos = pos_offset + v3(size*step, 0, i*step);
  }
  var vertical_vertices = slice_skip(vertices, size*2);
  for (i32 i = 0; i < size; ++i) {
    vertical_vertices[i*2].pos = pos_offset + v3(i*step, 0, 0);
    vertical_vertices[i*2+1].pos = pos_offset + v3(i*step, 0, size*step);
  }
  Mesh mesh = {
    .vertices = vertices,
  };
  return mesh;
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
  EntityId e_id = pool_push(g.entities, {});
  ++g.entities_count;
  return e_id;
}
EntityId e_alloc(GpuMeshId mesh_id, GpuMaterialId material_id, EntityThing thing) {
  GameState& g = st->game;
  Entity e = {
    .name = thing.name,
    .flags = thing.flags,
    .scale = v3_one(),
    .mesh_id = mesh_id,
    .material_id = material_id,
    .aabb = Rng3(v3_splat(-1), v3_splat(1)),
  };
  EntityId e_id = pool_push(g.entities, e);
  vk_make_renderable(e_id, mesh_id, material_id);
  ++g.entities_count;
  return e_id;
}
EntityId e_alloc(MeshEnum mesh_id, MaterialEnum material_id, EntityThing thing) {
  return e_alloc(mesh_get(mesh_id), material_get(material_id), thing);
}

StaticEntityId e_static_alloc(GpuMeshId mesh_id, GpuMaterialId material_id) {
  GameState& g = st->game;
  StaticEntity e = {
    .scale = v3_one(),
    .mesh_id = mesh_id,
    .material_id = material_id,
  };
  StaticEntityId e_id = pool_push(g.static_entities, e);
  vk_make_renderable_static(e_id, mesh_id, material_id);
  ++g.static_entities_count;
  return e_id;
}
StaticEntityId e_static_alloc(MeshEnum mesh_id, MaterialEnum material_id) {
  return e_static_alloc(mesh_get(mesh_id), material_get(material_id));
}

void e_free(EntityId e_id) {
  GameState& g = st->game;
  vk_remove_renderable(e_id);
  pool_remove(g.entities, e_id);
  --g.entities_count;
}

void e_static_free(StaticEntityId e_id) {
  GameState& g = st->game;
  vk_remove_static_renderable(e_id);
  pool_remove(g.static_entities, e_id);
  --g.static_entities_count;
}

void select_obj() {
  GameState& g = st->game;
  v3 dir = ray_from_screen(os_mouse_get_pos(), os_get_window_size(), g.cam.pos, st->view, st->projection).dir;
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
    if (os_key_is_down(Key_A)) {
      cam.yaw += -rotation_speed;
    }
    if (os_key_is_down(Key_D)) {
      cam.yaw += rotation_speed;
    }
    if (os_key_is_down(Key_R)) {
      cam.pitch += rotation_speed;
    }
    if (os_key_is_down(Key_F)) {
      cam.pitch += -rotation_speed;
    }
    if (g.fps_camera) {
      f32 rot_speed = 10;
      cam.pitch -= os_mouse_get_delta().y * get_dt() * rot_speed;
      cam.yaw += os_mouse_get_delta().x * get_dt() * rot_speed;
    }
  }

  // Camera movement
  {
    f32 speed = cam.speed*1;
    v3 velocity = {};
    if (os_key_is_down(Key_W)) {
      v3 forward = mat4_forward(view);
      velocity += forward;
    }
    if (os_key_is_down(Key_S)) {
      v3 backward = mat4_backward(view);
      velocity += backward;
    }
    if (os_key_is_down(Key_Q)) {
      v3 left = mat4_left(view);
      velocity += left;
    }
    if (os_key_is_down(Key_E)) {
      v3 right = mat4_right(view);
      velocity += right;
    }
    if (os_key_is_down(Key_Space)) {
      velocity.y += 1.0f;
    }
    if (os_key_is_down(Key_X)) {
      velocity.y -= 1.0f;
    }
    if (os_key_is_down(Key_Shift)) {
      speed *= 20;
    }
    if (os_key_is_down(Key_LAlt)) {
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
  monkey.aabb = Rng3(v3_splat(-1.2), v3_splat(1.2));
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
  {
    g.a = v3(-10, 0, 0);
    g.b = v3(0, 10, 0);
    g.e = e_alloc(Mesh_Cube, Material_Container);
    Entity& e = get_entity(g.e);
    e.pos = v3(10, 10, 20);
  }

  {
    v4 quat = quat_axis_angle(v3(1,0,0), degtorad(90));
    v4 quat1 = quat_axis_angle(v3(1,0,0), degtorad(90));
    quat = quat_mul(quat, quat1);
    v3 v = v3(0, 1, 0);
    v = quat_rotate(quat, v);
    Info("%f %f %f", v.x,v.y,v.z);
  }

  {
    EntityId e_id = e_alloc(Mesh_CubeGlft, Material_Container);
    Entity& e = get_entity(e_id);
    e.pos = v3(0,3,3);
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
  if (os_mouse_is_button_pressed(MouseButton_Left)) {
    // select_obj();
    // v3 dir = ray_from_screen();
    Ray ray = ray_from_screen(os_mouse_get_pos(), os_get_window_size(), g.cam.pos, st->view, st->projection);
    vk_draw_line_consistent(ray.pos - v3(0,0.1,0), g.cam.pos + ray.dir*100, ColorWhite);
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

  Entity& e = get_entity(g.e);
  // e.pos = v3_lerp(g.a, g.t, g.b);
  // f32 angle = Acos(v3_dot(v3_norm(g.a), v3_norm(g.b)));

  // slerp(a, b, t) =
  //     (sin((1 - t)θ) / sin(θ)) * a +
  //     (sin(tθ) / sin(θ)) * b;
  // e.pos = Sin((1 - g.t)*angle / Sin(angle)) * g.a + (Sin(angle * g.t) / Sin(angle)) * g.b;
  // v2_rotate_relative(a, b, cosine, sine);

  v2 pivot = v2(20, 10);
  // e.pos = v2_to_v3(v2_rotate_relative(v2_of_v3(e.pos), pivot, degtorad(20 * get_dt())), 0);
  // e.pos = v3_rotate_z(e.pos, degtorad(20) * get_dt());
  // e.pos = v3_rotate_y(e.pos, degtorad(20) * get_dt());
  // e.pos = v3_rotate_z(e.pos, degtorad(20) * get_dt());
  e.pos = v3_rotate_around_axis(e.pos, v3(1,1,1), degtorad(60)*get_dt());

  {
    if (os_key_is_pressed(Key_U)) {
      if (g.fps_camera) {
        os_cursor_unlock();
      } else {
        os_cursor_lock();
      }
      g.fps_camera = !g.fps_camera;
    }
  }

  vk_draw_rect(Rng2(v2(0,0), v2(100,100)), ColorWhite);
}

void game_save_state() {
  Scratch scratch;
  GameState& g = st->game;

  Dstring data = dstr_make(scratch);
  dstr_push(data, "Camera {\n");
  dstr_push(data, dumb_struct(scratch, slice(members_of_Camera), &g.cam));
  dstr_push(data, "}\n");

  {
    var& p = g.entities;
    for EachNodePool(i, p) {
      Entity& e = p.data[i].elem;
      dstr_push(data, "Entity {\n");
      dstr_push(data, dumb_struct(scratch, slice(members_of_Entity), &e, e.flags));
      dstr_push(data, "}\n");
    }
  }
  {
    var& p = g.static_entities;
    for EachNodePool(i, p) {
      StaticEntity& e = p.data[i].elem;
      dstr_push(data, "StaticEntity {\n");
      dstr_push(data, dumb_struct(scratch, slice(members_of_StaticEntity), &e));
      dstr_push(data, "}\n");
    }
  }

  OS_Handle file = os_file_open(push_strf(scratch, "%s/saved", os_get_current_directory(), String("saved")), OS_AccessFlag_Write | OS_AccessFlag_Trunc);
  os_file_write(file, dstr_slice(data));
  os_file_close(file);
}

void game_load_state() {
  GameState& g = st->game;
  Scratch scratch;
  Slice data = os_file_path_read_all(scratch, push_strf(scratch, "%s/saved", os_get_current_directory(), String("saved")));
  Slice tokens = tokens_from_str(scratch, str_make(data.data, data.size));
  Parser p = parser_make(tokens);

  {
    var& p = g.entities;
    for EachNodePool(i, p) {
      EntityId e_id = pool_get_handler(p, i);
      e_free(e_id);
    }
  }
  {
    var& p = g.static_entities;
    for EachNodePool(i, p) {
      StaticEntityId e_id = pool_get_handler(p, i);
      e_static_free(e_id);
    }
  }

  while (!tok_is_end(p)) {
    Token tok = tok_advance(p);
    switch (tok.type) {
      default:{} break;
      case TokenType_Identifier: {
        if (str_match(tok.str, "Camera")) {
          dumb_struct_load(slice(members_of_Camera), &g.cam, &p);
        } else if (str_match(tok.str, "Entity")) {
          EntityId e_id = e_alloc_bare();
          Entity& e = get_entity(e_id);
          dumb_struct_load(slice(members_of_Entity), &e, &p);
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
          dumb_struct_load(slice(members_of_StaticEntity), &entity, &p);
          StaticEntityId e_id = e_static_alloc(entity.mesh_id, entity.material_id);
          StaticEntity&e = get_static_entity(e_id);
          e = entity;
        }
      }
    }
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

  g.moving_cubes = darray_make<EntityId>(g.gpa);
  g.static_cubes = darray_make<StaticEntityId>(g.gpa);

  Mesh triangle_mesh = {.vertices = slice(triangle_vertices)};
  mesh_set(Mesh_Triangle, r_mesh_load(triangle_mesh));
  Mesh grid_mesh = grid_generate(scratch, 100, 1);
  mesh_set(Mesh_Grid, r_mesh_load(grid_mesh));
  Mesh axis_mesh = {.vertices = slice(axis_vertices)};
  mesh_set(Mesh_Axis, r_mesh_load(axis_mesh));
  Mesh sphere = sphere_generate(scratch);
  mesh_set(Mesh_Sphere, r_mesh_load(sphere));

  st->meshes_ids[Mesh_CubeGlft] = mesh_load("cube.gltf");

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
  if (os_key_is_down(Key_T)) {
    scene_deinit();
    scene_init();
  }
  if (os_key_is_down(Key_Escape)) {
    os_close_window();
  }
  scene_update();
}


