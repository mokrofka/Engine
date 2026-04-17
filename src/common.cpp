#include "common.h"
#include "stb_image.h"
#include "test.cpp"
#include "gltf.cpp"
#include "game/game.cpp"

void game_update();
void* game_init();
void game_hotreload(void* ctx);

struct ProfilerState {
  ProfileAnchor anchors[KB(4)];
  u32 anchors_count;
  u64 tsc_start;
  u64 tsc_end;
  u32 profiler_parent;
  // Map<String, u32> map;
  u32 hash_to_indices[KB(4)];

  ProfileAnchor prev_anchors[4096];
  u32 prev_anchors_count;
  u64 prev_tsc_start;
  u64 prev_tsc_end;
  u64 prev_tsc_elapsed;
};

struct WatchFile {
  String path;
  DenseTime modified;
  void (**callback)();
};

struct WatchDirectory {
  String path;
  OS_Watch watch;
  void (**callback)(String name);
};

struct WatchState {
  Allocator alloc;
  Array<WatchFile, 128> watches;
  Array<WatchDirectory, 128> directories;
};

struct GlobalState {
  Arena arena;
  f32 dt;
  f32 time;
  b32 should_hotreload;
  Transform* transforms;
  Transform* static_transforms;
#if BUILD_DEBUG
  u32* generations;
  u32* static_generations;
#endif

  Handle<GpuMesh> meshes_handlers[Mesh_COUNT];
  Handle<GpuTexture> textures_handlers[Texture_COUNT];
  Handle<GpuMaterial> materials_handlers[Material_COUNT];

  String asset_path;
  String shader_dir;
  String shader_compiled_dir;
  String models_dir;
  String textures_dir;
  Map<String, Handle<GpuTexture>> str_to_texture;
  Map<String, Handle<GpuMesh>> str_to_mesh;
  Map<String, Handle<GpuMaterial>> str_to_material;

  WatchState watch;
  ProfilerState profiler;

  void* game_st;
  void* vk_st;

  void (*callback_should_reload)();
  void (*callback_shader_compile)(String name);
  void (*callback_shader_reload)(String name);
};

global GlobalState* g_st;

void _callback_should_hotreload() {
  g_st->should_hotreload = true;
}
void _callback_shader_compile(String name) {
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
}
void _callback_shader_reload(String name) {
  Scratch scratch;
  String shader_name_with_format = str_chop_last_dot(name);
  String shader_name = str_chop_last_dot(shader_name_with_format);
  vk_shader_reload(shader_name);
}
void global_st_update_callback() {
  GlobalState& g = *g_st;
  g.callback_should_reload = _callback_should_hotreload;
  g.callback_shader_compile = _callback_shader_compile;
  g.callback_shader_reload = _callback_shader_reload;
}

f32 get_dt() { return g_st->dt; }
f32 get_time() { return g_st->time; }
f32 get_was_hotreload() { return g_st->should_hotreload; }

Transform& entity_transform(Handle<Entity> handle) { 
  Assert(handle.generation() == g_st->generations[handle.idx()]);
  return g_st->transforms[handle.idx()];
}
Transform& static_entity_transform(Handle<StaticEntity> handle) {
  Assert(handle.generation() == g_st->static_generations[handle.idx()]);
  return g_st->static_transforms[handle.idx()];
}
#if BUILD_DEBUG
u32* entities_generations() { return g_st->generations; }
u32* static_entities_generations() { return g_st->static_generations; }
#endif

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

////////////////////////////////////////////////////////////////////////
// Profiler

ProfileBlock::ProfileBlock(String label_, String func, String str_to_hash) {
  ProfilerState& g = g_st->profiler;
  // b32 exists = false;
  // anchor_idx = *g.map.exists_or_add(str_to_hash, g.anchors_count, &exists);
  u32 idx = ModPow2(hash(str_to_hash, hash(func)), KB(4));
  anchor_idx = g.hash_to_indices[idx];
  // if (!exists) {
  //   ++g.anchors_count;
  // }
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

  anchor->parent_idx = parent_idx;
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
  if (get_was_hotreload()) return {};
  ProfilerState& g = g_st->profiler;
  return Slice(g.prev_anchors+1, g.prev_anchors_count-1);
}

u64 profiler_get_tsc_elapsed() { return g_st->profiler.prev_tsc_elapsed; }

ProfilerInfo profiler_get_info() {
  ProfilerState& g = g_st->profiler;
  ProfilerInfo info = {
    .tsc_start = g.prev_tsc_start,
    .tsc_end = g.prev_tsc_end,
    .tsc_elapsed = g.prev_tsc_elapsed,
  };
  return info;
}

