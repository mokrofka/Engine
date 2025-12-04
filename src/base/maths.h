#pragma once
#include "defines.h"
#include "str.h"

#define PI             3.14159265358f
#define Tau            (PI * 2)

#define EulerNumber  2.71828182846f
#define GoldBig      1.61803398875f
#define GoldSmall    0.61803398875f

#define FloatEpsilon   1e-5f
#define MachineEpsilon 1.1920929e-7f

inline f32 degtorad(f32 degrees) { return degrees * PI / 180.0f; }
inline f32 radtodeg(f32 radians) { return radians * 180.0f / PI; }

struct v2 {
  f32 x;
  f32 y;

  v2 () = default;
  INLINE v2 (f32 x_, f32 y_) { x = x_, y = y_; }
  INLINE v2 (f32 scale) { x = scale; y = scale; }
};

struct v2i {
  i32 x;
  i32 y;

  v2i () = default;
  INLINE v2i (i32 x_, i32 y_) { x = x_, y = y_; }
  INLINE v2i (i32 scale) { x = scale; y = scale; }
};

struct v2u {
  u32 x;
  u32 y;

  v2u () = default;
  INLINE v2u (u32 x_, u32 y_) { x = x_, y = y_; }
  INLINE v2u (u32 scale) { x = scale; y = scale; }
};

union v3 {
  struct {
    f32 x;
    f32 y;
    f32 z;
  };
  f32 e[3];

  v3 () = default;
  INLINE v3 (f32 x_, f32 y_, f32 z_) { x = x_, y = y_, z = z_; }
  INLINE v3 (f32 scale) { x = scale; y = scale; z = scale; }
};

union v3u {
  struct {
    u32 x;
    u32 y;
    u32 z;
  };
  u32 e[3];

  v3u () = default;
  INLINE v3u (u32 x_, u32 y_, u32 z_) { x = x_, y = y_, z = z_; }
  INLINE v3u (u32 scale) { x = scale; y = scale; z = scale; }
};

struct v4 {
  f32 x;
  f32 y;
  f32 z;
  f32 w;

  v4 () = default;
  INLINE v4 (f32 x_, f32 y_, f32 z_, f32 w_) { x = x_, y = y_, z = z_, w = w_; }
  INLINE v4 (f32 scale) { x = scale; y = scale; z = scale; w = scale; }
};

union mat3 {
  f32 e[9];
  INLINE f32& operator[](u32 a) { return e[a]; }
};

typedef v4 quat;

union mat4 {
  f32 e[16];
  INLINE f32& operator[](u32 a) { return e[a]; }
};

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

INLINE f32 Sin(f32 a)           { return __builtin_sinf(a); }
INLINE f32 Cos(f32 a)           { return __builtin_cosf(a); }
INLINE f32 Tan(f32 a)           { return __builtin_tanf(a); }
INLINE f32 Asin(f32 a)          { return __builtin_asinf(a); }
INLINE f32 Acos(f32 a)          { return __builtin_acosf(a); }
INLINE f32 Atan2(f32 y, f32 x)  { return __builtin_atan2f(y,x); }
INLINE f32 Sqrt(f32 a)          { return __builtin_sqrtf(a); }
INLINE f32 Pow(f32 a, f32 b)    { return __builtin_powf(a, b); }
INLINE f32 Floor(f32 a)         { return __builtin_floorf(a); }
INLINE f32 Ceil(f32 a)          { return __builtin_ceilf(a); }
INLINE f32 Round(f32 a)         { return __builtin_roundf(a); }
INLINE f32 Mod(f32 a, f32 b)    { return __builtin_fmodf(a, b); }
INLINE f32 Exp(f32 a)           { return __builtin_expf(a); }
INLINE f32 LogE(f32 a)          { return __builtin_logf(a); }
INLINE f32 Log2(f32 a)          { return __builtin_log2f(a); }
INLINE f32 Log10(f32 a)         { return __builtin_log10f(a); }

NO_DEBUG inline f32 SinD(f32 a)                { return Sin(degtorad(a)); }
NO_DEBUG inline f32 CosD(f32 a)                { return Cos(degtorad(a)); }
NO_DEBUG inline f32 Lerp(f32 a, f32 t, f32 b)  { return (1 - t)*a + t*b; }
NO_DEBUG inline f32 Atan2_360(f32 y, f32 x)    { return Atan2(-y, -x) + PI; }

inline u32 next_pow2(u32 v) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}
inline u32 prev_pow2(u32 n) {
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	return n - (n >> 1);
}

////////////////////////////////////////////////////////////////////////
// Color

