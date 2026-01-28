#include "common.h"

////////////////////////////////////////////////////////////////////////
// Test

#define test_equal(actual, expected)                    \
  if (actual != expected) {                             \
    Error("Expected %i, but got %i", expected, actual); \
  }

#define test_not_equal(actual, expected)                \
  if (actual == expected) {                             \
    Error("Expected %i != %i, but equal", expected, actual); \
  }

#define test_true(actual)              \
  if (actual != true) {                \
    Error("Expected true, but false"); \
  }

#define test_false(actual)            \
  if (actual != false) {              \
    Error("Expected false, but true") \
  }

///////////////////////////////////
// Allocators

i32 alignments[] = { 8, 16, 32, 64 };

void test_global_alloc() {
  Array<u8*, 100> arr;

  Loop (i, 100) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = alignments[rand_range_u32(0, 3)];
    arr.append(global_alloc(size, align));
    MemZero(arr[i], size);
  }
  Array<u32, 100> indices;
  Loop(i, 100) indices.append(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    global_free(arr[indices[i]]);
  }

  arr.clear();
  Loop (i, 100) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = alignments[rand_range_u32(0, 3)];
    arr.append(global_alloc(size, align));
    MemZero(arr[i], size);
  }
  indices.clear();
  Loop(i, 100) indices.append(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    global_free(arr[indices[i]]);
  }
}

void test_arena_alloc() {
  Arena arena = arena_init();
  Array<u8*, 100> arr;
  Array<u32, 100> sizes;
  Array<u32, 100> values;
  Loop (i, 100) {
    u32 size = rand_range_u32(8, KB(1));
    u64 align = alignments[rand_range_u32(0, 3)];
    arr[i] = push_buffer(arena, size, align);
    sizes[i] = size;
    values[i] = rand_range_u32(0, 255);
    MemSet(arr[i], values[i], size);
  }
  Loop (i, 100) {
    u8* buff = arr[i];
    u32 size = sizes[i];
    u32 value = values[i];
    Loop (j, size) {
      Assert(buff[j] == value);
    }
  }
  arena_deinit(&arena);
}

void test_arena_list_alloc() {
  Scratch scratch;
  ArenaList arena(scratch);

  Array<u8*, 100> arr;
  Array<u32, 100> sizes;
  Array<u32, 100> values;
  Loop (i, 100) {
    u32 size = rand_range_u32(8, KB(1));
    u64 align = alignments[rand_range_u32(0, 3)];
    arr[i] = push_buffer(arena, size, align);
    sizes[i] = size;
    values[i] = rand_range_u32(0, 255);
    MemSet(arr[i], values[i], size);
  }
  Loop (i, 100) {
    u8* buff = arr[i];
    u32 size = sizes[i];
    u32 value = values[i];
    Loop (j, size) {
      Assert(buff[j] == value);
    }
  }

  arena.clear();

  Loop (i, 100) {
    u32 size = rand_range_u32(8, KB(1));
    u64 align = alignments[rand_range_u32(0, 3)];
    arr[i] = push_buffer(arena, size, align);
    sizes[i] = size;
    values[i] = rand_range_u32(0, 255);
    MemSet(arr[i], values[i], size);
  }
  Loop (i, 100) {
    u8* buff = arr[i];
    u32 size = sizes[i];
    u32 value = values[i];
    Loop (j, size) {
      Assert(buff[j] == value);
    }
  }

  arena.clear();
}

void test_seglist_alloc() {
  Scratch scratch;
  AllocSegList alloc(scratch);
  Array<u8*, 100> arr;

  Loop (i, 100) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = alignments[rand_range_u32(0, 3)];
    arr.append(mem_alloc(alloc, size, align));
    MemZero(arr[i], size);
  }
  Array<u32, 100> indices;
  Loop(i, 100) indices.append(i);
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
    u64 align = alignments[rand_range_u32(0, 3)];
    arr.append(mem_alloc(alloc, size, align));
    MemZero(arr[i], size);
  }
  indices.clear();
  Loop(i, 100) indices.append(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    mem_free(alloc, arr[indices[i]]);
  }
}

///////////////////////////////////
// Containters

