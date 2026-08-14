#pragma once
#include "lib.h"
#include "tokenizer.h"
#include "types.h"
#include "gfx.h"
#include "render.h"

// TODO:
// dummy assets/null 
// UI
// memory visualisation thread safe, and gpu memory
// thread graph visualisation
// make wayland backend work
// console
// thread safe allocator
// glb loader
// obj mouse selection

enum MetaType {
  MetaType_Null,
  MetaType_u32,
  MetaType_i32,
  MetaType_b32,
  MetaType_f32,
  MetaType_v2,
  MetaType_v3,
  MetaType_v4,
  MetaType_Rng2,
  MetaType_Rng3,
  MetaType_MeshId,
  MetaType_MaterialId,
  MetaType_RenderId,
  MetaType_String,
  MetaType_EntityFlags
};

struct MemberDefinition {
  MetaType type;
  String name;
  u64 offset;
};

enum ScrollType {
  ScrollType_Default,
  ScrollType_PowClamp,
};

struct ScrollState {
  v2 offset;
  v2 scale;
  f32 scale_level;
};

struct DebugWindow {
  v2 pos;
  v2 size;
  b32 toggle_fullscreen;
  b32 fullscreen;
  b32 open;
  ImGuiWindowFlags flags;
};

enum ProfTabActive {
  ProfileTabActive_Root,
  ProfileTabActive_Frames,
  ProfileTabActive_Time,
  ProfileTabActive_LaunchTime,
  ProfileTabActive_Memory,
};

struct ProfColors {
  v4 work;
  v4 sleep;
  v4 job;
  v4 async;
  v4 selected;
  v4 hovered;
  v4 border;
  v4 text;
  v4 text_dim;
  v4 current_frame;
  v4 frame_ok;     // < 16ms
  v4 frame_warn;   // 16-20ms
  v4 frame_bad;    // > 20ms
  v4 mem_used;
  v4 mem_committed;
  v4 mem_cap;
};

struct DebugProfWindow {
  DebugWindow win;
  ScrollState root_scroll_state;
  ScrollState frames_scroll_state;
  ScrollState launch_time_scroll_state;
  ScrollState mem_scroll_state;
  ProfTabActive active_tab;
  ProfTabActive future_active_tab;
  f32 frame_avg_time;
  f32 frame_min_time;
  f32 frame_max_time;
  ProfColors colors;
};

struct DebugState {
  DebugProfWindow prof_win;
  DebugWindow game_win;
  b32 imgui_demo_open;
  ImFont* font;
};

struct ImGui_DrawList {
  ImDrawList* draw;
};

enum JsType {
  JsType_Null,
  JsType_Bool,
  JsType_Number,
  JsType_Str,
  JsType_Array,
  JsType_Obj,
};

struct JsField {
  String key;
  struct JsVal* val;
};

struct JsObj {
  Slice<JsField> fields;
};

struct JsVal {
  JsType type;
  union {
    b32 boolean;
    f64 number;
    String str;
    Slice<JsVal*> array;
    JsObj obj;
  };
};

struct JsParser {
  Allocator arena;
  String str;
  u32 cursor;
};

struct Cooldown {
  f32 remaining;
};

#define MESH_LIST \
  X(Cube) \
  X(MonkeyGlb) \
  X(Triangle) \
  X(Grid) \
  X(Axis) \
  X(Sphere) \
  X(CubeGlft) \
  X(GreeMan) \
  X(Barrack) \

enum MeshEnum {
#define X(name) Glue(Mesh_, name),
  MESH_LIST
#undef X
  Mesh_COUNT,
};

#define TEXTURE_LIST \
  X(Orange) \
  X(Container) \
  X(Barrack) \

enum TextureEnum {
#define X(name) Glue(Texture_, name),
  TEXTURE_LIST
#undef X
  Texture_COUNT,
};

#define MATERIAL_LIST \
  X(Orange) \
  X(Container) \
  X(Axis) \
  X(Line) \
  X(Screen) \
  X(Barrack) \

enum MaterialEnum {
#define X(name) Glue(Material_, name),
  MATERIAL_LIST
#undef X
  Material_COUNT,
};

