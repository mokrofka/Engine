#pragma once
#include "base.h"

const f32 PI             = 3.14159265358f;
const f32 Tau            = (PI * 2);
const f32 EulerNumber    = 2.71828182846f;
const f32 GoldBig        = 1.61803398875f;
const f32 GoldSmall      = 0.61803398875f;
const f32 FloatEpsilon   = 1e-5f;
const f32 MachineEpsilon = 1.1920929e-7f;

f32 degtorad(f32 degrees);
f32 radtodeg(f32 radians);

///////////////////////////////////
// v2

union v2 {
  struct {
    f32 x;
    f32 y;
  };
  f32 v[2];
  v2() = default;
  v2(f32 x_, f32 y_);
};

union v2i {
  struct {
    i32 x;
    i32 y;
  };
  i32 v[2];
  v2i() = default;
  v2i(i32 x_, i32 y_);
};

union v2u {
  struct {
    u32 x;
    u32 y;
  };
  u32 v[2];
  v2u() = default;
  v2u(u32 x_, u32 y_);
};

union v2b {
  struct {
    b32 x;
    b32 y;
  };
  b32 v[2];
  v2b() = default;
  v2b(b32 x_, b32 y_);
};

///////////////////////////////////
// v3

union v3 {
  struct {
    f32 x;
    f32 y;
    f32 z;
  };
  f32 v[3];
  v3() = default;
  v3(f32 x_, f32 y_, f32 z_);
};

union v3u {
  struct {
    u32 x;
    u32 y;
    u32 z;
  };
  u32 v[3];
  v3u() = default;
  v3u(u32 x_, u32 y_, u32 z_);
};

u64 hash(v3u v);
b32 equal(v3u a, v3u b);

union v3b {
  struct {
    b32 x;
    b32 y;
    b32 z;
  };
  b32 v[3];
  v3b() = default;
  v3b(b32 x_, b32 y_, b32 z_);
};

///////////////////////////////////
// v4

union v4 {
  struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
  };
  f32 v[4];
  v4() = default;
  v4(f32 x_, f32 y_, f32 z_, f32 w_);
};

union mat3 {
  f32 v[3][3];
};

union mat4 {
  f32 v[4][4];
  struct {
    v4 x;
    v4 y;
    v4 z;
    v4 w;
  };
};

////////////////////////////////////////////////////////////////////////
// Ranges

///////////////////////////////////
// dim1

struct Rng1u {
  u32 min;
  u32 max;
};

struct Rng1i {
  i32 min;
  i32 max;
};

struct Rng1u64 {
  u64 min;
  u64 max;
};

struct Rng1 {
  f32 min;
  f32 max;
};

///////////////////////////////////
// Dim2

union Rng2 {
  struct {
    v2 min;
    v2 max;
  };
  struct {
    f32 x0;
    f32 y0;
    f32 x1;
    f32 y1;
  };
  Rng2() = default;
  Rng2(v2 min_, v2 max_);
};

///////////////////////////////////
// Dim3

union Rng3 {
  struct {
    v3 min;
    v3 max;
  };
  struct {
    f32 x0;
    f32 y0;
    f32 z0;
    f32 x1;
    f32 y1;
    f32 z1;
  };
  Rng3() = default;
  Rng3(v3 min_, v3 max_);
};

///////////////////////////////////
// Misc

struct Transform {
  v3 pos;
  v3 rot;
  v3 scale;
};

struct Ray {
  v3 pos;
  v3 dir;
};

////////////////////////////////////////////////////////////////////////
// Float Ops

NO_DEBUG f32 Sin(f32 x);
NO_DEBUG f32 Cos(f32 x);
NO_DEBUG f32 Tan(f32 x);
NO_DEBUG f32 Asin(f32 x);
NO_DEBUG f32 Acos(f32 x);
NO_DEBUG f32 Atan2(f32 y, f32 x);
NO_DEBUG f32 Sqrt(f32 x);
NO_DEBUG f32 Pow(f32 a, f32 b);
NO_DEBUG f32 Floor(f32 x);
NO_DEBUG f32 Ceil(f32 x);
NO_DEBUG f32 Round(f32 x);
NO_DEBUG f32 Mod(f32 a, f32 b);
NO_DEBUG f32 Exp(f32 x);
NO_DEBUG f32 LogE(f32 x);
NO_DEBUG f32 Log2(f32 x);
NO_DEBUG f32 Log10(f32 x);

