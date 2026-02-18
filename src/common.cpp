#include "common.h"
#include "stb_image.h"

Extern f32 g_dt;
Extern f32 g_time;
Extern Transform entities_transforms[MaxEntities];

////////////////////////////////////////////////////////////////////////
// Assets

///////////////////////////////////
// Shaders

Extern ShaderInfo shader_type[] = {
  // Drawing
  [ShaderType_Drawing] = {},
  [ShaderType_DrawingTransparent] = {
    .is_transparent = true,
  },
  [ShaderType_DrawingTransparentLine] = {
    .primitive = ShaderTopology_Line,
    .is_transparent = true,
  },

  // Screen
  [ShaderType_Screen] = {
    .use_depth = false,
  },

  // Cubemap
  [ShaderType_Cubemap] = {},

  // Compute
  [ShaderType_Compute] = {},
};
Extern ShaderDefinition shaders_info[Shader_COUNT] = {
  [Shader_Color] = "color_shader", ShaderType_Drawing,
  [Shader_Grid] = "grid_shader", ShaderType_DrawingTransparentLine,
  [Shader_Axis] = "axis_shader", ShaderType_DrawingTransparentLine,
};
Extern u32 shaders[Shader_COUNT];

///////////////////////////////////
// Meshes

String meshes_path[Mesh_Load_COUNT] = {
  [Mesh_GltfCube] = "cube.gltf",
  [Mesh_GlbCube] = "cube.glb",
};
Extern u32 meshes[Mesh_COUNT];

///////////////////////////////////
// Textures

Extern String textures_path[Texture_COUNT] = {
  [Texture_OrangeLines] = "orange_lines_512.png",
  [Texture_Container] = "container.jpg",
};
Extern u32 textures[Texture_COUNT];

///////////////////////////////////
// Materials

Extern Material materials_info[Material_COUNT] = {
  [Material_RedOrange] = {
    .ambient = v3(1,0,0),
    .diffuse = v3_scale(1),
    .specular = v3_scale(1),
    .shininess = 1,
    .texture = Texture_OrangeLines,
  },
  [Material_GreenContainer] = {
    .ambient = v3(0,1,0),
    .diffuse = v3_scale(1),
    .specular = v3_scale(1),
    .shininess = 1,
    .texture = Texture_Container,
  },
};
Extern u32 materials[Material_COUNT];

////////////////////////////////////////////////////////////////////////
// Test

///////////////////////////////////
// Allocators

global i32 test_alignments[] = { 8, 16, 32, 64 };

intern void test_global_alloc() {
  Array<u8*, 100> arr = {};
  Allocator alloc = mem_get_global_allocator();
  Loop (i, 100) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr.add(mem_alloc(alloc, size, align));
    MemZero(arr[i], size);
  }
  Array<u32, 100> indices = {};
  Loop(i, 100) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    mem_free(alloc, arr[indices[i]]);
  }
  arr.clear();
  Loop (i, 100) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr.add(mem_alloc(alloc, size, align));
    MemZero(arr[i], size);
  }
  indices.clear();
  Loop(i, 100) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    mem_free(alloc, arr[indices[i]]);
  }
}

intern void test_arena_alloc() {
  Arena arena = arena_init();
  Array<u8*, 100> arr = {};
  Array<u32, 100> sizes = {};
  Array<u32, 100> values = {};
  Loop (i, 100) {
    u32 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr[i] = push_buffer(arena, size, align);
    sizes[i] = size;
    values[i] = rand_range_u32(0, 255);
    MemSet(arr[i], values[i], size);
  }
  Loop (i, 100) {
    u8* buf = arr[i];
    u32 size = sizes[i];
    u32 value = values[i];
    Loop (j, size) {
      Assert(buf[j] == value);
    }
  }
  arena_deinit(&arena);
}

