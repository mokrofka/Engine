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

union v3b {
  struct {
    b32 x;
    b32 y;
    b32 z;
  };
  u32 v[3];
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
  f32 v[9];
  f32& operator[](u32 a);
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

struct Rng1u32 {
  u32 min;
  u32 max;
  Rng1u32() = default;
  Rng1u32(u32 min_, u32 max_);
};

struct Rng1i32 {
  i32 min;
  i32 max;
  Rng1i32() = default;
  Rng1i32(i32 min_, i32 max_);
};

struct Rng1u64 {
  u64 min;
  u64 max;
  Rng1u64() = default;
  Rng1u64(u64 min_, u64 max_);
};

struct Rng1f32 {
  f32 min;
  f32 max;
  Rng1f32() = default;
  Rng1f32(f32 min_, f32 max_);
};

///////////////////////////////////
// Dim2

union Rng2f32 {
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
  Rng2f32() = default;
  Rng2f32(v2 min_, v2 max_);
};

///////////////////////////////////
// Dim3

struct Rng3f32 {
  v3 min;
  v3 max;
  struct {
    f32 x0;
    f32 y0;
    f32 z0;
    f32 x1;
    f32 y1;
    f32 z1;
  };
  Rng3f32() = default;
  Rng3f32(v3 min_, v3 max_);
};

typedef Rng1f32 Rng1;
typedef Rng2f32 Rng2;
typedef Rng3f32 Rng3;

///////////////////////////////////
// Misc

// union Rect {
//   struct {
//     f32 x,y;
//     f32 width,height;
//   };
//   struct {
//     v2 min;
//     v2 max;
//   };
//   Rect() = default;
//   Rect(v2 min_, v2 max_) {
//     min = min_;
//     max = max_;
//   }
// };

struct Transform {
  v3 pos;
  v3 rot;
  v3 scale;
};

struct Ray {
  v3 origin;
  v3 dir;
};

////////////////////////////////////////////////////////////////////////
// Float Ops

NO_DEBUG f32 Sin(f32 a);
NO_DEBUG f32 Cos(f32 a);
NO_DEBUG f32 Tan(f32 a);
NO_DEBUG f32 Asin(f32 a);
NO_DEBUG f32 Acos(f32 a);
NO_DEBUG f32 Atan2(f32 y, f32 x);
NO_DEBUG f32 Sqrt(f32 a);
NO_DEBUG f32 Pow(f32 a, f32 b);
NO_DEBUG f32 Floor(f32 a);
NO_DEBUG f32 Ceil(f32 a);
NO_DEBUG f32 Round(f32 a);
NO_DEBUG f32 Mod(f32 a, f32 b);
NO_DEBUG f32 Exp(f32 a);
NO_DEBUG f32 LogE(f32 a);
NO_DEBUG f32 Log2(f32 a);
NO_DEBUG f32 Log10(f32 a);

NO_DEBUG void SinCos(f32 angle, f32* a, f32* b);

NO_DEBUG f32 SinD(f32 a);
NO_DEBUG f32 CosD(f32 a);
NO_DEBUG f32 Atan2_360(f32 y, f32 x);

////////////////////////////////////////////////////////////////////////
// Sort

template<typename T, typename Compare> void sort_insert(Slice<T> slice, Compare cmp) {
  for (i32 i = 1; i < slice.count; ++i) {
    T key = slice[i];
    i32 j = i - 1;
    while (j >= 0 && cmp(key, slice[j])) {
      slice[j + 1] = slice[j];
      j--;
    }
    slice[j + 1] = key;
  }
}
#define sort_insert_l(data, ...) sort_insert(data, [](var a, var b) __VA_ARGS__)

////////////////////////////////////////////////////////////////////////
// Color

v4 rgba_from_u32(u32 hex);
u32 u32_from_rgba(v4 rgba);

////////////////////////////////////////////////////////////////////////
// Hash

u64 squirrel3(u64 at);
u64 str_hash_FNV(String str);
u64 hash_memory(void* data, u64 size);
u64 hash(u64 x, u64 seed = 0);
u64 hash(String str, u64 seed = 0);

////////////////////////////////////////////////////////////////////////
// Random

u32 xorshift32(u32* seed);

NO_DEBUG u32 rand_u32();
NO_DEBUG u32 rand_range_u32(u32 min, u32 max);
NO_DEBUG i32 rand_i32();
NO_DEBUG i32 rand_range_i32(i32 min, i32 max);
NO_DEBUG f32 rand_f32_01();
NO_DEBUG f32 rand_f32_11();
NO_DEBUG f32 rand_f32();
NO_DEBUG f32 rand_range_f32(f32 min, f32 max);
NO_DEBUG b32 rand_b32();
NO_DEBUG void rand_seed();
NO_DEBUG u32 rand_get_seed();
template<typename T> void rand_shuffle(Slice<T> slice) {
  Loop (i, slice.count) {
    u32 j = rand_range_u32(i, slice.count - 1);
    Swap(slice[i], slice[j]);
  }
}

////////////////////////////////////////////////////////////////////////
// Misc

NO_DEBUG f32 Lerp(f32 a, f32 t, f32 b);
f32 inverse_lerp(f32 a, f32 x, f32 b);
f64 inverse_lerp_f64(f64 a, f64 x, f64 b);
f32 map_range_f32(f32 v, f32 old_min, f32 old_max, f32 new_min, f32 new_max);

////////////////////////////////////////////////////////////////////////
// Vector2

NO_DEBUG v2 v2_zero();
NO_DEBUG v2 v2_one();
NO_DEBUG v2 v2_scale(f32 a);
NO_DEBUG v2 v2_up();
NO_DEBUG v2 v2_down();
NO_DEBUG v2 v2_left();
NO_DEBUG v2 v2_right();
NO_DEBUG v2 v2_of_v3(v3 a);
NO_DEBUG v2 v2_of_v4(v4 a);
NO_DEBUG v3 v2_to_v3(v2 a, f32 b);
NO_DEBUG v4 v2_to_v4(v2 a, f32 b, f32 c);
NO_DEBUG v2 v2_of_v2i(v2i a);
NO_DEBUG v2 v2_of_v2u(v2u a);
NO_DEBUG v2i v2i_of_v2(v2 a);
NO_DEBUG v2 v2_add_x(v2 a, f32 x);
NO_DEBUG v2 v2_add_y(v2 a, f32 y);
NO_DEBUG v2  operator+(v2 a, v2 b);
NO_DEBUG v2  operator-(v2 a, v2 b);
NO_DEBUG v2  operator*(v2 a, f32 scalar);
NO_DEBUG v2  operator*(f32 scalar, v2 a);
NO_DEBUG v2  operator/(v2 a, f32 scalar);
NO_DEBUG v2  operator+=(v2& a, v2 b);
NO_DEBUG v2  operator-=(v2& a, v2 b);
NO_DEBUG v2  operator*=(v2& a, f32 scalar);
NO_DEBUG v2  operator/=(v2& a, f32 scalar);
NO_DEBUG b32 operator==(v2 a, v2 b);
NO_DEBUG b32 operator!=(v2 a, v2 b);
NO_DEBUG v2  operator-(v2 a);
NO_DEBUG f32 v2_length_squared(v2 a);
NO_DEBUG f32 v2_length(v2 a);
NO_DEBUG v2  v2_norm(v2 a);
NO_DEBUG f32 v2_distance(v2 a, v2 b);
NO_DEBUG f32 v2_dot(v2 a, v2 b);
NO_DEBUG f32 v2_cross(v2 a, v2 b);
NO_DEBUG v2  v2_lerp(v2 a, f32 t, v2 b);
NO_DEBUG v2  v2_skew(v2 a);
NO_DEBUG v2 v2_rand_range(v2 a, v2 b);
f32 v2_shortest_arc(v2 a, v2 b);
v2 v2_map_to_v2_11(v2 pos, v2 size);

////////////////////////////////////////////////////////////////////////
// Vector3

NO_DEBUG v3 v3_zero();
NO_DEBUG v3 v3_one();
NO_DEBUG v3 v3_scale(f32 a);
NO_DEBUG v3 v3_up();
NO_DEBUG v3 v3_down();
NO_DEBUG v3 v3_left();
NO_DEBUG v3 v3_right();
NO_DEBUG v3 v3_forward();
NO_DEBUG v3 v3_back();
NO_DEBUG v3 v3_of_v4(v4 a);
NO_DEBUG v4 v3_to_v4(v3 a, f32 b);
NO_DEBUG v3  operator+(v3 a, v3 b);
NO_DEBUG v3  operator-(v3 a, v3 b);
NO_DEBUG v3  operator*(v3 a, f32 scalar);
NO_DEBUG v3  operator*(f32 scalar, v3 a);
NO_DEBUG v3  operator/(v3 a, f32 scalar);
NO_DEBUG v3  operator+=(v3& a, v3 b);
NO_DEBUG v3  operator-=(v3& a, v3 b);
NO_DEBUG v3  operator*=(v3& a, f32 scalar);
NO_DEBUG v3  operator/=(v3& a, f32 scalar);
NO_DEBUG b32 operator==(v3 a, v3 b);
NO_DEBUG b32 operator==(v3u a, v3u b);
NO_DEBUG b32 operator!=(v3 a, v3 b);
NO_DEBUG v3  operator-(v3 a);
NO_DEBUG f32 v3_length_squared(v3 a);
NO_DEBUG f32 v3_length(v3 a);
NO_DEBUG v3  v3_norm(v3 a);
NO_DEBUG f32 v3_distance(v3 a, v3 b);
NO_DEBUG f32 v3_dot(v3 a, v3 b);
NO_DEBUG v3  v3_cross(v3 a, v3 b);
NO_DEBUG v3  v3_lerp(v3 a, f32 t, v3 b);
NO_DEBUG v3  v3_hadamard_div(v3 a, v3 b);
NO_DEBUG v3  v3_greater(v3 a, v3 b);
NO_DEBUG v3  v3_less(v3 a, v3 b);
NO_DEBUG v3 v3_pos_of_mat4(mat4 mat);
// NO_DEBUG v3 v3_rot_of_mat4(mat4 mat);
// NO_DEBUG v3 v3_scale_of_mat4(mat4 mat);
NO_DEBUG v3 v3_rand_range(v3 a, v3 b);

////////////////////////////////////////////////////////////////////////
// Vector4

NO_DEBUG v4  v4_zero();
NO_DEBUG v4  v4_one();
NO_DEBUG v4  v4_scale(f32 a);
NO_DEBUG v4  operator+(v4 a, v4 b);
NO_DEBUG v4  operator-(v4 a, v4 b);
NO_DEBUG v4  operator*(v4 a, f32 scalar);
NO_DEBUG v4  operator*(f32 scalar, v4 a);
NO_DEBUG v4  operator/(v4 a, f32 scalar);
NO_DEBUG v4  operator+=(v4& a, v4 b);
NO_DEBUG v4  operator-=(v4& a, v4 b);
NO_DEBUG v4  operator*=(v4& a, f32 scalar);
NO_DEBUG v4  operator/=(v4& a, f32 scalar);
NO_DEBUG v4  operator-(v4 a);
NO_DEBUG f32 v4_length_squared(v4 a);
NO_DEBUG f32 v4_length(v4 a);
NO_DEBUG v4  v4_normalize(v4 a);
NO_DEBUG v4  v4_hadamard(v4 a, v4 b);

////////////////////////////////////////////////////////////////////////
// Matrix3

NO_DEBUG mat3 mat3_identity();
NO_DEBUG mat3 mat3_translate(v2 pos);
NO_DEBUG mat3 mat3_scale(v2 scale);
NO_DEBUG mat3 operator*(mat3 a, mat3 b);
NO_DEBUG mat3& operator*=(mat3& a, mat3 b);
NO_DEBUG v3 operator*(mat3 mat, v3 vec);

////////////////////////////////////////////////////////////////////////
// Matrix4

mat4 mat4_identity();
mat4 mat4_translate(v3 pos);
mat4 mat4_scale(v3 scale);
mat4 mat4_rotate_x(f32 angle_radians);
mat4 mat4_rotate_y(f32 angle_radians);
mat4 mat4_rotate_z(f32 angle_radians);
mat4 operator*(mat4 a, mat4 b);
mat4& operator*=(mat4& a, mat4 b);
v4 operator*(mat4 mat, v4 vec);
mat4 mat4_rotate_xyz(v3 rot);
mat4 mat4_transform(v3 pos, v3 rot, v3 scale);
mat4 mat4_transform(Transform trans);
mat4 mat4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
mat4 mat4_perspective(f32 fov_radians, f32 aspect_ratio, f32 near, f32 far);
mat4 mat4_look_at(v3 pos, v3 dir, v3 up);
mat4 mat4_transpose(mat4 matrix);
mat4 mat4_inverse(mat4 matrix);
v3 mat4_forward(mat4 matrix);
v3 mat4_backward(mat4 matrix);
v3 mat4_up(mat4 matrix);
v3 mat4_down(mat4 matrix);
v3 mat4_right(mat4 matrix);
v3 mat4_left(mat4 matrix);

////////////////////////////////////////////////////////////////////////
// Range Ops

///////////////////////////////////
// Dim 1
Rng1u32 shift_1u32(Rng1u32 r, u32 x);
Rng1u32 pad_1u32(Rng1u32 r, u32 x);
u32 center_1u32(Rng1u32 r);
b32 contains_1u32(Rng1u32 r, u32 x);
u32 dim_1u32(Rng1u32 r);
Rng1u32 union_1u32(Rng1u32 a, Rng1u32 b);
Rng1u32 intersect_1u32(Rng1u32 a, Rng1u32 b);
u32 clamp_1u32(Rng1u32 r, u32 v);

Rng1i32 shift_1i32(Rng1i32 r, i32 x);
Rng1i32 pad_1i32(Rng1i32 r, i32 x);
i32 center_1i32(Rng1i32 r);
b32 contains_1i32(Rng1i32 r, i32 x);
i32 dim_1i32(Rng1i32 r);
Rng1i32 union_1i32(Rng1i32 a, Rng1i32 b);
Rng1i32 intersect_1i32(Rng1i32 a, Rng1i32 b);
i32 clamp_1i32(Rng1i32 r, i32 v);

Rng1u64 shift_1u64(Rng1u64 r, u64 x);
Rng1u64 pad_1u64(Rng1u64 r, u64 x);
u64 center_1u64(Rng1u64 r);
b32 contains_1u64(Rng1u64 r, u64 x);
u64 dim_1u64(Rng1u64 r);
Rng1u64 union_1u64(Rng1u64 a, Rng1u64 b);
Rng1u64 intersect_1u64(Rng1u64 a, Rng1u64 b);
u64 clamp_1u64(Rng1u64 r, u64 v);

Rng1f32 shift_1f32(Rng1f32 r, f32 x);
Rng1f32 pad_1f32(Rng1f32 r, f32 x);
f32 center_1f32(Rng1f32 r);
b32 contains_1f32(Rng1f32 r, f32 x);
f32 dim_1f32(Rng1f32 r);
Rng1f32 union_1f32(Rng1f32 a, Rng1f32 b);
Rng1f32 intersect_1f32(Rng1f32 a, Rng1f32 b);
f32 clamp_1f32(Rng1f32 r, f32 v);

///////////////////////////////////
// Dim 2
Rng2f32 shift_2f32(Rng2f32 r, v2 x);
Rng2f32 pad_2f32(Rng2f32 r, f32 x);
v2 center_2f32(Rng2f32 r);
b32 contains_2f32(Rng2f32 r, v2 x);
v2 dim_2f32(Rng2f32 r);
Rng2f32 union_2f32(Rng2f32 a, Rng2f32 b);
Rng2f32 intersect_2f32(Rng2f32 a, Rng2f32 b);
v2 clamp_2f32(Rng2f32 r, v2 v);
Rng2f32 center_size_2f32(Rng2f32 r, v2 x);

///////////////////////////////////
// Dim3
Rng3f32 shift_3f32(Rng3f32 r, v3 x);
Rng3f32 pad_3f32(Rng3f32 r, f32 x);
v3 center_3f32(Rng3f32 r);
b32 contains_3f32(Rng3f32 r, v3 x);
v3 dim_3f32(Rng3f32 r);
Rng3f32 union_3f32(Rng3f32 a, Rng3f32 b);
Rng3f32 intersect_3f32(Rng3f32 a, Rng3f32 b);
v3 clamp_3f32(Rng3f32 r, v3 v);

