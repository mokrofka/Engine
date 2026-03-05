#pragma once
#include "lib.h"

const u32 MaxEntities  = KB(1);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// vk.cpp

struct GpuTexture;
struct GpuMesh;
struct GpuShader;
struct GpuMaterial;
struct GpuCubemap;
struct Entity;

enum RenderpassType {
  RenderpassType_World,
  RenderpassType_UI,
  RenderpassType_Screen,
};

struct PushConstant {
  u32 entity_idx;
  u32 texture_id;
};

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

struct MaterialDesc {
  v3 ambient;
  v3 diffuse;
  v3 specular;
  f32 shininess;
  Handle<GpuTexture> texture;
};

struct DrawLine {
  v3 a;
  v3 b;
  v3 color;
};

struct TextureDesc {
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

struct MeshDesc {
  Vertex* vertices;
  u32* indexes;
  u32 vert_count;
  u32 index_count;
};

enum ShaderTopology {
  ShaderTopology_Triangle,
  ShaderTopology_Line,
  ShaderTopology_Point,
};

enum ShaderType {
  ShaderType_Drawing,
  ShaderType_Screen,
  ShaderType_Cube,
  ShaderType_Compute,
};

struct ShaderDesc {
  ShaderType type;
  ShaderTopology primitive;
  b8 is_transparent;
  b8 use_depth;
};

Handle<GpuMesh> vk_mesh_load(MeshDesc mesh_desc);

void vk_init();
void vk_shutdown();

void vk_begin_frame();
void vk_end_frame();
void vk_begin_renderpass(RenderpassType renderpass);
void vk_end_renderpass(RenderpassType renderpass);

KAPI Handle<GpuShader> vk_shader_load(String name, ShaderDesc info);
KAPI Handle<GpuTexture> vk_texture_load(TextureDesc texture_info);
KAPI Handle<GpuMaterial> vk_material_load(MaterialDesc material);
KAPI Handle<GpuCubemap> vk_cubemap_load(TextureDesc* textures);

void vk_draw();
void vk_draw_screen();
void vk_draw_compute();

// Entity
KAPI void vk_make_renderable(Handle<Entity> entity_handle, Handle<GpuMesh> mesh_handle, Handle<GpuShader> shader_handle, Handle<GpuMaterial> material_handle);
KAPI void vk_remove_renderable(Handle<Entity> entity_handle);

// // Point light
// KAPI void vk_point_light_create(u32 entity_id);
// KAPI void vk_point_light_remoove(u32 entity_id);
// KAPI PointLight& vk_get_point_light_shader(u32 entity_id);

// // Directional light
// KAPI void vk_dir_light_make(u32 entity_id);
// KAPI void vk_dir_light_remove(u32 entity_id);
// KAPI DirLight& vk_dir_light_get(u32 entity_id);

// // Spot light
// KAPI void vk_spot_light_create(u32 entity_id);
// KAPI void vk_spot_light_destroy(u32 entity_id);
// KAPI SpotLight& vk_spot_light_get(u32 entity_id);

// Util

mat4& vk_get_view();
mat4& vk_get_projection();
KAPI void vk_shader_reload(String name, Handle<GpuShader> shader_handle, ShaderDesc info);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// common.cpp

////////////////////////////////////////////////////////////////////////
// Debug drawing

KAPI void debug_draw_line(v3 a, v3 b, v3 color);

KAPI extern f32 g_dt;
KAPI extern f32 g_time;
KAPI extern Transform entities_transforms[MaxEntities];

////////////////////////////////////////////////////////////////////////
// Assets

///////////////////////////////////
// Shaders

struct ShaderDefinition {
  String path;
  ShaderDesc state;
};
enum ShaderId {
  Shader_Color,
  Shader_Grid,
  Shader_Axis,
  Shader_Cubemap,
  Shader_COUNT,
};
extern ShaderDefinition shaders_info[Shader_COUNT];
extern Handle<GpuShader> shaders[Shader_COUNT];

///////////////////////////////////
// Meshes

enum MeshId {
  Mesh_GltfCube,
  Mesh_GlbCube,
  Mesh_Load_COUNT,

