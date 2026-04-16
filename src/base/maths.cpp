#include "maths.h"

f32 degtorad(f32 degrees) { return degrees * PI / 180.0f; }
f32 radtodeg(f32 radians) { return radians * 180.0f / PI; }

v2::v2(f32 x_, f32 y_) { x = x_, y = y_; }

v2i::v2i(i32 x_, i32 y_) { x = x_, y = y_; }

v2u::v2u(u32 x_, u32 y_) { x = x_, y = y_; }

v3::v3(f32 x_, f32 y_, f32 z_) { x = x_, y = y_, z = z_; }

v3u::v3u(u32 x_, u32 y_, u32 z_) { x = x_, y = y_, z = z_; }

v4::v4(f32 x_, f32 y_, f32 z_, f32 w_) { x = x_, y = y_, z = z_, w = w_; }

f32& mat3::operator[](u32 a) { return e[a]; }

f32 Sin(f32 a)           { return __builtin_sinf(a); }
f32 Cos(f32 a)           { return __builtin_cosf(a); }
f32 Tan(f32 a)           { return __builtin_tanf(a); }
f32 Asin(f32 a)          { return __builtin_asinf(a); }
f32 Acos(f32 a)          { return __builtin_acosf(a); }
f32 Atan2(f32 y, f32 x)  { return __builtin_atan2f(y,x); }
f32 Sqrt(f32 a)          { return __builtin_sqrtf(a); }
f32 Pow(f32 a, f32 b)    { return __builtin_powf(a, b); }
f32 Floor(f32 a)         { return __builtin_floorf(a); }
f32 Ceil(f32 a)          { return __builtin_ceilf(a); }
f32 Round(f32 a)         { return __builtin_roundf(a); }
f32 Mod(f32 a, f32 b)    { return __builtin_fmodf(a, b); }
f32 Exp(f32 a)           { return __builtin_expf(a); }
f32 LogE(f32 a)          { return __builtin_logf(a); }
f32 Log2(f32 a)          { return __builtin_log2f(a); }
f32 Log10(f32 a)         { return __builtin_log10f(a); }

void SinCos(f32 angle, f32* a, f32* b) { __builtin_sincosf(angle, a, b); }

f32 SinD(f32 a)                { return Sin(degtorad(a)); }
f32 CosD(f32 a)                { return Cos(degtorad(a)); }
f32 Lerp(f32 a, f32 t, f32 b)  { return (1 - t)*a + t*b; }
f32 Atan2_360(f32 y, f32 x)    { return Atan2(-y, -x) + PI; }

u32 next_pow2(u32 v) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}
u32 prev_pow2(u32 n) {
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	return n - (n >> 1);
}

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
  result |= ((u32)((u8)(rgba.x*255.f))) << 24;
  result |= ((u32)((u8)(rgba.y*255.f))) << 16;
  result |= ((u32)((u8)(rgba.z*255.f))) <<  8;
  result |= ((u32)((u8)(rgba.w*255.f))) <<  0;
  return result;
}
u32 u32_from_argb(v4 argb) {
  u32 result = 0;
  result |= ((u32)((u8)(argb.w*255.f))) << 24;
  result |= ((u32)((u8)(argb.x*255.f))) << 16;
  result |= ((u32)((u8)(argb.y*255.f))) << 8;
  result |= ((u32)((u8)(argb.z*255.f))) << 0;
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

u64 hash(u64 x) { return squirrel3(x); }
u64 hash(u64 x, u64 seed) { return squirrel3(x + seed); }
u64 hash(String str) { return str_hash_FNV(str); }

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

b32 v2_in_rect(Rect rect, v2 p) {
    return (p.x >= rect.min.x &&
            p.y >= rect.min.y &&
            p.x <= rect.max.x &&
            p.y <= rect.max.y);
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
    mat.e[0]*vec.x + mat.e[1]*vec.y + mat.e[2]*vec.z,
    mat.e[3]*vec.x + mat.e[4]*vec.y + mat.e[5]*vec.z,
    mat.e[6]*vec.x + mat.e[7]*vec.y + mat.e[8]*vec.z,
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
      m.e[row][col] *= scale;
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
      c.e[row][col] = b.e[row][0] * a.e[0][col] +
                      b.e[row][1] * a.e[1][col] +
                      b.e[row][2] * a.e[2][col] +
                      b.e[row][3] * a.e[3][col];
    }
  }
  return c;
}
mat4& operator*=(mat4& a, mat4 b) { return a = b * a; }