NO_DEBUG void SinCos(f32 rad, f32* a, f32* b);

NO_DEBUG f32 SinD(f32 x);
NO_DEBUG f32 CosD(f32 x);

////////////////////////////////////////////////////////////////////////
// Color

v4 rgba_from_u32(u32 hex);
u32 u32_from_rgba(v4 rgba);

////////////////////////////////////////////////////////////////////////
// Hash

u64 squirrel3(u64 x);
u64 str_hash_FNV(String str);
u64 hash_memory(void* data, u64 size);
u64 hash(u64 x, u64 seed = 0);
u64 hash(String str, u64 seed = 0);

////////////////////////////////////////////////////////////////////////
// Random

u32 xorshift32(u32* seed);

NO_DEBUG u32 rand_u32();
NO_DEBUG u32 rand_rng_u32(u32 min, u32 max);
NO_DEBUG i32 rand_i32();
NO_DEBUG i32 rand_rng_i32(i32 min, i32 max);
NO_DEBUG f32 rand_f32_01();
NO_DEBUG f32 rand_f32_11();
NO_DEBUG f32 rand_f32();
NO_DEBUG f32 rand_rng_f32(f32 min, f32 max);
NO_DEBUG b32 rand_b32();
NO_DEBUG void rand_set_seed();
NO_DEBUG u32 rand_get_seed();
template<typename T> void rand_shuffle(Slice<T> data) {
  Loop (i, data.count) {
    u32 j = rand_rng_u32(i, data.count - 1);
    Swap(data[i], data[j]);
  }
}

////////////////////////////////////////////////////////////////////////
// Misc

i32 wrap_i32(i32 min, i32 x, i32 max);
f32 wrap_f32(f32 min, f32 x, f32 max);
f32 Lerp(f32 a, f32 t, f32 b);
f32 Unlerp(f32 a, f32 x, f32 b);
f64 Unlerpf64(f64 a, f64 x, f64 b);
f32 remap(f32 x, f32 old_min, f32 old_max, f32 new_min, f32 new_max);
f64 remapf64(f64 x, f64 old_min, f64 old_max, f64 new_min, f64 new_max);
f32 remap_clamped(f32 x, f32 old_min, f32 old_max, f32 new_min, f32 new_max);

////////////////////////////////////////////////////////////////////////
// Vector2

NO_DEBUG v2  operator+(v2 a, v2 b);
NO_DEBUG v2  operator-(v2 a, v2 b);
NO_DEBUG v2  operator*(v2 v, f32 scalar);
NO_DEBUG v2  operator*(f32 scalar, v2 v);
NO_DEBUG v2  operator/(v2 v, f32 scalar);
NO_DEBUG v2  operator+=(v2& a, v2 b);
NO_DEBUG v2  operator-=(v2& a, v2 b);
NO_DEBUG v2  operator*=(v2& v, f32 scalar);
NO_DEBUG v2  operator/=(v2& v, f32 scalar);
NO_DEBUG b32 operator==(v2 a, v2 b);
NO_DEBUG b32 operator!=(v2 a, v2 b);
NO_DEBUG v2  operator-(v2 v);

NO_DEBUG b32 operator==(v2u a, v2u b);
NO_DEBUG b32 operator!=(v2u a, v2u b);