void test_object_pool() {
  Scratch scratch;
  struct A {
    u32 a;
    u32 b;
  };
  ObjectPool<A> pool;
  Array<A, 100> values;
  Array<u32, 100> handlers;
  Loop (i, 100) {
    values[i].a = rand_range_u32(0, 100);
    values[i].b = rand_range_u32(0, 100);
  };
  Loop (i, 100) {
    handlers[i] = pool.append(values[i]);
  }
  Loop (i, 100) {
    Assert(MemMatchStruct(&values[i], &pool.get(handlers[i])));
  }
  Array<u32, 100> indices;
  Loop(i, 100) indices.append(i);
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
    handlers[i] = pool.append(values[i]);
  }
  Loop (i, 100) {
    Assert(MemMatchStruct(&values[i], &pool.get(handlers[i])));
  }
  Loop(i, 100) indices.append(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    pool.remove(handlers[i]);
  }
}

void test_handle_darray() {
  Scratch scratch;
  struct A {
    u32 a;
    u32 b;
  };
  HandlerDarray<A> arr;
  Array<A, 100> values;
  Array<u32, 100> handlers;
  Loop (i, 100) {
    values[i].a = rand_range_u32(0, 100);
    values[i].b = rand_range_u32(0, 100);
  };
  Loop (i, 100) {
    handlers[i] = arr.append(values[i]);
  }
  Loop (i, 100) {
    Assert(MemMatchStruct(&values[i], &arr[handlers[i]]));
  }
  Array<u32, 100> indices;
  Loop(i, 100) indices.append(i);
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
    handlers[i] = arr.append(values[i]);
  }
  Loop (i, 100) {
    Assert(MemMatchStruct(&values[i], &arr[handlers[i]]));
  }
  Loop(i, 100) indices.append(i);
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
  test_object_pool();
  i32 a = 1;
}

////////////////////////////////////////////////////////////////////////
// Profiler

global Profiler profiler_st;

u64 cpu_frequency() {
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
    profiler_st.anchors.append(&anchor_);
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
  u64 cpu_freq = cpu_frequency();
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
  Arena arena;
  Array<FileWatch, 128> watches;
  Array<DirectoryWatch, 128> directories;
};

global AssetWatchState file_watch_st;

void asset_watch_init() {
  file_watch_st.arena = arena_init();
}

void asset_watch_add(String watch_name, void (*callback)()) {
  FileProperties props = os_file_path_properties(watch_name);
  file_watch_st.watches.append({
    .path = watch_name,
    .modified = props.modified,
    .callback = callback,
  });
}

void asset_watch_directory_add(String watch_name, void (*reload_callback)(String name), OS_WatchFlags flags) {
  String dir_path = push_strf(file_watch_st.arena, "%s", watch_name);
  OS_Watch watch = os_watch_open(flags);
  os_watch_attach(watch, dir_path);
  file_watch_st.directories.append({
    .path = dir_path,
    .watch = watch,
    .callback = reload_callback,
  });
}

