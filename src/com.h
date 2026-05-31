#pragma once
#include "lib.h"
#include "tokenizer.h"
#include "meta.h"

// TODO:
// dummy assets/null 
// obj mouse selection
// UI rendering
// memory visualisation thread safe, and gpu memory
// thread graph visualisation
// sample profiler
// make wayland backend work
// fix static non indexed - doesn't render
// console
// thread safe allocator
// async
// glb loader
// introspection
// upgrade vulkan
// font rendering
// reading json 
// editor
// full metaprogramming
// metadesk tables?
// serelization/deserialization
// imgui draw text font

////////////////////////////////////////////////////////////////////////
// @Common

#define IM_VEC2_CLASS_EXTRA                               \
        constexpr ImVec2(const v2& f) : x(f.x), y(f.y) {} \
        operator v2() const { return v2(x,y); }
#include "imgui/imgui.h"

struct ImGui_DrawList {
  ImDrawList* draw;
};

ImGui_DrawList imgui_get_window_drawlist();
void imgui_draw_rect(ImGui_DrawList draw, Rng2 rect, v4 col, f32 rounding = 0, ImDrawFlags flags = 0, f32 thickness = 1);
void imgui_draw_rect_filled(ImGui_DrawList draw, Rng2 rect, v4 col, f32 rounding = 0, ImDrawFlags flags = 0);
void imgui_draw_push_clip_rect(ImGui_DrawList draw, Rng2 rect);
void imgui_draw_pop_clip_rect(ImGui_DrawList draw);
void imgui_draw_line(ImGui_DrawList draw, v2 p0, v2 p1, v4 col, f32 thickness = 1);
void imgui_draw_text(ImGui_DrawList draw, v2 pos, v4 col, String fmt, ...);
void imgui_text(String fmt, ...);
v2 imgui_calc_text_size(String str);
void imgui_begin_tab_item(String str);

const v4 ColorWhite       = v4(1,    1,    1,    1);
const v4 ColorBlack       = v4(0,    0,    0,    1);
const v4 ColorGrey        = v4(0.5,  0.5,  0.5,  1);
const v4 ColorGreyDark    = v4(0.25, 0.25, 0.25, 1);
const v4 ColorGreyLight   = v4(0.75, 0.75, 0.75, 1);
const v4 ColorGrey0       = v4(0.15, 0.15, 0.15, 1);
const v4 ColorGrey1       = v4(0.35, 0.35, 0.35, 1);
const v4 ColorGrey2       = v4(0.60, 0.60, 0.60, 1);
const v4 ColorGrey3       = v4(0.85, 0.85, 0.85, 1);

const v4 ColorRed         = v4(0.89, 0.33, 0.23, 1.0);
const v4 ColorOrange      = v4(0.95, 0.65, 0.30, 1.0);
const v4 ColorYellow      = v4(0.90, 0.82, 0.35, 1.0);
const v4 ColorGreen       = v4(0.14, 0.8,  0.01, 1.0);
const v4 ColorCyan        = v4(0.35, 0.78, 0.82, 1.0);
const v4 ColorBlue        = v4(0.38, 0.65, 0.95, 1.0);
const v4 ColorPurple      = v4(0.72, 0.55, 0.92, 1.0);
const v4 ColorPink        = v4(0.90, 0.45, 0.65, 1.0);

const v4 ColorRedUi        = v4(0.3, 0.15, 0.15, 1);
const v4 ColorRedUiBright  = v4(0.41, 0.19, 0.21, 1);
const v4 ColorGreenUi      = v4(0.3, 0.4, 0.2, 1);
const v4 ColorBlueUi       = v4(0.15, 0.3, 0.5, 1);
const v4 ColorOrangeUi     = v4(0.4, 0.28, 0.15, 1);
const v4 ColorYellowUi     = v4(0.4, 0.43, 0.2, 1);
const v4 ColorCyanUi       = v4(0.15, 0.4, 0.36, 1);
const v4 ColorPurpleUi     = v4(0.32, 0.19, 0.41, 1);

const v4 ColorBounds     = v4(0.38, 0.65, 0.95, 1);
const v4 ColorSelection  = v4(0.95, 0.65, 0.30, 1);
const v4 ColorCollision  = v4(0.86, 0.33, 0.33, 1);

const u32 MaxEntities = KB(10);
const u32 MaxStaticEntities = KB(10);

struct GpuTextureId { u32 v; };
struct GpuMaterialId { u32 v; };
struct GpuMeshId { u32 v; };
struct GpuShaderId { u32 v; };
struct GpuCubemapId { u32 v; };
struct EntityId { u32 v; };
struct StaticEntityId { u32 v; };
struct IndexId { u32 v; };

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
struct DebugDrawLine { Vertex vert[2]; };
struct DebugDrawRect { Vertex vert[6]; };

