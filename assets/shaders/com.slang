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

#define VkBind(idx) [[vk::binding(idx, 0)]]
#define alignas(x)

v4 operator*(mat4 mat, v4 vec) { return mul(mat, vec); }
mat4 operator*(mat4 a, mat4 b) { return mul(a, b); }

global const u32 MaxMaterials = KB(1);
global const u32 MaxLights = KB(1);

global const f32 PI = 3.14159265358f;

struct Entity {
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

struct IndirectDrawCall {
  u32 indexCount;
  u32 instanceCount;
  u32 firstIndex;
  i32 vertexOffset;
  u32 firstInstance;
  u32 base;
};

struct DrawCallData {
  mat4 model;
  v4 color;
  u32 tex;
  u32 mat;
};

// struct State {
//   mat4 projection_view;
//   mat4 projection;
//   mat4 view;
//   mat4 mat;
//   v4 ambient_color;
//   f32 time;
//   u32 point_light_count;
//   u32 dir_light_count;
//   u32 spot_light_count;
// };

struct VSInput {
  v3 pos;
  v3 norm;
  v2 uv;
  v4 color;
  u32 vert_id : SV_VertexID;
  u32 draw_id : SV_DrawIndex;
  u32 inst_id : SV_InstanceID;
};

struct VSOutput {
  v4 pos : SV_Position;
  v3 norm;
  v2 uv;
  v4 color;
  u32 id;
};

struct PushConstants {
  u32 drawcall_base;
}

[[vk::push_constant]]
PushConstants push;

#include "../../src/vk_bindings.slang"

VkBind(Bindings.State) StructuredBuffer<GpuState> state;
VkBind(Bindings.Textures) Texture2D textures[];
VkBind(Bindings.Samplers) SamplerState samplers[];
VkBind(Bindings.CubeTextures) TextureCube cube_texture[];
VkBind(Bindings.Drawinfo) StructuredBuffer<IndirectDrawCall> drawinfo;
VkBind(Bindings.Entities) StructuredBuffer<Entity> entities;
VkBind(Bindings.Materials) StructuredBuffer<Material> materials;
VkBind(Bindings.PointLights) StructuredBuffer<PointLight> point_lights;
VkBind(Bindings.DirLights) StructuredBuffer<DirLight> dir_lights;
VkBind(Bindings.SpotLights) StructuredBuffer<SpotLight> spot_lights;
VkBind(Bindings.DrawCtx) StructuredBuffer<DrawCallData> drawcalls_data;

DrawCallData get_data(u32 draw_id) {
  return drawcalls_data[push.drawcall_base + draw_id];
}

u32 get_id(u32 draw_id) {
  return push.drawcall_base + draw_id;
}

v2 flip_uv_y(v2 uv) { return v2(uv.x, -uv.y); }

f32 Max(f32 a, f32 b)                 { return a > b ? a : b; }
f32 Min(f32 a, f32 b)                 { return a < b ? a : b; }
f32 Max3(f32 a, f32 b, f32 c)         { return Max(Max(a,b), c); }
f32 Min3(f32 a, f32 b, f32 c)         { return Min(Min(a,b), c); }
f32 ClampTop(f32 x, f32 a)            { return Min(x, a); }
f32 ClampBot(f32 x, f32 a)            { return Max(x, a); }
f32 Clamp(f32 a, f32 x, f32 b)        { return x < a ? a : x > b ? b : x; }
f32 Clamp01(f32 x)                    { return Clamp(0, x, 1); }
f32 ReverseClamp(f32 a, f32 x, f32 b) { return x < a ? b : b < x ? a : x; }
f32 Square(f32 x)                     { return x * x; }
f32 Cube(f32 x)                       { return x * x * x; }
f32 Sign(f32 x)                       { return x < 0 ? -1 : x > 0 ? 1 : 0; }
f32 Abs(f32 x)                        { return x < 0 ? -x : x; }

f32 deg2rad(f32 degrees) { return degrees * PI / 180.0f; }
f32 rad2deg(f32 radians) { return radians * 180.0f / PI; }

f32 Lerp(f32 a, f32 t, f32 b)      { return (1 - t)*a + t*b; }
f32 LerpClamp(f32 a, f32 t, f32 b) { return Lerp(a, Clamp01(t), b); }
f32 Unlerp(f32 a, f32 x, f32 b)    { return (x - a) / (b - a); }
f32 remap(f32 x, f32 old_min, f32 old_max, f32 new_min, f32 new_max) {
  return new_min + (((x - old_min) * (new_max - new_min)) / (old_max - old_min));
}