inline v4 rgba_from_u32(u32 hex) {
  v4 result = v4(((hex & 0xff000000) >> 24) / 255.f,
                 ((hex & 0x00ff0000) >> 16) / 255.f,
                 ((hex & 0x0000ff00) >> 8)  / 255.f,
                 ((hex & 0x000000ff) >> 0)  / 255.f);
  return result;
}
inline u32 u32_from_rgba(v4 rgba) {
  u32 result = 0;
  result |= ((u32)((u8)(rgba.x*255.f))) << 24;
  result |= ((u32)((u8)(rgba.y*255.f))) << 16;
  result |= ((u32)((u8)(rgba.z*255.f))) <<  8;
  result |= ((u32)((u8)(rgba.w*255.f))) <<  0;
  return result;
}
inline u32 u32_from_argb(v4 argb) {
  u32 result = 0;
  result |= ((u32)((u8)(argb.w*255.f))) << 24;
  result |= ((u32)((u8)(argb.x*255.f))) << 16;
  result |= ((u32)((u8)(argb.y*255.f))) << 8;
  result |= ((u32)((u8)(argb.z*255.f))) << 0;
  return result;
}

////////////////////////////////////////////////////////////////////////
// Hash

inline u64 squirrel3(u64 at) {
  constexpr u64 BIT_NOISE1 = 0x9E3779B185EBCA87ULL;
  constexpr u64 BIT_NOISE2 = 0xC2B2AE3D27D4EB4FULL;
  constexpr u64 BIT_NOISE3 = 0x27D4EB2F165667C5ULL;
  at *= BIT_NOISE1;
  at ^= (at >> 8);
  at += BIT_NOISE2;
  at ^= (at << 8);
  at *= BIT_NOISE3;
  at ^= (at >> 8);
  return at;
}
inline u64 hash(u64 x) {
  return squirrel3(x);
}
inline u64 str_hash_FNV(String str) {
  u32 hash = 0x811c9dc5;
  Loop (i, str.size) {
    hash = (*str.str++ ^ hash) * 0x01000193;
  }
  return hash;
}
inline u64 hash(String str) {
  return str_hash_FNV(str);
}

////////////////////////////////////////////////////////////////////////
// Random

inline u32 xorshift32(u32* seed) {
  u32 x = *seed;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return *seed = x;
}

KAPI extern u32 _seed;
NO_DEBUG inline u32 rand_u32()                        { return xorshift32(&_seed); }
NO_DEBUG inline u32 rand_range_u32(u32 min, u32 max)  { return (rand_u32() % (max - min + 1)) + min; }
NO_DEBUG inline i32 rand_i32()                        { return rand_u32(); }
NO_DEBUG inline i32 rand_range_i32(i32 min, i32 max)  { return (i32)(rand_u32() % (u32)(max - min + 1)) + min; }
NO_DEBUG inline f32 rand_f32_01()                     { return rand_u32() / (f32)U32_MAX; }
NO_DEBUG inline f32 rand_f32_11()                     { return rand_f32_01() * 2.f - 1.f; }
NO_DEBUG inline f32 rand_f32()                        { return rand_f32_01() * 2 * U16_MAX - U16_MAX; }
NO_DEBUG inline f32 rand_range_f32(f32 min, f32 max)  { return rand_f32_01() * (max - min) + min ; }
NO_DEBUG inline b32 rand_b32()                        { return rand_u32()%2; }
NO_DEBUG inline void rand_seed()                      { _seed = CpuTimerNow(); }

////////////////////////////////////////////////////////////////////////
// Vector2

NO_DEBUG inline v2 v2_zero()                     { return v2{}; }
NO_DEBUG inline v2 v2_one()                      { return v2(1.0f, 1.0f); }
NO_DEBUG inline v2 v2_up()                       { return v2(0.0f, 1.0f); }
NO_DEBUG inline v2 v2_down()                     { return v2(0.0f, -1.0f); }
NO_DEBUG inline v2 v2_left()                     { return v2(-1.0f, 0.0f); }
NO_DEBUG inline v2 v2_right()                    { return v2(1.0f, 0.0f); }
NO_DEBUG inline v2 v2_of_v3(v3 a)                { return v2(a.x, a.y); }
NO_DEBUG inline v2 v2_of_v4(v4 a)                { return v2(a.x, a.y); }
NO_DEBUG inline v3 v2_to_v3(v2 a, f32 b)         { return v3(a.x, a.y, b); }
NO_DEBUG inline v4 v2_to_v4(v2 a, f32 b, f32 c)  { return v4(a.x, a.y, b, c); }
NO_DEBUG inline v2 v2_of_v2i(v2i a)              { return v2(a.x, a.y); }
NO_DEBUG inline v2i v2i_of_v2(v2 a)              { return v2i(a.x, a.y); }