intern void test_arena_list_alloc() {
  Scratch scratch;
  ArenaList arena(scratch);
  Array<u8*, 100> arr = {};
  Array<u32, 100> sizes = {};
  Array<u32, 100> values = {};
  Loop (i, 100) {
    u32 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr[i] = push_buffer(arena, size, align);
    sizes[i] = size;
    values[i] = rand_range_u32(0, 255);
    MemSet(arr[i], values[i], size);
  }
  Loop (i, 100) {
    u8* buf = arr[i];
    u32 size = sizes[i];
    u32 value = values[i];
    Loop (j, size) {
      Assert(buf[j] == value);
    }
  }
  arena.clear();
  Loop (i, 100) {
    u32 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr[i] = push_buffer(arena, size, align);
    sizes[i] = size;
    values[i] = rand_range_u32(0, 255);
    MemSet(arr[i], values[i], size);
  }
  Loop (i, 100) {
    u8* buf = arr[i];
    u32 size = sizes[i];
    u32 value = values[i];
    Loop (j, size) {
      Assert(buf[j] == value);
    }
  }
  arena.clear();
}

intern void test_seglist_alloc() {
  Scratch scratch;
  AllocSegList alloc(scratch);
  Array<u8*, 100> arr = {};
  Loop (i, 100) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr.add(mem_alloc(alloc, size, align));
    MemZero(arr[i], size);
  }
  Array<u32, 100> indices = {};
  Loop(i, 100) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    mem_free(alloc, arr[indices[i]]);
  }
  arr.clear();
  Loop (i, 100) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr.add(mem_alloc(alloc, size, align));
    MemZero(arr[i], size);
  }
  indices.clear();
  Loop(i, 100) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    mem_free(alloc, arr[indices[i]]);
  }
}

intern void test_gpu_seglist_alloc() {
  Scratch scratch;
  GpuAllocSegList alloc = {.cap = MB(1)};
  alloc.init(scratch);
  Array<GpuMemHandler, 100> arr = {};
  Loop (i, 100) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr.add(alloc.alloc(size, align));
    GpuAllocSegList::RangeList range = alloc.data[arr[i]];
  }
  Array<u32, 100> indices = {};
  Loop(i, 100) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    alloc.free(arr[indices[i]]);
  }
  arr.clear();
  Loop (i, 100) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr.add(alloc.alloc(size, align));
  }
  indices.clear();
  Loop(i, 100) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    alloc.free(arr[indices[i]]);
  }
}

///////////////////////////////////
// Containters

intern void test_object_pool() {
  Scratch scratch;
  struct A {
    u32 a;
    u32 b;
  };
  ObjectPool<A> pool(scratch);
  Array<A, 100> values = {};
  Array<u32, 100> handlers = {};
  Loop (i, 100) {
    values[i].a = rand_range_u32(0, 100);
    values[i].b = rand_range_u32(0, 100);
  };
  Loop (i, 100) {
    handlers[i] = pool.add(values[i]);
  }
  Loop (i, 100) {
    Assert(MemMatchStruct(&values[i], &pool.get(handlers[i])));
  }
  Array<u32, 100> indices = {};
  Loop(i, 100) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    pool.remove(handlers[i]);
  }
  indices.clear();
  Loop (i, 100) {
    values[i].a = rand_range_u32(0, 100);
    values[i].b = rand_range_u32(0, 100);
  };
  Loop (i, 100) {
    handlers[i] = pool.add(values[i]);
  }
  Loop (i, 100) {
    Assert(MemMatchStruct(&values[i], &pool.get(handlers[i])));
  }
  Loop(i, 100) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    pool.remove(handlers[i]);
  }
}

