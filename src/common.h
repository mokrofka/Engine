#pragma once
#include "lib.h"

// TODO:
// dummy assets/null 
// memory visualisation
// thread graph visualisation
// obj mouse selection
// serelization/deserialization
// UI rendering
// profiler
// make wayland backend work
// linux crushes when I try sleep and mount?

const v3 ColorRed   = v3(1,0,0);
const v3 ColorGreen = v3(0,1,0);
const v3 ColorBlue  = v3(0,0,1);
const v3 ColorWhite = v3(1,1,1);
const v3 ColorBlack = v3(0,0,0);
const v3 ColorGrey  = v3(0.8,0.8,0.8);

const u32 MaxEntities = KB(10);
const u32 MaxStaticEntities = KB(100);

struct Entity;
struct StaticEntity;
struct GpuTexture;
struct GpuMesh;
struct GpuShader;
struct GpuMaterial;
struct GpuCubemap;

f32 get_dt();
f32 get_time();
f32 get_was_hotreload();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// vk.cpp

struct PointLight {
  v3 color;
  v3 pos;
  f32 intensity;
  f32 rad;
};

struct DirLight {
  v3 color;
  v3 dir;
  f32 intensity;
};

struct SpotLight {
  v3 color;
  v3 pos;
  v3 dir;
  f32 intensity;
  f32 inner_cutoff;
  f32 outer_cutoff;
};

struct MaterialProps {
  v3 ambient;
  v3 diffuse;
  v3 specular;
  f32 shininess;
};

struct Texture {
  u32 width;
  u32 height;
  u8* data;
};

struct Vertex {
  v3 pos;
  v3 norm;
  v2 uv;
  v3 color;
};
u64 hash(Vertex vert);
b32 equal(Vertex a, Vertex b);

struct Mesh {
  Vertex* vertices;
  u32* indices;
  u32 vert_count;
  u32 index_count;
};

enum ShaderType {
  ShaderType_Drawing,
  ShaderType_Screen,
  ShaderType_Cube,
  ShaderType_Compute,
};

enum ShaderTopology {
  ShaderTopology_Triangle,
  ShaderTopology_Line,
  ShaderTopology_Point,
};

struct ShaderState {
  ShaderType type;
  ShaderTopology topology;
  u32 samples = 4;
  b8 is_transparent;
  b8 use_depth = true;
};

struct Shader {
  String name;
  ShaderState state;
};

struct Material {
  Shader shader;
  MaterialProps props;
  String texture;
  Handle<GpuTexture> texture_handle;
  // Handle<GpuTexture> texture;
  // Handle<GpuTexture> texture1;
  // Handle<GpuTexture> texture2;
  // Handle<GpuTexture> texture3;
};

void* vk_init();
void vk_shutdown();
void vk_hotreload(void* ctx);

Handle<GpuTexture> vk_texture_load(Texture texture);
Handle<GpuMaterial> vk_material_load(Material material);
Handle<GpuCubemap> vk_cubemap_load(Texture* textures);
Handle<GpuMesh> vk_mesh_load(Mesh mesh);

void vk_shader_reload(String name);

void vk_begin_draw_frame();
void vk_end_draw_frame();

// Entity
void vk_make_renderable_(Handle<Entity> entity_handle, Handle<GpuMesh> mesh_handle, Handle<GpuMaterial> material_handle);
void vk_make_renderable_static(Handle<StaticEntity> entity_handle, Handle<GpuMesh> mesh_handle, Handle<GpuMaterial> material_handle);
void vk_remove_renderable(Handle<Entity> entity_handle);
void vk_set_entity_color(Handle<Entity> entity_handle, v4 color);

mat4& vk_get_view();
mat4& vk_get_projection();

void debug_draw_line(v3 a, v3 b, v3 color);
void debug_draw_aabb(v3 min, v3 max, v3 color);
void draw_squad(v2 min, v2 max, v3 color);

void ui_begin();
void ui_end();
void ui_push_box(String str);
void ui_pop_box();
b32 ui_begin_window(u32 id, v2 size);
b32 ui_button(u32 id, v2 min, v2 max);

void imgui_init();
void imgui_begin_frame();
void imgui_end_frame();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// common.cpp

Transform& entity_transform(Handle<Entity> handle);
Transform& static_entity_transform(Handle<StaticEntity> handle);

#if BUILD_DEBUG
u32* entities_generations();
u32* static_entities_generations();
#endif

////////////////////////////////////////////////////////////////////////
// Assets

#define MESH_LIST \
  X(Mesh_MonkeyGlb, monkey.glb) \
  X(Mesh_Cube, cube.glb) \
  // X(Mesh_Castle, castle.obj) \