NO_DEBUG v2  v2_of_v3(v3 v);
NO_DEBUG v2  v2_of_v4(v4 v);
NO_DEBUG v3  v2_to_v3(v2 v, f32 a);
NO_DEBUG v4  v2_to_v4(v2 v, f32 a, f32 b);
NO_DEBUG v2  v2_of_v2i(v2i v);
NO_DEBUG v2  v2_of_v2u(v2u v);
NO_DEBUG v2i v2i_of_v2(v2 v);
NO_DEBUG v2  v2_up();
NO_DEBUG v2  v2_down();
NO_DEBUG v2  v2_left();
NO_DEBUG v2  v2_right();
NO_DEBUG v2  v2_zero();
NO_DEBUG v2  v2_one();
NO_DEBUG v2  v2_splat(f32 x);
NO_DEBUG v2  v2_add_scalar(v2 v, f32 x);
NO_DEBUG v2  v2_subtract_scalar(v2 v, f32 x);
NO_DEBUG v2  v2_min(v2 a, v2 b);
NO_DEBUG v2  v2_max(v2 a, v2 b);
NO_DEBUG v2  v2_invert(v2 v);
NO_DEBUG v2b v2_greater(v2 a, v2 b);
NO_DEBUG v2b v2_less(v2 a, v2 b);
NO_DEBUG b32 v2_greater_any(v2 a, v2 b);
NO_DEBUG b32 v2_less_any(v2 a, v2 b);
NO_DEBUG v2  v2_clamp(v2 min, v2 v, v2 max);
NO_DEBUG v2  v2_sign(v2 v);
NO_DEBUG v2  v2_rand_rng(v2 a, v2 b);

NO_DEBUG f32 v2_length(v2 v);
NO_DEBUG f32 v2_length_sqr(v2 v);
NO_DEBUG v2  v2_norm(v2 v);
NO_DEBUG f32 v2_distance(v2 a, v2 b);
NO_DEBUG f32 v2_distance_sqr(v2 a, v2 b);
NO_DEBUG f32 v2_dot(v2 a, v2 b);
NO_DEBUG f32 v2_cross(v2 a, v2 b);  // if > 0 then a is at right
NO_DEBUG v2  v2_lerp(v2 a, f32 t, v2 b);
NO_DEBUG v2  v2_hadamard(v2 a, v2 b);
NO_DEBUG v2  v2_hadamard_div(v2 a, v2 b);
NO_DEBUG v2  v2_project(v2 a, v2 b);
NO_DEBUG v2  v2_project_on_unit(v2 a, v2 b);
NO_DEBUG f32 v2_length_projection(v2 a, v2 b);
NO_DEBUG v2  v2_reject(v2 a, v2 b);
NO_DEBUG v2  v2_reflect(v2 v, v2 normal);
NO_DEBUG f32 v2_angle(v2 a, v2 b);

v2 v2_step_to(v2 v, v2 target, f32 step);
v2 v2_clamp_length(f32 min, v2 v, f32 max);

NO_DEBUG v2  v2_flip_y(v2 v);
NO_DEBUG v2  v2_remap_01_to_11(v2 pos, v2 range);
NO_DEBUG f32 v2_line_angle(v2 start, v2 end);
NO_DEBUG v2  v2_rotate_90(v2 v);
NO_DEBUG v2  v2_rotate_negative_90(v2 v);
NO_DEBUG v2  v2_rotate(v2 v, f32 sine, f32 cosine);
NO_DEBUG v2  v2_rotate(v2 v, f32 rad);
NO_DEBUG v2  v2_rotate_relative(v2 v, v2 pivot, f32 rad);

////////////////////////////////////////////////////////////////////////
// Vector3

NO_DEBUG v3  operator+(v3 a, v3 b);
NO_DEBUG v3  operator-(v3 a, v3 b);
NO_DEBUG v3  operator*(v3 v, f32 scalar);
NO_DEBUG v3  operator*(f32 scalar, v3 v);
NO_DEBUG v3  operator/(v3 v, f32 scalar);
NO_DEBUG v3  operator+=(v3& a, v3 b);
NO_DEBUG v3  operator-=(v3& a, v3 b);
NO_DEBUG v3  operator*=(v3& v, f32 scalar);
NO_DEBUG v3  operator/=(v3& v, f32 scalar);
NO_DEBUG b32 operator==(v3 a, v3 b);
NO_DEBUG b32 operator!=(v3 a, v3 b);
NO_DEBUG v3  operator-(v3 v);

NO_DEBUG b32 operator==(v3u a, v3u b);