intern void test_handle_darray() {
  Scratch scratch;
  struct A {
    u32 a;
    u32 b;
  };
  DarrayHandler<A> arr = {};
  Array<A, 100> values = {};
  Array<u32, 100> handlers = {};
  Loop (i, 100) {
    values[i].a = rand_range_u32(0, 100);
    values[i].b = rand_range_u32(0, 100);
  };
  Loop (i, 100) {
    handlers[i] = arr.add(values[i]);
  }
  Loop (i, 100) {
    Assert(MemMatchStruct(&values[i], &arr.get(handlers[i])));
  }
  Array<u32, 100> indices = {};
  Loop(i, 100) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    arr.remove(handlers[i]);
  }
  indices.clear();
  Loop (i, 100) {
    values[i].a = rand_range_u32(0, 100);
    values[i].b = rand_range_u32(0, 100);
  };
  Loop (i, 100) {
    handlers[i] = arr.add(values[i]);
  }
  Loop (i, 100) {
    Assert(MemMatchStruct(&values[i], &arr.get(handlers[i])));
  }
  Loop(i, 100) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    arr.remove(handlers[i]);
  }
}

void test() {
  test_global_alloc();
  test_arena_alloc();
  test_arena_list_alloc();
  test_seglist_alloc();
  test_gpu_seglist_alloc();
  test_object_pool();
}

////////////////////////////////////////////////////////////////////////
// Profiler

struct Profiler {
  Array<ProfileAnchor*, 4096> anchors;
  u64 start_TSC;
  u64 end_TSC;
  ProfileAnchor* profiler_parent;
};

global Profiler profiler_st;

u64 profiler_cpu_frequency() {
  u64 os_freq = os_timer_frequency();
  u64 cpu_start = cpu_timer_now();
  u64 os_start = os_timer_now();
  u64 milliseconds = 1;
  u64 os_end = 0;
  u64 os_elapsed = 0;
  u64 os_wait_time = os_freq * milliseconds / 1000;
  while (os_elapsed < os_wait_time) {
    os_end = os_timer_now();
    os_elapsed = os_end - os_start;
  }
  u64 cpu_end = cpu_timer_now();
  u64 cpu_elapsed = cpu_end - cpu_start;
  u64 cpu_freq = 0;
  if (cpu_elapsed) {
    cpu_freq = os_freq * cpu_elapsed / os_elapsed;
  }
  return cpu_freq;
}

ProfileBlock::ProfileBlock(String label_, ProfileAnchor& anchor_) {
  if (!anchor_.label.size) {
    profiler_st.anchors.add(&anchor_);
  }
  parent = profiler_st.profiler_parent;
  anchor = &anchor_;
  anchor->label = label_;
  profiler_st.profiler_parent = anchor;
  old_TSC_elapsed_at_root = anchor->TSC_elapsed_at_root;
  start_TSC = cpu_timer_now();
}

ProfileBlock::~ProfileBlock() {
  u64 elapsed = cpu_timer_now() - start_TSC;
  profiler_st.profiler_parent = parent;
  if (parent) {
    parent->TSC_elapsed_children += elapsed;
  }
  anchor->TSC_elapsed_at_root = old_TSC_elapsed_at_root + elapsed;
  anchor->TSC_elapsed += elapsed;
  ++anchor->hit_count;
}

void profiler_print_time_elapsed(u64 total_TSC_elapsed, ProfileAnchor* anchor) {
  u64 elapsed = anchor->TSC_elapsed - anchor->TSC_elapsed_children;
  f64 percent = 100.0 * ((f64)elapsed / (f64)total_TSC_elapsed);
  print("  %s[%u64]: %u64 (%.2f%%)", anchor->label, anchor->hit_count, elapsed, percent);
  if (anchor->TSC_elapsed_at_root != elapsed) {
    f64 percent_with_children = 100.0 * ((f64)anchor->TSC_elapsed / (f64)total_TSC_elapsed);
    print(", %.2f%% w/children", percent_with_children);
  }
  println("");
}

void profiler_begin() {
  profiler_st.start_TSC = cpu_timer_now();
}

