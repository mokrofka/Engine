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

typedef float3x3 m3x3;
typedef float4x4 m4x4;

#define VkBind(idx) [[vk::binding(idx, 0)]]
#define alignas(x)
#define Glue(a, b) b

v4 operator*(m4x4 mat, v4 vec) { return mul(mat, vec); }
m4x4 operator*(m4x4 a, m4x4 b) { return mul(a, b); }

global const u32 MaxMaterials = KB(1);
global const u32 MaxLights = KB(1);

global const f32 PI = 3.14159265358f;

#include "../../src/shader_header.h"

struct IndirectDrawCall {
	u32 indexCount;
	u32 instanceCount;
	u32 firstIndex;
	i32 vertexOffset;
	u32 firstInstance;
	u32 base;
};

struct VSOutput {
	v4 pos: SV_Position;
	v3 norm;
	v2 uv;
	v4 color;
	u32 id; // drawcalls_idx
};

struct UI_VSOutput {
	v4 vert : SV_Position;
	v2 uv;
	v4 color;
	v2 dst_pos;
	v2 dst_center;
	v2 dst_half_size;
	f32 corner_radius;
	f32 edge_softness;
	u32 texture;
	u32 flags;
};

struct PushConstants {
	u32 idx;
}

[[vk::push_constant]]
PushConstants push;

VkBind(Bindings.Textures)       Texture2D textures[];
VkBind(Bindings.Samplers)       SamplerState samplers[];
VkBind(Bindings.CubeTextures)   TextureCube cube_texture[];
VkBind(Bindings.State)          StructuredBuffer<GpuState> state;
VkBind(Bindings.Entities)       StructuredBuffer<Entity> entities;
VkBind(Bindings.Materials)      StructuredBuffer<Material> materials;
VkBind(Bindings.PointLights)    StructuredBuffer<PointLight> point_lights;
VkBind(Bindings.DirLights)      StructuredBuffer<DirLight> dir_lights;
VkBind(Bindings.SpotLights)     StructuredBuffer<SpotLight> spot_lights;
VkBind(Bindings.Drawinfo)       StructuredBuffer<IndirectDrawCall> drawinfo;
VkBind(Bindings.DrawCtx)        StructuredBuffer<DrawCall> drawcalls;
VkBind(Bindings.SoftwareRender) StructuredBuffer<u32> software_render;
VkBind(Bindings.UI_RectBinding) StructuredBuffer<UI_Rect> ui_rects;
VkBind(Bindings.Vertices)       StructuredBuffer<Vertex> vertices;

bool flag_has(u64 x, u64 f) { return (x & f) == (f); }

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

v4 v4_hadamard(v4 a, v4 b) { return v4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w); }
v4 v4_hadamard_div(v4 a, v4 b) { return v4(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w); }

v4 rgba_from_u32(u32 hex) {
	v4 result = v4(((hex >>  0) & 0xff) / 255.f,
																((hex >>  8) & 0xff) / 255.f,
																((hex >> 16) & 0xff) / 255.f,
																((hex >> 24) & 0xff) / 255.f);
	return result;
}

u32 u32_from_rgba(v4 rgba) {
	u32 result = 0;
	result |= ((u32)((u8)(rgba.x*255.f))) <<  0;
	result |= ((u32)((u8)(rgba.y*255.f))) <<  8;
	result |= ((u32)((u8)(rgba.z*255.f))) << 16;
	result |= ((u32)((u8)(rgba.w*255.f))) << 24;
	return result;
}