NO_DEBUG inline v2  operator+(v2 a, v2 b)          { return v2(a.x + b.x, a.y + b.y); }
NO_DEBUG inline v2  operator-(v2 a, v2 b)          { return v2(a.x - b.x, a.y - b.y); }
NO_DEBUG inline v2  operator*(v2 a, f32 scalar)    { return v2(a.x*scalar, a.y*scalar); }
NO_DEBUG inline v2  operator*(f32 scalar, v2 a)    { return v2(a.x*scalar, a.y*scalar); }
NO_DEBUG inline v2  operator/(v2 a, f32 scalar)    { return v2(a.x/scalar, a.y/scalar); }
NO_DEBUG inline v2  operator+=(v2& a, v2 b)        { return a = a + b; }
NO_DEBUG inline v2  operator-=(v2& a, v2 b)        { return a = a - b; }
NO_DEBUG inline v2  operator*=(v2& a, f32 scalar)  { return a = a*scalar; }
NO_DEBUG inline v2  operator/=(v2& a, f32 scalar)  { return a = a/scalar; }
NO_DEBUG inline b32 operator==(v2 a, v2 b)         { return (Abs(a.x - b.x) <= FloatEpsilon) && (Abs(a.y - b.y) <= FloatEpsilon); }
NO_DEBUG inline b32 operator!=(v2 a, v2 b)         { return !(a == b); }
NO_DEBUG inline v2  operator-(v2 a)                { return v2(-a.x, -a.y); }

NO_DEBUG inline f32 v2_length_squared(v2 a)    { return Sqr(a.x) + Sqr(a.y); }
NO_DEBUG inline f32 v2_length(v2 a)            { return Sqrt(v2_length_squared(a)); }
NO_DEBUG inline v2  v2_norm(v2 a)              { return a * (1.0f/v2_length(a)); }
NO_DEBUG inline f32 v2_distance(v2 a, v2 b)    { return v2_length(v2(a.x - b.x, a.y - b.y)); }
NO_DEBUG inline f32 v2_dot(v2 a, v2 b)         { return a.x*b.x + a.y*b.y; }
NO_DEBUG inline f32 v2_cross(v2 a, v2 b)       { return a.x*b.y - a.y*b.x; } // if > 0 (b is to the left of a), if < 0 (b is to the right of a), if == 0 (collinear)
NO_DEBUG inline v2  v2_lerp(v2 a, f32 t, v2 b) { return v2(Lerp(a.x, t, b.x), Lerp(a.y, t, b.y));}
NO_DEBUG inline v2  v2_skew(v2 a)              { return v2(-a.y, a.x); }

NO_DEBUG inline f32 v2_shortest_arc(v2 a, v2 b) {
	a = v2_norm(a);
	b = v2_norm(b);
	f32 c = v2_dot(a, b);
	f32 s = v2_cross(a, b);
	f32 theta = Acos(c);
	if (s > 0)
		return theta;
	else
		return -theta;
}

////////////////////////////////////////////////////////////////////////
// Vector3

NO_DEBUG inline v3 v3_zero()              { return v3{}; }
NO_DEBUG inline v3 v3_one()               { return v3(1, 1, 1); }
NO_DEBUG inline v3 v3_up()                { return v3(0.0f, 1.0f, 0.0f); }
NO_DEBUG inline v3 v3_down()              { return v3(0.0f, -1.0f, 0.0f); }
NO_DEBUG inline v3 v3_left()              { return v3(-1.0f, 0.0f, 0.0f); }
NO_DEBUG inline v3 v3_right()             { return v3(1.0f, 0.0f, 0.0f); }
NO_DEBUG inline v3 v3_forward()           { return v3(0.0f, 0.0f, -1.0f); }
NO_DEBUG inline v3 v3_back()              { return v3(0.0f, 0.0f, 1.0f); }
NO_DEBUG inline v3 v3_of_v4(v4 a)         { return v3(a.x, a.y, a.z); }
NO_DEBUG inline v4 v3_to_v4(v3 a, f32 b)  { return v4(a.x, a.y, a.z, b); }

