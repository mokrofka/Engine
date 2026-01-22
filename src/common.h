#pragma once
#include "lib.h"
#include "r_types.h"
#include "vk.h"
#include "profiler.h"

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

struct JsonReader {
  u8* cur;
  u8* end;
  i32 depth;
  String error;
};

struct JsonValue {
  JsonType type;
  String str;
  i32 depth;
};

JsonReader json_reader_init(String buffer);
JsonValue json_read(JsonReader* r);
b32 json_iter_object(JsonReader* r, JsonValue obj, JsonValue *key, JsonValue *val);
b32 json_iter_array(JsonReader* r, JsonValue arr, JsonValue* val);

////////////////////////////////////////////////////////////////////////
// Shaders

struct ShaderDefinition {
  String path;
  ShaderType type;
};

enum Shader_Name {
  Shader_Color = 1,
  Shader_COUNT,
};

KAPI ShaderDefinition shaders_definition(u32 idx);
KAPI u32& shaders(u32 idx);

////////////////////////////////////////////////////////////////////////
// Meshes

enum {
  Mesh_Cube = 1,
  Mesh_Room,
  Mesh_COUNT,
};

KAPI String meshes_path(u32 idx);
KAPI u32& meshes(u32 idx);

////////////////////////////////////////////////////////////////////////
// Textures

enum {
  Texture_OrangeLines = 1,
  Texture_Container,
  Texture_Room,
  Texture_COUNT,
};

KAPI String textures_path(u32 idx);
KAPI u32& textures(u32 idx);