enum MeshId {
#define X(enum_name, name) enum_name,
  MESH_LIST
#undef X
  Mesh_Load_COUNT,
  
  Mesh_Triangle,
  Mesh_Grid,
  Mesh_Axis,
  Mesh_Sphere,
  Mesh_COUNT,
};

#define TEXTURE_LIST \
  X(Texture_Orange, orange_lines_512.png) \
  X(Texture_Container, container.jpg) \
  // X(Texture_Castle, castle_diffuse.png) \

enum TextureId {
#define X(enum_name, name) enum_name,
  TEXTURE_LIST
#undef X
  Texture_COUNT,
};

#define MATERIAL_LIST \
  X(Material_Orange, .name = "e_texture", .texture = "orange_lines_512.png") \
  X(Material_Container, .name = "e_texture", .texture = "container.jpg") \
  X(Material_Axis, .name = "e_vert_color", .state.topology = ShaderTopology_Line) \
  X(Material_Line, .name = "e_color", .state.topology = ShaderTopology_Line) \
  X(Material_Screen, .name = "e_texture", .texture_handle.handle = null, .texture = "") \

enum MaterialId {
#define X(enum_name, ...) enum_name,
  MATERIAL_LIST
#undef X
  Material_COUNT,
};

Handle<GpuMesh> mesh_get(MeshId id);
void mesh_set(MeshId id, Handle<GpuMesh> mesh_handle);
Handle<GpuTexture> texture_get(TextureId id);
Handle<GpuMaterial> material_get(MaterialId id);

void asset_load();

////////////////////////////////////////////////////////////////////////
// Profiler

struct ProfilerInfo {
  u64 tsc_start;
  u64 tsc_end;
  u64 tsc_elapsed;
};

struct ProfileAnchor {
  u64 tsc_elapsed_exclusive; // without children
  u64 tsc_elapsed_inclusive; // with children
  u64 hit_count;
  String label;
  // for fraph
  u32 parent_idx;
  u64 tsc_start;
  u64 tsc_end;
};

struct ProfileBlock {
  u64 old_tsc_elapsed_inclusive;
  u64 start_tsc;
  u32 parent_idx;
  u32 anchor_idx;
  ProfileBlock(String label_, String func, String str_to_hash);
  ~ProfileBlock();
};

void profiler_begin();
void profiler_end();
Slice<ProfileAnchor> profiler_get_anchors();
u64 profiler_get_tsc_elapsed();
ProfilerInfo profiler_get_info();

#define TimeBlock(Name) ProfileBlock Glue(__profiler_block, __LINE__)(Name, __func__, Name)
#define TimeFunction TimeBlock(__func__)

// #define TimeBlock(Name)
// #define TimeFunction

////////////////////////////////////////////////////////////////////////
// Asset watcher

void watch_add(String watch_name, void (**callback)());
void watch_directory_add(String watch_name, void (**reload_callback)(String name), OS_WatchFlags flags = OS_WatchFlag_Modify);
void watch_update();

////////////////////////////////////////////////////////////////////////
// Common

void common_init();
void r_shutdown();

String asset_base_path();
Handle<GpuMesh> mesh_load(String name);
Handle<GpuShader> shader_load(Shader shader);
Handle<GpuTexture> texture_load(String name);
Handle<GpuCubemap> cubemap_load(String name);

struct Timer {
  f32 passed;
  f32 interval;
};

Timer timer_init(f32 interval);
b32 timer_tick(Timer& t);

////////////////////////////////////////////////////////////////////////
// Json

enum JsonType {
  JsonType_Error,
  JsonType_Bool,
  JsonType_Number,
  JsonType_String,
  JsonType_Array,
  JsonType_Object,
  JsonType_Null,
  JsonType_End,
};

struct JsonValue {
  JsonType type;
  String str;
  i32 depth;
  b32 match(String name) { return str_match(str, name); };
};

struct JsonReader {
  u8* cur;
  u8* end;
  i32 depth;
  String error;
  JsonValue base_obj;
};

JsonReader json_reader_init(String buffer);
b32 json_iter_object(JsonReader* r, JsonValue obj, JsonValue *key, JsonValue *val);
b32 json_iter_array(JsonReader* r, JsonValue arr, JsonValue* val);

#define JSON_OBJ(r, o) for (JsonValue k, v; json_iter_object(&r, o, &k, &v);)
#define JSON_OBJ_(r, o) for (JsonValue key, val; json_iter_object(&r, o, &key, &val);)
#define JSON_ARR(r, val) for (JsonValue obj; json_iter_array(&r, val, &obj);)