struct Mesh {
  Vertex* vertices;
  u32* indices;
  u32 vert_count;
  u32 index_count;
  f32 bounds_min;
  f32 bounds_max;
  f32 bounds_rad;
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

struct ShaderDesc {
  String name;
  ShaderState state;
};

struct MaterialProps {
  v3 ambient;
  v3 diffuse;
  v3 specular;
  f32 shininess;
};

struct MaterialDesc {
  ShaderDesc shader;
  MaterialProps props;
  String texture;
};

struct Timer {
  f32 passed;
  f32 interval;
};

Timer timer_make(f32 interval);
void timer_tick(Timer& t);
b32 timer_passed(Timer& t);

f64 tsc_to_ms(u64 tsc);

struct TimeScope {
  u64 tsc_start;
  TimeScope() {
    tsc_start = cpu_timer_now();
  }
  ~TimeScope() {
    u64 elapsed = cpu_timer_now() - tsc_start;
    Info("%fms", tsc_to_ms(elapsed));
  }
};

f32 get_dt();
f32 get_time();

Mesh load_obj(Allocator arena, String name);
Mesh load_gltf(Allocator arena, String name);
Mesh load_glb(Allocator arena, String name);
Texture load_image(String filepath);

////////////////////////////////////////////////////////////////////////
// @Assets

#define MESH_LIST \
  X(Mesh_MonkeyGlb, monkey.glb) \
  X(Mesh_Cube, cube.glb) \
  // X(Mesh_Castle, castle.obj) \

#define MESH_0_LIST \
  X(Mesh_Triangle) \
  X(Mesh_Grid) \
  X(Mesh_Axis) \
  X(Mesh_Sphere)

enum MeshEnum {
#define X(enum_name, name) enum_name,
  MESH_LIST
#undef X
#define X(enum_name) enum_name,
  MESH_0_LIST
#undef X
  Mesh_COUNT,
};

#define TEXTURE_LIST \
  X(Texture_Orange, orange_lines_512.png) \
  X(Texture_Container, container.jpg) \
  // X(Texture_Castle, castle_diffuse.png) \

enum TextureEnum {
#define X(enum_name, name) enum_name,
  TEXTURE_LIST
#undef X
  Texture_COUNT,
};

#define MATERIAL_LIST \
  X(Material_Orange) \
  X(Material_Container) \
  X(Material_Axis) \
  X(Material_Line) \
  X(Material_Screen) \

enum MaterialEnum {
#define X(enum_name) enum_name,
  MATERIAL_LIST
#undef X
  Material_COUNT,
};

GpuMeshId mesh_get(MeshEnum id);
void mesh_set(MeshEnum id, GpuMeshId mesh_handle);
GpuMaterialId material_get(MaterialEnum id);
GpuMeshId mesh_load(String name);
GpuShaderId shader_load(ShaderDesc shader);
GpuCubemapId cubemap_load(String name);
void assets_load();

////////////////////////////////////////////////////////////////////////
// @Json

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

////////////////////////////////////////////////////////////////////////
// @Serialization

struct Serealizer {
  u8* base;
  u32 offset;
};

void serialize_data(Serealizer ser, Slice<MemberDefinition> members, void* ptr);

////////////////////////////////////////////////////////////////////////
// @Input

struct InputState {
  b8 consumed[Key_COUNT];
};

b32 key_pressed(Key key);
b32 key_pressed_consume(Key key);
b32 key_down(Key key);
b32 key_down_consume(Key key);
void key_consume(Key key);
void input_update();

////////////////////////////////////////////////////////////////////////
// @UI

enum ScrollType {
  ScrollType_Default,
  ScrollType_PowClamp,
};

struct ScrollState {
  v2 offset;
  v2 scale;
  f32 scale_level;
};

struct ImguiWindow {
  v2 pos;
  v2 size;
  b32 toggle_fullscreen;
  b32 fullscreen;
  b32 open;
  ImGuiWindowFlags flags;
};

struct ProfWindow : ImguiWindow {
  ScrollState root_scroll_state;
  ScrollState frames_scroll_state;
  ScrollState launch_time_scroll_state;
  ScrollState mem_scroll_state;
};

ScrollState scroll_state_make(f32 scale);
void ui_handle_scroll(ScrollState& s, ScrollType type = ScrollType_Default);
void imgui_window_toggle_fullscreen(ImguiWindow& window);
void imgui_window_apply_state(ImguiWindow& window);
void imgui_window_track_state(ImguiWindow& window);

// struct UI_Window {
//   v2 pos;
//   v2 size;
// };

// struct UI_Box {
//   v2 pos;
//   v2 size;
//   u64 hash;
// };

// struct UI_State {
//   u32 hot;
//   u32 last_hot;
//   u32 active;
//   u32 active_window;
//   v2 drag_offset;
//   UI_Window windows[10];
//   UI_Box boxes[10];
//   u32 boxes_count;
//   HashedStrMap<u32> hashes;
// };

// void ui_begin();
// void ui_end();
// void ui_push_box(String str);
// void ui_pop_box();
// b32 ui_begin_window(u32 id, v2 size);
// b32 ui_button(u32 id, v2 min, v2 max);

////////////////////////////////////////////////////////////////////////
// @Watch

enum WatchOp {
  WatchOp_NotifyHotreload = 1,
  WatchOp_RecompileShader,
  WatchOp_ShaderReload,
};

struct WatchFile {
  String path;
  DenseTime modified;
  WatchOp op;
};

struct WatchDirectory {
  String path;
  OS_Watch watch;
  WatchOp op;
};

struct WatchState {
  Allocator arena;
  Array<WatchFile, 128> watches;
  Array<WatchDirectory, 128> directories;
};

void watch_add(String watch_name, WatchOp op);
void watch_directory_add(String watch_name, WatchOp op, OS_WatchFlags flags = OS_WatchFlag_Modify);
void watch_update();

////////////////////////////////////////////////////////////////////////
// @Game

Introspect struct Camera {
  v3 pos;
  v3 dir;
  f32 yaw;
  f32 pitch;
  f32 fov;
  f32 speed;
};

enum EntityFlags {
  EntityFlag_Referenced = 1,
  EntityFlag_NotRender,
};

struct EntityThing {
  String name;
  EntityFlags flags;
};

Introspect struct Entity {
  String name;
  EntityFlags flags;
  v3 pos;
  v3 rot;
  v3 scale;
  Rng3 aabb;
  v3 vel;
  GpuMeshId mesh_id;
  GpuMaterialId material_id;
};

Entity& get_entity(EntityId id);

Introspect struct StaticEntity {
  v3 pos;
  v3 rot;
  v3 scale;
  GpuMeshId mesh_id;
  GpuMaterialId material_id;
};

StaticEntity& get_static_entity(StaticEntityId id);

struct GameState {
  Arena arena;
  AllocSegList gpa;
  Camera cam;
  Timer timer;