NO_DEBUG v3  v3_of_v4(v4 v);
NO_DEBUG v4  v3_to_v4(v3 v, f32 a);
NO_DEBUG v3  v3_up();
NO_DEBUG v3  v3_down();
NO_DEBUG v3  v3_left();
NO_DEBUG v3  v3_right();
NO_DEBUG v3  v3_forward();
NO_DEBUG v3  v3_back();
NO_DEBUG v3  v3_zero();
NO_DEBUG v3  v3_one();
NO_DEBUG v3  v3_splat(f32 x);
NO_DEBUG v3  v3_add_scalar(v3 v, f32 x);
NO_DEBUG v3  v3_subtract_scalar(v3 v, f32 x);
NO_DEBUG v3  v3_min(v3 a, v3 b);
NO_DEBUG v3  v3_max(v3 a, v3 b);
NO_DEBUG v3  v3_invert(v3 v);
NO_DEBUG v3b v3_greater(v3 a, v3 b);
NO_DEBUG v3b v3_less(v3 a, v3 b);
NO_DEBUG b32 v3_greater_any(v3 a, v3 b);
NO_DEBUG b32 v3_less_any(v3 a, v3 b);
NO_DEBUG v3  v3_clamp(v3 min, v3 v, v3 max);
NO_DEBUG v3  v3_sign(v3 v);
NO_DEBUG v3  v3_rand_rng(v3 a, v3 b);

NO_DEBUG f32 v3_length(v3 v);
NO_DEBUG f32 v3_length_sqr(v3 v);
NO_DEBUG v3  v3_norm(v3 v);
NO_DEBUG f32 v3_distance(v3 a, v3 b);
NO_DEBUG f32 v3_distance_sqr(v3 a, v3 b);
NO_DEBUG f32 v3_dot(v3 a, v3 b);
NO_DEBUG v3  v3_cross(v3 a, v3 b);
NO_DEBUG v3  v3_lerp(v3 a, f32 t, v3 b);
NO_DEBUG v3  v3_hadamard(v3 a, v3 b);
NO_DEBUG v3  v3_hadamard_div(v3 a, v3 b);
NO_DEBUG v3  v3_project(v3 a, v3 b);
NO_DEBUG v3  v3_project_on_unit(v3 a, v3 b);
NO_DEBUG f32 v3_length_projection(v3 a, v3 b);
NO_DEBUG v3  v3_reject(v3 a, v3 b);
NO_DEBUG v3  v3_reject_on_unit(v3 a, v3 b);
NO_DEBUG v3  v3_reflect(v3 v, v3 normal);
NO_DEBUG f32 v3_angle(v3 a, v3 b);

v3 v3_step_to(v3 v, v3 target, f32 step);
v3 v3_clamp_length(f32 min, v3 v, f32 max);

NO_DEBUG v3  v3_pos_of_mat4(mat4 mat);
NO_DEBUG v3  v3_rotate_x(v3 v, f32 rad);
NO_DEBUG v3  v3_rotate_y(v3 v, f32 rad);
NO_DEBUG v3  v3_rotate_z(v3 v, f32 rad);

v3 v3_barycentric(v3 v, v3 a, v3 b, v3 c);
v3 v3_perpendicular(v3 v);
v3 v3_rotate_around_axis(v3 v, v3 axis, f32 rad);
v3 v3_refract(v3 v, v3 n, f32 r);

////////////////////////////////////////////////////////////////////////
// Vector4

NO_DEBUG v4  operator+(v4 a, v4 b);
NO_DEBUG v4  operator-(v4 a, v4 b);
NO_DEBUG v4  operator*(v4 v, f32 scalar);
NO_DEBUG v4  operator*(f32 scalar, v4 v);
NO_DEBUG v4  operator/(v4 v, f32 scalar);
NO_DEBUG v4  operator+=(v4& a, v4 b);
NO_DEBUG v4  operator-=(v4& a, v4 b);
NO_DEBUG v4  operator*=(v4& v, f32 scalar);
NO_DEBUG v4  operator/=(v4& v, f32 scalar);
NO_DEBUG v4  operator-(v4 v);

NO_DEBUG v4  v4_zero();
NO_DEBUG v4  v4_one();
NO_DEBUG v4  v4_set_w(v4 v, f32 w);
NO_DEBUG v4  v4_splat(f32 x);
NO_DEBUG v4  v4_add_scalar(v4 v, f32 x);
NO_DEBUG v4  v4_subtract_scalar(v4 v, f32 x);
NO_DEBUG v4  v4_min(v4 a, v4 b);
NO_DEBUG v4  v4_max(v4 a, v4 b);
NO_DEBUG v4  v4_invert(v4 v);