void asset_watch_update() {
  Scratch scratch;
  for (FileWatch& x : file_watch_st.watches) {
    FileProperties props = os_file_path_properties(x.path);
    if (props.modified > x.modified) {
      x.callback();
      x.modified = props.modified;
    }
  }
  for (DirectoryWatch x : file_watch_st.directories) {
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
  Arena* arena;
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
  event_st.registered[code].append({listener, on_event});
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
// Common

Extern Transform entities_transforms[MaxEntities];

Extern f32 delta_time;

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

  {
    common_st.shader_dir = push_str_cat(common_st.arena, asset_base_path(), "/shaders");
    common_st.shader_compiled_dir = push_str_cat(common_st.arena, common_st.shader_dir, "/compiled");
    asset_watch_directory_add(push_str_cat(scratch, asset_base_path(), "/shaders"), [](String name) {
      Scratch scratch;
      String shader_filepath = push_strf(scratch, "%s/%s", common_st.shader_dir, name);
      String shader_compiled_filepath = push_strf(scratch, "%s/%s%s", common_st.shader_compiled_dir, name, String(".spv"));
      StringList list = {};
      str_list_pushf(scratch, &list, "glslangValidator");
      str_list_pushf(scratch, &list, "-V");
      str_list_pushf(scratch, &list, "%s", shader_filepath);
      str_list_pushf(scratch, &list, "-o");
      str_list_pushf(scratch, &list, "%s", shader_compiled_filepath);
      // String cmd = push_strf(scratch, "glslangValidator -V %s -o %s", shader_filepath, shader_compiled_filepath);
      os_process_launch(list);
    }, OS_WatchFlag_Modify);
    asset_watch_directory_add(push_str_cat(scratch, asset_base_path(), "/shaders/compiled"), [](String name) {
      Scratch scratch;
      String shader_name_with_format = str_chop_last_dot(name);
      String shader_name = str_chop_last_dot(shader_name_with_format);
      u32* id = common_st.shader_map.get(shader_name);
      vk_shader_reload(shader_name, *id);
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
  return common_st.asset_path;
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

  Darray<Vertex> vertices(common_st.arena);
  Darray<u32> final_indices(common_st.arena);
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
  Buffer buff1 = os_file_read_all(scratch, push_strf(scratch, "%s/%s", model_dir, info.file_name));
  v3* vertices_pos = (v3*)Offset(buff1.data, info.ranges[0].offset);
  v3* vertices_norm = (v3*)Offset(buff1.data, info.ranges[1].offset);
  v2* vertices_uv = (v2*)Offset(buff1.data, info.ranges[2].offset);
  u16* vertices_indices = (u16*)Offset(buff1.data, info.ranges[3].offset);
  Vertex* vertices = push_array(common_st.arena, Vertex, info.vert_count);
  u32* indices = push_array(common_st.arena, u32, info.index_count);
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

Mesh mesh_load_glb(String name) {
  Scratch scratch;
  Buffer buff = os_file_read_all(scratch, name);
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
  FileHeader* header = (FileHeader*)buff.data;
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
  JsonReader r = json_reader_init({(u8*)&json_chunk->chunk_data, buff.size});
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
  Vertex* vertices = push_array(common_st.arena, Vertex, vertex_count);
  u32* indices = push_array(common_st.arena, u32, index_count);
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

u32 mesh_load(String name) {
  Scratch scratch;
  String filepath = push_strf(scratch, "%s/%s/%s", asset_base_path(), String("models"), name);
  String format = str_skip_last_dot(name);
  Mesh mesh = {};
  if (str_match(format, "glb")) {
    mesh = mesh_load_glb(filepath);
  } else if (str_match(format, "gltf")) {
    mesh = mesh_load_gltf(filepath);
  } else if (str_match(format, "obj")) {
    mesh = mesh_load_obj(filepath);
  } else {
    Assert(true);
  }
  u32 id = vk_mesh_load(mesh);
  return id;
}

u32 shader_load(String name, ShaderType type) {
  Scratch scratch;
  u32 id = vk_shader_load(name, type);
  common_st.shader_map.insert(name, id);
  return id;
}

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
  texture.data = data;
  return texture;
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
    // str_skip_last_dot();
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

b32 timer_tick(Timer& t, f32 dt) {
  t.passed += dt;
  if (t.passed >= t.interval) {
    t.passed = 0;
    return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////
// Assets

///////////////////////////////////
// Shaders

global ShaderDefinition shaders_definition_[Shader_COUNT-1] = {
  [Shader_Color-1] = "color_shader", ShaderType_Drawing,
};
ShaderDefinition shaders_definition(u32 idx) {
  IsInRange<u32>(1, idx, Shader_COUNT);
  Assert(IsInRange<u32>(1, idx, Shader_COUNT));
  return shaders_definition_[idx-1];
}
global u32 shaders_[Shader_COUNT-1];
u32& shaders(u32 idx) { 
  Assert(IsInRange((u32)1, idx, (u32)Shader_COUNT));
  return shaders_[idx-1];
}

///////////////////////////////////
// Meshes

global String meshes_path_[Mesh_COUNT-1] = {
  // [Mesh_Cube-1] = "cube.obj",
  [Mesh_GltfCube-1] = "cube.gltf",
  // [Mesh_GltfHelmet-1] = "helmet.gltf",
  // [Mesh_GlbHelmet-1] = "helmet.glb",
  // [Mesh_GlbMonkey-1] = "monkey.glb",
  // [Mesh_Room] = "room.obj",
};
String meshes_path(u32 idx) {
  Assert(IsInRange<u32>(1, idx, Mesh_COUNT));
  return meshes_path_[idx-1];
}
global u32 meshes_[Mesh_COUNT-1];
u32& meshes(u32 idx) {
  Assert(IsInRange<u32>(1, idx, Mesh_COUNT));
  return meshes_[idx-1];
}

///////////////////////////////////
// Textures

global String textures_path_[Texture_COUNT-1] = {
  [Texture_OrangeLines-1] = "orange_lines_512.png",
  [Texture_Container-1] = "container.jpg",
  // [Texture_Room-1] = "image.png",
};
String textures_path(u32 idx) { 
  Assert(IsInRange<u32>(1, idx, Texture_COUNT));
  return textures_path_[idx-1];
}
global u32 textures_[Texture_COUNT-1];
u32& textures(u32 idx) {
  Assert(IsInRange<u32>(1, idx, Texture_COUNT));
  return textures_[idx-1];
}


