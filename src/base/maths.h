#pragma once
#include "base.h"
#include "str.h"

#define PI  3.14159265358f
#define Tau (PI * 2)

#define EulerNumber  2.71828182846f
#define GoldBig      1.61803398875f
#define GoldSmall    0.61803398875f

#define FloatEpsilon   1e-5f
#define MachineEpsilon 1.1920929e-7f

KAPI f32 degtorad(f32 degrees);
KAPI f32 radtodeg(f32 radians);

union v2 {
  struct {
    f32 x;
    f32 y;
  };
  f32 e[2];
  v2() = default;
  v2(f32 x_, f32 y_);
  v2(f32 scale);
};

union v2i {
  struct {
    i32 x;
    i32 y;
  };
  i32 e[2];
  v2i() = default;
  v2i(i32 x_, i32 y_);
  v2i(i32 scale);
};

union v2u {
  struct {
    u32 x;
    u32 y;
  };
  u32 e[2];
  v2u() = default;
  v2u(u32 x_, u32 y_);
  v2u(u32 scale);
};

union v3 {
  struct {
    f32 x;
    f32 y;
    f32 z;
  };
  f32 e[3];
  v3() = default;
  v3(f32 x_, f32 y_, f32 z_);
  v3(f32 scale);
};

union v3u {
  struct {
    u32 x;
    u32 y;
    u32 z;
  };
  u32 e[3];
  v3u() = default;
  v3u(u32 x_, u32 y_, u32 z_);
  v3u(u32 scale);
};

union v4 {
  struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
  };
  f32 e[4];
  v4() = default;
  v4(f32 x_, f32 y_, f32 z_, f32 w_);
  v4(f32 scale);
};

union mat3 {
  f32 e[9];
  f32& operator[](u32 a);
};

union mat4 {
  f32 e[16];
  struct {
    v4 x;
    v4 y;
    v4 z;
    v4 w;
  };
  f32& operator[](u32 a);
};

typedef v4 quat;

union Rect {
  struct {
    f32 x,y;
    f32 width,height;
  };
  v2 min;
  v2 max;
};

union RectI {
  struct {
    i32 x,y;
    i32 width,height;
  };
  v2i min;
  v2i max;
};

struct Transform {
  v3 pos;
  v3 rot;
  v3 scale;
};

KAPI NO_DEBUG f32 Sin(f32 a);
KAPI NO_DEBUG f32 Cos(f32 a);
KAPI NO_DEBUG f32 Tan(f32 a);
KAPI NO_DEBUG f32 Asin(f32 a);
KAPI NO_DEBUG f32 Acos(f32 a);
KAPI NO_DEBUG f32 Atan2(f32 y, f32 x);
KAPI NO_DEBUG f32 Sqrt(f32 a);
KAPI NO_DEBUG f32 Pow(f32 a, f32 b);
KAPI NO_DEBUG f32 Floor(f32 a);
KAPI NO_DEBUG f32 Ceil(f32 a);
KAPI NO_DEBUG f32 Round(f32 a);
KAPI NO_DEBUG f32 Mod(f32 a, f32 b);
KAPI NO_DEBUG f32 Exp(f32 a);
KAPI NO_DEBUG f32 LogE(f32 a);
KAPI NO_DEBUG f32 Log2(f32 a);
KAPI NO_DEBUG f32 Log10(f32 a);

KAPI NO_DEBUG void SinCos(f32 angle, f32* a, f32* b);

KAPI NO_DEBUG f32 SinD(f32 a);
KAPI NO_DEBUG f32 CosD(f32 a);
KAPI NO_DEBUG f32 Lerp(f32 a, f32 t, f32 b);
KAPI NO_DEBUG f32 Atan2_360(f32 y, f32 x);

KAPI u32 next_pow2(u32 v);
KAPI u32 prev_pow2(u32 n);

////////////////////////////////////////////////////////////////////////
// Color

KAPI v4 rgba_from_u32(u32 hex);
KAPI u32 u32_from_rgba(v4 rgba);
KAPI u32 u32_from_argb(v4 argb);

////////////////////////////////////////////////////////////////////////
// Hash

KAPI u64 squirrel3(u64 at);
KAPI u64 str_hash_FNV(String str);
KAPI u64 hash_memory(void* data, u64 size);
KAPI u64 hash(u64 x);
KAPI u64 hash(String str);
KAPI b32 equal(i64 a, i64 b);

////////////////////////////////////////////////////////////////////////
// Random

KAPI u32 xorshift32(u32* seed);

KAPI NO_DEBUG u32 rand_u32();
KAPI NO_DEBUG u32 rand_range_u32(u32 min, u32 max);
KAPI NO_DEBUG i32 rand_i32();
KAPI NO_DEBUG i32 rand_range_i32(i32 min, i32 max);
KAPI NO_DEBUG f32 rand_f32_01();
KAPI NO_DEBUG f32 rand_f32_11();
KAPI NO_DEBUG f32 rand_f32();
KAPI NO_DEBUG f32 rand_range_f32(f32 min, f32 max);
KAPI NO_DEBUG b32 rand_b32();
KAPI NO_DEBUG void rand_seed();