NO_DEBUG f32 v4_length(v4 v);
NO_DEBUG f32 v4_length_squared(v4 v);
NO_DEBUG v4  v4_norm(v4 v);
NO_DEBUG f32 v4_distance(v4 a, v4 b);
NO_DEBUG f32 v4_distance_sqr(v4 a, v4 b);
NO_DEBUG f32 v4_dot(v4 v);
NO_DEBUG v4  v4_lerp(v4 a, f32 t, v4 b);
NO_DEBUG v4  v4_hadamard(v4 a, v4 b);

////////////////////////////////////////////////////////////////////////
// Quatornion

v4 quat_identity();
v4 quat_norm(v4 q);
v4 quat_conjugate(v4 q);
v4 quat_inverse(v4 q);
v4 quat_axis_angle(v3 axis, f32 rad);
v3 quat_rotate(v4 q, v3 v);
v4 quat_mul(v4 a, v4 b);
mat3 quat_to_mat3(v4 q);
mat4 quat_to_mat4(v4 q);
v4 quat_slerp(v4 a, f32 t, v4 b);
v4 quat_nlerp(v4 a, f32 t, v4 b);
v4 quat_from_to(v3 a, v3 b);
v4 quat_from_euler(v3 e);

////////////////////////////////////////////////////////////////////////
// Matrix2

////////////////////////////////////////////////////////////////////////
// Matrix3

mat3 operator*(mat3 a, mat3 b);
mat3& operator*=(mat3& a, mat3 b);
v3 operator*(mat3 mat, v3 vec);

mat3 mat3_identity();
mat3 mat3_translate(v2 pos);
mat3 mat3_scale(v2 scale);

////////////////////////////////////////////////////////////////////////
// Matrix4

mat4 operator*(mat4 a, mat4 b);
mat4& operator*=(mat4& a, mat4 b);
v4 operator*(mat4 mat, v4 vec);

v3 mat4_forward(mat4 matrix);
v3 mat4_backward(mat4 matrix);
v3 mat4_up(mat4 matrix);
v3 mat4_down(mat4 matrix);
v3 mat4_right(mat4 matrix);
v3 mat4_left(mat4 matrix);

mat4 mat4_identity();
mat4 mat4_translate(v3 pos);
mat4 mat4_scale(v3 scale);
mat4 mat4_scale_all_elements(mat4 mat, f32 scale);
mat4 mat4_rotate_x(f32 rad);
mat4 mat4_rotate_y(f32 rad);
mat4 mat4_rotate_z(f32 rad);
mat4 mat4_rotate_xyz(v3 rad);
mat4 mat4_rotate_around_axis(v3 axis, f32 rad);
mat4 mat4_transform(v3 pos, v3 rot, v3 scale);
mat4 mat4_transform(Transform trans);
mat4 mat4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
mat4 mat4_perspective(f32 fov_radians, f32 aspect_ratio, f32 near, f32 far);
mat4 mat4_look_at(v3 pos, v3 dir, v3 up);
mat4 mat4_transpose(mat4 matrix);
mat4 mat4_inverse(mat4 matrix);

////////////////////////////////////////////////////////////////////////
// Misc

Ray ray_make(v3 pos, v3 dir);
Ray ray_from_screen(v2 screen_pos, v2u viewport_rect, v3 origin, mat4 view, mat4 projection);
v2 world_to_screen(v3 pos, v3 camera_pos, mat4 view, mat4 projection);

////////////////////////////////////////////////////////////////////////
// Range Ops

///////////////////////////////////
// Dim1
Rng1u rng1u_shift(Rng1u r, u32 x);
Rng1u rng1u_pad(Rng1u r, u32 x);
u32 rng1u_center(Rng1u r);
b32 rng1u_contains(Rng1u r, u32 x);
u32 rng1u_dim(Rng1u r);
Rng1u rng1u_union(Rng1u a, Rng1u b);
Rng1u rng1u_intersect(Rng1u a, Rng1u b);
u32 rng1u_clamp(Rng1u r, u32 v);