NO_DEBUG inline v3  operator+(v3 a, v3 b)          { return v3(a.x + b.x, a.y + b.y, a.z + b.z); }
NO_DEBUG inline v3  operator-(v3 a, v3 b)          { return v3(a.x - b.x, a.y - b.y, a.z - b.z); }
NO_DEBUG inline v3  operator*(v3 a, f32 scalar)    { return v3(a.x*scalar, a.y*scalar, a.z*scalar); }
NO_DEBUG inline v3  operator*(f32 scalar, v3 a)    { return v3(a.x*scalar, a.y*scalar, a.z*scalar); }
NO_DEBUG inline v3  operator/(v3 a, f32 scalar)    { return v3(a.x/scalar, a.y/scalar, a.z/scalar); }
NO_DEBUG inline v3  operator+=(v3& a, v3 b)        { return a = a + b; }
NO_DEBUG inline v3  operator-=(v3& a, v3 b)        { return a = a - b; }
NO_DEBUG inline v3  operator*=(v3& a, f32 scalar)  { return a = a * scalar; }
NO_DEBUG inline v3  operator/=(v3& a, f32 scalar)  { return a = a / scalar; }
NO_DEBUG inline b32 operator==(v3 a, v3 b)         { return (Abs(a.x - b.x) <= FloatEpsilon) && (Abs(a.y - b.y) <= FloatEpsilon) && (Abs(a.z - b.z) <= FloatEpsilon); }
NO_DEBUG inline b32 operator==(v3u a, v3u b)       { return a.x == b.x && a.y == b.y && a.z == b.z; }
NO_DEBUG inline b32 operator!=(v3 a, v3 b)         { return !(a == b); }
NO_DEBUG inline v3  operator-(v3 a)                { return v3(-a.x, -a.y, -a.z); }

NO_DEBUG inline f32 v3_length_squared(v3 a)    { return Sqr(a.x) + Sqr(a.y) + Sqr(a.z); }
NO_DEBUG inline f32 v3_length(v3 a)            { return Sqrt(v3_length_squared(a)); }
NO_DEBUG inline v3  v3_norm(v3 a)              { return a * (1.0f/v3_length(a)); }
NO_DEBUG inline f32 v3_distance(v3 a, v3 b)    { return v3_length(v3(a.x - b.x, a.y - b.y, a.z - b.z)); }
NO_DEBUG inline f32 v3_dot(v3 a, v3 b)         { return a.x*b.x + a.y*b.y + a.z*b.z; }
NO_DEBUG inline v3  v3_cross(v3 a, v3 b)       { return v3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }
NO_DEBUG inline v3  v3_lerp(v3 a, f32 t, v3 b) { return v3(Lerp(a.x, t, b.x), Lerp(a.y, t, b.y), Lerp(a.z, t, b.z));}

NO_DEBUG inline v3 v3_pos_of_mat4(mat4 mat) {
  v3 vec = {
    mat[9],
    mat[10],
    mat[11],
  };
  return vec;
};
NO_DEBUG inline v3 v3_rot_of_mat4(mat4 mat) {
  v3 vec = {
    mat[9],
    mat[10],
    mat[11],
  };
  return vec;
};
NO_DEBUG inline v3 v3_scale_of_mat4(mat4 mat) {
  v3 vec = {
    mat[9],
    mat[10],
    mat[11],
  };
  return vec;
};

NO_DEBUG inline v3 v3_rand_range(v3 a, v3 b) {
  v3 vec = {
    rand_range_f32(a.x, b.x),
    rand_range_f32(a.y, b.y),
    rand_range_f32(a.z, b.z),
  };
  return vec;
}

////////////////////////////////////////////////////////////////////////
// Vector4

