#include "maths.h"

f32 degtorad(f32 degrees) { return degrees * PI / 180.0f; }
f32 radtodeg(f32 radians) { return radians * 180.0f / PI; }

v2::v2(f32 x_, f32 y_) { x = x_, y = y_; }
v2i::v2i(i32 x_, i32 y_) { x = x_, y = y_; }
v2u::v2u(u32 x_, u32 y_) { x = x_, y = y_; }
v3::v3(f32 x_, f32 y_, f32 z_) { x = x_, y = y_, z = z_; }
v3u::v3u(u32 x_, u32 y_, u32 z_) { x = x_, y = y_, z = z_; }
v4::v4(f32 x_, f32 y_, f32 z_, f32 w_) { x = x_, y = y_, z = z_, w = w_; }

f32& mat3::operator[](u32 a) { return v[a]; }

Rng1u32::Rng1u32(u32 min_, u32 max_) { min = min_; max = max_; }
Rng1i32::Rng1i32(i32 min_, i32 max_) { min = min_; max = max_; }
Rng1f32::Rng1f32(f32 min_, f32 max_) { min = min_; max = max_; }
Rng2f32::Rng2f32(v2 min_, v2 max_) { min = min_; max = max_; }
Rng3f32::Rng3f32(v3 min_, v3 max_) { min = min_; max = max_; }

f32 Sin(f32 a)                         { return __builtin_sinf(a); }
f32 Cos(f32 a)                         { return __builtin_cosf(a); }
f32 Tan(f32 a)                         { return __builtin_tanf(a); }
f32 Asin(f32 a)                        { return __builtin_asinf(a); }
f32 Acos(f32 a)                        { return __builtin_acosf(a); }
f32 Atan2(f32 y, f32 x)                { return __builtin_atan2f(y,x); }
f32 Sqrt(f32 a)                        { return __builtin_sqrtf(a); }
f32 Pow(f32 a, f32 b)                  { return __builtin_powf(a, b); }
f32 Floor(f32 a)                       { return __builtin_floorf(a); }
f32 Ceil(f32 a)                        { return __builtin_ceilf(a); }
f32 Round(f32 a)                       { return __builtin_roundf(a); }
f32 Mod(f32 a, f32 b)                  { return __builtin_fmodf(a, b); }
f32 Exp(f32 a)                         { return __builtin_expf(a); }
f32 LogE(f32 a)                        { return __builtin_logf(a); }
f32 Log2(f32 a)                        { return __builtin_log2f(a); }
f32 Log10(f32 a)                       { return __builtin_log10f(a); }
void SinCos(f32 angle, f32* a, f32* b) { __builtin_sincosf(angle, a, b); }

f32 SinD(f32 a)                { return Sin(degtorad(a)); }
f32 CosD(f32 a)                { return Cos(degtorad(a)); }
f32 Atan2_360(f32 y, f32 x)    { return Atan2(-y, -x) + PI; }

////////////////////////////////////////////////////////////////////////
// Color

v4 rgba_from_u32(u32 hex) {
  v4 result = v4(((hex & 0xff000000) >> 24) / 255.f,
                 ((hex & 0x00ff0000) >> 16) / 255.f,
                 ((hex & 0x0000ff00) >> 8)  / 255.f,
                 ((hex & 0x000000ff) >> 0)  / 255.f);
  return result;
}
u32 u32_from_rgba(v4 rgba) {
  u32 result = 0;
  result |= ((u32)((u8)(rgba.w*255.f))) << 24;
  result |= ((u32)((u8)(rgba.z*255.f))) << 16;
  result |= ((u32)((u8)(rgba.y*255.f))) <<  8;
  result |= ((u32)((u8)(rgba.x*255.f))) <<  0;
  return result;
}

////////////////////////////////////////////////////////////////////////
// Hash