////////////////////////////////////////////////////////////////////////
// Vector2

KAPI NO_DEBUG v2 v2_zero();
KAPI NO_DEBUG v2 v2_one();
KAPI NO_DEBUG v2 v2_up();
KAPI NO_DEBUG v2 v2_down();
KAPI NO_DEBUG v2 v2_left();
KAPI NO_DEBUG v2 v2_right();
KAPI NO_DEBUG v2 v2_of_v3(v3 a);
KAPI NO_DEBUG v2 v2_of_v4(v4 a);
KAPI NO_DEBUG v3 v2_to_v3(v2 a, f32 b);
KAPI NO_DEBUG v4 v2_to_v4(v2 a, f32 b, f32 c);
KAPI NO_DEBUG v2 v2_of_v2i(v2i a);
KAPI NO_DEBUG v2i v2i_of_v2(v2 a);
KAPI NO_DEBUG v2  operator+(v2 a, v2 b);
KAPI NO_DEBUG v2  operator-(v2 a, v2 b);
KAPI NO_DEBUG v2  operator*(v2 a, f32 scalar);
KAPI NO_DEBUG v2  operator*(f32 scalar, v2 a);
KAPI NO_DEBUG v2  operator/(v2 a, f32 scalar);
KAPI NO_DEBUG v2  operator+=(v2& a, v2 b);
KAPI NO_DEBUG v2  operator-=(v2& a, v2 b);
KAPI NO_DEBUG v2  operator*=(v2& a, f32 scalar);
KAPI NO_DEBUG v2  operator/=(v2& a, f32 scalar);
KAPI NO_DEBUG b32 operator==(v2 a, v2 b);
KAPI NO_DEBUG b32 operator!=(v2 a, v2 b);
KAPI NO_DEBUG v2  operator-(v2 a);
KAPI NO_DEBUG f32 v2_length_squared(v2 a);
KAPI NO_DEBUG f32 v2_length(v2 a);
KAPI NO_DEBUG v2  v2_norm(v2 a);
KAPI NO_DEBUG f32 v2_distance(v2 a, v2 b);
KAPI NO_DEBUG f32 v2_dot(v2 a, v2 b);
KAPI NO_DEBUG f32 v2_cross(v2 a, v2 b);
KAPI NO_DEBUG v2  v2_lerp(v2 a, f32 t, v2 b);
KAPI NO_DEBUG v2  v2_skew(v2 a);
KAPI f32 v2_shortest_arc(v2 a, v2 b);

////////////////////////////////////////////////////////////////////////
// Vector3

KAPI NO_DEBUG v3 v3_zero();
KAPI NO_DEBUG v3 v3_one();
KAPI NO_DEBUG v3 v3_up();
KAPI NO_DEBUG v3 v3_down();
KAPI NO_DEBUG v3 v3_left();
KAPI NO_DEBUG v3 v3_right();
KAPI NO_DEBUG v3 v3_forward();
KAPI NO_DEBUG v3 v3_back();
KAPI NO_DEBUG v3 v3_of_v4(v4 a);
KAPI NO_DEBUG v4 v3_to_v4(v3 a, f32 b);
KAPI NO_DEBUG v3  operator+(v3 a, v3 b);
KAPI NO_DEBUG v3  operator-(v3 a, v3 b);
KAPI NO_DEBUG v3  operator*(v3 a, f32 scalar);
KAPI NO_DEBUG v3  operator*(f32 scalar, v3 a);
KAPI NO_DEBUG v3  operator/(v3 a, f32 scalar);
KAPI NO_DEBUG v3  operator+=(v3& a, v3 b);
KAPI NO_DEBUG v3  operator-=(v3& a, v3 b);
KAPI NO_DEBUG v3  operator*=(v3& a, f32 scalar);
KAPI NO_DEBUG v3  operator/=(v3& a, f32 scalar);
KAPI NO_DEBUG b32 operator==(v3 a, v3 b);
KAPI NO_DEBUG b32 operator==(v3u a, v3u b);
KAPI NO_DEBUG b32 operator!=(v3 a, v3 b);
KAPI NO_DEBUG v3  operator-(v3 a);
KAPI NO_DEBUG f32 v3_length_squared(v3 a);
KAPI NO_DEBUG f32 v3_length(v3 a);
KAPI NO_DEBUG v3  v3_norm(v3 a);
KAPI NO_DEBUG f32 v3_distance(v3 a, v3 b);
KAPI NO_DEBUG f32 v3_dot(v3 a, v3 b);
KAPI NO_DEBUG v3  v3_cross(v3 a, v3 b);
KAPI NO_DEBUG v3  v3_lerp(v3 a, f32 t, v3 b);
KAPI NO_DEBUG v3 v3_pos_of_mat4(mat4 mat);
KAPI NO_DEBUG v3 v3_rot_of_mat4(mat4 mat);
KAPI NO_DEBUG v3 v3_scale_of_mat4(mat4 mat);
KAPI NO_DEBUG v3 v3_rand_range(v3 a, v3 b);

