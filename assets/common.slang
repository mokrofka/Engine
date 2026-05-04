
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

u32 foo() {
  return 1;
}

struct VSInput {
  v3 pos;
  u32 vert_id : SV_VertexID;
  u32 draw_id : SV_DrawIndex;
  u32 inst_id : SV_InstanceID;
};

struct VSOutput {
  v4 pos : SV_Position;
};

struct PushConstants {
  u32 drawcall_offset;
}

[[vk::push_constant]]
PushConstants push;