void profiler_end_and_print_() {
  profiler_st.end_TSC = cpu_timer_now();
  u64 cpu_freq = profiler_cpu_frequency();
  u64 total_CPU_elapsed = profiler_st.end_TSC - profiler_st.start_TSC;
  if (cpu_freq) {
    Info("Total time: %.4fms (CPU freq %u64)", 1000.0 * (f64)total_CPU_elapsed / (f64)cpu_freq, cpu_freq);
  }
  for (ProfileAnchor* x : profiler_st.anchors) {
    profiler_print_time_elapsed(total_CPU_elapsed, x);
  }
}

// https://github.com/rxi/sj.h.git
////////////////////////////////////////////////////////////////////////
// Json

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
// Asset watcher

struct FileWatch {
  String path;
  DenseTime modified;
  void (*callback)();
};

struct DirectoryWatch {
  String path;
  OS_Watch watch;
  void (*callback)(String name);
};

struct AssetWatchState {
  Allocator alloc;
  Array<FileWatch, 128> watches;
  Array<DirectoryWatch, 128> directories;
};

global AssetWatchState asset_watch_st;

void asset_watch_init(Allocator alloc) {
  asset_watch_st.alloc = alloc;
}

void asset_watch_add(String watch_name, void (*callback)()) {
  FileProperties props = os_file_path_properties(watch_name);
  FileWatch file_watch = {
    .path = watch_name,
    .modified = props.modified,
    .callback = callback,
  };
  asset_watch_st.watches.add(file_watch);
}

void asset_watch_directory_add(String watch_name, OS_WatchFlags flags, void (*reload_callback)(String name)) {
  String dir_path = push_strf(asset_watch_st.alloc, "%s", watch_name);
  OS_Watch watch = os_watch_open(flags);
  os_watch_attach(watch, dir_path);
  DirectoryWatch dir_watch = {
    .path = dir_path,
    .watch = watch,
    .callback = reload_callback,
  };
  asset_watch_st.directories.add(dir_watch);
}

void asset_watch_update() {
  Scratch scratch;
  for (FileWatch& x : asset_watch_st.watches) {
    FileProperties props = os_file_path_properties(x.path);
    if (props.modified > x.modified) {
      x.callback();
      x.modified = props.modified;
    }
  }
  for (DirectoryWatch x : asset_watch_st.directories) {
    StringList list = os_watch_check(scratch, x.watch);
    for (StringNode* node = list.first; node != null; node = node->next) {
      x.callback(node->string);
    }
  }
}

////////////////////////////////////////////////////////////////////////
// Events

struct RegisteredEvent {
  void* listener;
  PFN_On_Event callback;
};

struct EventSystemState {
  Darray<RegisteredEvent> registered[EventCode_COUNT];
};

global EventSystemState event_st;

void event_register(u32 code, void* listener, PFN_On_Event on_event) {
  for (RegisteredEvent e : event_st.registered[code]) {
    if(e.listener == listener) {
      Warn("You're registering the same event!");
      return;
    }
  }
  event_st.registered[code].add({listener, on_event});
}

void event_unregister(u32 code, void* listener, PFN_On_Event on_event) {
  if (event_st.registered[code].count == 0) {
    Assert(!"you're trying to unregister nothing!");
  }
  u32 index = 0;
  for (RegisteredEvent e : event_st.registered[code]) {
    if (e.listener == listener && e.callback == on_event) {
      event_st.registered[code].swap_remove(index);
    }
    ++index;
  }
}

void event_fire(u32 code, void* sender, EventContext context) {
  for (RegisteredEvent e : event_st.registered[code]) {
    if (e.callback(code, sender, e.listener, context)) {
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////////
// Loaders

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
      vertices.add(vertex);
    }
    final_indices.add(vertex_index);
  }
  Mesh mesh = {
    .vertices = vertices.data,
    .indexes = (u32*)final_indices.data,
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
    .indexes = (u32*)indices,
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
    .indexes = indices,
    .vert_count = vertex_count,
    .index_count = index_count,
  };
  return mesh;
}

