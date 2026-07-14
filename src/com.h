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

#include "types.h"
#include "gfx.h"
#include "render.h"
#include "debug.h"

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
};

struct GameState {
  Arena arena;
  Alloc gpa;
  Camera cam;
  R_Camera r_cam;
  b32 fps_camera;
  Timer timer;

  u32 entities_count;
  PoolLinkList<Entity, MaxEntities, EntityId> entities;

  Darray<EntityId> moving_cubes;
  EntityId axis_attached_to_cam_id;
  EntityId monkey_id;
  EntityId rotating_cube_id;
  Map<String, EntityId, 32> find_entity;

  R_Font font;
  v3 a;
  v3 b;
  v2 a_v2;
  v2 b_v2;
  EntityId e;
  f32 t;
  v4 color;
  EntityId my;
  EntityId thing;
  v3 euler;
  v3 point;
};

struct GlobalState {
  Arena arena;
  Arena frame_arena;
  Alloc gpa;
  f32 dt;
  f32 time;
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
  GameState game;
  InputState input;
  R_State r;
  Gfx_State gfx;
  DebugState debug;
};

extern GlobalState* st;

void test();

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