////////////////////////////////////////////////////////////////////////
// Vector4

KAPI NO_DEBUG v4  operator+(v4 a, v4 b);
KAPI NO_DEBUG v4  operator-(v4 a, v4 b);
KAPI NO_DEBUG v4  operator*(v4 a, f32 scalar);
KAPI NO_DEBUG v4  operator*(f32 scalar, v4 a);
KAPI NO_DEBUG v4  operator/(v4 a, f32 scalar);
KAPI NO_DEBUG v4  operator+=(v4& a, v4 b);
KAPI NO_DEBUG v4  operator-=(v4& a, v4 b);
KAPI NO_DEBUG v4  operator*=(v4& a, f32 scalar);
KAPI NO_DEBUG v4  operator/=(v4& a, f32 scalar);
KAPI NO_DEBUG v4  operator-(v4 a);
KAPI NO_DEBUG f32 v4_length_squared(v4 a);
KAPI NO_DEBUG f32 v4_length(v4 a);
KAPI NO_DEBUG v4  v4_normalize(v4 a);

////////////////////////////////////////////////////////////////////////
// Matrix3

KAPI NO_DEBUG mat3 mat3_identity();
KAPI NO_DEBUG mat3 mat3_translate(v2 pos);
KAPI NO_DEBUG mat3 mat3_scale(v2 scale);
KAPI NO_DEBUG mat3 operator*(mat3 a, mat3 b);
KAPI NO_DEBUG mat3& operator*=(mat3& a, mat3 b);
KAPI NO_DEBUG v3 operator*(mat3 mat, v3 vec);

////////////////////////////////////////////////////////////////////////
// Matrix4

KAPI NO_DEBUG mat4 mat4_identity();
KAPI NO_DEBUG mat4 mat4_translate(v3 pos);
KAPI NO_DEBUG mat4 mat4_scale(v3 scale);
KAPI NO_DEBUG mat4 mat4_rotate_x(f32 angle_radians);
KAPI NO_DEBUG mat4 mat4_rotate_y(f32 angle_radians);
KAPI NO_DEBUG mat4 mat4_rotate_z(f32 angle_radians);
KAPI NO_DEBUG mat4 operator*(mat4 a, mat4 b);
KAPI NO_DEBUG mat4& operator*=(mat4& a, mat4 b);
KAPI NO_DEBUG v4 operator*(mat4 mat, v4 vec);
KAPI NO_DEBUG mat4 mat4_rotate_xyz(v3 rot);
KAPI NO_DEBUG mat4 mat4_transform(v3 pos, v3 rot, v3 scale);
KAPI NO_DEBUG mat4 mat4_transform(Transform trans);
KAPI NO_DEBUG mat4 mat4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
KAPI NO_DEBUG mat4 mat4_perspective(f32 fov_radians, f32 aspect_ratio, f32 Near, f32 Far);
KAPI NO_DEBUG mat4 mat4_look_at(v3 pos, v3 dir, v3 up);
KAPI NO_DEBUG mat4 mat4_transpose(mat4 matrix);
KAPI NO_DEBUG mat4 mat4_inverse(mat4 matrix);
KAPI NO_DEBUG v3 mat4_forward(mat4 matrix);
KAPI NO_DEBUG v3 mat4_backward(mat4 matrix);
KAPI NO_DEBUG v3 mat4_up(mat4 matrix);
KAPI NO_DEBUG v3 mat4_down(mat4 matrix);
KAPI NO_DEBUG v3 mat4_left(mat4 matrix);
KAPI NO_DEBUG v3 mat4_right(mat4 matrix);

////////////////////////////////////////////////////////////////////////
// Quaternions

KAPI NO_DEBUG quat quat_identity();
KAPI NO_DEBUG f32 quat_normal(quat q);
KAPI NO_DEBUG quat quat_normalize(quat q);
KAPI NO_DEBUG quat quat_conjugate(quat q);
KAPI NO_DEBUG quat quat_inverse(quat q);
KAPI NO_DEBUG quat quat_mul(quat q_0, quat q_1);
KAPI NO_DEBUG f32 quat_dot(quat q_0, quat q_1);
KAPI NO_DEBUG mat4 quat_to_mat4(quat q);
KAPI NO_DEBUG mat4 quat_to_rotation_matrix(quat q, v3 center);
KAPI NO_DEBUG quat quat_from_axis_angle(v3 axis, f32 angle, b32 normalize);
KAPI NO_DEBUG quat quat_slerp(quat q_0, quat q_1, f32 percentage);