struct TimeScope {
  u64 tsc_start;
  TimeScope();
  ~TimeScope();
};

struct Serealizer {
  u8* base;
  u32 offset;
};

struct InputState {
  b8 consumed[Key_COUNT];
};

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

Introspect struct Camera {
  v3 pos;
  v3 dir;
  f32 yaw;
  f32 pitch;
  f32 fov;
  f32 speed;
};

typedef u32 EntityFlags;
enum {
  EntityFlag_Referenced = 1,
  EntityFlag_NotRender,
};

struct EntityThing {
  String name;
  EntityFlags flags;
};

#define THING_DESC(...) \
  ((ThingDesc){       \
      .rot = quat_identity(), \
      .scale = v3(1), \
      __VA_ARGS__      \
  })

struct ThingDesc {
  v3 pos;
  v4 rot;
  v3 scale;
  MeshEnum mesh;
  MaterialEnum mat;
};

Introspect struct Thing {
  ThingId parent;
  ThingId next;
  ThingId prev;
  ThingId first;
  ThingId last;
  String name;
  EntityFlags flags;
  union {
    Transform trans;
    struct {
      v3 pos;
      v4 rot;
      v3 scale;
    };
  };
  Rng3 aabb;
  v3 vel;
  R_Mesh mesh;
  R_Material mat;
  v4 color;
  f32 elapsed;
};

struct UI_State {
  u32 hotitem;
  u32 activeitem;
  b32 mouse_down;

  u32 kbditem;
  u32 last_widget;
  b32 tab;
  b32 enter;
  b32 up;
  b32 down;
};

#include "ui.h"

typedef u32 ThingState;
enum {
  ThingState_OnFire = Bit(0),
  ThingState_Flying = Bit(1),
  ThingState_Poisoned = Bit(2),
};

struct GlobalState {
  Arena arena;
  Arena frame_arena;
  Alloc gpa;
  f32 dt;
  f64 time;
  u32 current_frame;
  b32 should_hotreload;
  mat4 view;
  mat4 projection;
  v4 ambient_color;

  R_Mesh meshes_ids[Mesh_COUNT];
  R_Texture textures_ids[Texture_COUNT];
  R_Material materials_ids[Material_COUNT];

  String asset_dir;
  String shader_dir;
  String shader_compiled_dir;
  String models_dir;
  String textures_dir;
  Map<String, R_Texture, R_MaxTextures> str_to_texture_id;
  Map<String, R_Mesh, R_MaxMeshes> str_to_mesh_id;
  Map<String, R_Material, R_MaxMaterials> str_to_material_id;
  Array<String, R_MaxTextures> texture_id_to_str;
  Array<String, R_MaxMeshes> mesh_id_to_str;
  Array<String, R_MaxMaterials> material_id_to_str;

  WatchState watch;
  InputState input;
  R_State r;
  Gfx_State gfx;
  DebugState debug;
  UI_State ui;
  UI_State0* ui0;

  Camera cam;
  R_Camera r_cam;
  b32 fps_camera;
  Timer timer;

  u32 entities_count;
  PoolLinkList<Thing, MaxEntities, ThingId> entities;
  // PoolLinkList<Thing, MaxEntities, ThingId> things;

  Darray<ThingId> moving_cubes;
  ThingId axis_attached_to_cam_id;
  ThingId monkey_id;
  ThingId rotating_cube_id;
  Map<String, ThingId, 32> find_entity;

  R_Font font;
  v3 a;
  v3 b;
  v2 a_v2;
  v2 b_v2;
  ThingId e;
  f32 t;
  v4 color;
  ThingId my;
  ThingId thing;
  v3 euler;
  v3 point;
  ThingId t_id;

  Coroutine co;
};

extern GlobalState* st;

ThingDesc default_thing_desc();

void ui_draw_rect(Rng2 rect, v4 color);
b32 ui_button(u32 id, v2 pos);
b32 ui_slider(u32 id, v2 pos, i32 max, i32& v);
void ui_begin();
void ui_end();