NO_DEBUG inline v4  operator+(v4 a, v4 b)          { return v4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
NO_DEBUG inline v4  operator-(v4 a, v4 b)          { return v4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
NO_DEBUG inline v4  operator*(v4 a, f32 scalar)    { return v4(a.x*scalar, a.y*scalar, a.z*scalar, a.w*scalar); }
NO_DEBUG inline v4  operator*(f32 scalar, v4 a)    { return v4(a.x*scalar, a.y*scalar, a.z*scalar, a.w*scalar); }
NO_DEBUG inline v4  operator/(v4 a, f32 scalar)    { return v4(a.x/scalar, a.y/scalar, a.z/scalar, a.w/scalar); }
NO_DEBUG inline v4  operator+=(v4& a, v4 b)        { return a = a + b; }
NO_DEBUG inline v4  operator-=(v4& a, v4 b)        { return a = a - b; }
NO_DEBUG inline v4  operator*=(v4& a, f32 scalar)  { return a = a * scalar; }
NO_DEBUG inline v4  operator/=(v4& a, f32 scalar)  { return a = a / scalar; }
NO_DEBUG inline v4  operator-(v4 a)                { return v4(-a.x, -a.y, -a.z, -a.w); }

NO_DEBUG inline f32 v4_length_squared(v4 a)  { return Sqr(a.x) + Sqr(a.y) + Sqr(a.z) + Sqr(a.w); }
NO_DEBUG inline f32 v4_length(v4 a)          { return Sqrt(v4_length_squared(a)); }
NO_DEBUG inline v4  v4_normalize(v4 a)       { return a * (1.0f / v4_length(a)); }

////////////////////////////////////////////////////////////////////////
// Matrix3

NO_DEBUG inline mat3 mat3_identity() {
  mat3 result = {
    1,0,0,
    0,1,0,
    0,0,1
  };
  return result;
}

NO_DEBUG inline mat3 mat3_translate(v2 pos) {
  mat3 result = {
    1,     0,     0,
    0,     1,     0,
    pos.x, pos.y, 1
  };
  return result;
}

NO_DEBUG inline mat3 mat3_scale(v2 scale) {
  mat3 result = {
    scale.x, 0,       0,
    0,       scale.y, 0,
    0,       0,       1
  };
  return result;
}

NO_DEBUG inline mat3 operator*(mat3 a, mat3 b) {
  mat3 c = {};
  Loop (row, 3) {
    Loop (col, 3) {
      c[row*3 + col] = b[row*3 + 0] * a[0*3 + col] +
                       b[row*3 + 1] * a[1*3 + col] +
                       b[row*3 + 2] * a[2*3 + col];
    }
  }
  return c;
}
NO_DEBUG inline mat3& operator*=(mat3& a, mat3 b) { return a = b * a; }

NO_DEBUG inline v3 operator*(mat3 mat, v3 vec) {
  v3 result = {
    mat.e[0]*vec.x + mat.e[1]*vec.y + mat.e[2]*vec.z,
    mat.e[3]*vec.x + mat.e[4]*vec.y + mat.e[5]*vec.z,
    mat.e[6]*vec.x + mat.e[7]*vec.y + mat.e[8]*vec.z,
  };
  return result;
}

////////////////////////////////////////////////////////////////////////
// Matrix4

NO_DEBUG inline mat4 mat4_identity() {
  mat4 result = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
  };
  return result;
}

NO_DEBUG inline mat4 mat4_translate(v3 pos) {
  mat4 result = {
    1,     0,     0,     0,
    0,     1,     0,     0,
    0,     0,     1,     0,
    pos.x, pos.y, pos.z, 1
  };
  return result;
}

NO_DEBUG inline mat4 mat4_scale(v3 scale) {
  mat4 mat = {
    scale.x, 0,       0,       0,
    0,       scale.y, 0,       0,
    0,       0,       scale.z, 0,
    0,       0,       0,       1};
  return mat;
}

NO_DEBUG inline mat4 mat4_rotate_x(f32 angle_radians) {
  f32 cos = Cos(angle_radians);
  f32 sin = Sin(angle_radians);
  mat4 mat = {
    1,   0,    0,   0,
    0, cos,  sin,   0,
    0,-sin,  cos,   0,
    0,   0,    0,   1,
  };
  return mat;
}

NO_DEBUG inline mat4 mat4_rotate_y(f32 angle_radians) {
  f32 cos = Cos(angle_radians);
  f32 sin = Sin(angle_radians);
  mat4 mat = {
    cos, 0,  -sin, 0,
    0,   1,   0,   0,
    sin, 0,   cos, 0,
    0,   0,   0,   1
  };
  return mat;
}

NO_DEBUG inline mat4 mat4_rotate_z(f32 angle_radians) {
  f32 cos = Cos(angle_radians);
  f32 sin = Sin(angle_radians);
  mat4 mat = {
    cos, sin, 0,   0,
   -sin, cos, 0,   0,
    0,   0,   1,   0,
    0,   0,   0,   1
  };
  return mat;
}

NO_DEBUG inline mat4 operator*(mat4 a, mat4 b) {
  mat4 c = {};
  Loop (row, 4) {
    Loop (col, 4) {
      c[row*4 + col] = b[row*4 + 0] * a[0*4 + col] +
                       b[row*4 + 1] * a[1*4 + col] +
                       b[row*4 + 2] * a[2*4 + col] +
                       b[row*4 + 3] * a[3*4 + col];
    }
  }
  return c;
}
NO_DEBUG inline mat4& operator*=(mat4& a, mat4 b) { return a = b * a; }

NO_DEBUG inline v4 operator*(mat4 mat, v4 vec) {
  v4 result = {
    mat.e[0 ]*vec.x + mat.e[1 ]*vec.y + mat.e[2 ]*vec.z + mat.e[3 ]*vec.w,
    mat.e[4 ]*vec.x + mat.e[5 ]*vec.y + mat.e[6 ]*vec.z + mat.e[7 ]*vec.w,
    mat.e[8 ]*vec.x + mat.e[9 ]*vec.y + mat.e[10]*vec.z + mat.e[11]*vec.w,
    mat.e[12]*vec.x + mat.e[13]*vec.y + mat.e[14]*vec.z + mat.e[15]*vec.w,
  };
  return result;
}

NO_DEBUG inline mat4 mat4_rotate_xyz(v3 rot) {
  mat4 rx = mat4_rotate_x(rot.x);
  mat4 ry = mat4_rotate_y(rot.y);
  mat4 rz = mat4_rotate_z(rot.z);
  mat4 mat = rx * ry * rz;
  return mat;
}

NO_DEBUG inline mat4 mat4_transform(v3 pos, v3 rot, v3 scale) {
  mat4 result = mat4_translate(pos) * mat4_rotate_xyz(rot) * mat4_scale(scale);
  return result;
}
NO_DEBUG inline mat4 mat4_transform(Transform trans) {
  mat4 result = mat4_translate(trans.pos) * mat4_rotate_xyz(trans.rot) * mat4_scale(trans.scale);
  return result;
}

NO_DEBUG inline mat4 mat4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) {
  mat4 result = {
    2/(right - left), 0,                0,                  0,
    0,                2/(top - bottom), 0,                  0,
    0,                0,                1/(far - near),     0,
    0,                0,                -near/(far - near), 1
  };
  return result;
}