u64 squirrel3(u64 at) {
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

u64 str_hash_FNV(String str) {
  u32 hash = 0x811c9dc5;
  Loop (i, str.size) {
    hash = (*str.str++ ^ hash) * 0x01000193;
  }
  return hash;
}

u64 hash_memory(void* data, u64 size) {
  u8*p = (u8*)data;
  uint64_t h = 1469598103934665603ull;
  Loop (i, size) {
    h ^= p[i];
    h *= 1099511628211ull;
  }
  return h;
}

u64 hash(u64 x, u64 seed) { return squirrel3(x + seed); }
u64 hash(String str, u64 seed) { return str_hash_FNV(str) + seed; }

////////////////////////////////////////////////////////////////////////
// Random

u32 xorshift32(u32* seed) {
  u32 x = *seed;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return *seed = x;
}

global thread_local u32 _seed = 0x95123512;
u32 rand_u32()                        { return xorshift32(&_seed); }
u32 rand_range_u32(u32 min, u32 max)  { return (rand_u32() % (max - min + 1)) + min; }
i32 rand_i32()                        { return rand_u32(); }
i32 rand_range_i32(i32 min, i32 max)  { return (i32)(rand_u32() % (u32)(max - min + 1)) + min; }
f32 rand_f32_01()                     { return rand_u32() / (f32)U32_MAX; }
f32 rand_f32_11()                     { return rand_f32_01()*2.0f - 1.0f; }
f32 rand_f32()                        { return rand_f32_01()*2*U16_MAX - U16_MAX; }
f32 rand_range_f32(f32 min, f32 max)  { return rand_f32_01()*(max - min) + min ; }
b32 rand_b32()                        { return rand_u32() % 2; }
void rand_seed()                      { _seed = cpu_timer_now(); }
u32 rand_get_seed()                   { return _seed; }

////////////////////////////////////////////////////////////////////////
// Misc

f32 Lerp(f32 a, f32 t, f32 b)  { return (1 - t)*a + t*b; }
f32 inverse_lerp(f32 a, f32 x, f32 b) { return (x - a) / (b - a); }
f64 inverse_lerp_f64(f64 a, f64 x, f64 b) { return (x - a) / (b - a); }
f32 map_range_f32(f32 v, f32 old_min, f32 old_max, f32 new_min, f32 new_max) {
  return new_min + (((v - old_min) * (new_max - new_min)) / (old_max - old_min));
}

////////////////////////////////////////////////////////////////////////
// Vector2

v2 v2_zero()                     { return v2{}; }
v2 v2_one()                      { return v2(1.0f, 1.0f); }
v2 v2_scale(f32 a)               { return v2(a, a); }
v2 v2_up()                       { return v2(0.0f, 1.0f); }
v2 v2_down()                     { return v2(0.0f, -1.0f); }
v2 v2_left()                     { return v2(-1.0f, 0.0f); }
v2 v2_right()                    { return v2(1.0f, 0.0f); }
v2 v2_of_v3(v3 a)                { return v2(a.x, a.y); }
v2 v2_of_v4(v4 a)                { return v2(a.x, a.y); }
v3 v2_to_v3(v2 a, f32 b)         { return v3(a.x, a.y, b); }
v4 v2_to_v4(v2 a, f32 b, f32 c)  { return v4(a.x, a.y, b, c); }
v2 v2_of_v2i(v2i a)              { return v2(a.x, a.y); }
v2 v2_of_v2u(v2u a)              { return v2(a.x, a.y); }
v2i v2i_of_v2(v2 a)              { return v2i(a.x, a.y); }
v2 v2_add_x(v2 a, f32 x)         { return v2(a.x + x, a.y); }
v2 v2_add_y(v2 a, f32 y)         { return v2(a.x, a.y + y); }

v2  operator+(v2 a, v2 b)          { return v2(a.x + b.x, a.y + b.y); }
v2  operator-(v2 a, v2 b)          { return v2(a.x - b.x, a.y - b.y); }
v2  operator*(v2 a, f32 scalar)    { return v2(a.x*scalar, a.y*scalar); }
v2  operator*(f32 scalar, v2 a)    { return v2(a.x*scalar, a.y*scalar); }
v2  operator/(v2 a, f32 scalar)    { return v2(a.x/scalar, a.y/scalar); }
v2  operator+=(v2& a, v2 b)        { return a = a + b; }
v2  operator-=(v2& a, v2 b)        { return a = a - b; }
v2  operator*=(v2& a, f32 scalar)  { return a = a*scalar; }
v2  operator/=(v2& a, f32 scalar)  { return a = a/scalar; }
b32 operator==(v2 a, v2 b)         { return (Abs(a.x - b.x) <= FloatEpsilon) && (Abs(a.y - b.y) <= FloatEpsilon); }
b32 operator!=(v2 a, v2 b)         { return !(a == b); }
v2  operator-(v2 a)                { return v2(-a.x, -a.y); }

f32 v2_length_squared(v2 a)    { return Square(a.x) + Square(a.y); }
f32 v2_length(v2 a)            { return Sqrt(v2_length_squared(a)); }
v2  v2_norm(v2 a)              { return a * (1.0f/v2_length(a)); }
f32 v2_distance(v2 a, v2 b)    { return v2_length(v2(a.x - b.x, a.y - b.y)); }
f32 v2_dot(v2 a, v2 b)         { return a.x*b.x + a.y*b.y; }
f32 v2_cross(v2 a, v2 b)       { return a.x*b.y - a.y*b.x; } // if > 0 (b is to the left of a), if < 0 (b is to the right of a), if == 0 (collinear)
v2  v2_lerp(v2 a, f32 t, v2 b) { return v2(Lerp(a.x, t, b.x), Lerp(a.y, t, b.y));}
v2  v2_skew(v2 a)              { return v2(-a.y, a.x); }
v2  v2_rand_range(v2 a, v2 b) {
  v2 vec = {
    rand_range_f32(a.x, b.x),
    rand_range_f32(a.y, b.y),
  };
  return vec;
}

f32 v2_shortest_arc(v2 a, v2 b) {
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

v2 v2_map_to_v2_11(v2 pos, v2 size) {
  v2 result = v2(2 * (pos.x/size.x) - 1, 2 * -(pos.y/size.y) + 1);
  return result;
}

////////////////////////////////////////////////////////////////////////
// Vector3

v3 v3_zero()              { return v3{}; }
v3 v3_one()               { return v3(1, 1, 1); }
v3 v3_scale(f32 a)        { return v3(a, a, a);}
v3 v3_up()                { return v3(0.0f, 1.0f, 0.0f); }
v3 v3_down()              { return v3(0.0f, -1.0f, 0.0f); }
v3 v3_left()              { return v3(-1.0f, 0.0f, 0.0f); }
v3 v3_right()             { return v3(1.0f, 0.0f, 0.0f); }
v3 v3_forward()           { return v3(0.0f, 0.0f, 1.0f); }
v3 v3_back()              { return v3(0.0f, 0.0f, -1.0f); }
v3 v3_of_v4(v4 a)         { return v3(a.x, a.y, a.z); }
v4 v3_to_v4(v3 a, f32 b)  { return v4(a.x, a.y, a.z, b); }

v3  operator+(v3 a, v3 b)          { return v3(a.x + b.x, a.y + b.y, a.z + b.z); }
v3  operator-(v3 a, v3 b)          { return v3(a.x - b.x, a.y - b.y, a.z - b.z); }
v3  operator*(v3 a, f32 scalar)    { return v3(a.x*scalar, a.y*scalar, a.z*scalar); }
v3  operator*(f32 scalar, v3 a)    { return v3(a.x*scalar, a.y*scalar, a.z*scalar); }
v3  operator/(v3 a, f32 scalar)    { return v3(a.x/scalar, a.y/scalar, a.z/scalar); }
v3  operator+=(v3& a, v3 b)        { return a = a + b; }
v3  operator-=(v3& a, v3 b)        { return a = a - b; }
v3  operator*=(v3& a, f32 scalar)  { return a = a * scalar; }
v3  operator/=(v3& a, f32 scalar)  { return a = a / scalar; }
b32 operator==(v3 a, v3 b)         { return (Abs(a.x - b.x) <= FloatEpsilon) && (Abs(a.y - b.y) <= FloatEpsilon) && (Abs(a.z - b.z) <= FloatEpsilon); }
b32 operator==(v3u a, v3u b)       { return a.x == b.x && a.y == b.y && a.z == b.z; }
b32 operator!=(v3 a, v3 b)         { return !(a == b); }
v3  operator-(v3 a)                { return v3(-a.x, -a.y, -a.z); }

f32 v3_length_squared(v3 a)     { return Square(a.x) + Square(a.y) + Square(a.z); }
f32 v3_length(v3 a)             { return Sqrt(v3_length_squared(a)); }
v3  v3_norm(v3 a)               { return a * (1.0f/v3_length(a)); }
f32 v3_distance(v3 a, v3 b)     { return v3_length(v3(a.x - b.x, a.y - b.y, a.z - b.z)); }
f32 v3_dot(v3 a, v3 b)          { return a.x*b.x + a.y*b.y + a.z*b.z; }
v3  v3_cross(v3 a, v3 b)        { return v3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }
v3  v3_lerp(v3 a, f32 t, v3 b)  { return v3(Lerp(a.x, t, b.x), Lerp(a.y, t, b.y), Lerp(a.z, t, b.z)); }
v3  v3_hadamard(v3 a, v3 b)     { return v3(a.x*b.x, a.y*b.y, a.z*b.z); }
v3  v3_hadamard_div(v3 a, v3 b) { return v3(a.x/b.x, a.y/b.y, a.z/b.z); }
v3  v3_greater(v3 a, v3 b)      { return v3(a.x>b.x, a.y>b.y, a.z>b.z); }
v3  v3_less(v3 a, v3 b)         { return v3(a.x<b.x, a.y<b.y, a.z<b.z); }

v3 v3_pos_of_mat4(mat4 mat) {
  v3 pos = v3_of_v4(mat.w);
  return pos;
};
// v3 v3_rot_of_mat4(mat4 mat) {
//   v3 vec = {
//     mat[9],
//     mat[10],
//     mat[11],
//   };
//   return vec;
// };
// v3 v3_scale_of_mat4(mat4 mat) {
//   v3 vec = {
//     mat[9],
//     mat[10],
//     mat[11],
//   };
//   return vec;
// };

v3 v3_rand_range(v3 a, v3 b) {
  v3 vec = {
    rand_range_f32(a.x, b.x),
    rand_range_f32(a.y, b.y),
    rand_range_f32(a.z, b.z),
  };
  return vec;
}

////////////////////////////////////////////////////////////////////////
// Vector4

v4 v4_zero()       { return v4{}; }
v4 v4_one()        { return v4(1, 1, 1, 1); }
v4 v4_scale(f32 a) { return v4(a,a,a,a); }

v4  operator+(v4 a, v4 b)          { return v4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
v4  operator-(v4 a, v4 b)          { return v4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
v4  operator*(v4 a, f32 scalar)    { return v4(a.x*scalar, a.y*scalar, a.z*scalar, a.w*scalar); }
v4  operator*(f32 scalar, v4 a)    { return v4(a.x*scalar, a.y*scalar, a.z*scalar, a.w*scalar); }
v4  operator/(v4 a, f32 scalar)    { return v4(a.x/scalar, a.y/scalar, a.z/scalar, a.w/scalar); }
v4  operator+=(v4& a, v4 b)        { return a = a + b; }
v4  operator-=(v4& a, v4 b)        { return a = a - b; }
v4  operator*=(v4& a, f32 scalar)  { return a = a * scalar; }
v4  operator/=(v4& a, f32 scalar)  { return a = a / scalar; }
v4  operator-(v4 a)                { return v4(-a.x, -a.y, -a.z, -a.w); }

f32 v4_length_squared(v4 a)          { return Square(a.x) + Square(a.y) + Square(a.z) + Square(a.w); }
f32 v4_length(v4 a)                  { return Sqrt(v4_length_squared(a)); }
v4  v4_normalize(v4 a)               { return a * (1.0f / v4_length(a)); }
v4  v4_hadamard(v4 a, v4 b)  { return v4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w); }

////////////////////////////////////////////////////////////////////////
// Matrix3

mat3 mat3_identity() {
  mat3 result = {
    1,0,0,
    0,1,0,
    0,0,1
  };
  return result;
}

mat3 mat3_translate(v2 pos) {
  mat3 result = {
    1,     0,     0,
    0,     1,     0,
    pos.x, pos.y, 1
  };
  return result;
}

mat3 mat3_scale(v2 scale) {
  mat3 result = {
    scale.x, 0,       0,
    0,       scale.y, 0,
    0,       0,       1
  };
  return result;
}

mat3 operator*(mat3 a, mat3 b) {
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
mat3& operator*=(mat3& a, mat3 b) { return a = b * a; }

v3 operator*(mat3 mat, v3 vec) {
  v3 result = {
    mat.v[0]*vec.x + mat.v[1]*vec.y + mat.v[2]*vec.z,
    mat.v[3]*vec.x + mat.v[4]*vec.y + mat.v[5]*vec.z,
    mat.v[6]*vec.x + mat.v[7]*vec.y + mat.v[8]*vec.z,
  };
  return result;
}

////////////////////////////////////////////////////////////////////////
// Matrix4

mat4 mat4_identity() {
  mat4 result = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
  };
  return result;
}

mat4 mat4_translate(v3 pos) {
  mat4 result = {
    1,     0,     0,     0,
    0,     1,     0,     0,
    0,     0,     1,     0,
    pos.x, pos.y, pos.z, 1
  };
  return result;
}

mat4 mat4_scale(v3 scale) {
  mat4 mat = {
    scale.x, 0,       0,       0,
    0,       scale.y, 0,       0,
    0,       0,       scale.z, 0,
    0,       0,       0,       1};
  return mat;
}

mat4 mat4_scale(mat4 m, f32 scale) {
  Loop (row, 4) {
    Loop (col, 4) {
      m.v[row][col] *= scale;
    }
  }
  return m;
}

mat4 mat4_rotate_x(f32 angle_radians) {
  f32 sin, cos;
  SinCos(angle_radians, &sin, &cos);
  mat4 mat = {
    1,   0,    0,   0,
    0, cos,  sin,   0,
    0,-sin,  cos,   0,
    0,   0,    0,   1,
  };
  return mat;
}

mat4 mat4_rotate_y(f32 angle_radians) {
  f32 sin, cos;
  SinCos(angle_radians, &sin, &cos);
  mat4 mat = {
    cos, 0,  -sin, 0,
    0,   1,   0,   0,
    sin, 0,   cos, 0,
    0,   0,   0,   1
  };
  return mat;
}

mat4 mat4_rotate_z(f32 angle_radians) {
  f32 sin, cos;
  SinCos(angle_radians, &sin, &cos);
  mat4 mat = {
    cos, sin, 0,   0,
   -sin, cos, 0,   0,
    0,   0,   1,   0,
    0,   0,   0,   1
  };
  return mat;
}

mat4 operator*(mat4 a, mat4 b) {
  mat4 c;
  Loop (row, 4) {
    Loop (col, 4) {
      c.v[row][col] = b.v[row][0] * a.v[0][col] +
                      b.v[row][1] * a.v[1][col] +
                      b.v[row][2] * a.v[2][col] +
                      b.v[row][3] * a.v[3][col];
    }
  }
  return c;
}
mat4& operator*=(mat4& a, mat4 b) { return a = b * a; }

v4 operator*(mat4 mat, v4 vec) {
  v4 result = {
    mat.v[0][0]*vec.x + mat.v[0][1]*vec.y + mat.v[0][2]*vec.z + mat.v[0][3]*vec.w,
    mat.v[1][0]*vec.x + mat.v[1][1]*vec.y + mat.v[1][2]*vec.z + mat.v[1][3]*vec.w,
    mat.v[2][0]*vec.x + mat.v[2][1]*vec.y + mat.v[2][2]*vec.z + mat.v[2][3]*vec.w,
    mat.v[3][0]*vec.x + mat.v[3][1]*vec.y + mat.v[3][2]*vec.z + mat.v[3][3]*vec.w,
  };
  return result;
}

mat4 mat4_rotate_xyz(v3 rot) {
  mat4 rx = mat4_rotate_x(rot.x);
  mat4 ry = mat4_rotate_y(rot.y);
  mat4 rz = mat4_rotate_z(rot.z);
  mat4 mat = rx * ry * rz;
  return mat;
}

mat4 mat4_transform(v3 pos, v3 rot, v3 scale) {
  mat4 result = mat4_translate(pos) * mat4_rotate_xyz(rot) * mat4_scale(scale);
  return result;
}
mat4 mat4_transform(Transform trans) {
  mat4 result = mat4_transform(trans.pos, trans.rot, trans.scale);
  return result;
}

mat4 mat4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) {
  mat4 result = {
    2/(right - left), 0,                0,                  0,
    0,                2/(top - bottom), 0,                  0,
    0,                0,                1/(far - near),     0,
    0,                0,                -near/(far - near), 1
  };
  return result;
}

// NOTE: I use camera looking at -Z direction since I found it's more intuitive
mat4 mat4_perspective(f32 fov_radians, f32 aspect_ratio, f32 near, f32 far) {
  f32 fov = Tan(fov_radians/2.0f);
  // mat4 result = {
  //   1/(fov*aspect_ratio), 0,                       0,                    0,
  //   0,                    1/fov,                   0,                    0,
  //   0,                    0,                       Far/(Far-Near),       1,
  //   0,                    0,                       -Far*Near/(Far-Near), 0
  // };
  // NOTE: This flips Z value so camera is looking at -Z direction
  mat4 result = {
    1/(fov*aspect_ratio), 0,                0,                             0,
    0,                    1/(fov),          0,                             0,
    0,                    0,                -(far + near)/(far - near),   -1,
    0,                    0,                (-2.0f*far*near)/(far - near), 0
  };
  return result;
}

// NOTE: we negate Z direction since camera is looking at -Z direction
mat4 mat4_look_at(v3 pos, v3 dir, v3 up) {
  v3 z = -v3_norm(dir);
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

mat4 mat4_transpose(mat4 mat) {
  mat4 result = {
    mat.v[0][0], mat.v[1][0], mat.v[2][0], mat.v[3][0],
    mat.v[0][1], mat.v[1][1], mat.v[2][1], mat.v[3][1],
    mat.v[0][2], mat.v[1][2], mat.v[2][2], mat.v[3][2],
    mat.v[0][3], mat.v[1][3], mat.v[2][3], mat.v[3][3],
  };
  return result;
}

mat4 mat4_inverse(mat4 m) {
  f32 coef00 = m.v[2][2] * m.v[3][3] - m.v[3][2] * m.v[2][3];
  f32 coef02 = m.v[1][2] * m.v[3][3] - m.v[3][2] * m.v[1][3];
  f32 coef03 = m.v[1][2] * m.v[2][3] - m.v[2][2] * m.v[1][3];
  f32 coef04 = m.v[2][1] * m.v[3][3] - m.v[3][1] * m.v[2][3];
  f32 coef06 = m.v[1][1] * m.v[3][3] - m.v[3][1] * m.v[1][3];
  f32 coef07 = m.v[1][1] * m.v[2][3] - m.v[2][1] * m.v[1][3];
  f32 coef08 = m.v[2][1] * m.v[3][2] - m.v[3][1] * m.v[2][2];
  f32 coef10 = m.v[1][1] * m.v[3][2] - m.v[3][1] * m.v[1][2];
  f32 coef11 = m.v[1][1] * m.v[2][2] - m.v[2][1] * m.v[1][2];
  f32 coef12 = m.v[2][0] * m.v[3][3] - m.v[3][0] * m.v[2][3];
  f32 coef14 = m.v[1][0] * m.v[3][3] - m.v[3][0] * m.v[1][3];
  f32 coef15 = m.v[1][0] * m.v[2][3] - m.v[2][0] * m.v[1][3];
  f32 coef16 = m.v[2][0] * m.v[3][2] - m.v[3][0] * m.v[2][2];
  f32 coef18 = m.v[1][0] * m.v[3][2] - m.v[3][0] * m.v[1][2];
  f32 coef19 = m.v[1][0] * m.v[2][2] - m.v[2][0] * m.v[1][2];
  f32 coef20 = m.v[2][0] * m.v[3][1] - m.v[3][0] * m.v[2][1];
  f32 coef22 = m.v[1][0] * m.v[3][1] - m.v[3][0] * m.v[1][1];
  f32 coef23 = m.v[1][0] * m.v[2][1] - m.v[2][0] * m.v[1][1];
  
  v4 fac0 = { coef00, coef00, coef02, coef03 };
  v4 fac1 = { coef04, coef04, coef06, coef07 };
  v4 fac2 = { coef08, coef08, coef10, coef11 };
  v4 fac3 = { coef12, coef12, coef14, coef15 };
  v4 fac4 = { coef16, coef16, coef18, coef19 };
  v4 fac5 = { coef20, coef20, coef22, coef23 };
  
  v4 vec0 = { m.v[1][0], m.v[0][0], m.v[0][0], m.v[0][0] };
  v4 vec1 = { m.v[1][1], m.v[0][1], m.v[0][1], m.v[0][1] };
  v4 vec2 = { m.v[1][2], m.v[0][2], m.v[0][2], m.v[0][2] };
  v4 vec3 = { m.v[1][3], m.v[0][3], m.v[0][3], m.v[0][3] };
  
  v4 inv0 = (v4_hadamard(vec1, fac0) - v4_hadamard(vec2, fac1)) + v4_hadamard(vec3, fac2);
  v4 inv1 = (v4_hadamard(vec0, fac0) - v4_hadamard(vec2, fac3)) + v4_hadamard(vec3, fac4);
  v4 inv2 = (v4_hadamard(vec0, fac1) - v4_hadamard(vec1, fac3)) + v4_hadamard(vec3, fac5);
  v4 inv3 = (v4_hadamard(vec0, fac2) - v4_hadamard(vec1, fac4)) + v4_hadamard(vec2, fac5);
  
  v4 sign_a = { +1, -1, +1, -1 };
  v4 sign_b = { -1, +1, -1, +1 };
  
  mat4 inverse;
  Loop (i, 4) {
    inverse.v[0][i] = inv0.v[i] * sign_a.v[i];
    inverse.v[1][i] = inv1.v[i] * sign_b.v[i];
    inverse.v[2][i] = inv2.v[i] * sign_a.v[i];
    inverse.v[3][i] = inv3.v[i] * sign_b.v[i];
  }
  
  v4 row0 = { inverse.v[0][0], inverse.v[1][0], inverse.v[2][0], inverse.v[3][0] };
  v4 m0 = { m.v[0][0], m.v[0][1], m.v[0][2], m.v[0][3] };
  v4 dot0 = v4_hadamard(m0, row0);
  f32 dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);
  
  f32 one_over_det = 1 / dot1;
  
  return mat4_scale(inverse, one_over_det);
}

// NOTE: camera is looking at -z so forward is negative
v3 mat4_forward(mat4 matrix) {
  v3 res = {
    -matrix.v[0][2],
    -matrix.v[1][2],
    -matrix.v[2][2],
  };
  res = v3_norm(res);
  return res;
}

v3 mat4_backward(mat4 matrix) {
  v3 res = {
    matrix.v[0][2],
    matrix.v[1][2],
    matrix.v[2][2],
  };
  res = v3_norm(res);
  return res;
}

v3 mat4_up(mat4 matrix) {
  v3 up = {
    matrix.v[0][1],
    matrix.v[1][1],
    matrix.v[2][1],
  };
  up = v3_norm(up);
  return up;
}

v3 mat4_down(mat4 matrix) {
  v3 down = {
    -matrix.v[0][1],
    -matrix.v[1][1],
    -matrix.v[2][1],
  };
  down = v3_norm(down);
  return down;
}

v3 mat4_right(mat4 matrix) {
  v3 left = {
    matrix.v[0][0],
    matrix.v[1][0],
    matrix.v[2][0],
  };
  left = v3_norm(left);
  return left;
}

v3 mat4_left(mat4 matrix) {
  v3 right = {
    -matrix.v[0][0],
    -matrix.v[1][0],
    -matrix.v[2][0],
  };
  right = v3_norm(right);
  return right;
}

////////////////////////////////////////////////////////////////////////
// Range Ops

///////////////////////////////////
// Dim1
Rng1u32 shift_1u32(Rng1u32 r, u32 x)         { return Rng1u32{r.min + x, r.max + x}; }
Rng1u32 pad_1u32(Rng1u32 r, u32 x)           { return Rng1u32{r.min - x, r.max + x}; }
u32 center_1u32(Rng1u32 r)                   { return (r.min+r.max)/2; }
b32 contains_1u32(Rng1u32 r, u32 x)          { return (r.min <= x && x < r.max); }
u32 dim_1u32(Rng1u32 r)                      { return r.max - r.min; }
Rng1u32 union_1u32(Rng1u32 a, Rng1u32 b)     { return Rng1u32{Min(a.min, b.min), Max(a.max, b.max)}; }
Rng1u32 intersect_1u32(Rng1u32 a, Rng1u32 b) { return Rng1u32{Max(a.min, b.min), Min(a.max, b.max)}; }
u32 clamp_1u32(Rng1u32 r, u32 v)             { return Clamp(r.min, v, r.max); }

Rng1i32 shift_1i32(Rng1i32 r, i32 x)         { return Rng1i32{r.min + x, r.max + x}; }
Rng1i32 pad_1i32(Rng1i32 r, i32 x)           { return Rng1i32{r.min - x, r.max + x}; }
i32 center_1i32(Rng1i32 r)                   { return (r.min+r.max)/2; }
b32 contains_1i32(Rng1i32 r, i32 x)          { return (r.min <= x && x < r.max); }
i32 dim_1i32(Rng1i32 r)                      { return r.max - r.min; }
Rng1i32 union_1i32(Rng1i32 a, Rng1i32 b)     { return Rng1i32{Min(a.min, b.min), Max(a.max, b.max)}; }
Rng1i32 intersect_1i32(Rng1i32 a, Rng1i32 b) { return Rng1i32{Max(a.min, b.min), Min(a.max, b.max)}; }
i32 clamp_1i32(Rng1i32 r, i32 v)             { return Clamp(r.min, v, r.max); }

Rng1u64 shift_1u64(Rng1u64 r, u64 x)         { return Rng1u64{r.min + x, r.max + x}; }
Rng1u64 pad_1u64(Rng1u64 r, u64 x)           { return Rng1u64{r.min - x, r.max + x}; }
u64 center_1u64(Rng1u64 r)                   { return (r.min+r.max)/2; }
b32 contains_1u64(Rng1u64 r, u64 x)          { return (r.min <= x && x < r.max); }
u64 dim_1u64(Rng1u64 r)                      { return r.max - r.min; }
Rng1u64 union_1u64(Rng1u64 a, Rng1u64 b)     { return Rng1u64{Min(a.min, b.min), Max(a.max, b.max)}; }
Rng1u64 intersect_1u64(Rng1u64 a, Rng1u64 b) { return Rng1u64{Max(a.min, b.min), Min(a.max, b.max)}; }
u64 clamp_1u64(Rng1u64 r, u64 v)             { return Clamp(r.min, v, r.max); }

Rng1f32 shift_1f32(Rng1f32 r, f32 x)         { return Rng1f32{r.min + x, r.max + x}; }
Rng1f32 pad_1f32(Rng1f32 r, f32 x)           { return Rng1f32{r.min - x, r.max + x}; }
f32 center_1f32(Rng1f32 r)                   { return (r.min+r.max)/2; }
b32 contains_1f32(Rng1f32 r, f32 x)          { return (r.min <= x && x < r.max); }
f32 dim_1f32(Rng1f32 r)                      { return r.max - r.min; }
Rng1f32 union_1f32(Rng1f32 a, Rng1f32 b)     { return Rng1f32{Min(a.min, b.min), Max(a.max, b.max)}; }
Rng1f32 intersect_1f32(Rng1f32 a, Rng1f32 b) { return Rng1f32{Max(a.min, b.min), Min(a.max, b.max)}; }
f32 clamp_1f32(Rng1f32 r, f32 v)             { return Clamp(r.min, v, r.max); }

///////////////////////////////////
// Dim2
Rng2f32 shift_2f32(Rng2f32 r, v2 x)          { return Rng2f32{r.min + x, r.max + x};}
Rng2f32 pad_2f32(Rng2f32 r, f32 x)           { return Rng2f32{r.min - v2_scale(x), r.max + v2_scale(x)};}
v2 center_2f32(Rng2f32 r)                    { return v2((r.min.x+r.max.x)/2, (r.min.y+r.max.y)/2); }
b32 contains_2f32(Rng2f32 r, v2 x)           { return (r.min.x <= x.x && x.x < r.max.x && r.min.y <= x.y && x.y < r.max.y); }
v2 dim_2f32(Rng2f32 r)                       { return v2(r.max.x - r.min.x, r.max.y - r.min.y); }
Rng2f32 union_2f32(Rng2f32 a, Rng2f32 b)     { return Rng2f32{v2(Min(a.min.x, b.min.x), Min(a.min.y, b.min.y)), v2(Max(a.max.x, b.max.x), Max(a.max.y, b.max.y))}; }
Rng2f32 intersect_2f32(Rng2f32 a, Rng2f32 b) { return Rng2f32{v2(Max(a.min.x, b.min.x), Max(a.min.y, b.min.y)), v2(Min(a.max.x, b.max.x), Min(a.max.y, b.max.y))}; }
v2 clamp_2f32(Rng2f32 r, v2 v)               { return v2(Clamp(r.min.x, v.x, r.max.x), Clamp(r.min.y, v.y, r.max.y)); }
Rng2f32 center_size_2f32(Rng2f32 r, v2 x)    { v2 center = center_2f32(r); return Rng2f32(v2(center.x - x.x / 2, center.y - x.y / 2), v2(center.x + x.x / 2, center.y + x.y / 2)); }

///////////////////////////////////
// Dim3
Rng3f32 shift_3f32(Rng3f32 r, v3 x) { return Rng3f32{r.min + x, r.max + x}; }
Rng3f32 pad_3f32(Rng3f32 r, f32 x)  { return Rng3f32{r.min - v3_scale(x), r.max + v3_scale(x)}; }
v3 center_3f32(Rng3f32 r)           { return v3((r.min.x + r.max.x)/2, (r.min.y + r.max.y)/2, (r.min.z + r.max.z)/2); }
b32 contains_3f32(Rng3f32 r, v3 x)  { return (r.min.x <= x.x && x.x < r.max.x && r.min.y <= x.y && x.y < r.max.y && r.min.z <= x.z && x.z < r.max.z); }
v3 dim_3f32(Rng3f32 r)              { return v3(r.max.x - r.min.x, r.max.y - r.min.y, r.max.z - r.min.z); }
Rng3f32 union_3f32(Rng3f32 a, Rng3f32 b) {
  return Rng3f32{v3(Min(a.min.x, b.min.x), Min(a.min.y, b.min.y), Min(a.min.z, b.min.z)),
                 v3(Max(a.max.x, b.max.x), Max(a.max.y, b.max.y), Max(a.max.z, b.max.z))};
}
Rng3f32 intersect_3f32(Rng3f32 a, Rng3f32 b) {
  return Rng3f32{v3(Max(a.min.x, b.min.x), Max(a.min.y, b.min.y), Max(a.min.z, b.min.z)),
                 v3(Min(a.max.x, b.max.x), Min(a.max.y, b.max.y), Min(a.max.z, b.max.z))};
}
v3 clamp_3f32(Rng3f32 r, v3 v) { return v3(Clamp(r.min.x, v.x, r.max.x), Clamp(r.min.y, v.y, r.max.y), Clamp(r.min.z, v.z, r.max.z)); }