Rng1i rng1i_shift(Rng1i r, i32 x);
Rng1i rng1i_pad(Rng1i r, i32 x);
i32 rng1i_center(Rng1i r);
b32 rng1i_contains(Rng1i r, i32 x);
i32 rng1i_dim(Rng1i r);
Rng1i rng1i_union(Rng1i a, Rng1i b);
Rng1i rng1i_intersect(Rng1i a, Rng1i b);
i32 rng1i_clamp(Rng1i r, i32 v);

Rng1u64 rng1u64_shift(Rng1u64 r, u64 x);
Rng1u64 rng1u64_pad(Rng1u64 r, u64 x);
u64 rng1u64_center(Rng1u64 r);
b32 rng1u64_contains(Rng1u64 r, u64 x);
u64 rng1u64_dim(Rng1u64 r);
Rng1u64 rng1u64_union(Rng1u64 a, Rng1u64 b);
Rng1u64 rng1u64_intersect(Rng1u64 a, Rng1u64 b);
u64 rng1u64_clamp(Rng1u64 r, u64 v);

Rng1 rng1_shift(Rng1 r, f32 x);
Rng1 rng1_pad(Rng1 r, f32 x);
f32 rng1_center(Rng1 r);
b32 rng1_contains(Rng1 r, f32 x);
f32 rng1_dim(Rng1 r);
Rng1 rng1_union(Rng1 a, Rng1 b);
Rng1 rng1_intersect(Rng1 a, Rng1 b);
f32 rng1_clamp(Rng1 r, f32 v);

f32 rng1_lerp(Rng1 r, f32 t);
f32 rng1_unlerp(Rng1 r, f32 x);

///////////////////////////////////
// Dim2
Rng2 rng2_shift(Rng2 r, v2 x);
Rng2 rng2_pad(Rng2 r, f32 x);
v2 rng2_center(Rng2 r);
b32 rng2_contains(Rng2 r, v2 x);
v2 rng2_dim(Rng2 r);
Rng2 rng2_union(Rng2 a, Rng2 b);
Rng2 rng2_intersect(Rng2 a, Rng2 b);
v2 rng2_clamp(Rng2 r, v2 x);

Rng2 rng2_make(v2 min, v2 size);             
Rng2 rng2_make_centered(v2 pos, v2 halfdim); 
Rng2 rng2_scale_centered(Rng2 r, f32 scale); 
Rng2 rng2_scale_centered(Rng2 r, v2 scale);  
Rng2 rng2_scale(Rng2 r, f32 scale);          
Rng2 rng2_scale(Rng2 r, v2 scale);           

Rng2 rng2_subrng_x(Rng2 r, Rng1 x);
Rng2 rng2_subrng_y(Rng2 r, Rng1 y);
Rng2 rng2_subrng_x01(Rng2 r, Rng1 sub);
Rng2 rng2_subrng_y01(Rng2 r, Rng1 sub);
Rng2 rng2_align_dim_at_center(Rng2 r, v2 size);

///////////////////////////////////
// Dim3
Rng3 rng3_shift(Rng3 r, v3 x);
Rng3 rng3_pad(Rng3 r, f32 x);
v3 rng3_center(Rng3 r);
b32 rng3_contains(Rng3 r, v3 x);
v3 rng3_dim(Rng3 r);
Rng3 rng3_union(Rng3 a, Rng3 b);
Rng3 rng3_intersect(Rng3 a, Rng3 b);
v3 rng3_clamp(Rng3 r, v3 x);

Rng3 rng3_make(v3 min, v3 size);
Rng3 rng3_make_centered(v3 pos, v3 halfdim);
Rng3 rng3_scale_centered(Rng3 r, f32 scale);
Rng3 rng3_scale_centered(Rng3 r, v3 scale);
Rng3 rng3_scale(Rng3 r, f32 scale);
Rng3 rng3_scale(Rng3 r, v3 scale);

struct Rng2Cursor {
  v2 pos;
};

Rng2 layout_row(Rng2Cursor& c, Rng1 x, f32 h);
void layout_next(Rng2Cursor& c, f32 h);

