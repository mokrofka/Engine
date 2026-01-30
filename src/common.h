#pragma once
#include "lib.h"
#include "r_types.h"
#include "vk.h"

////////////////////////////////////////////////////////////////////////
// Test

KAPI void test();

////////////////////////////////////////////////////////////////////////
// Profiler

KAPI u64 cpu_frequency();

struct ProfileAnchor {
  u64 TSC_elapsed;
  u64 TSC_elapsed_children;
  u64 TSC_elapsed_at_root;
  u64 hit_count;
  String label;
};

struct Profiler {
  Array<ProfileAnchor*, 4096> anchors;
  u64 start_TSC;
  u64 end_TSC;
  ProfileAnchor* profiler_parent;
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

KAPI void asset_watch_init();
KAPI void asset_watch_add(String watch_name, void (*callback)());
KAPI void asset_watch_directory_add(String watch_name, void (*reload_callback)(String filename), OS_WatchFlags flags);
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

// Should return false if you want other listeners to listen
using PFN_On_Event = b32 (*)(u32 code, void* sender, void* listener_inst, EventContext data);

KAPI void event_register(u32 code, void* listener, PFN_On_Event on_event);
KAPI void event_unregister(u32 code, void* listener, PFN_On_Event on_event);
KAPI void event_fire(u32 code, void* sender, EventContext on_event);

enum SystemEventCode {
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

////////////////////////////////////////////////////////////////////////
// Common

#define MaxLights KB(1)
#define MaxEntities KB(4)
#define MaxMeshes KB(1)
#define MaxShaders KB(1)
#define MaxTextures KB(1)

KAPI extern Transform entities_transforms[MaxEntities];

KAPI extern f32 delta_time;

KAPI void common_init();
KAPI void r_shutdown();
KAPI void r_begin_draw_frame();
KAPI void r_end_draw_frame();

KAPI String asset_base_path();
KAPI u32 mesh_load(String name);
KAPI u32 shader_load(String shader, ShaderType type);
KAPI u32 texture_load(String name);
KAPI u32 cubemap_load(String name);

struct Timer {
  f32 passed;
  f32 interval;
};

KAPI Timer timer_init(f32 interval);
KAPI b32 timer_tick(Timer& t, f32 dt);

////////////////////////////////////////////////////////////////////////
// Assets

///////////////////////////////////
// Shaders

struct ShaderDefinition {
  String path;
  ShaderType type;
};

enum ShaderThing {
  Shader_Color,
  Shader_COUNT,
};
global ShaderDefinition shaders_info[Shader_COUNT] = {
  [Shader_Color] = "color_shader", ShaderType_Drawing,
};
global u32 shaders[Shader_COUNT];

///////////////////////////////////
// Meshes

enum {
  // Mesh_Cube = 1,
  Mesh_GltfCube,
  // Mesh_GltfHelmet,
  // Mesh_GlbHelmet,
  // Mesh_GlbMonkey,
  // Mesh_Room,
  Mesh_COUNT,
};
global String meshes_path[Mesh_COUNT] = {
  // [Mesh_Cube-1] = "cube.obj",
  [Mesh_GltfCube] = "cube.gltf",
  // [Mesh_GltfHelmet-1] = "helmet.gltf",
  // [Mesh_GlbHelmet-1] = "helmet.glb",
  // [Mesh_GlbMonkey-1] = "monkey.glb",
  // [Mesh_Room] = "room.obj",
};
global u32 meshes[Mesh_COUNT];

///////////////////////////////////
// Textures

enum {
  Texture_OrangeLines,
  Texture_Container,
  // Texture_Room,
  Texture_COUNT,
};
global String textures_path[Texture_COUNT] = {
  [Texture_OrangeLines] = "orange_lines_512.png",
  [Texture_Container] = "container.jpg",
  // [Texture_Room-1] = "image.png",
};
global u32 textures[Texture_COUNT];

///////////////////////////////////
// Materials

enum {
  Material_RedOrange,
  Material_GreenContainer,
  Material_COUNT,
};

global Material materials_info[Material_COUNT] = {
  [Material_RedOrange] = {
    .ambient = v3(1,0,0),
    .diffuse = v3(1),
    .specular = v3(1),
    .shininess = 1,
    .texture = Texture_OrangeLines,
  },
  [Material_GreenContainer] = {
    .ambient = v3(0,1,0),
    .diffuse = v3(1),
    .specular = v3(1),
    .shininess = 1,
    .texture = Texture_Container,
  },
};
global u32 materials[Material_COUNT];



