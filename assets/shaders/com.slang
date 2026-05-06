#define NO_IMPORT_LIB
#include "lib.slang"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t	i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

typedef u8  b8;
typedef i32 b32;

typedef float2 v2;
typedef float3 v3;
typedef float4 v4;

typedef float3x3 mat3;
typedef float4x4 mat4;

v4 operator*(mat4 mat, v4 vec) { return mul(mat, vec); }
mat4 operator*(mat4 a, mat4 b) { return mul(a, b); }

global const u32 MaxEntities = KB(10);
global const u32 MaxStaticEntities = KB(10);
global const u32 MaxMaterials = KB(1);
global const u32 MaxLights = KB(1);

struct Entity {
  mat4 model;
  v4 color;
  u32 material;
};

struct Material {
  v3 ambient;
  v3 diffuse;
  v3 specular;
  f32 shininess;
  u32 texture;
};

struct PointLight {
  v3 color;
  v3 pos;
  f32 intensity;
  f32 rad;
};

struct DirLight {
  v3 color;
  v3 direction;
  f32 intensity;
};

struct SpotLight {
  v3 color;
  v3 pos;
  v3 direction;
  f32 intensity;
  f32 inner_cutoff; // Cosine of inner cone angle
  f32 outer_cutoff; // Cosine of outer cone angle
};

struct DrawCallInfo {
  u32 pad[5];
  u32 entity_id;
};

struct State {
  mat4 projection_view;
  mat4 projection;
  mat4 view;
  mat4 mat;
  v4 ambient_color;
  f32 time;
  u32 point_light_count;
  u32 dir_light_count;
  u32 spot_light_count;
  u32 entity_indices[MaxEntities+MaxStaticEntities];
};

struct VSInput {
  v3 pos;
  v3 norm;
  v2 uv;
  v3 color;
  u32 vert_id : SV_VertexID;
  u32 draw_id : SV_DrawIndex;
  u32 inst_id : SV_InstanceID;
};

struct VSOutput {
  v4 pos : SV_Position;
  v3 norm;
  v2 uv;
  v3 color;
  u32 entity_id;
};

struct PushConstants {
  u32 drawcall_offset;
}

[[vk::push_constant]]
PushConstants push;

#include "../../src/vk_bindings.h"

VkBind(Bindings.State) StructuredBuffer<State> st;
VkBind(Bindings.Textures) Texture2D textures[];
VkBind(Bindings.Samplers) SamplerState samplers[];
VkBind(Bindings.CubeTextures) TextureCube cube_texture;
VkBind(Bindings.Drawinfo) StructuredBuffer<DrawCallInfo> drawinfo;
VkBind(Bindings.Entities) StructuredBuffer<Entity> entities;
VkBind(Bindings.Materials) StructuredBuffer<Material> materials;
VkBind(Bindings.PointLights) StructuredBuffer<PointLight> point_lights;
VkBind(Bindings.DirLights) StructuredBuffer<DirLight> dir_lights;
VkBind(Bindings.SpotLights) StructuredBuffer<SpotLight> spot_lights;

v2 in_uv(v2 uv) { return v2(uv.x, -uv.y); }