u64 profiler_get_start_tsc() { return g_st->profiler.tsc_start; }
u64 profiler_get_end_tsc() { return g_st->profiler.tsc_end; }

////////////////////////////////////////////////////////////////////////
// Watch

void watch_add(String watch_name, void (**callback)()) {
  WatchState& g = g_st->watch;
  FileProperties props = os_file_path_properties(watch_name);
  WatchFile file_watch = {
    .path = watch_name,
    .modified = props.modified,
    .callback = callback,
  };
  g.watches.add(file_watch);
}

void watch_directory_add(String watch_name, void (**reload_callback)(String name), OS_WatchFlags flags) {
  WatchState& g = g_st->watch;
  String dir_path = push_strf(g.alloc, "%s", watch_name);
  OS_Watch watch = os_watch_open(flags);
  os_watch_attach(watch, dir_path);
  WatchDirectory dir_watch = {
    .path = dir_path,
    .watch = watch,
    .callback = reload_callback,
  };
  g.directories.add(dir_watch);
}

void watch_update() {
  WatchState& g = g_st->watch;
  Scratch scratch;
  for (WatchFile& x : g.watches) {
    FileProperties props = os_file_path_properties(x.path);
    if (props.modified > x.modified) {
      (*x.callback)();
      x.modified = props.modified;
    }
  }
  for (WatchDirectory x : g.directories) {
    StringList list = os_watch_check(scratch, x.watch);
    for (StringNode* node = list.first; node != null; node = node->next) {
      (*x.callback)(node->string);
    }
  }
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
// Common

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

String asset_base_path() { return g_st->asset_path; }

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

void common_init() {
  GlobalState& g = *g_st;
  global_allocator_init();
  // test();
  os_gfx_init();
  estimate_cpu_frequency();
  g.arena = arena_init();
  g.asset_path = push_strf(g.arena, "%s/%s", os_get_current_directory(), String("../assets"));
  g.shader_dir = push_str_cat(g.arena, asset_base_path(), "/shaders");
  g.shader_compiled_dir = push_str_cat(g.arena, g.shader_dir, "/compiled");
  g.models_dir = push_str_cat(g.arena, asset_base_path(), "/models");
  g.textures_dir = push_str_cat(g.arena, asset_base_path(), "/textures");
  watch_directory_add(g.shader_dir, &g.callback_shader_compile);
  watch_directory_add(g.shader_compiled_dir, &g.callback_shader_compile);
  g.transforms = push_array(g.arena, Transform, MaxEntities);
  g.static_transforms = push_array(g.arena, Transform, MaxStaticEntities);
#if BUILD_DEBUG
  g.generations = push_array(g.arena, u32, MaxEntities);
  g.static_generations = push_array(g.arena, u32, MaxStaticEntities);
#endif
  g.vk_st = vk_init();
  imgui_init();
  g.game_st = game_init();
}

shared_function void common_update(HotReloadData* data) {
  Scratch scratch;

if (data->ctx == null) {
    Arena arena = arena_init();
    data->ctx = push_struct_zero(arena, GlobalState);
    g_st = (GlobalState*)data->ctx;
    g_st->arena = arena;
    common_init();
    global_st_update_callback();
#if HOTRELOAD_BUILD
    GlobalState& g = *g_st;
    watch_add(data->lib, &g.callback_should_reload);
#endif
  }
  if (data->was_hotreload) {
    g_st = (GlobalState*)data->ctx;
    GlobalState& g = *g_st;
    g.should_hotreload = false;
    data->was_hotreload = false;

    global_st_update_callback();

    game_hotreload(g.game_st);
    vk_hotreload(g.vk_st);
  }

  GlobalState& g = *g_st;

  u64 target_fps = Billion(1) / 60;
  u64 last_time = os_now_ns();
  Timer timer = timer_init(1);

  while (!os_window_should_close()) {
    if (g_st->should_hotreload) {
      data->was_hotreload = true;
      goto hotreload;
    }

    profiler_begin();
    {TimeBlock("frame");
    os_pump_messages();
    u64 start_time = os_now_ns();
    g.dt = f32(start_time - last_time) / Billion(1);
    g.time += g.dt;
    last_time = start_time;
    {
      // TimeBlock("main");

    // Main logic
    vk_begin_draw_frame();
    ui_begin();
    game_update();
    ui_end();
    vk_end_draw_frame();
    os_input_update();
    watch_update();
    }

    u64 frame_duration = os_now_ns() - start_time;
    if (frame_duration < target_fps) {
      u64 sleep_time = target_fps - frame_duration;
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

////////////////////////////////////////////////////////////////////////
// Json
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