  Entity* entities;
  IndexId id_track_entities[MaxEntities];
  StaticEntity* static_entities;
  IndexId id_track_static_entities[MaxStaticEntities];
  u32 entities_count;
  u32 static_entities_count;
  StaticIdPool<MaxEntities> entity_id_pool;
  StaticIdPool<MaxStaticEntities> static_entity_id_pool;

  Darray<EntityId> moving_cubes;
  Darray<StaticEntityId> static_cubes;
  ObjPoolLinklist<EntityId, IndexId> all_dynamic_entities;
  ObjPoolLinklist<StaticEntityId, IndexId> all_static_entities;
  EntityId axis_attached_to_cam_id;
  EntityId monkey_id;
  EntityId rotating_cube_id;
  Map<String, EntityId> find_entity;
  EntityId referenced_entities[0];

  v4 color;
};

void game_init();
void game_update();
void game_view();

#include "vk.h"

////////////////////////////////////////////////////////////////////////
// @State

struct GlobalState {
  Arena arena;
  AllocSegList gpa;
  f32 dt;
  f32 time;
  u32 current_frame;
  b32 should_hotreload;
  mat4 view;
  mat4 projection;

  GpuMeshId meshes_ids[Mesh_COUNT];
  GpuTextureId textures_ids[Texture_COUNT];
  GpuMaterialId materials_ids[Material_COUNT];

  // Array<MaterialDesc, MaxMaterials> materials_desc;

  String asset_path;
  String shader_dir;
  String shader_compiled_dir;
  String models_dir;
  String textures_dir;
  Map<String, GpuTextureId> str_to_texture_id;
  Map<String, GpuMeshId> str_to_mesh_id;
  Map<String, GpuMaterialId> str_to_material_id;
  Array<String, MaxTextures> texture_id_to_str;
  Array<String, MaxMeshes> mesh_id_to_str;
  Array<String, MaxMaterials> material_id_to_str;

  WatchState watch;
  ProfWindow profile_win;
  GameState game;
  ImguiWindow game_win;
  b32 imgui_demo_open;
  InputState input;

  Darray<OS_Handle> shader_module_compilation_pids;
  Slice<String> shader_module_compiled_names;

  VK_State vk;
};

extern GlobalState* st;