v4 operator*(mat4 mat, v4 vec) {
  v4 result = {
    mat.e[0][0]*vec.x + mat.e[0][1]*vec.y + mat.e[0][2]*vec.z + mat.e[0][3]*vec.w,
    mat.e[1][0]*vec.x + mat.e[1][1]*vec.y + mat.e[1][2]*vec.z + mat.e[1][3]*vec.w,
    mat.e[2][0]*vec.x + mat.e[2][1]*vec.y + mat.e[2][2]*vec.z + mat.e[2][3]*vec.w,
    mat.e[3][0]*vec.x + mat.e[3][1]*vec.y + mat.e[3][2]*vec.z + mat.e[3][3]*vec.w,
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
mat4 mat4_perspective(f32 fov_radians, f32 aspect_ratio, f32 Near, f32 Far) {
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
    0,                    0,                -(Far + Near)/(Far - Near),   -1,
    0,                    0,                (-2.0f*Far*Near)/(Far - Near), 0
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

mat4 mat4_transpose(mat4 matrix) {
  mat4 out_matrix = mat4_identity();
  // out_matrix.e[0] = matrix.e[0];
  // out_matrix.e[1] = matrix.e[4];
  // out_matrix.e[2] = matrix.e[8];
  // out_matrix.e[3] = matrix.e[12];
  // out_matrix.e[4] = matrix.e[1];
  // out_matrix.e[5] = matrix.e[5];
  // out_matrix.e[6] = matrix.e[9];
  // out_matrix.e[7] = matrix.e[13];
  // out_matrix.e[8] = matrix.e[2];
  // out_matrix.e[9] = matrix.e[6];
  // out_matrix.e[10] = matrix.e[10];
  // out_matrix.e[11] = matrix.e[14];
  // out_matrix.e[12] = matrix.e[3];
  // out_matrix.e[13] = matrix.e[7];
  // out_matrix.e[14] = matrix.e[11];
  // out_matrix.e[15] = matrix.e[15];
  // mat4 result = {
  //   matrix[0],
  // };
  return out_matrix;
}

mat4 mat4_inverse(mat4 m) {
  f32 coef00 = m.e[2][2] * m.e[3][3] - m.e[3][2] * m.e[2][3];
  f32 coef02 = m.e[1][2] * m.e[3][3] - m.e[3][2] * m.e[1][3];
  f32 coef03 = m.e[1][2] * m.e[2][3] - m.e[2][2] * m.e[1][3];
  f32 coef04 = m.e[2][1] * m.e[3][3] - m.e[3][1] * m.e[2][3];
  f32 coef06 = m.e[1][1] * m.e[3][3] - m.e[3][1] * m.e[1][3];
  f32 coef07 = m.e[1][1] * m.e[2][3] - m.e[2][1] * m.e[1][3];
  f32 coef08 = m.e[2][1] * m.e[3][2] - m.e[3][1] * m.e[2][2];
  f32 coef10 = m.e[1][1] * m.e[3][2] - m.e[3][1] * m.e[1][2];
  f32 coef11 = m.e[1][1] * m.e[2][2] - m.e[2][1] * m.e[1][2];
  f32 coef12 = m.e[2][0] * m.e[3][3] - m.e[3][0] * m.e[2][3];
  f32 coef14 = m.e[1][0] * m.e[3][3] - m.e[3][0] * m.e[1][3];
  f32 coef15 = m.e[1][0] * m.e[2][3] - m.e[2][0] * m.e[1][3];
  f32 coef16 = m.e[2][0] * m.e[3][2] - m.e[3][0] * m.e[2][2];
  f32 coef18 = m.e[1][0] * m.e[3][2] - m.e[3][0] * m.e[1][2];
  f32 coef19 = m.e[1][0] * m.e[2][2] - m.e[2][0] * m.e[1][2];
  f32 coef20 = m.e[2][0] * m.e[3][1] - m.e[3][0] * m.e[2][1];
  f32 coef22 = m.e[1][0] * m.e[3][1] - m.e[3][0] * m.e[1][1];
  f32 coef23 = m.e[1][0] * m.e[2][1] - m.e[2][0] * m.e[1][1];
  
  v4 fac0 = { coef00, coef00, coef02, coef03 };
  v4 fac1 = { coef04, coef04, coef06, coef07 };
  v4 fac2 = { coef08, coef08, coef10, coef11 };
  v4 fac3 = { coef12, coef12, coef14, coef15 };
  v4 fac4 = { coef16, coef16, coef18, coef19 };
  v4 fac5 = { coef20, coef20, coef22, coef23 };
  
  v4 vec0 = { m.e[1][0], m.e[0][0], m.e[0][0], m.e[0][0] };
  v4 vec1 = { m.e[1][1], m.e[0][1], m.e[0][1], m.e[0][1] };
  v4 vec2 = { m.e[1][2], m.e[0][2], m.e[0][2], m.e[0][2] };
  v4 vec3 = { m.e[1][3], m.e[0][3], m.e[0][3], m.e[0][3] };
  
  v4 inv0 = (v4_hadamard(vec1, fac0) - v4_hadamard(vec2, fac1)) + v4_hadamard(vec3, fac2);
  v4 inv1 = (v4_hadamard(vec0, fac0) - v4_hadamard(vec2, fac3)) + v4_hadamard(vec3, fac4);
  v4 inv2 = (v4_hadamard(vec0, fac1) - v4_hadamard(vec1, fac3)) + v4_hadamard(vec3, fac5);
  v4 inv3 = (v4_hadamard(vec0, fac2) - v4_hadamard(vec1, fac4)) + v4_hadamard(vec2, fac5);
  
  v4 sign_a = { +1, -1, +1, -1 };
  v4 sign_b = { -1, +1, -1, +1 };
  
  mat4 inverse;
  Loop (i, 4) {
    inverse.e[0][i] = inv0.e[i] * sign_a.e[i];
    inverse.e[1][i] = inv1.e[i] * sign_b.e[i];
    inverse.e[2][i] = inv2.e[i] * sign_a.e[i];
    inverse.e[3][i] = inv3.e[i] * sign_b.e[i];
  }
  
  v4 row0 = { inverse.e[0][0], inverse.e[1][0], inverse.e[2][0], inverse.e[3][0] };
  v4 m0 = { m.e[0][0], m.e[0][1], m.e[0][2], m.e[0][3] };
  v4 dot0 = v4_hadamard(m0, row0);
  f32 dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);
  
  f32 one_over_det = 1 / dot1;
  
  return mat4_scale(inverse, one_over_det);
}

v3 mat4_forward(mat4 matrix) {
  v3 res = {
    -matrix.e[0][2],
    -matrix.e[1][2],
    -matrix.e[2][2],
  };
  res = v3_norm(res);
  return res;
}

v3 mat4_backward(mat4 matrix) {
  v3 res = {
    matrix.e[0][2],
    matrix.e[1][2],
    matrix.e[2][2],
  };
  res = v3_norm(res);
  return res;
}

v3 mat4_up(mat4 matrix) {
  v3 up = {
    matrix.e[0][1],
    matrix.e[1][1],
    matrix.e[2][1],
  };
  up = v3_norm(up);
  return up;
}

v3 mat4_down(mat4 matrix) {
  v3 down = {
    -matrix.e[0][1],
    -matrix.e[1][1],
    -matrix.e[2][1],
  };
  down = v3_norm(down);
  return down;
}

v3 mat4_right(mat4 matrix) {
  v3 left = {
    matrix.e[0][0],
    matrix.e[1][0],
    matrix.e[2][0],
  };
  left = v3_norm(left);
  return left;
}

v3 mat4_left(mat4 matrix) {
  v3 right = {
    -matrix.e[0][0],
    -matrix.e[1][0],
    -matrix.e[2][0],
  };
  right = v3_norm(right);
  return right;
}

////////////////////////////////////////////////////////////////////////
// Quaternions

quat quat_identity() {
  return quat{0, 0, 0, 1.0f};
}

f32 quat_normal(quat q) {
  return Sqrt(
      q.x * q.x +
      q.y * q.y +
      q.z * q.z +
      q.w * q.w);
}

quat quat_normalize(quat q) {
  f32 normal = quat_normal(q);
  return quat{
      q.x / normal,
      q.y / normal,
      q.z / normal,
      q.w / normal};
}

quat quat_conjugate(quat q) {
  return quat{
      -q.x,
      -q.y,
      -q.z,
      q.w};
}

quat quat_inverse(quat q) {
  return quat_normalize(quat_conjugate(q));
}

quat quat_mul(quat q_0, quat q_1) {
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

f32 quat_dot(quat q_0, quat q_1) {
  return q_0.x * q_1.x +
         q_0.y * q_1.y +
         q_0.z * q_1.z +
         q_0.w * q_1.w;
}

mat4 quat_to_mat4(quat q) {
  mat4 out_matrix = mat4_identity();

  // https://stackoverflow.com/questions/1556260/convert-quaternion-rotation-to-rotation-matrix

  // quat n = quat_normalize(q);

  // out_matrix.e[0] = 1.0f - 2.0f * n.y * n.y - 2.0f * n.z * n.z;
  // out_matrix.e[1] = 2.0f * n.x * n.y - 2.0f * n.z * n.w;
  // out_matrix.e[2] = 2.0f * n.x * n.z + 2.0f * n.y * n.w;

  // out_matrix.e[4] = 2.0f * n.x * n.y + 2.0f * n.z * n.w;
  // out_matrix.e[5] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.z * n.z;
  // out_matrix.e[6] = 2.0f * n.y * n.z - 2.0f * n.x * n.w;

  // out_matrix.e[8] = 2.0f * n.x * n.z - 2.0f * n.y * n.w;
  // out_matrix.e[9] = 2.0f * n.y * n.z + 2.0f * n.x * n.w;
  // out_matrix.e[10] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.y * n.y;

  return out_matrix;
}

// Calculates a rotation matrix based on the quaternion and the passed in center point.
mat4 quat_to_rotation_matrix(quat q, v3 center) {
  mat4 out_matrix;

  // f32* o = out_matrix.e;
  // o[0] = (q.x * q.x) - (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
  // o[1] = 2.0f * ((q.x * q.y) + (q.z * q.w));
  // o[2] = 2.0f * ((q.x * q.z) - (q.y * q.w));
  // o[3] = center.x - center.x * o[0] - center.y * o[1] - center.z * o[2];

  // o[4] = 2.0f * ((q.x * q.y) - (q.z * q.w));
  // o[5] = -(q.x * q.x) + (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
  // o[6] = 2.0f * ((q.y * q.z) + (q.x * q.w));
  // o[7] = center.y - center.x * o[4] - center.y * o[5] - center.z * o[6];

  // o[8] = 2.0f * ((q.x * q.z) + (q.y * q.w));
  // o[9] = 2.0f * ((q.y * q.z) - (q.x * q.w));
  // o[10] = -(q.x * q.x) - (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
  // o[11] = center.z - center.x * o[8] - center.y * o[9] - center.z * o[10];

  // o[12] = 0.0f;
  // o[13] = 0.0f;
  // o[14] = 0.0f;
  // o[15] = 1.0f;
  return out_matrix;
}

quat quat_from_axis_angle(v3 axis, f32 angle, b32 normalize) {
  const f32 half_angle = 0.5f * angle;
  f32 s, c;
  SinCos(half_angle, &s, &c);

  quat q = quat{s * axis.x, s * axis.y, s * axis.z, c};
  if (normalize) {
    return quat_normalize(q);
  }
  return q;
}

quat quat_slerp(quat q_0, quat q_1, f32 percentage) {
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

f32 map_range_f32(f32 v, f32 old_min, f32 old_max, f32 new_min, f32 new_max) {
  return new_min + (((v - old_min) * (new_max - new_min)) / (old_max - old_min));
}

