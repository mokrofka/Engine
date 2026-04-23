#pragma once
#include "lib.h"

#define IM_VEC2_CLASS_EXTRA                               \
        constexpr ImVec2(const v2& f) : x(f.x), y(f.y) {} \
        operator v2() const { return v2(x,y); }
#include "imgui/imgui.h"

struct ImString {
  u8* str;
  u64 size;
  ImString(const String& f) : str(f.str), size(f.size) {}
  operator char*() { return (char*)str; }
};

#define IM_RECT(rect) rect.min, rect.max

// TODO:
// dummy assets/null 
// obj mouse selection
// serelization/deserialization
// UI rendering
// memory visualisation
// thread graph visualisation
// profiler
// make wayland backend work
// linux crushes when I try sleep and mount?
// fix static non indexed - doesn't render

const v3 ColorRed   = v3(1,0,0);
const v3 ColorGreen = v3(0,1,0);
const v3 ColorBlue  = v3(0,0,1);
const v3 ColorWhite = v3(1,1,1);
const v3 ColorBlack = v3(0,0,0);
const v3 ColorGrey  = v3(0.8,0.8,0.8);

const u32 MaxEntities = KB(10);
const u32 MaxStaticEntities = KB(10);

struct Entity;
struct StaticEntity;
struct GpuTexture;
struct GpuMesh;
struct GpuShader;
struct GpuMaterial;
struct GpuCubemap;

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

struct Timer {
  f32 passed;
  f32 interval;
};

Timer timer_init(f32 interval);
b32 timer_tick(Timer& t);

void draw_squad(v2 min, v2 max, v3 color);

// Transform& entity_transform(Handle<Entity> handle);
// Transform& static_entity_transform(Handle<StaticEntity> handle);

f32 get_dt();
f32 get_time();

f32 get_was_hotreload();

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
Handle<GpuMaterial> material_get(MaterialId id);
Handle<GpuMesh> mesh_load(String name);
Handle<GpuShader> shader_load(Shader shader);
Handle<GpuCubemap> cubemap_load(String name);
void asset_load();

////////////////////////////////////////////////////////////////////////
// Profiler

struct ProfileAnchor {
  u64 tsc_elapsed_exclusive; // without children
  u64 tsc_elapsed_inclusive; // with children
  u64 hit_count;
  String label;
  // for fraph
  u32 depth;
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

struct ProfilerState {
  // struct FrameState {
  //   ProfileAnchor anchors[KB(4)];
  //   u32 anchors_count;
  //   u64 tsc_start;
  //   u64 tsc_end;
  //   u32 profiler_parent;
  // } frames[10];

  ProfileAnchor anchors[KB(4)];
  u32 anchors_count;
  u64 tsc_start;
  u64 tsc_end;
  u32 profiler_parent;
  // Map<String, u32> map;
  u32 hash_to_indices[KB(4)];
  u32 depth;

  ProfileAnchor prev_anchors[4096];
  u32 prev_anchors_count;
  u64 prev_tsc_start;
  u64 prev_tsc_end;
  u64 prev_tsc_elapsed;

  struct {
    f32 scale_x;
    f32 scale_y;
    f32 offset_x;
    f32 offset_y;
    b32 fullscreen;
    v2 pos;
    v2 size;
  } window;

};

void profiler_begin();
void profiler_end();
Slice<ProfileAnchor> profiler_get_anchors();
u64 profiler_get_tsc_elapsed();

#if PROFILE_BUILD
  #define TimeBlock(Name) ProfileBlock Glue(__profiler_block, __LINE__)(Name, __func__, Name)
  #define TimeFunction TimeBlock(__func__)
#else
  #define TimeBlock(Name)
  #define TimeFunction
#endif

////////////////////////////////////////////////////////////////////////
// Watch

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
  Allocator alloc;
  Array<WatchFile, 128> watches;
  Array<WatchDirectory, 128> directories;
};

void watch_add(String watch_name, WatchOp op);
void watch_directory_add(String watch_name, WatchOp op, OS_WatchFlags flags = OS_WatchFlag_Modify);
void watch_update();

////////////////////////////////////////////////////////////////////////
// UI

struct UI_Window {
  v2 pos;
  v2 size;
};

struct UI_Box {
  v2 pos;
  v2 size;
  u64 hash;
};

struct UI_State {
  u32 hot;
  u32 last_hot;
  u32 active;
  u32 active_window;
  v2 drag_offset;
  UI_Window windows[10];
  UI_Box boxes[10];
  u32 boxes_count;
  HashedStrMap<u32> hashes;
};

void ui_begin();
void ui_end();
void ui_push_box(String str);
void ui_pop_box();
b32 ui_begin_window(u32 id, v2 size);
b32 ui_button(u32 id, v2 min, v2 max);

////////////////////////////////////////////////////////////////////////
// game

struct Camera {
  v3 pos;
  v3 dir;
  f32 yaw;
  f32 pitch;
  f32 fov;
};

struct Entity {
  v3 vel;
  Rng3 aabb;
};

template<>
struct Handle<Entity> {
  u32 handle;
  Transform& trans();
  v3& pos();
  v3& rot();
  v3& scale();
  Entity& get();
  Rng3& aabb();
  v3& vel();
#if BUILD_DEBUG
  u32 idx() { return handle & INDEX_MASK; }
  u32 generation() { return handle >> INDEX_BITS; }
#else
  u32 idx() { return handle; }
#endif
};

struct StaticEntity {
};

template<>
struct Handle<StaticEntity> {
  u32 handle;
  Transform& trans();
  v3& pos();
  v3& rot();
  v3& scale();
#if BUILD_DEBUG
  u32 idx() { return handle & INDEX_MASK; }
  u32 generation() { return handle >> INDEX_BITS; }
#else
  u32 idx() { return handle; }
#endif
};

struct GameState {
  Arena arena;
  Arena persistent_arena;
  AllocSegList gpa;
  AllocSegList gpa_arena0;
  AllocSegList gpa_arena1;
  AllocSegList gpa_gpa0;
  AllocSegList gpa_gpa1;
  Camera cam;
  Timer timer;

  Entity* entities;
  StaticEntity* static_entities;
  StaticIdPool entity_id_pool;
  StaticIdPool static_entity_id_pool;

  Darray<Handle<Entity>> moving_cubes;

  Handle<Entity> axis_attached_to_cam;
  Handle<Entity> grid;
  Handle<Entity> monkey;
  Handle<Entity> rotating_cube;
  Handle<Entity> sphere;
};  

struct GlobalState {
  Arena arena;
  f32 dt;
  f32 time;
  b32 should_hotreload;
  Transform* transforms;
  Transform* static_transforms;

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
  UI_State ui;
  GameState game;

  void* vk_st;
};

extern GlobalState* g_st;

void common_init();