ImGui_DrawList imgui_get_window_drawlist();
void imgui_draw_rect(ImGui_DrawList draw, Rng2 rect, v4 col, f32 rounding = 0, ImDrawFlags flags = 0, f32 thickness = 1);
void imgui_draw_rect_filled(ImGui_DrawList draw, Rng2 rect, v4 col, f32 rounding = 0, ImDrawFlags flags = 0);
void imgui_draw_push_clip_rect(ImGui_DrawList draw, Rng2 rect);
void imgui_draw_pop_clip_rect(ImGui_DrawList draw);
void imgui_draw_line(ImGui_DrawList draw, v2 p0, v2 p1, v4 col, f32 thickness = 1);
void imgui_draw_text(ImGui_DrawList draw, v2 pos, v4 col, String fmt, ...);
void imgui_draw_text(ImGui_DrawList draw, ImFont* font, f32 font_size, v2 pos, v4 col, String fmt, ...);
void imgui_text(String fmt, ...);
v2 imgui_calc_text_size(String str);
void imgui_begin_tab_item(String str);

Rng2 debug_window_get_rect(DebugWindow win);
void debug_window_apply_state(DebugWindow& win);
void debug_window_track_state(DebugWindow& win);
void debug_window_toggle_fullscreen(DebugWindow& win);
void debug_init();
void debug_update();
void debug_game();
void debug_prof_view();

void test();

u64 hash(R_Vertex vert);
b32 equal(R_Vertex a, R_Vertex b);

Timer timer_make(f32 interval);
b32 timer_update(Timer& t);
b32 timer_ready(Timer& t);
void timer_trigger(Timer& t);

f64 tsc_to_ms(u64 tsc);

f32 get_dt();
f32 get_time();
b32 time_on_interval(f64 time, f32 delta, f32 interval, f32 offset);
b32 time_on_interval(f32 interval, f32 offset = 0);
b32 time_on_time(f64 time, f64 timestamp);

R_MeshDesc load_obj(Allocator arena, String name);
R_MeshDesc load_gltf(Allocator arena, String path, b32 is_glb);

JsParser js_parse_make(Allocator arena, String str);
u8 js_peek(JsParser* p);
void js_advance(JsParser* p);
void js_skip_ws(JsParser* p);
String js_parse_str(JsParser* p);
f64 js_parse_number(JsParser* p);
JsVal js_parse(JsParser* p);
JsVal js_get_val(JsVal val, String key);
b32 js_get_bool(JsObj obj, String key);
f64 js_get_number(JsObj obj, String key);
String js_get_str(JsObj obj, String key);
Slice<JsVal*> js_get_array(JsObj obj, String key);
JsObj js_get_obj(JsObj obj, String key);

void serialize_data(Serealizer ser, Slice<MemberDefinition> members, void* ptr);

b32 key_pressed(Key key);
b32 key_pressed_consume(Key key);
b32 key_down(Key key);
b32 key_down_consume(Key key);
void key_consume(Key key);

ScrollState scroll_state_make(f32 scale);
void scroll_state_update(ScrollState& s, ScrollType type = ScrollType_Default);

void watch_add(String watch_name, WatchOp op);
void watch_directory_add(String watch_name, WatchOp op, OS_WatchFlags flags = OS_WatchFlag_Modify);
void watch_update();

Thing& get_thing(ThingId id);
Transform get_entity_transform(ThingId id);

ThingId e_alloc_bare();
ThingId e_alloc(R_Mesh mesh_id, R_Material material_id, EntityThing thing = {});
ThingId e_alloc(MeshEnum mesh_id, MaterialEnum material_id, EntityThing thing = {});
ThingId make_thing(ThingDesc desc);
void destroy_thing(ThingId id);
void game_init();
void update_game();
void game_view();
void game_save_state();
void game_load_state();

String dumb_struct(Allocator arena, Slice<MemberDefinition> members, void* ptr, EntityFlags flags = {});
void dumb_struct_load(Slice<MemberDefinition> members, void* ptr, Parser* parser);

R_Mesh get_mesh(MeshEnum id);
void mesh_set(MeshEnum mesh_enum, R_Mesh id);
R_Material get_material(MaterialEnum id);
void assets_load();