  Mesh_Triangle,
  Mesh_Grid,
  Mesh_Axis,
  Mesh_COUNT,
};
extern String meshes_path[Mesh_Load_COUNT];
extern Handle<GpuMesh> meshes[Mesh_COUNT];

///////////////////////////////////
// Textures

enum TextureId {
  Texture_OrangeLines,
  Texture_Container,
  Texture_COUNT,
};
extern String textures_path[Texture_COUNT];
extern Handle<GpuTexture> textures[Texture_COUNT];

///////////////////////////////////
// Materials

enum MaterialId {
  Material_RedOrange,
  Material_GreenContainer,
  Material_COUNT,
};
extern MaterialDesc materials_info[Material_COUNT];
extern Handle<GpuMaterial> materials[Material_COUNT];

////////////////////////////////////////////////////////////////////////
// Test

KAPI void test();

////////////////////////////////////////////////////////////////////////
// Profiler

KAPI u64 profiler_cpu_frequency();

struct ProfileAnchor {
  u64 TSC_elapsed;
  u64 TSC_elapsed_children;
  u64 TSC_elapsed_at_root;
  u64 hit_count;
  String label;
};

struct ProfileBlock {
  ProfileAnchor* anchor;
  ProfileAnchor* parent;
  u64 start_TSC;
  u64 old_TSC_elapsed_at_root;
  ProfileBlock(String label_, ProfileAnchor& anchor_);
  ~ProfileBlock();
};

KAPI void profiler_print_time_elapsed(u64 total_TSC_elapsed, ProfileAnchor* anchor);
KAPI void profiler_begin();
KAPI void profiler_end_and_print_();

#define TimeFunction TimeBlock(__func__)
#define TimeBlock(name) \
static ProfileAnchor Glue(_anchor, __LINE__); ProfileBlock Glue(_block, __LINE__)(name, Glue(_anchor, __LINE__))

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

#define JSON_OBJ(r, o) for (JsonValue k, v; json_iter_object(&r, o, &k, &v);)
#define JSON_OBJ_(r, o) for (JsonValue key, val; json_iter_object(&r, o, &key, &val);)
#define JSON_ARR(r, val) for (JsonValue obj; json_iter_array(&r, val, &obj);)

////////////////////////////////////////////////////////////////////////
// Asset watcher

KAPI void asset_watch_add(String watch_name, void (*callback)());
KAPI void asset_watch_directory_add(String watch_name, OS_WatchFlags flags, void (*reload_callback)(String filename));
KAPI void asset_watch_update();

////////////////////////////////////////////////////////////////////////
// Events

union EventContext {
  void* data;
  i64 i64[2];
  u64 u64[2];
  f64 f64[2];
  
  i32 i32[4];
  u32 u32[4];
  f32 f32[4];
  
  i16 i16[8];
  u16 u16[8];
  
  i8 i8[16];
  u8 u8[16];
  
  char c[16];
};

enum EventCode {
  EventCode_ApplicationQuit,
  EventCode_KeyPressed,
  EventCode_KeyReleased,
  EventCode_ButtonPressed,
  EventCode_ButtonReleased,
  EventCode_MouseMoved,
  EventCode_MouseWheel,
  EventCode_Resized,
  EventCode_ViewportResized,
  EventCode_COUNT
};

// Should return false if you want other listeners to listen
using PFN_On_Event = b32 (*)(u32 code, void* sender, void* listener_inst, EventContext data);

KAPI void event_register(u32 code, void* listener, PFN_On_Event on_event);
KAPI void event_unregister(u32 code, void* listener, PFN_On_Event on_event);
KAPI void event_fire(u32 code, void* sender, EventContext on_event);

////////////////////////////////////////////////////////////////////////
// Common

KAPI void common_init();
KAPI void r_shutdown();
KAPI void r_begin_draw_frame();
KAPI void r_end_draw_frame();

KAPI String asset_base_path();
KAPI Handle<GpuMesh> mesh_load(String name);
KAPI Handle<GpuShader> shader_load(String name, ShaderDesc info);
KAPI Handle<GpuTexture> texture_load(String name);
KAPI Handle<GpuCubemap> cubemap_load(String name);

struct Timer {
  f32 passed;
  f32 interval;
};

KAPI Timer timer_init(f32 interval);
KAPI b32 timer_tick(Timer& t);