NO_DEBUG inline mat4 mat4_perspective(f32 fov_radians, f32 aspect_ratio, f32 Near, f32 Far) {
  f32 fov = Tan(fov_radians/2.0f);
  mat4 result = {
    1/(fov*aspect_ratio),                   0,                       0,                    0,
    0,                                      1/fov,                   0,                    0,
    0,                                      0,                       Far/(Far-Near),       1,
    0,                                      0,                       -Far*Near/(Far-Near), 0
  };

  // NOTE: This flips z value
  // mat4 result = {
  //   1/(fov*aspect_ratio), 0,                0,                             0,
  //   0,                    1/(fov),          0,                             0,
  //   0,                    0,                -(Far + Near)/(Far - Near),   -1,
  //   0,                    0,                (-2.0f*Far*Near)/(Far - Near), 0
  // };

  return result;
}

NO_DEBUG inline mat4 mat4_look_at(v3 pos, v3 dir, v3 up) {
  v3 z = v3_norm(dir);
  v3 x = v3_norm(v3_cross(up, z));
  v3 y = v3_cross(z, x);
  mat4 camera_view = {
    x.x, y.x, z.x, 0,
    x.y, y.y, z.y, 0,
    x.z, y.z, z.z, 0,
   -v3_dot(x, pos), -v3_dot(y, pos), -v3_dot(z, pos), 1
  };
  return camera_view;
}

NO_DEBUG inline mat4 mat4_transpose(mat4 matrix) {
  mat4 out_matrix = mat4_identity();
  out_matrix.e[0] = matrix.e[0];
  out_matrix.e[1] = matrix.e[4];
  out_matrix.e[2] = matrix.e[8];
  out_matrix.e[3] = matrix.e[12];
  out_matrix.e[4] = matrix.e[1];
  out_matrix.e[5] = matrix.e[5];
  out_matrix.e[6] = matrix.e[9];
  out_matrix.e[7] = matrix.e[13];
  out_matrix.e[8] = matrix.e[2];
  out_matrix.e[9] = matrix.e[6];
  out_matrix.e[10] = matrix.e[10];
  out_matrix.e[11] = matrix.e[14];
  out_matrix.e[12] = matrix.e[3];
  out_matrix.e[13] = matrix.e[7];
  out_matrix.e[14] = matrix.e[11];
  out_matrix.e[15] = matrix.e[15];
  return out_matrix;
}

NO_DEBUG inline mat4 mat4_inverse(mat4 matrix) {
  const f32* m = matrix.e;

  f32 t0 = m[10] * m[15];
  f32 t1 = m[14] * m[11];
  f32 t2 = m[6] * m[15];
  f32 t3 = m[14] * m[7];
  f32 t4 = m[6] * m[11];
  f32 t5 = m[10] * m[7];
  f32 t6 = m[2] * m[15];
  f32 t7 = m[14] * m[3];
  f32 t8 = m[2] * m[11];
  f32 t9 = m[10] * m[3];
  f32 t10 = m[2] * m[7];
  f32 t11 = m[6] * m[3];
  f32 t12 = m[8] * m[13];
  f32 t13 = m[12] * m[9];
  f32 t14 = m[4] * m[13];
  f32 t15 = m[12] * m[5];
  f32 t16 = m[4] * m[9];
  f32 t17 = m[8] * m[5];
  f32 t18 = m[0] * m[13];
  f32 t19 = m[12] * m[1];
  f32 t20 = m[0] * m[9];
  f32 t21 = m[8] * m[1];
  f32 t22 = m[0] * m[5];
  f32 t23 = m[4] * m[1];

  mat4 out_matrix;
  f32* o = out_matrix.e;

  o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) - (t1 * m[5] + t2 * m[9] + t5 * m[13]);
  o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) - (t0 * m[1] + t7 * m[9] + t8 * m[13]);
  o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) - (t3 * m[1] + t6 * m[5] + t11 * m[13]);
  o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) - (t4 * m[1] + t9 * m[5] + t10 * m[9]);

  f32 d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);

  o[0] = d * o[0];
  o[1] = d * o[1];
  o[2] = d * o[2];
  o[3] = d * o[3];
  o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) - (t0 * m[4] + t3 * m[8] + t4 * m[12]));
  o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) - (t1 * m[0] + t6 * m[8] + t9 * m[12]));
  o[6] = d * ((t3 * m[0] + t6 * m[4] + t11 * m[12]) - (t2 * m[0] + t7 * m[4] + t10 * m[12]));
  o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) - (t5 * m[0] + t8 * m[4] + t11 * m[8]));
  o[8] = d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) - (t13 * m[7] + t14 * m[11] + t17 * m[15]));
  o[9] = d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) - (t12 * m[3] + t19 * m[11] + t20 * m[15]));
  o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) - (t15 * m[3] + t18 * m[7] + t23 * m[15]));
  o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) - (t16 * m[3] + t21 * m[7] + t22 * m[11]));
  o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) - (t16 * m[14] + t12 * m[6] + t15 * m[10]));
  o[13] = d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) - (t18 * m[10] + t21 * m[14] + t13 * m[2]));
  o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) - (t22 * m[14] + t14 * m[2] + t19 * m[6]));
  o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) - (t20 * m[6] + t23 * m[10] + t17 * m[2]));

  return out_matrix;
}

