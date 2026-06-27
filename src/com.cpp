#include "com.h"

#include "gfx.cpp"
#include "render.cpp"
#include "test.cpp"
#include "tokenizer.cpp"
#include "debug.cpp"

TimeScope::TimeScope() {
  tsc_start = cpu_timer_now();
}
TimeScope::~TimeScope() {
  u64 elapsed = cpu_timer_now() - tsc_start;
  Info("%fms", tsc_to_ms(elapsed));
}

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
  {.pos = v3( 0.0,   0.5, 0), .uv = v2(0.5, 0), .color = v4(1,0,0,1)},
  {.pos = v3(-0.5,  -0.5, 0), .uv = v2(0.0, 1), .color = v4(0,1,0,1)},
  {.pos = v3( 0.5,  -0.5, 0), .uv = v2(1.0, 1), .color = v4(0,0,1,1)},
};

Vertex axis_vertices[] = {
  {.pos = v3_zero(), .color = v4(1,0,0,1)},
  {.pos = v3(1,0,0), .color = v4(1,0,0,1)},
  {.pos = v3_zero(), .color = v4(0,1,0,1)},
  {.pos = v3(0,1,0), .color = v4(0,1,0,1)},
  {.pos = v3_zero(), .color = v4(0,0,1,1)},
  {.pos = v3(0,0,1), .color = v4(0,0,1,1)},
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

b32 timer_update(Timer& t) {
  t.acc += get_dt();
  if (t.acc >= t.interval) {
    t.acc -= t.interval;
    return true;
  }
  return false;
}

b32 timer_passed(Timer& t) {
  return t.acc >= t.interval;
}

void timer_tick(Timer& t) {
  t.acc += get_dt();
}

void timer_trigger(Timer& t) {
  t.acc -= t.interval;
}

void timer_reset(Timer& t) {
  t.acc = 0;
}

b32 cooldown_ready(Cooldown& cd) {
  return cd.remaining <= 0;
}

void cooldown_tick(Cooldown& cd, f32 dt) {
  if (cd.remaining > 0) cd.remaining -= dt;
}

void cooldown_start(Cooldown& cd, f32 duration) {
  cd.remaining = duration;
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

struct WordLexer {
  String str;
  u32 cursor;
};

WordLexer word_lexer_make(String str) {
  WordLexer res = {
    .str = str,
  };
  return res;
}

String word_lexer_next(WordLexer& l) {
  while (l.cursor < l.str.size && char_is_ws(l.str.str[l.cursor])) {
    ++l.cursor;
  }
  u32 word_base = l.cursor;
  while (l.cursor < l.str.size && !char_is_ws(l.str.str[l.cursor])) {
    ++l.cursor;
  }
  String res = str_make(l.str.str + word_base, l.cursor - word_base);
  return res;
}

MeshDesc load_obj(Allocator arena, String name) {
  Scratch scratch(arena);
  var positions = array_make(v3, scratch);
  var normals = array_make(v3, scratch);
  var uvs = array_make(v2, scratch);
  var indexes = array_make(v3u, scratch);

  // String str = R"(
  //   v  -4.4   14 4.1
  //   v   1.4  -14 4.1
  //   vt -4.4   14
  //   vt  1.4  -14
  //   vn -4.4   14 4.1
  //   vn  1.4  -14 4.1
  //   f 10/4/1 1/2/3 11/22/33
  //   f 20/2/2 91/42/13 141/22/33
  // )";
  WordLexer l = word_lexer_make(os_file_path_read_all_str(scratch, name));
  for (String word = {}; (word = word_lexer_next(l)).size;) {
    // Info("%s", word);
    if (word.str[0] == 'v' && word.str[1] == ' ') {
      v3 pos = {
        f32_from_str(word_lexer_next(l)),
        f32_from_str(word_lexer_next(l)),
        f32_from_str(word_lexer_next(l)),
      };
      array_push(positions, pos);
      // Info("%f %f %f", pos.x, pos.y, pos.z);
    } else if (word.str[0] == 'v' && word.str[1] == 'n') {
      v3 norm = {
        f32_from_str(word_lexer_next(l)),
        f32_from_str(word_lexer_next(l)),
        f32_from_str(word_lexer_next(l)),
      };
      array_push(normals, norm);
      // Info("%f %f %f", norm.x, norm.y, norm.z);
    } else if (word.str[0] == 'v' && word.str[1] == 't') {
      v2 uv = {
        f32_from_str(word_lexer_next(l)),
        f32_from_str(word_lexer_next(l)),
      };
      array_push(uvs, uv);
      // Info("%f %f", uv.x, uv.y);
    } else if (word.str[0] == 'f' && word.str[1] == ' ') {
      Loop (i, 3) {
        v3u raw = {};
        String word = word_lexer_next(l);
        u32 cursor = 0;
        String r = {};
        Loop (i, 3) {
          u32 num_base = cursor;
          while (cursor < word.size && char_is_digit(word.str[cursor])) {
            ++cursor;
          }
          r = str_make(word.str+num_base, cursor - num_base);
          raw.v[i] = u64_from_str(r) - 1;
          ++cursor;
        }
        // Info("%u %u %u", raw.x, raw.y, raw.z);
        v3u v = {raw.x, raw.z, raw.y};
        array_push(indexes, v);
      }
    }
  }

  var vertices = array_make(Vertex, arena);
  var final_indices = array_make(u32, arena);
  var map = map_make(v3u, u32, scratch);
  Loop (i, indexes.count) {
    v3u idx = indexes[i];
    Result r = map_get(map, idx);
    if (r.err) {
      Vertex v = {
        positions[idx.x],
        normals[idx.y],
        uvs[idx.z],
      };
      u32 new_index = vertices.count;
      array_push(vertices, v);
      array_push(final_indices, new_index);
      map_set(map, idx, new_index);
    } else {
      array_push(final_indices, (u32)r);
    }
  }
  MeshDesc mesh = {
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

MeshDesc load_gltf(Allocator arena, String path, b32 is_glb) {
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
  Slice pos = slice_reinterpret<v3>(slice_n(data, gltf.pos_buffer_view.offset, gltf.pos_buffer_view.size));
  Slice norm = slice_reinterpret<v3>(slice_n(data, gltf.norm_buffer_view.offset, gltf.norm_buffer_view.size));
  Slice texcoord = slice_reinterpret<v2>(slice_n(data, gltf.texcoord_buffer_view.offset, gltf.texcoord_buffer_view.size));
  Slice indices_u16 = slice_reinterpret<u16>(slice_n(data, gltf.indices_buffer_view.offset, gltf.indices_buffer_view.size));

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
  MeshDesc mesh = {
    .vertices = vertices,
    .indices = indices,
  };
  return mesh;
}

TextureDesc image_load(String filepath) {
  Scratch scratch;
  u32 required_channel_count = 4;
  u32 channel_count;
  Slice buf = os_file_path_read_all(scratch, filepath);
  TextureDesc res = {};
  res.data = stbi_load_from_memory(buf.data, buf.count, (i32*)&res.width, (i32*)&res.height, (i32*)&channel_count, required_channel_count);
  Assert(res.data);
  return res;
}

////////////////////////////////////////////////////////////////////////
// @Assets

global String meshes_strs[] = {
#define X(name) [Glue(Mesh_, name)] = Stringify(name),
  MESH_LIST
#undef X
};

global String textures_strs[] = {
#define X(name) [Glue(Texture_, name)] = Stringify(name),
  TEXTURE_LIST
#undef X
};

global String materials_strs[] = {
#define X(name) [Glue(Material_, name)] = Stringify(name),
  MATERIAL_LIST
#undef X
};

R_Mesh mesh_get(MeshEnum mesh_enum) { return st->meshes_ids[mesh_enum]; }
void mesh_set(MeshEnum mesh_enum, R_Mesh id) { 
  GlobalState& g = *st;
  g.meshes_ids[mesh_enum] = id;
  String str = push_str_copy(g.arena, meshes_strs[mesh_enum]);
  map_set(g.str_to_mesh_id, str, id);
  g.mesh_id_to_str[id.idx] = str;
}
R_Material material_get(MaterialEnum id) { return st->materials_ids[id]; }

constexpr MaterialProps material_default_props() {
  MaterialProps props = {
    .ambient = v3_splat(1),
    .diffuse = v3_splat(1),
    .specular = v3_splat(1),
    .shininess = 1,
  };
  return props;
}

////////////////////////////////////////////////////////////////////////
// @Json

JsParser js_parse_make(Allocator arena, String str) {
  JsParser res = {
    .arena = arena,
    .str = str,
  };
  return res;
}

u8 js_peek(JsParser* p) {
  if (p->cursor >= p->str.size) return 0;
  return p->str.str[p->cursor];
}

b32 js_at_end(JsParser* p) {
  return p->cursor >= p->str.size;
}

void js_advance(JsParser* p) {
  ++p->cursor;
}

void js_skip_ws(JsParser* p) {
  while (char_is_ws(js_peek(p))) {
    js_advance(p);
  }
}

String js_parse_str(JsParser* p) {
  Assert(js_peek(p) == '\"');
  js_advance(p);
  u32 start = p->cursor;
  while (js_peek(p) != '\"' && !js_at_end(p)) {
    js_advance(p);
  }
  Assert(js_peek(p) == '\"'); 
  String res = str_substr(p->str, Rng1u(start, p->cursor));
  js_advance(p);
  return res;
}

f64 js_parse_number(JsParser* p) {
  u32 start = p->cursor;
  // while (char_is_number_cont(js_peek(p))) {
  //   js_advance(p);
  // }

  if (js_peek(p) == '-')
    js_advance(p);

  while (char_is_digit(js_peek(p)))
    js_advance(p);

  if (js_peek(p) == '.') {
    js_advance(p);
    while (char_is_digit(js_peek(p)))
      js_advance(p);
  }

  if (js_peek(p) == 'e' || js_peek(p) == 'E') {
    js_advance(p);

    if (js_peek(p) == '+' || js_peek(p) == '-')
      js_advance(p);

    while (char_is_digit(js_peek(p)))
      js_advance(p);
  }

  String str = str_substr(p->str, Rng1u(start, p->cursor));
  f64 res = f64_from_str(str);
  return res;
}

JsVal js_parse(JsParser* p) {
  js_skip_ws(p);
  switch (js_peek(p)) {
    default:   return { .type = JsType_Number, .number = js_parse_number(p) };
    case '\"': return { .type = JsType_Str, .str = js_parse_str(p), };
    case 't': p->cursor += 4; return { .type = JsType_Bool, .boolean = true, };
    case 'f': p->cursor += 5; return { .type = JsType_Bool, .boolean = false, };
    case 'n': p->cursor += 4; return { .type = JsType_Null, };
    case '[': {
      js_advance(p);
      var arr = array_make(JsVal*, p->arena);
      while (true) {
        js_skip_ws(p);
        if (js_peek(p) == ']') {
          js_advance(p);
          break;
        }
        JsVal* val = push_struct(p->arena, JsVal);
        *val = js_parse(p);
        // switch (val->type) {
        //   case JsType_Null: Info("null"); break;
        //   case JsType_Bool: Info("bool: %u", val->boolean); break;
        //   case JsType_Number: Info("num: %f", val->number); break;
        //   case JsType_Str: Info("str: %s", val->str); break;
        //   case JsType_Array: Info("array"); break;
        //   case JsType_Obj:  Info("obj"); break;
        // }
        array_push(arr, val);
        js_skip_ws(p);
        if (js_peek(p) == ',') {
          js_advance(p);
        }
      }
      JsVal res = {
        .type = JsType_Array,
        .array = slice(arr),
      };
      return res;
    } break;
    case '{': {
      js_advance(p);
      var fields = array_make(JsField, p->arena);
      while (true) {
        js_skip_ws(p);
        if (js_peek(p) == '}') {
          js_advance(p);
          break;
        }
        String key = js_parse_str(p);
        js_skip_ws(p);
        Assert(js_peek(p) == ':');
        js_advance(p);
        
        JsVal* val = push_struct(p->arena, JsVal);
        *val = js_parse(p);
        // switch (val->type) {
        //   case JsType_Null: Info("null"); break;
        //   case JsType_Bool: Info("bool: %u", val->boolean); break;
        //   case JsType_Number: Info("num: %f", val->number); break;
        //   case JsType_Str: Info("str: %s", val->str); break;
        //   case JsType_Array: Info("array"); break;
        //   case JsType_Obj:  Info("obj"); break;
        // }
        array_push(fields, {.key = key, .val = val});
        js_skip_ws(p);
        if (js_peek(p) == ',') {
          js_advance(p);
        }
      }
      JsVal res = {
        .type = JsType_Obj,
        .obj = slice(fields),
      };
      return res;
    } break;
  }
}

JsVal js_get_val(JsVal val, String key) {
  Loop (i, val.obj.fields.count) {
    JsField& field = val.obj.fields[i];
    if (str_match(field.key, key)) {
      // Assert(field.val->type == JsType_Obj);
      return *field.val;
    }
  }
  return {};
}

b32 js_get_bool(JsObj obj, String key) {
  Loop (i, obj.fields.count) {
    JsField& field = obj.fields[i];
    if (str_match(field.key, key)) {
      Assert(field.val->type == JsType_Bool);
      return field.val->boolean;
    }
  }
  return {};
}

f64 js_get_number(JsObj obj, String key) {
  Loop (i, obj.fields.count) {
    JsField& field = obj.fields[i];
    if (str_match(field.key, key)) {
      Assert(field.val->type == JsType_Number);
      return field.val->number;
    }
  }
  return {};
}

String js_get_str(JsObj obj, String key) {
  Loop (i, obj.fields.count) {
    JsField& field = obj.fields[i];
    if (str_match(field.key, key)) {
      Assert(field.val->type == JsType_Str);
      return field.val->str;
    }
  }
  return {};
}

Slice<JsVal*> js_get_array(JsObj obj, String key) {
  Loop (i, obj.fields.count) {
    JsField& field = obj.fields[i];
    if (str_match(field.key, key)) {
      Assert(field.val->type == JsType_Array);
      return field.val->array;
    }
  }
  return {};
}

JsObj js_get_obj(JsObj obj, String key) {
  Loop (i, obj.fields.count) {
    JsField& field = obj.fields[i];
    if (str_match(field.key, key)) {
      Assert(field.val->type == JsType_Obj);
      return field.val->obj;
    }
  }
  return {};
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
        v2 v = *(v2*)member_ptr;
        dstr_push(string, push_strf(scratch, "%s %f %f\n", member.name, v.x, v.y));
      } break;
      case MetaType_v3: {
        v3 v = *(v3*)member_ptr;
        dstr_push(string, push_strf(scratch, "%s %f %f %f\n", member.name, v.x, v.y, v.z));
      } break;
      case MetaType_v4: {
        v4 v = *(v4*)member_ptr;
        dstr_push(string, push_strf(scratch, "%s %f %f %f %f\n", member.name, v.x, v.y, v.z, v.w));
      } break;
      case MetaType_Rng3: {
        Rng3 v = *(Rng3*)member_ptr;
        dstr_push(string, push_strf(scratch, "%s %f %f %f %f %f %f\n", member.name, v.min.x,v.min.y,v.min.z, v.max.x,v.max.y,v.max.z));
      } break;
      case MetaType_MeshId: {
        R_Mesh v = *(R_Mesh*)member_ptr;
        dstr_push(string, push_strf(scratch, "%s \"%s\"\n", member.name, st->mesh_id_to_str[v.idx]));
      } break;
      case MetaType_MaterialId: {
        R_Material v = *(R_Material*)member_ptr;
        dstr_push(string, push_strf(scratch, "%s \"%s\"\n", member.name, st->material_id_to_str[v.idx]));
      } break;
      case MetaType_String: {
        if (FlagHas(flags, EntityFlag_Referenced)) {
          String v = *(String*)member_ptr;
          dstr_push(string, push_strf(scratch, "%s \"%s\"\n", member.name, v));
        }
      } break;
      case MetaType_EntityFlags: {
        EntityFlags v = *(EntityFlags*)member_ptr;
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
    u8* mem = Offset(ptr, member.offset);
    switch (member.type) {
      default:{} break;
      case MetaType_u32: {
        Token tok = tok_advance(p);
        *(u32*)mem = u32_from_str(tok.str);
      } break;
      case MetaType_i32: {
        *(u32*)mem = parse_i32(p);
      } break;
      case MetaType_b32: {
        Token tok = tok_advance(p);
        *(u32*)mem = u32_from_str(tok.str);
      } break;
      case MetaType_f32: {
        *(f32*)mem = parse_f32(p);
      } break;
      case MetaType_v2: {
        *(v2*)mem = v2(parse_f32(p), parse_f32(p));
      } break;
      case MetaType_v3: {
        *(v3*)mem = v3(parse_f32(p), parse_f32(p), parse_f32(p));
      } break;
      case MetaType_v4: {
        *(v4*)mem = v4(parse_f32(p), parse_f32(p), parse_f32(p), parse_f32(p));
      } break;
      case MetaType_Rng3: {
        *(Rng3*)mem = Rng3(v3(parse_f32(p), parse_f32(p), parse_f32(p)), v3(parse_f32(p), parse_f32(p), parse_f32(p)));
      } break;
      case MetaType_MeshId: {
        Token tok = tok_require(p, TokenType_String);
        Result mesh = map_get(st->str_to_mesh_id, tok.str);
        Assert(!mesh.err);
        *(R_Mesh*)mem = mesh;
      } break;
      case MetaType_MaterialId: {
        Token tok = tok_require(p, TokenType_String);
        Result material = map_get(st->str_to_material_id, tok.str);
        Assert(!material.err);
        *(R_Material*)mem = material;
      } break;
      case MetaType_String: {
        Token tok = tok_require(p, TokenType_String);
        *(String*)mem = push_str_copy(st->arena, tok.str);
      } break;
      case MetaType_EntityFlags: {
        *(EntityFlags*)mem = parse_u32(p);
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

void foo_js() {
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

  MakeId(A_ID)
  struct A {
    A_ID next;
    A_ID prev;
  };
  A_ID first = {};
  A arr[128];
  for (u32 i = first.idx; i; i = arr[i].next.idx) {
  }
  {
    struct List {
      A_ID first;
      A_ID last;
    } list = {};
    A_ID thing = {};
    // hdll_push_back(arr, list.first, list.last, thing);
  }

  // LoopHNode (i, first, arr) {
  //   A* d = &arr[i.idx];
  // }
  {
    struct List {
      u32 first;
      u32 last;
    };

  }

  foo_js();
  GlobalState& g = *st;
  estimate_cpu_frequency();
  global_allocator_init();
  os_gfx_init();
  prof_init(g.arena);
  prof_launch_begin();

  {
    ProfBlock("init");
    thread_pool_init();
    test();

    g.gpa = alloc_seglist_make(g.arena);
    g.asset_dir = push_strf(g.arena, "%s/%s", os_get_current_directory(), String("../assets"));
    g.shader_dir = push_str_cat(g.arena, g.asset_dir, "/shaders");
    g.shader_compiled_dir = push_str_cat(g.arena, g.shader_dir, "/compiled");
    g.models_dir = push_str_cat(g.arena, g.asset_dir, "/models");
    g.textures_dir = push_str_cat(g.arena, g.asset_dir, "/textures");
    g.watch.arena = g.arena;
    r_shaders_compile(scratch);
    r_init();
    debug_init();
    game_init();
    watch_directory_add(g.shader_dir, WatchOp_RecompileShader);
    watch_directory_add(g.shader_compiled_dir, WatchOp_ShaderReload);
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
      thread_push({.fn = [](void* ctx) {os_sleep_ms(rand_u32()%4);}, .priority = TaskPriority_Low});
    }
    WaitGroup id0 = thread_push({.fn = [](void* ctx) { os_sleep_ms(2); }});
    WaitGroup id1 = thread_push({.fn = [](void* ctx) { os_sleep_ms(2); }});
    WaitGroup id2 = thread_push({.fn = [](void* ctx) { os_sleep_ms(2); }});
    WaitGroup id3 = thread_push({.fn = [](void* ctx) { os_sleep_ms(2); }});

    TaskDesc tasks[] = {
      {.fn = [](void* ctx) { os_sleep_ms(2); }},
      {.fn = [](void* ctx) { os_sleep_ms(2); }},
      {.fn = [](void* ctx) { os_sleep_ms(2); }},
      {.fn = [](void* ctx) { os_sleep_ms(2); }},
    };
    WaitGroup batch = thread_push_batch(slice(tasks));

    thread_wg_wait(id0);
    thread_wg_wait(id1);
    thread_wg_wait(id2);
    thread_wg_wait(id3);
    thread_wg_wait(batch);

    // struct Ctx {
    //   u32 i;
    // };
    // var& ctx = thread_push_ctx(Ctx);
    // ctx.i = 1;
    // thread_push({.ctx = &ctx, .fn = [](void* ctx) {
    //   Ctx* data = (Ctx*)ctx;
    //   Info("%i", data->i);
    //   os_sleep_ms(1);
    // }});

    // Scratch scratch;
    // u64 size = KB(1);
    // var s = push_slice(scratch, u32, size);
    // Loop (i, size) {
    //   s[i] = 1;
    // }
    // thread_parallel_for(s, [](Slice<u32> s) {
      // u32 sum = 0;
      // Loop (i, s.count) {
      //   sum += s[i];
      // }
      // Info("%i", sum);
      // os_sleep_ms(1);
    // });
    // u32 a = 1;
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
  thread_wait_remanings();
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

MeshDesc sphere_generate(Allocator arena) {
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
  MeshDesc mesh = {
    .vertices = vertices,
    .indices = indices,
  };
  return mesh;
}

MeshDesc grid_generate(Allocator arena, u32 size, f32 step) {
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
  MeshDesc mesh = {
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
void e_init(EntityId e_id) {
  Entity& e = get_entity(e_id);
  r_set_entity_color(e_id, e.color);
}

EntityId e_alloc(R_Mesh mesh_id, R_Material material_id, EntityThing thing) {
  GameState& g = st->game;
  Entity e = {
    .name = thing.name,
    .flags = thing.flags,
    .scale = v3_one(),
    .mesh_id = mesh_id,
    .material_id = material_id,
    .aabb = Rng3(v3_splat(-1), v3_splat(1)),
    .color = ColorWhite,
  };
  EntityId e_id = pool_push(g.entities, e);
  ++g.entities_count;
  return e_id;
}
EntityId e_alloc(MeshEnum mesh_id, MaterialEnum material_id, EntityThing thing) {
  return e_alloc(mesh_get(mesh_id), material_get(material_id), thing);
}

void e_free(EntityId e_id) {
  GameState& g = st->game;
  pool_remove(g.entities, e_id);
  --g.entities_count;
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
    Entity& grid = get_entity(grid_id);
    grid.color = v4_splat(0.6);
    e_init(grid_id);
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
  Loop (i, 10) {
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
    EntityId e_id = e_alloc(meshes[rand_rng_u32(0, ArrayCount(meshes)-1)], materials[rand_rng_u32(0, ArrayCount(materials)-1)]);
    // array_push(g.static_cubes, e_id);
    // u32 range = KB(1);
    u32 range = 100;
    Entity& e = get_entity(e_id);
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
    EntityId e_id = e_alloc(Mesh_Cube, Material_Orange);
    Entity& e = get_entity(e_id);
    e.pos = v3(0,3,3);
  }
  {
    EntityId e_id = e_alloc(Mesh_Barrack, Material_Barrack);
    Entity& e = get_entity(e_id);
    e.pos = v3(-3,3,-33);
  }
  {
    EntityId e_id = e_alloc(Mesh_Cube, Material_Container);
    g.my = e_id;
    Entity& e = get_entity(e_id);
    e.pos = v3(3,3,0);
    Loop (i, 4) {
      EntityId child_id = e_alloc(Mesh_Cube, Material_Container);
      hdll_list_push_back(g.entities.data, e, child_id);
      Loop (i, 4) {
        EntityId new_child_id = e_alloc(Mesh_Cube, Material_Container);
        hdll_list_push_back(g.entities.data, get_entity(child_id), new_child_id);
      }
    }
  }
}

void scene_deinit() {
  GameState& g = st->game;
  pool_clear(g.entities);
}

void scene_update() {
  Scratch scratch;
  GameState& g = st->game;
  if (os_mouse_is_button_pressed(MouseButton_Left)) {
    // select_obj();
    // v3 dir = ray_from_screen();
    Ray ray = ray_from_screen(os_mouse_get_pos(), os_get_window_size(), g.cam.pos, st->view, st->projection);
    r_debug_line_persistent(ray.pos - v3(0,0.1,0), g.cam.pos + ray.dir*100, ColorWhite);
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
    r_debug_cuboid(rng3_shift(e.aabb, e.pos), ColorWhite);
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
  // thread_parallel_for(KB(1), slice(g.moving_cubes), [](Slice<EntityId> entities) {
  //   Loop (i, entities.count) {
  //     Entity& e = get_entity(entities[i]);
  //     e.pos += e.vel * get_dt();
  //     v3 center = {0, 0, 0};
  //     v3 dir = e.pos - center;
  //     v3 tangent = v3_norm(v3{-dir.z, 0, dir.x});
  //     e.vel += tangent * 2.0f * get_dt();
  //     e.vel += -dir * 0.5f * get_dt();
  //   }
  // });
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

  r_draw_rect(Rng2(v2(0,0), v2(100,100)), ColorWhite);

  LoopINode (i, g.entities.first, g.entities.data) {
    EntityId e_id = pool_get_handle(g.entities, i);
    Entity& e = pool_get(g.entities, e_id);
    r_push_mesh(e_id, e.mesh_id, e.material_id);
  }
  LoopINode (i, g.entities.first, g.entities.data) {
    EntityId e_id = pool_get_handle(g.entities, i);
    Entity& e = pool_get(g.entities, e_id);
    r_push_mesh(e_id, e.mesh_id, e.material_id);
  }
  {
    // r_debug_grid(v3(0, 20, 0), 10, 10, ColorGrey);
  }

  {
    var& my = get_entity(g.my);
    my.pos.z += get_dt() * 1;
    u32 i = 1;
    LoopHNode (it, my.first, g.entities.data) {
      var& child = get_entity(it);
      child.pos = my.pos + v3(0,3,0)*i++;
      u32 j = 1;
      LoopHNode (i, child.first, g.entities.data) {
        var& new_child = get_entity(i);
        new_child.pos = child.pos + v3(3,0,0)*j;
        ++j;
      }
    }
  }
}

void game_save_state() {
  Scratch scratch;
  GameState& g = st->game;

  Dstring data = dstr_make(scratch);
  dstr_push(data, "Camera {\n");
  dstr_push(data, dumb_struct(scratch, slice(members_of_Camera), &g.cam));
  dstr_push(data, "}\n");

  {
    Entity e = get_entity(g.e);
    dstr_push(data, "e {\n");
    dstr_push(data, dumb_struct(scratch, slice(members_of_Entity), &e));
    dstr_push(data, "}\n");
  }
  {
    var& p = g.entities;
    LoopINode (i, g.entities.first, g.entities.data) {
      Entity& e = p.data[i].elem;
      dstr_push(data, "Entity {\n");
      dstr_push(data, dumb_struct(scratch, slice(members_of_Entity), &e, e.flags));
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
    LoopINode (i, g.entities.first, g.entities.data) {
      EntityId e_id = pool_get_handle(p, i);
      e_free(e_id);
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
          e_init(e_id);
          if (FlagHas(e.flags, EntityFlag_Referenced)) {
            if (str_match("monkey", e.name)) {
              g.monkey_id = e_id;
            } else if (str_match("axis_attached_to_cam", e.name)) {
              g.axis_attached_to_cam_id = e_id;
            } else if (str_match("rotating_cube", e.name)) {
              g.rotating_cube_id = e_id;
            } 
          }
        } else if (str_match(tok.str, "e")) {
          EntityId e_id = e_alloc_bare();
          Entity& e = get_entity(e_id);
          dumb_struct_load(slice(members_of_Entity), &e, &p);
          e_init(e_id);
          g.e = e_id;
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

  g.moving_cubes = array_make(EntityId, g.gpa);
  // g.static_cubes = array_make<StaticEntityId>(g.gpa);

  MeshDesc triangle_mesh = {.vertices = slice(triangle_vertices)};
  mesh_set(Mesh_Triangle, r_mesh_make(triangle_mesh));
  MeshDesc grid_mesh = grid_generate(scratch, 100, 1);
  mesh_set(Mesh_Grid, r_mesh_make(grid_mesh));
  MeshDesc axis_mesh = {.vertices = slice(axis_vertices)};
  mesh_set(Mesh_Axis, r_mesh_make(axis_mesh));
  MeshDesc sphere = sphere_generate(scratch);
  mesh_set(Mesh_Sphere, r_mesh_make(sphere));

  r_texture_cube_load("night_cubemap");

  {
    GlobalState& g = *st;
    var m_load = [&](MeshEnum enum_name, String name) {
      // R_Mesh id = r_mesh_load(name);
      R_Mesh id = r_mesh_load_async(name);
      g.meshes_ids[enum_name] = id;
      String str = push_str_copy(g.arena, name);
      map_set(g.str_to_mesh_id, str, id);
      g.mesh_id_to_str[id.idx] = str;
    };
    m_load(Mesh_Cube, "cube_ok_uv.glb");
    m_load(Mesh_MonkeyGlb, "monkey.glb");
    m_load(Mesh_CubeGlft, "cube.gltf");
    m_load(Mesh_Barrack, "castle.gltf");
    // m_load(Mesh_Barrack, "castle.obj");
    // m_load(Mesh_GreeMan, "greenman.glb");
  
    var t_load = [&](TextureEnum enum_name, String name) {
      R_Texture id = r_texture_load(name);
      g.textures_ids[enum_name] = id;
      String str = push_str_copy(g.arena, name);
      map_set(g.str_to_texture_id, str, id);
      g.texture_id_to_str[id.idx] = str;
    };
    t_load(Texture_Orange, "orange_lines_512.png");
    t_load(Texture_Container, "container.jpg");
    t_load(Texture_Barrack, "castle_diffuse.png");
  
    var mat_load = [&](MaterialEnum enum_name, MaterialDesc desc) {
      R_Material id = r_material_make(desc);
      g.materials_ids[enum_name] = id;
      String str = push_str_copy(g.arena, materials_strs[enum_name]);
      map_set(g.str_to_material_id, str, id);
      g.material_id_to_str[id.idx] = str;
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
    mat_load(Material_Barrack, {
      .shader_name = "e_texture",
      .pipeline_desc = {
        .depth = {
          .compare = Gfx_CompareOp_Less,
          .write_enabled = true,
        }
      },
      .props = material_default_props(),
      .texture = "castle_diffuse.png",
    });
  }

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
  //   R_Mesh meshes[] = {
  //     // Mesh_MonkeyGlb,
  //     // Mesh_Triangle,
  //     Mesh_Cube,
  //   };
  //   R_Material materials[] = {
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
  if (timer_update(g.timer)) {
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
