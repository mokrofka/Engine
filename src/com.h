#pragma once
#include "lib.h"
#include "tokenizer.h"
#include "meta.h"

// TODO:
// dummy assets/null 
// obj mouse selection
// UI
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
// font rendering
// editor

const u32 MaxEntities = KB(4);

MakeId(OpaqueId)
MakeId(EntityId)

#include "gfx.h"
#include "render.h"
#include "debug.h"

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

enum LightType {
  LightType_Point,
  LightType_Dir,
  LightType_Spot,
};

struct Light {
  LightType type;
  v3 pos;
  v3 dir;
  v3 color;
  f32 intensity;
  f32 radius;
  f32 cone_angle;
  b32 cast_shadow;
};

struct TextureDesc {
  u32 width;
  u32 height;
  union {
    u8* data;
    u8* cube[6];
  };
};

struct Timer {
  f32 interval;
  f32 acc;
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

Introspect struct Entity {
  EntityId parent;
  EntityId next;
  EntityId prev;
  EntityId first;
  EntityId last;
  String name;
  EntityFlags flags;
  v3 pos;
  v3 rot;
  v3 scale;
  Rng3 aabb;
  v3 vel;
  R_Mesh mesh_id;
  R_Material material_id;
  v4 color;
};

struct GameState {
  Arena arena;
  AllocSegList gpa;
  Camera cam;
  b32 fps_camera;
  Timer timer;

  u32 entities_count;
  PoolLinkList<Entity, MaxEntities, EntityId> entities;

  Darray<EntityId> moving_cubes;
  EntityId axis_attached_to_cam_id;
  EntityId monkey_id;
  EntityId rotating_cube_id;
  Map<String, EntityId, 32> find_entity;

  v3 a;
  v3 b;
  v2 a_v2;
  v2 b_v2;
  EntityId e;
  f32 t;
  v4 color;
  EntityId my;
};

struct GlobalState {
  Arena arena;
  AllocSegList gpa;
  f32 dt;
  f32 time;
  u32 current_frame;
  b32 should_hotreload;
  mat4 view;
  mat4 projection;

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
  GameState game;
  InputState input;
  R_State r;
  Gfx_State gfx;
  DebugState debug;
};

extern GlobalState* st;

u64 hash(Vertex vert);
b32 equal(Vertex a, Vertex b);

Timer timer_make(f32 interval);
b32 timer_update(Timer& t);
b32 timer_ready(Timer& t);
void timer_trigger(Timer& t);

f64 tsc_to_ms(u64 tsc);

f32 get_dt();
f32 get_time();

MeshDesc load_obj(Allocator arena, String name);
MeshDesc load_gltf(Allocator arena, String path, b32 is_glb);

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

Entity& get_entity(EntityId id);
Transform get_entity_transform(EntityId id);

EntityId e_alloc_bare();
EntityId e_alloc(R_Mesh mesh_id, R_Material material_id, EntityThing thing = {});
EntityId e_alloc(MeshEnum mesh_id, MaterialEnum material_id, EntityThing thing = {});
void e_free(EntityId e_id);
void game_init();
void game_update();
void game_view();
void game_save_state();
void game_load_state();

String dumb_struct(Allocator arena, Slice<MemberDefinition> members, void* ptr, EntityFlags flags = {});
void dumb_struct_load(Slice<MemberDefinition> members, void* ptr, Parser* parser);

R_Mesh mesh_get(MeshEnum id);
void mesh_set(MeshEnum mesh_enum, R_Mesh id);
R_Material material_get(MaterialEnum id);
void assets_load();

#include "generated.h"