NO_DEBUG inline v3 mat4_forward(mat4 matrix) {
  v3 forward = {
    matrix.e[2],
    matrix.e[6],
    matrix.e[10],
  };
  forward = v3_norm(forward);
  return forward;
}

NO_DEBUG inline v3 mat4_backward(mat4 matrix) {
  v3 forward = {
    -matrix.e[2],
    -matrix.e[6],
    -matrix.e[10],
  };
  forward = v3_norm(forward);
  return forward;
}

NO_DEBUG inline v3 mat4_up(mat4 matrix) {
  v3 up = {
    matrix.e[1],
    matrix.e[5],
    matrix.e[9],
  };
  up = v3_norm(up);
  return up;
}

NO_DEBUG inline v3 mat4_down(mat4 matrix) {
  v3 down = {
    -matrix.e[1],
    -matrix.e[5],
    -matrix.e[9],
  };
  down = v3_norm(down);
  return down;
}

NO_DEBUG inline v3 mat4_left(mat4 matrix) {
  v3 right = {
    -matrix.e[0],
    -matrix.e[4],
    -matrix.e[8],
  };
  right = v3_norm(right);
  return right;
}

NO_DEBUG inline v3 mat4_right(mat4 matrix) {
  v3 left = {
    matrix.e[0],
    matrix.e[4],
    matrix.e[8],
  };
  left = v3_norm(left);
  return left;
}

////////////////////////////////////////////////////////////////////////
// Quaternions

NO_DEBUG inline quat quat_identity() {
  return quat{0, 0, 0, 1.0f};
}

NO_DEBUG inline f32 quat_normal(quat q) {
  return Sqrt(
      q.x * q.x +
      q.y * q.y +
      q.z * q.z +
      q.w * q.w);
}

NO_DEBUG inline quat quat_normalize(quat q) {
  f32 normal = quat_normal(q);
  return quat{
      q.x / normal,
      q.y / normal,
      q.z / normal,
      q.w / normal};
}

NO_DEBUG inline quat quat_conjugate(quat q) {
  return quat{
      -q.x,
      -q.y,
      -q.z,
      q.w};
}

NO_DEBUG inline quat quat_inverse(quat q) {
  return quat_normalize(quat_conjugate(q));
}

NO_DEBUG inline quat quat_mul(quat q_0, quat q_1) {
  quat out_quaternion;

  out_quaternion.x = q_0.x * q_1.w +
                     q_0.y * q_1.z -
                     q_0.z * q_1.y +
                     q_0.w * q_1.x;

  out_quaternion.y = -q_0.x * q_1.z +
                     q_0.y * q_1.w +
                     q_0.z * q_1.x +
                     q_0.w * q_1.y;

  out_quaternion.z = q_0.x * q_1.y -
                     q_0.y * q_1.x +
                     q_0.z * q_1.w +
                     q_0.w * q_1.z;

  out_quaternion.w = -q_0.x * q_1.x -
                     q_0.y * q_1.y -
                     q_0.z * q_1.z +
                     q_0.w * q_1.w;

  return out_quaternion;
}

NO_DEBUG inline f32 quat_dot(quat q_0, quat q_1) {
  return q_0.x * q_1.x +
         q_0.y * q_1.y +
         q_0.z * q_1.z +
         q_0.w * q_1.w;
}