intern Texture texture_image_load(String name) {
  Scratch scratch;
  Texture texture = {};
  u32 required_channel_count = 4;
  u32 channel_count;
  String filepath = push_strf(scratch, "%s/%s/%s", asset_base_path(), String("textures"), name);
  Buffer buf = os_file_path_read_all(scratch, filepath);
  u8* data = stbi_load_from_memory(buf.data, buf.size, (i32*)&texture.width, (i32*)&texture.height, (i32*)&channel_count, required_channel_count);
  Assert(data);
  texture.data = data;
  return texture;
}

////////////////////////////////////////////////////////////////////////
// Common

struct CommonState {
  Arena arena;
  String asset_path;
  String shader_dir;
  String shader_compiled_dir;
  Map<String, u32> shader_map;
};

global CommonState common_st;

void common_init() {
  Scratch scratch;
  common_st.arena = arena_init();
  common_st.asset_path = push_strf(common_st.arena, "%s/%s", os_get_current_directory(), String("../assets"));
  common_st.shader_dir = push_str_cat(common_st.arena, asset_base_path(), "/shaders");
  common_st.shader_compiled_dir = push_str_cat(common_st.arena, common_st.shader_dir, "/compiled");
  asset_watch_directory_add(common_st.shader_dir, OS_WatchFlag_Modify, [](String name) {
    Scratch scratch;
    String shader_filepath = push_strf(scratch, "%s/%s", common_st.shader_dir, name);
    String shader_compiled_filepath = push_strf(scratch, "%s/%s%s", common_st.shader_compiled_dir, name, String(".spv"));
    StringList list = {};
    str_list_pushf(scratch, &list, "glslangValidator");
    str_list_pushf(scratch, &list, "-V");
    str_list_pushf(scratch, &list, "%s", shader_filepath);
    str_list_pushf(scratch, &list, "-o");
    str_list_pushf(scratch, &list, "%s", shader_compiled_filepath);
    os_process_launch(list);
  });
  asset_watch_directory_add(common_st.shader_compiled_dir, OS_WatchFlag_Modify, [](String name) {
    Scratch scratch;
    String shader_name_with_format = str_chop_last_dot(name);
    String shader_name = str_chop_last_dot(shader_name_with_format);
    u32* id = common_st.shader_map.get(shader_name);
    vk_shader_reload(shader_name, *id);
  });
  vk_init();
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
  return common_st.asset_path;
}

u32 mesh_load(String name) {
  Scratch scratch;
  String filepath = push_strf(scratch, "%s/%s/%s", asset_base_path(), String("models"), name);
  String format = str_skip_last_dot(name);
  Mesh mesh = {};
  if (str_match(format, "glb")) {
    mesh = mesh_load_glb(scratch, filepath);
  } else if (str_match(format, "gltf")) {
    mesh = mesh_load_gltf(scratch, filepath);
  } else if (str_match(format, "obj")) {
    mesh = mesh_load_obj(scratch, filepath);
  } else {
    Assert(true);
  }
  u32 id = vk_mesh_load(mesh);
  return id;
}

u32 shader_load(String name, ShaderType type) {
  Scratch scratch;
  u32 id = vk_shader_load(name, type);
  common_st.shader_map.add(name, id);
  return id;
}

u32 texture_load(String name) {
  Texture texture = texture_image_load(name);
  u32 id = vk_texture_load(texture);
  return id;
}

u32 cubemap_load(String name) {
  Scratch scratch;
  Texture textures[6];
  String sides[] = {
    "right", "left",
    "top", "bottom",
    "front", "back",
  };
  Loop (i, ArrayCount(textures)) {
    String texture_name = push_strf(scratch, "%s/%s%s", name, sides[i], String(".png"));
    textures[i] = texture_image_load(texture_name);
  }
  vk_cubemap_load(textures);
  return 0;
}

Timer timer_init(f32 interval) {
  Timer timer = {
    .interval = interval,
  };
  return timer;
}

b32 timer_tick(Timer& t) {
  t.passed += g_dt;
  if (t.passed >= t.interval) {
    t.passed = 0;
    return true;
  }
  return false;
}