NO_DEBUG inline mat4 quat_to_mat4(quat q) {
  mat4 out_matrix = mat4_identity();

  // https://stackoverflow.com/questions/1556260/convert-quaternion-rotation-to-rotation-matrix

  quat n = quat_normalize(q);

  out_matrix.e[0] = 1.0f - 2.0f * n.y * n.y - 2.0f * n.z * n.z;
  out_matrix.e[1] = 2.0f * n.x * n.y - 2.0f * n.z * n.w;
  out_matrix.e[2] = 2.0f * n.x * n.z + 2.0f * n.y * n.w;

  out_matrix.e[4] = 2.0f * n.x * n.y + 2.0f * n.z * n.w;
  out_matrix.e[5] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.z * n.z;
  out_matrix.e[6] = 2.0f * n.y * n.z - 2.0f * n.x * n.w;

  out_matrix.e[8] = 2.0f * n.x * n.z - 2.0f * n.y * n.w;
  out_matrix.e[9] = 2.0f * n.y * n.z + 2.0f * n.x * n.w;
  out_matrix.e[10] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.y * n.y;

  return out_matrix;
}

// Calculates a rotation matrix based on the quaternion and the passed in center point.
NO_DEBUG inline mat4 quat_to_rotation_matrix(quat q, v3 center) {
  mat4 out_matrix;

  f32* o = out_matrix.e;
  o[0] = (q.x * q.x) - (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
  o[1] = 2.0f * ((q.x * q.y) + (q.z * q.w));
  o[2] = 2.0f * ((q.x * q.z) - (q.y * q.w));
  o[3] = center.x - center.x * o[0] - center.y * o[1] - center.z * o[2];

  o[4] = 2.0f * ((q.x * q.y) - (q.z * q.w));
  o[5] = -(q.x * q.x) + (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
  o[6] = 2.0f * ((q.y * q.z) + (q.x * q.w));
  o[7] = center.y - center.x * o[4] - center.y * o[5] - center.z * o[6];

  o[8] = 2.0f * ((q.x * q.z) + (q.y * q.w));
  o[9] = 2.0f * ((q.y * q.z) - (q.x * q.w));
  o[10] = -(q.x * q.x) - (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
  o[11] = center.z - center.x * o[8] - center.y * o[9] - center.z * o[10];

  o[12] = 0.0f;
  o[13] = 0.0f;
  o[14] = 0.0f;
  o[15] = 1.0f;
  return out_matrix;
}

NO_DEBUG inline quat quat_from_axis_angle(v3 axis, f32 angle, b32 normalize) {
  const f32 half_angle = 0.5f * angle;
  f32 s = Sin(half_angle);
  f32 c = Cos(half_angle);

  quat q = quat{s * axis.x, s * axis.y, s * axis.z, c};
  if (normalize) {
    return quat_normalize(q);
  }
  return q;
}

NO_DEBUG inline quat quat_slerp(quat q_0, quat q_1, f32 percentage) {
  quat out_quaternion;
  // Source: https://en.wikipedia.org/wiki/Slerp
  // Only unit quaternions are valid rotations.
  // Normalize to avoid undefined behavior.
  quat v0 = quat_normalize(q_0);
  quat v1 = quat_normalize(q_1);

  // Compute the cosine of the angle between the two vectors.
  f32 dot = quat_dot(v0, v1);

  // If the dot product is negative, slerp won't take
  // the shorter path. Note that v1 and -v1 are equivalent when
  // the negation is applied to all four components. Fix by
  // reversing one quaternion.
  if (dot < 0.0f) {
    v1.x = -v1.x;
    v1.y = -v1.y;
    v1.z = -v1.z;
    v1.w = -v1.w;
    dot = -dot;
  }

  const f32 DOT_THRESHOLD = 0.9995f;
  if (dot > DOT_THRESHOLD) {
    // If the inputs are too close for comfort, linearly interpolate
    // and normalize the result.
    out_quaternion = quat{
        v0.x + ((v1.x - v0.x) * percentage),
        v0.y + ((v1.y - v0.y) * percentage),
        v0.z + ((v1.z - v0.z) * percentage),
        v0.w + ((v1.w - v0.w) * percentage)};

    return quat_normalize(out_quaternion);
  }

  // Since dot is in range [0, DOT_THRESHOLD], acos is safe
  f32 theta_0 = Acos(dot);         // theta_0 = angle between input vectors
  f32 theta = theta_0 * percentage; // theta = angle between v0 and result
  f32 sin_theta = Sin(theta);      // compute this value only once
  f32 sin_theta_0 = Sin(theta_0);  // compute this value only once

  f32 s0 = Cos(theta) - dot * sin_theta / sin_theta_0; // == sin(theta_0 - theta) / sin(theta_0)
  f32 s1 = sin_theta / sin_theta_0;

  return quat{
      (v0.x * s0) + (v1.x * s1),
      (v0.y * s0) + (v1.y * s1),
      (v0.z * s0) + (v1.z * s1),
      (v0.w * s0) + (v1.w * s1)};
}
