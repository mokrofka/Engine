#include "maths.h"
#include "os/os_core.h"

f32 degtorad(f32 degrees) { return degrees * PI / 180.0f; }
f32 radtodeg(f32 radians) { return radians * 180.0f / PI; }

v2::v2(f32 x_, f32 y_) { x = x_, y = y_; }
v2i::v2i(i32 x_, i32 y_) { x = x_, y = y_; }
v2u::v2u(u32 x_, u32 y_) { x = x_, y = y_; }
v2b::v2b(b32 x_, b32 y_) { x = x_, y = y_; }
v3::v3(f32 x_, f32 y_, f32 z_) { x = x_, y = y_, z = z_; }
v3u::v3u(u32 x_, u32 y_, u32 z_) { x = x_, y = y_, z = z_; }
v3b::v3b(b32 x_, b32 y_, b32 z_) { x = x_, y = y_, z = z_; }
v4::v4(f32 x_, f32 y_, f32 z_, f32 w_) { x = x_, y = y_, z = z_, w = w_; }

Rng2::Rng2(v2 min_, v2 max_) { min = min_; max = max_; }
Rng3::Rng3(v3 min_, v3 max_) { min = min_; max = max_; }

f32 Sin(f32 x)                         { return __builtin_sinf(x); }
f32 Cos(f32 x)                         { return __builtin_cosf(x); }
f32 Tan(f32 x)                         { return __builtin_tanf(x); }
f32 Asin(f32 x)                        { return __builtin_asinf(x); }
f32 Acos(f32 x)                        { return __builtin_acosf(x); }
f32 Atan2(f32 y, f32 x)                { return __builtin_atan2f(y,x); }
f32 Sqrt(f32 x)                        { return __builtin_sqrtf(x); }
f32 Pow(f32 a, f32 b)                  { return __builtin_powf(a, b); }
f32 Floor(f32 x)                       { return __builtin_floorf(x); }
f32 Ceil(f32 x)                        { return __builtin_ceilf(x); }
f32 Round(f32 x)                       { return __builtin_roundf(x); }
f32 Mod(f32 a, f32 b)                  { return __builtin_fmodf(a, b); }
f32 Exp(f32 x)                         { return __builtin_expf(x); }
f32 LogE(f32 x)                        { return __builtin_logf(x); }
f32 Log2(f32 x)                        { return __builtin_log2f(x); }
f32 Log10(f32 x)                       { return __builtin_log10f(x); }
void SinCos(f32 rad, f32* s, f32* c) { __builtin_sincosf(rad, s, c); }

f32 SinD(f32 x)                { return Sin(degtorad(x)); }
f32 CosD(f32 x)                { return Cos(degtorad(x)); }

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

u64 squirrel3(u64 x) {
  x *= 0x9E3779B185EBCA87ULL;
  x ^= (x >> 8);
  x += 0xC2B2AE3D27D4EB4FULL;
  x ^= (x << 8);
  x *= 0x27D4EB2F165667C5ULL;
  x ^= (x >> 8);
  return x;
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
u32 rand_rng_u32(u32 min, u32 max)    { return (rand_u32() % (max - min + 1)) + min; }
i32 rand_i32()                        { return rand_u32(); }
i32 rand_rng_i32(i32 min, i32 max)    { return (i32)(rand_u32() % (u32)(max - min + 1)) + min; }
f32 rand_f32_01()                     { return rand_u32() / (f32)U32_MAX; }
f32 rand_f32_11()                     { return rand_f32_01()*2.0f - 1.0f; }
f32 rand_f32()                        { return rand_f32_01()*2*U16_MAX - U16_MAX; }
f32 rand_rng_f32(f32 min, f32 max)    { return rand_f32_01()*(max - min) + min ; }
b32 rand_b32()                        { return rand_u32() % 2; }
void rand_set_seed()                      { _seed = cpu_timer_now(); }
u32 rand_get_seed()                   { return _seed; }

////////////////////////////////////////////////////////////////////////
// Misc

i32 wrap_i32(i32 min, i32 x, i32 max) {
  i32 res = (x - min) % (max - min);
  return res + min;
}
f32 wrap_f32(f32 min, f32 x, f32 max) {
  f32 res = x - (max - min)*Floor((x - min)/(max - min));
  return res;
}
f32 Lerp(f32 a, f32 t, f32 b)   { return (1 - t)*a + t*b; }
f32 Unlerp(f32 a, f32 x, f32 b) { return (x - a) / (b - a); }
f32 remap(f32 x, f32 old_min, f32 old_max, f32 new_min, f32 new_max) {
  return new_min + (((x - old_min) * (new_max - new_min)) / (old_max - old_min));
}
f32 remap(f32 x, Rng1 old, Rng1 new_) { return remap(x, old.min, old.max, new_.min, new_.max);}
f32 remap_clamped(f32 x, f32 old_min, f32 old_max, f32 new_min, f32 new_max) {
  return remap(Clamp(old_min, x, old_max), old_min, old_max, new_min, new_max);
}

////////////////////////////////////////////////////////////////////////
// Vector2

v2  operator+(v2 a, v2 b)          { return v2(a.x + b.x, a.y + b.y); }
v2  operator-(v2 a, v2 b)          { return v2(a.x - b.x, a.y - b.y); }
v2  operator*(v2 v, f32 scalar)    { return v2(v.x*scalar, v.y*scalar); }
v2  operator*(f32 scalar, v2 v)    { return v2(v.x*scalar, v.y*scalar); }
v2  operator/(v2 v, f32 scalar)    { return v2(v.x/scalar, v.y/scalar); }
v2  operator+=(v2& a, v2 b)        { return a = a + b; }
v2  operator-=(v2& a, v2 b)        { return a = a - b; }
v2  operator*=(v2& v, f32 scalar)  { return v = v*scalar; }
v2  operator/=(v2& v, f32 scalar)  { return v = v/scalar; }
b32 operator==(v2 a, v2 b)         { return (Abs(a.x - b.x) <= FloatEpsilon) && (Abs(a.y - b.y) <= FloatEpsilon); }
b32 operator!=(v2 a, v2 b)         { return !(a == b); }
v2  operator-(v2 v)                { return v2(-v.x, -v.y); }

b32 operator==(v2u a, v2u b)       { return (a.x == b.x) && (a.y == b.y); }
b32 operator!=(v2u a, v2u b)       { return !(a == b); }

v2  v2_of_v3(v3 v)                  { return v2(v.x, v.y); }
v2  v2_of_v4(v4 v)                  { return v2(v.x, v.y); }
v3  v2_to_v3(v2 v, f32 a)           { return v3(v.x, v.y, a); }
v4  v2_to_v4(v2 v, f32 a, f32 b)    { return v4(v.x, v.y, a, b); }
v2  v2_of_v2i(v2i v)                { return v2(v.x, v.y); }
v2  v2_of_v2u(v2u v)                { return v2(v.x, v.y); }
v2i v2i_of_v2(v2 v)                 { return v2i(v.x, v.y); }
v2  v2_up()                         { return v2(0.0f, 1.0f); }
v2  v2_down()                       { return v2(0.0f, -1.0f); }
v2  v2_left()                       { return v2(-1.0f, 0.0f); }
v2  v2_right()                      { return v2(1.0f, 0.0f); }
v2  v2_zero()                       { return v2{}; }
v2  v2_one()                        { return v2(1.0f, 1.0f); }
v2  v2_splat(f32 x)                 { return v2(x, x); }
v2  v2_add_scalar(v2 v, f32 x)      { return v2(v.x+x, v.y+x); }
v2  v2_subtract_scalar(v2 v, f32 x) { return v2_add_scalar(v, -x); }
v2  v2_min(v2 a, v2 b)              { return v2(Min(a.x,b.x), Min(a.y,b.y)); }
v2  v2_max(v2 a, v2 b)              { return v2(Max(a.x,b.x), Max(a.y,b.y)); }
v2  v2_invert(v2 v)                 { return v2(1.0f/v.x, 1.0f/v.y); }
v2b v2_greater(v2 a, v2 b)          { return v2b(a.x>b.x, a.y>b.y); }
v2b v2_less(v2 a, v2 b)             { return v2b(a.x<b.x, a.y<b.y); }
b32 v2_greater_any(v2 a, v2 b)      { return a.x>b.x || a.y>b.y; }
b32 v2_less_any(v2 a, v2 b)         { return a.x<b.x || a.y<b.y; }
v2  v2_clamp(v2 min, v2 v, v2 max)  { return v2(Clamp(min.x, v.x, max.x), Clamp(min.y, v.y, max.y)); }
v2  v2_sign(v2 v)                   { return v2(Sign(v.x), Sign(v.y)) ; }
v2  v2_rand_rng(v2 a, v2 b)         { return v2(rand_rng_f32(a.x, b.x), rand_rng_f32(a.y, b.y)); }

f32 v2_length(v2 v)                             { return Sqrt(v2_length_sqr(v)); }
f32 v2_length_sqr(v2 v)                         { return Square(v.x) + Square(v.y); }
v2  v2_norm(v2 v)                               { return v * (1.0f/v2_length(v)); }
f32 v2_distance(v2 a, v2 b)                     { return v2_length(a - b); }
f32 v2_distance_sqr(v2 a, v2 b)                 { return v2_length_sqr(a - b); }
f32 v2_dot(v2 a, v2 b)                          { return a.x*b.x + a.y*b.y; }
f32 v2_cross(v2 a, v2 b)                        { return a.x*b.y - a.y*b.x; }
v2  v2_lerp(v2 a, f32 t, v2 b)                  { return v2(Lerp(a.x, t, b.x), Lerp(a.y, t, b.y));}
v2  v2_hadamard(v2 a, v2 b)                     { return v2(a.x*b.x, a.y*b.y); }
v2  v2_hadamard_div(v2 a, v2 b)                 { return v2(a.x/b.x, a.y/b.y); }
v2  v2_project(v2 a, v2 b)                      { return v2_dot(a, b) / v2_length_sqr(b) * b; }
v2  v2_project_on_unit(v2 a, v2 b)              { return v2_dot(a, b) * b; }
f32 v2_length_projection(v2 a, v2 b)            { return v2_dot(a, v2_norm(b)); }
v2  v2_reject(v2 a, v2 b)                       { return a - v2_project(a, b); }
v2  v2_reflect(v2 v, v2 normal)                 { return v - 2*normal*v2_dot(v, normal); }
f32 v2_angle(v2 a, v2 b)                        { return Atan2(v2_cross(a, b), v2_dot(a, b)); }

v2  v2_flip_y(v2 v)                             { return v2(v.x, -v.y); }
v2  v2_remap_01_to_11(v2 pos, v2 range)         { return v2(2 * (pos.x/range.x) - 1, 2 * (pos.y/range.y) - 1); }
f32 v2_line_angle(v2 start, v2 end)             { v2 dir = end - start; return Atan2(dir.y, dir.x); }
v2  v2_rotate_90(v2 v)                          { return v2(-v.y, v.x); }
v2  v2_rotate_negative_90(v2 v)                 { return v2(v.y, -v.x); }
v2  v2_rotate(v2 v, f32 sine, f32 cosine)       { return v2(v.x*cosine - v.y*sine, v.x*sine + v.y*cosine); }
v2  v2_rotate(v2 v, f32 rad)                    { f32 s; f32 c; SinCos(rad, &s, &c); return v2_rotate(v, s, c);}
v2  v2_rotate_relative(v2 v, v2 pivot, f32 rad) { return v2_rotate(v - pivot, rad) + pivot; }

v2 v2_step_to(v2 v, v2 target, f32 step) {
  v2 d = target - v;
  f32 sqr_dist = v2_length_sqr(d);
  if ((sqr_dist == 0) || ((step >= 0) && (sqr_dist <= Square(step)))) return target;
  return v + v2_norm(d)*step;
}
v2 v2_clamp_length(f32 min, v2 v, f32 max) {
  v2 res = v;
  f32 len = v2_length_sqr(v);
  if (len > 0.0f) {
    len = Sqrt(len);
    f32 scale = 1;
    if (len < min) scale = min/len;
    else if (len > min) scale = max/len;
    res *= scale;
  }
  return res;
}

////////////////////////////////////////////////////////////////////////
// Vector3

v3  operator+(v3 a, v3 b)          { return v3(a.x + b.x, a.y + b.y, a.z + b.z); }
v3  operator-(v3 a, v3 b)          { return v3(a.x - b.x, a.y - b.y, a.z - b.z); }
v3  operator*(v3 v, f32 scalar)    { return v3(v.x*scalar, v.y*scalar, v.z*scalar); }
v3  operator*(f32 scalar, v3 v)    { return v3(v.x*scalar, v.y*scalar, v.z*scalar); }
v3  operator/(v3 v, f32 scalar)    { return v3(v.x/scalar, v.y/scalar, v.z/scalar); }
v3  operator+=(v3& a, v3 b)        { return a = a + b; }
v3  operator-=(v3& a, v3 b)        { return a = a - b; }
v3  operator*=(v3& v, f32 scalar)  { return v = v * scalar; }
v3  operator/=(v3& v, f32 scalar)  { return v = v / scalar; }
b32 operator==(v3 a, v3 b)         { return (Abs(a.x - b.x) <= FloatEpsilon) && (Abs(a.y - b.y) <= FloatEpsilon) && (Abs(a.z - b.z) <= FloatEpsilon); }
b32 operator!=(v3 a, v3 b)         { return !(a == b); }
v3  operator-(v3 v)                { return v3(-v.x, -v.y, -v.z); }

b32 operator==(v3u a, v3u b)       { return a.x == b.x && a.y == b.y && a.z == b.z; }

v3  v3_of_v4(v4 v)                  { return v3(v.x, v.y, v.z); }
v4  v3_to_v4(v3 v, f32 a)           { return v4(v.x, v.y, v.z, a); }
v3  v3_up()                         { return v3(0.0f, 1.0f, 0.0f); }
v3  v3_down()                       { return v3(0.0f, -1.0f, 0.0f); }
v3  v3_left()                       { return v3(-1.0f, 0.0f, 0.0f); }
v3  v3_right()                      { return v3(1.0f, 0.0f, 0.0f); }
v3  v3_forward()                    { return v3(0.0f, 0.0f, 1.0f); }
v3  v3_back()                       { return v3(0.0f, 0.0f, -1.0f); }
v3  v3_zero()                       { return v3{}; }
v3  v3_one()                        { return v3(1, 1, 1); }
v3  v3_splat(f32 x)                 { return v3(x, x, x);}
v3  v3_add_scalar(v3 v, f32 x)      { return v3(v.x+x, v.y+x, v.z+x); }
v3  v3_subtract_scalar(v3 v, f32 x) { return v3_add_scalar(v, -x); }
v3  v3_min(v3 a, v3 b)              { return v3(Min(a.x,b.x), Min(a.y,b.y), Min(a.z,b.z)); }
v3  v3_max(v3 a, v3 b)              { return v3(Max(a.x,b.x), Max(a.y,b.y), Max(a.z,b.z)); }
v3  v3_invert(v3 v)                 { return v3(1.0f/v.x, 1.0f/v.y, 1.0f/v.z); }
v3b v3_greater(v3 a, v3 b)          { return v3b(a.x>b.x, a.y>b.y, a.z>b.z); }
v3b v3_less(v3 a, v3 b)             { return v3b(a.x<b.x, a.y<b.y, a.z<b.z); }
b32 v3_greater_any(v3 a, v3 b)      { return a.x>b.x || a.y>b.y || a.z>b.z; }
b32 v3_less_any(v3 a, v3 b)         { return a.x<b.x || a.y<b.y || a.z<b.z; }
v3  v3_clamp(v3 min, v3 v, v3 max)  { return v3(Clamp(min.x, v.x, max.x), Clamp(min.y, v.y, max.y), Clamp(min.z, v.z, max.z)); }
v3  v3_sign(v3 v)                   { return v3(Sign(v.x), Sign(v.y), Sign(v.z)); }
v3  v3_rand_rng(v3 a, v3 b)         { return v3(rand_rng_f32(a.x, b.x), rand_rng_f32(a.y, b.y), rand_rng_f32(a.z, b.z)); }

f32 v3_length(v3 v)                  { return Sqrt(v3_length_sqr(v)); }
f32 v3_length_sqr(v3 v)              { return Square(v.x) + Square(v.y) + Square(v.z); }
v3  v3_norm(v3 v)                    { return v * (1.0f/v3_length(v)); }
f32 v3_distance(v3 a, v3 b)          { return v3_length(a - b); }
f32 v3_distance_sqr(v3 a, v3 b)      { return v3_length_sqr(a - b); }
f32 v3_dot(v3 a, v3 b)               { return a.x*b.x + a.y*b.y + a.z*b.z; }
v3  v3_cross(v3 a, v3 b)             { return v3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }
v3  v3_lerp(v3 a, f32 t, v3 b)       { return v3(Lerp(a.x, t, b.x), Lerp(a.y, t, b.y), Lerp(a.z, t, b.z)); }
v3  v3_hadamard(v3 a, v3 b)          { return v3(a.x*b.x, a.y*b.y, a.z*b.z); }
v3  v3_hadamard_div(v3 a, v3 b)      { return v3(a.x/b.x, a.y/b.y, a.z/b.z); }
v3  v3_project(v3 a, v3 b)           { return v3_dot(a, b) / v3_length_sqr(b) * b; }
v3  v3_project_on_unit(v3 a, v3 b)   { return v3_dot(a, b) * b; }
f32 v3_length_projection(v3 a, v3 b) { return v3_dot(a, v3_norm(b)); }
v3  v3_reject(v3 a, v3 b)            { return a - v3_project(a, b); }
v3  v3_reject_on_unit(v3 a, v3 b)    { return a - v3_project_on_unit(a, b); }
v3  v3_reflect(v3 v, v3 n)           { return v - 2*n*v3_dot(v, n); }
f32 v3_angle(v3 a, v3 b)             { return Atan2(v3_length(v3_cross(a, b)), v3_dot(a, b)); }

v3 v3_step_to(v3 v, v3 target, f32 step) {
  v3 d = target - v;
  f32 sqr_dist = v3_length_sqr(d);
  if ((sqr_dist == 0) || ((step >= 0) && (sqr_dist <= Square(step)))) return target;
  return v + v3_norm(d)*step;
}
v3 v3_clamp_length(f32 min, v3 v, f32 max) {
  v3 res = v;
  f32 len = v3_length_sqr(v);
  if (len > 0.0f) {
    len = Sqrt(len);
    f32 scale = 1;
    if (len < min) scale = min/len;
    else if (len > min) scale = max/len;
    res *= scale;
  }
  return res;
}

v3  v3_pos_of_mat4(mat4 mat)        { return v3_of_v4(mat.w); };
v3  v3_rotate_x(v3 v, f32 rad)      { v2 r = v2_rotate(v2(v.y, v.z), rad); return v3(v.x, r.v[0], r.v[1]); }
v3  v3_rotate_y(v3 v, f32 rad)      { v2 r = v2_rotate(v2(v.x, v.z), rad); return v3(r.v[0], v.y, r.v[1]); }
v3  v3_rotate_z(v3 v, f32 rad)      { v2 r = v2_rotate(v2(v.x, v.y), rad); return v3(r.v[0], r.v[1], v.z); }

v3 v3_barycentric(v3 v, v3 a, v3 b, v3 c) {
  v3 v0 = b-a;
  v3 v1 = c-a;
  v3 v2 = v-a;
  f32 d00 = v3_dot(v0,v0);
  f32 d01 = v3_dot(v0,v1);
  f32 d11 = v3_dot(v1,v1);
  f32 d20 = v3_dot(v2,v0);
  f32 d21 = v3_dot(v2,v1);
  f32 denom = d00 * d11 - d01 * d01;
  v3 res;
  res.y = (d11 * d20 - d01 * d21) / denom;
  res.z = (d00 * d21 - d01 * d20) / denom;
  res.x = 1.0f - (res.z + res.y);
  return res;
}
v3 v3_perpendicular(v3 v) {
  v3 cardinal_axis = v3(1,0,0);
  f32 min = Abs(v.x);
  if (Abs(v.y) < min) {
    min = Abs(v.y);
    cardinal_axis = v3(0,1,0);
  }
  if (Abs(v.z) < min) {
    min = Abs(v.z);
    cardinal_axis = v3(0,0,1);
  }
  return v3_cross(v, cardinal_axis);
}
v3 v3_rotate_around_axis(v3 v, v3 axis, f32 rad) {
  axis = v3_norm(axis);
  v3 proj = v3_project_on_unit(v, axis);
  v3 reject = v3_reject_on_unit(v, axis);
  v3 cross = v3_cross(axis, v);
  f32 sine;
  f32 cosine;
  SinCos(rad, &sine, &cosine);
  return proj + reject*cosine + cross*sine;
}
v3 v3_refract(v3 v, v3 n, f32 r) {
  v3 res = {};
  f32 v_norm_len = v3_dot(v, n); // v and n are normalized
  f32 v_tangent_len_sqr = 1 - Square(v_norm_len);
  f32 r_tangent_len_sqr = Square(r)*(v_tangent_len_sqr); // Snell's law: n1sin(theta1) = n2sin(theta2), r = n1/n2, v_tangent_len = sin(theta), r_tangent = r*v_tangent
  f32 r_norm_len_sqr = 1 - r_tangent_len_sqr;
  if (r_norm_len_sqr < 0) {
    return v3_reflect(v, n);
  }
  f32 r_norm_len = Sqrt(r_norm_len_sqr);
  res = r*v - (r*v_norm_len + r_norm_len)*n;
  return res;
}

////////////////////////////////////////////////////////////////////////
// Vector4

v4  operator+(v4 a, v4 b)          { return v4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
v4  operator-(v4 a, v4 b)          { return v4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
v4  operator*(v4 v, f32 scalar)    { return v4(v.x*scalar, v.y*scalar, v.z*scalar, v.w*scalar); }
v4  operator*(f32 scalar, v4 v)    { return v4(v.x*scalar, v.y*scalar, v.z*scalar, v.w*scalar); }
v4  operator/(v4 v, f32 scalar)    { return v4(v.x/scalar, v.y/scalar, v.z/scalar, v.w/scalar); }
v4  operator+=(v4& a, v4 b)        { return a = a + b; }
v4  operator-=(v4& a, v4 b)        { return a = a - b; }
v4  operator*=(v4& v, f32 scalar)  { return v = v * scalar; }
v4  operator/=(v4& v, f32 scalar)  { return v = v / scalar; }
b32 operator==(v4 a, v4 b)         { return (Abs(a.x - b.x) <= FloatEpsilon) && (Abs(a.y - b.y) <= FloatEpsilon) && (Abs(a.z - b.z) <= FloatEpsilon) && (Abs(a.w - b.w) <= FloatEpsilon); }
b32 operator!=(v4 a, v4 b)         { return !(a == b); }
v4  operator-(v4 v)                { return v4(-v.x, -v.y, -v.z, -v.w); }

v4 v4_zero()                       { return v4{}; }
v4 v4_one()                        { return v4(1, 1, 1, 1); }
v4 v4_set_w(v4 v, f32 w)           { return v4(v.x, v.y, v.x, w); }
v4 v4_splat(f32 x)                 { return v4(x,x,x,x); }
v4 v4_add_scalar(v4 v, f32 x)      { return v4(v.x+x,v.y+x,v.z+x,v.w+x); }
v4 v4_subtract_scalar(v4 v, f32 x) { return v4_add_scalar(v, -x); }
v4 v4_min(v4 a, v4 b)              { return v4(Min(a.x,b.x), Min(a.y,b.y), Min(a.z,b.z), Min(a.w,b.w)); }
v4 v4_max(v4 a, v4 b)              { return v4(Max(a.x,b.x), Max(a.y,b.y), Max(a.z,b.z), Max(a.w,b.w)); }
v4 v4_invert(v4 v)                 { return v4(1.0f/v.x,1.0f/v.y,1.0f/v.z,1.0f/v.w); }

f32 v4_length(v4 v)                { return Sqrt(v4_length_squared(v)); }
f32 v4_length_squared(v4 v)        { return Square(v.x) + Square(v.y) + Square(v.z) + Square(v.w); }
v4  v4_norm(v4 v)                  { return v * (1.0f / v4_length(v)); }
f32 v4_distance(v4 a, v4 b)        { return v4_length(a - b); }
f32 v4_distance_sqr(v4 a, v4 b)    { return v4_length_squared(a - b); }
f32 v4_dot(v4 a, v4 b)             { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }
v4  v4_lerp(v4 a, f32 t, v4 b)     { return v4(Lerp(a.x, t, b.x), Lerp(a.y, t, b.y), Lerp(a.z, t, b.z), Lerp(a.w, t, b.w)); }
v4  v4_hadamard(v4 a, v4 b)        { return v4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w); }

////////////////////////////////////////////////////////////////////////
// Quatornion

v4 quat_identity()      { return v4(0, 0, 0, 1); }
v4 quat_norm(v4 q)      { return v4_norm(q); }
v4 quat_conjugate(v4 q) { return v4(-q.x, -q.y, -q.z, q.w); }
v4 quat_inverse(v4 q)   { return quat_norm(quat_conjugate(q)); }

v4 quat_axis_angle(v3 axis, f32 rad) {
  axis = v3_norm(axis);
  f32 s;
  f32 c;
  SinCos(rad / 2, &s, &c);
  v4 res = {axis.x*s, axis.y*s, axis.z*s, c};
  return res;
}

v3 quat_rotate(v4 q, v3 v) {
  v3 u = v3(q.x, q.y, q.z);
  f32 s = q.w;
  v3 t = 2 * v3_cross(u, v);
  // result = v + s*t + cross(u, t)
  v3 cross_ut = v3_cross(u, t);
  v3 res = {
    v.x + s * t.x + cross_ut.x,
    v.y + s * t.y + cross_ut.y,
    v.z + s * t.z + cross_ut.z
  };
  return res;
}

v4 quat_mul(v4 a, v4 b) {
  v3 vec_a = v3_of_v4(a);
  v3 vec_b = v3_of_v4(b);
  f32 scalar = a.w*b.w - v3_dot(vec_a, vec_b);
  v3 v = a.w*vec_b + b.w*vec_a + v3_cross(vec_a, vec_b);
  v4 res = v3_to_v4(v, scalar);
  return res;
}

mat3 quat_to_mat3(v4 q) {
  mat3 res;
  res.v[0][0] = 1.0f - 2.0f * (Square(q.y) + Square(q.z));
  res.v[0][1] = 2.0f * (q.x*q.y - q.w*q.z);
  res.v[0][2] = 2.0f * (q.x*q.z + q.w*q.y);
  res.v[1][0] = 2.0f * (q.x*q.y + q.w*q.z);
  res.v[1][1] = 1.0f - 2.0f * (Square(q.x) + Square(q.z));
  res.v[1][2] = 2.0f * (q.y*q.z - q.w*q.x);
  res.v[2][0] = 2.0f * (q.x*q.z - q.w*q.y);
  res.v[2][1] = 2.0f * (q.y*q.z + q.w*q.x);
  res.v[2][2] = 1.0f - 2.0f * (Square(q.x) + Square(q.y));
  return res;
}

mat4 quat_to_mat4(v4 q) {
  mat3 r = quat_to_mat3(q);
  mat4 res = mat4_identity();
  res.v[0][0] = r.v[0][0];
  res.v[0][1] = r.v[0][1];
  res.v[0][2] = r.v[0][2];
  res.v[1][0] = r.v[1][0];
  res.v[1][1] = r.v[1][1];
  res.v[1][2] = r.v[1][2];
  res.v[2][0] = r.v[2][0];
  res.v[2][1] = r.v[2][1];
  res.v[2][2] = r.v[2][2];
  return res;
}

v4 quat_slerp(v4 a, f32 t, v4 b) {
  f32 dot = v4_dot(a, b);

  // take shortest path
  if (dot < 0.0f) {
    b = -b;
    dot = -dot;
  }

  // if very close → use lerp (avoid division by zero)
  if (dot > 0.9995f) {
    v4 r = {
      a.x + t * (b.x - a.x),
      a.y + t * (b.y - a.y),
      a.z + t * (b.z - a.z),
      a.w + t * (b.w - a.w)
    };
    // important: renormalize
    r = v4_norm(r);
    return r;
  }

  f32 theta = Acos(dot);
  f32 sin_theta = Sin(theta);
  f32 w1 = Sin((1 - t) * theta) / sin_theta;
  f32 w2 = Sin(t * theta) / sin_theta;
  v4 res = {
    a.x * w1 + b.x * w2,
    a.y * w1 + b.y * w2,
    a.z * w1 + b.z * w2,
    a.w * w1 + b.w * w2
  };
  return res;
}

v4 quat_nlerp(v4 a, f32 t, v4 b) {
  if (v4_dot(a, b) < 0) {
    b = -b;
  }
  v4 res = quat_norm(v4_lerp(a, t, b));
  return res;
}

v4 quat_from_to(v3 a, v3 b) {
  a = v3_norm(a);
  b = v3_norm(b);
  f32 dot = v3_dot(a, b);
  if (dot > 0.9999f)
    return quat_identity();
  if (dot < -0.9999f) {
    // we do this since v3_cross(a, -a) == v3_zero() and quat will be invalid
    v3 axis = v3_cross(v3(1, 0, 0), a);
    if (v3_dot(axis, axis) < 0.0001f) {
      axis = v3_cross(v3(0, 1, 0), a);
    }
    axis = v3_norm(axis);
    return quat_axis_angle(axis, PI);
  }
  v3 c = v3_cross(a, b);
  v4 res = quat_norm(v4(c.x, c.y, c.z, 1.0f + dot));
  return res;
}

v4 quat_from_euler(v3 e) {
  f32 cy, cx, cz;
  f32 sy, sx, sz;
  SinCos(e.y/2, &sy, &cy);
  SinCos(e.x/2, &sx, &cx);
  SinCos(e.z/2, &sz, &cz);
  v4 qy = v4(0, sy, 0, cy);
  v4 qx = v4(sx, 0, 0, cx);
  v4 qz = v4(0, 0, sz, cz);
  v4 res = quat_mul(qy, quat_mul(qx, qz));
  return quat_norm(res);
}

////////////////////////////////////////////////////////////////////////
// Matrix3

mat3 operator*(mat3 a, mat3 b) {
  mat3 res = {};
  Loop (j, 3) {
    Loop (i, 3) {
      res.v[j][i] = b.v[j][0] * a.v[0][i] +
                    b.v[j][1] * a.v[1][i] +
                    b.v[j][2] * a.v[2][i];
    }
  }
  return res;
}
mat3& operator*=(mat3& a, mat3 b) { return a = b * a; }

v3 operator*(mat3 mat, v3 vec) {
  v3 res = {
    mat.v[0][0]*vec.x + mat.v[0][1]*vec.y + mat.v[0][2]*vec.z,
    mat.v[1][0]*vec.x + mat.v[1][1]*vec.y + mat.v[1][2]*vec.z,
    mat.v[2][0]*vec.x + mat.v[2][1]*vec.y + mat.v[2][2]*vec.z,
  };
  return res;
}

mat3 mat3_identity() {
  mat3 result = {
    1,0,0,
    0,1,0,
    0,0,1
  };
  return result;
}

mat3 mat3_translate(v2 pos) {
  mat3 res = {
    1,     0,     0,
    0,     1,     0,
    pos.x, pos.y, 1
  };
  return res;
}

mat3 mat3_scale(v2 scale) {
  mat3 res = {
    scale.x, 0,       0,
    0,       scale.y, 0,
    0,       0,       1
  };
  return res;
}

////////////////////////////////////////////////////////////////////////
// Matrix4

mat4 operator*(mat4 a, mat4 b) {
  mat4 res = {};
  Loop (j, 4) {
    Loop (i, 4) {
      res.v[j][i] = b.v[j][0] * a.v[0][i] +
                    b.v[j][1] * a.v[1][i] +
                    b.v[j][2] * a.v[2][i] +
                    b.v[j][3] * a.v[3][i];
    }
  }
  return res;
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

mat4 mat4_identity() {
  mat4 res = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
  };
  return res;
}

mat4 mat4_translate(v3 pos) {
  mat4 res = {
    1,     0,     0,     0,
    0,     1,     0,     0,
    0,     0,     1,     0,
    pos.x, pos.y, pos.z, 1
  };
  return res;
}

mat4 mat4_scale(v3 scale) {
  mat4 res = {
    scale.x, 0,       0,       0,
    0,       scale.y, 0,       0,
    0,       0,       scale.z, 0,
    0,       0,       0,       1
  };
  return res;
}

mat4 mat4_scale_all_elements(mat4 mat, f32 scale) {
  Loop (row, 4) {
    Loop (col, 4) {
      mat.v[row][col] *= scale;
    }
  }
  return mat;
}

mat4 mat4_rotate_x(f32 rad) {
  f32 sin, cos;
  SinCos(rad, &sin, &cos);
  mat4 mat = {
    1,   0,    0,   0,
    0, cos,  sin,   0,
    0,-sin,  cos,   0,
    0,   0,    0,   1,
  };
  return mat;
}

mat4 mat4_rotate_y(f32 rad) {
  f32 sin, cos;
  SinCos(rad, &sin, &cos);
  mat4 mat = {
    cos, 0,  -sin, 0,
    0,   1,   0,   0,
    sin, 0,   cos, 0,
    0,   0,   0,   1
  };
  return mat;
}

mat4 mat4_rotate_z(f32 rad) {
  f32 sin, cos;
  SinCos(rad, &sin, &cos);
  mat4 mat = {
    cos, sin, 0,   0,
   -sin, cos, 0,   0,
    0,   0,   1,   0,
    0,   0,   0,   1
  };
  return mat;
}

mat4 mat4_rotate_xyz(v3 rad) {
  mat4 rx = mat4_rotate_x(rad.x);
  mat4 ry = mat4_rotate_y(rad.y);
  mat4 rz = mat4_rotate_z(rad.z);
  mat4 res = rz * ry * rx;
  return res;
}

mat4 mat4_rotate_around_axis(v3 axis, f32 rad) {
  f32 len_sqr = v3_length_sqr(axis);
  if ((len_sqr != 1.0f) && (len_sqr != 0.0f)) {
    axis = v3_norm(axis);
  }
  f32 sine;
  f32 cosine;
  SinCos(rad, &sine, &cosine);
  f32 x = axis.x, y = axis.y, z = axis.z;
  f32 t = 1.0f - cosine;
  mat4 res = {
    x*x*t + cosine, y*x*t + z*sine, z*x*t - y*sine, 0,
    x*y*t - z*sine, y*y*t + cosine, z*y*t + x*sine, 0,
    x*z*t + y*sine, y*z*t - x*sine, z*z*t + cosine, 0,
    0,              0,              0,              1,
  };
  return res;
}

mat4 mat4_transform(v3 pos, v3 rot, v3 scale) {
  mat4 res = mat4_translate(pos) * mat4_rotate_xyz(rot) * mat4_scale(scale);
  return res;
}
mat4 mat4_transform(Transform trans) {
  mat4 res = mat4_transform(trans.pos, trans.rot, trans.scale);
  return res;
}

mat4 mat4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) {
  mat4 res = {
    2/(right - left), 0,                0,                  0,
    0,                2/(top - bottom), 0,                  0,
    0,                0,                1/(far - near),     0,
    0,                0,                -near/(far - near), 1
  };
  return res;
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
  mat4 res = {
    1/(fov*aspect_ratio), 0,                0,                             0,
    0,                    1/(fov),          0,                             0,
    0,                    0,                -(far + near)/(far - near),   -1,
    0,                    0,                (-2.0f*far*near)/(far - near), 0
  };
  return res;
}

// NOTE: we negate Z direction since camera is looking at -Z direction
mat4 mat4_look_at(v3 pos, v3 dir, v3 up) {
  v3 z = -v3_norm(dir);
  v3 x = v3_norm(v3_cross(up, z));
  v3 y = v3_cross(z, x);
  mat4 res = {
    x.x, y.x, z.x, 0,
    x.y, y.y, z.y, 0,
    x.z, y.z, z.z, 0,
   -v3_dot(x, pos), -v3_dot(y, pos), -v3_dot(z, pos), 1
  };
  return res;
}

mat4 mat4_transpose(mat4 mat) {
  mat4 res = {
    mat.v[0][0], mat.v[1][0], mat.v[2][0], mat.v[3][0],
    mat.v[0][1], mat.v[1][1], mat.v[2][1], mat.v[3][1],
    mat.v[0][2], mat.v[1][2], mat.v[2][2], mat.v[3][2],
    mat.v[0][3], mat.v[1][3], mat.v[2][3], mat.v[3][3],
  };
  return res;
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
  
  return mat4_scale_all_elements(inverse, one_over_det);
}

////////////////////////////////////////////////////////////////////////
// Misc

Ray ray_make(v3 pos, v3 dir) { return Ray(pos, dir); }
Ray ray_from_screen(v2 screen_pos, v2u viewport_rect, v3 origin, mat4 view, mat4 projection) {
  v2 mouse_pos = screen_pos;
  v2u win_size = viewport_rect;
  v2 norm_coords = v2(2 * (mouse_pos.x/win_size.x) - 1, 2 * -(mouse_pos.y/win_size.y) + 1);
  v4 clip_coords = v4(norm_coords.x, norm_coords.y, -1, 1);
  v4 eye_coord = mat4_inverse(projection) * clip_coords;
  eye_coord = v4(eye_coord.x, eye_coord.y, -1, 0);
  v3 world_coord = v3_of_v4(view * eye_coord);
  world_coord = v3_norm(world_coord);
  return ray_make(origin, world_coord);
}
v2 world_to_screen(v3 pos, v3 camera_pos, mat4 view, mat4 projection) {

  return {};
}

////////////////////////////////////////////////////////////////////////
// Range Ops

///////////////////////////////////
// Dim1
Rng1u rng1u_shift(Rng1u r, u32 x)       { return Rng1u(r.min + x, r.max + x); }
Rng1u rng1u_pad(Rng1u r, u32 x)         { return Rng1u(r.min - x, r.max + x); }
u32 rng1u_center(Rng1u r)               { return (r.min+r.max)/2; }
b32 rng1u_contains(Rng1u r, u32 x)      { return (r.min <= x && x < r.max); }
u32 rng1u_dim(Rng1u r)                  { return r.max - r.min; }
Rng1u rng1u_union(Rng1u a, Rng1u b)     { return Rng1u(Min(a.min, b.min), Max(a.max, b.max)); }
Rng1u rng1u_intersect(Rng1u a, Rng1u b) { return Rng1u(Max(a.min, b.min), Min(a.max, b.max)); }
u32 rng1u_clamp(Rng1u r, u32 x)         { return Clamp(r.min, x, r.max); }

Rng1i rng1i_shift(Rng1i r, i32 x)       { return Rng1i(r.min + x, r.max + x); }
Rng1i rng1i_pad(Rng1i r, i32 x)         { return Rng1i(r.min - x, r.max + x); }
i32 rng1i_center(Rng1i r)               { return (r.min+r.max)/2; }
b32 rng1i_contains(Rng1i r, i32 x)      { return (r.min <= x && x < r.max); }
i32 rng1i_dim(Rng1i r)                  { return r.max - r.min; }
Rng1i rng1i_union(Rng1i a, Rng1i b)     { return Rng1i(Min(a.min, b.min), Max(a.max, b.max)); }
Rng1i rng1i_intersect(Rng1i a, Rng1i b) { return Rng1i(Max(a.min, b.min), Min(a.max, b.max)); }
i32 rng1i_clamp(Rng1i r, i32 x)         { return Clamp(r.min, x, r.max); }

Rng1u64 rng1u64_shift(Rng1u64 r, u64 x)         { return Rng1u64(r.min + x, r.max + x); }
Rng1u64 rng1u64_pad(Rng1u64 r, u64 x)           { return Rng1u64(r.min - x, r.max + x); }
u64 rng1u64_center(Rng1u64 r)                   { return (r.min+r.max)/2; }
b32 rng1u64_contains(Rng1u64 r, u64 x)          { return (r.min <= x && x < r.max); }
u64 rng1u64_dim(Rng1u64 r)                      { return r.max - r.min; }
Rng1u64 rng1u64_union(Rng1u64 a, Rng1u64 b)     { return Rng1u64(Min(a.min, b.min), Max(a.max, b.max)); }
Rng1u64 rng1u64_intersect(Rng1u64 a, Rng1u64 b) { return Rng1u64(Max(a.min, b.min), Min(a.max, b.max)); }
u64 rng1u64_clamp(Rng1u64 r, u64 x)             { return Clamp(r.min, x, r.max); }

Rng1 rng1_shift(Rng1 r, f32 x)         { return Rng1(r.min + x, r.max + x); }
Rng1 rng1_pad(Rng1 r, f32 x)           { return Rng1(r.min - x, r.max + x); }
f32 rng1_center(Rng1 r)                { return (r.min+r.max)/2; }
b32 rng1_contains(Rng1 r, f32 x)       { return (r.min <= x && x < r.max); }
f32 rng1_dim(Rng1 r)                   { return r.max - r.min; }
Rng1 rng1_union(Rng1 a, Rng1 b)        { return Rng1(Min(a.min, b.min), Max(a.max, b.max)); }
Rng1 rng1_intersect(Rng1 a, Rng1 b)    { return Rng1(Max(a.min, b.min), Min(a.max, b.max)); }
f32 rng1_clamp(Rng1 r, f32 x)          { return Clamp(r.min, x, r.max); }

Rng1 rng1_subrng(Rng1 r, Rng1 sub)   { return Rng1(r.min + sub.min, r.min+sub.min + rng1_dim(sub)); }
Rng1 rng1_subrng01(Rng1 r, Rng1 sub) { f32 w = rng1_dim(r); return Rng1(r.min + w*sub.min, r.min + w*sub.max); }
f32 rng1_lerp(Rng1 r, f32 t)         { return Lerp(r.min, t, r.max); } 
f32 rng1_unlerp(Rng1 r, f32 x)       { return Unlerp(r.min, x, r.max); }

///////////////////////////////////
// Dim2
Rng2 rng2_shift(Rng2 r, v2 x)       { return Rng2(r.min + x, r.max + x);}
Rng2 rng2_pad(Rng2 r, f32 x)        { return Rng2(r.min - v2_splat(x), r.max + v2_splat(x));}
v2 rng2_center(Rng2 r)              { return v2((r.min + r.max)/2); }
b32 rng2_contains(Rng2 r, v2 x)     { return (r.min.x <= x.x && x.x < r.max.x && r.min.y <= x.y && x.y < r.max.y); }
v2 rng2_dim(Rng2 r)                 { return v2(r.max.x - r.min.x, r.max.y - r.min.y); }
Rng2 rng2_union(Rng2 a, Rng2 b)     { return Rng2(v2(Min(a.min.x, b.min.x), Min(a.min.y, b.min.y)), v2(Max(a.max.x, b.max.x), Max(a.max.y, b.max.y))); }
Rng2 rng2_intersect(Rng2 a, Rng2 b) { return Rng2(v2(Max(a.min.x, b.min.x), Max(a.min.y, b.min.y)), v2(Min(a.max.x, b.max.x), Min(a.max.y, b.max.y))); }
v2 rng2_clamp(Rng2 r, v2 x)         { return v2(Clamp(r.min.x, x.x, r.max.x), Clamp(r.min.y, x.y, r.max.y)); }

Rng2 rng2_make(v2 min, v2 size)             { return Rng2(min, min+size); }
Rng2 rng2_make_centered(v2 pos, v2 halfdim) { return Rng2(pos - halfdim, pos + halfdim); }
Rng2 rng2_scale_centered(Rng2 r, f32 scale) { v2 halfdim = rng2_dim(r)/2; v2 c = rng2_center(r); return Rng2(c - halfdim*scale, c + halfdim*scale); }
Rng2 rng2_scale_centered(Rng2 r, v2 scale)  { v2 halfdim = rng2_dim(r)/2; v2 c = rng2_center(r); return Rng2(c - v2_hadamard(halfdim, scale), c + v2_hadamard(halfdim, scale)); }
Rng2 rng2_scale(Rng2 r, f32 scale)          { return Rng2(r.min*scale, r.max*scale); }
Rng2 rng2_scale(Rng2 r, v2 scale)           { return Rng2(v2_hadamard(r.min, scale), v2_hadamard(r.max, scale)); }

Rng2 rng2_subrng_x(Rng2 r, Rng1 sub)      { return Rng2(v2(r.min.x + sub.min, r.min.y), v2(r.min.x+sub.min + rng1_dim(sub), r.max.y)); }
Rng2 rng2_subrng_y(Rng2 r, Rng1 sub)      { return Rng2(v2(r.min.x, r.min.y + sub.min), v2(r.min.x, r.max.y+sub.min + rng1_dim(sub))); }
Rng2 rng2_subrng_x01(Rng2 r, Rng1 sub)    { f32 w = rng2_dim(r).x; return Rng2(v2(r.min.x + w*sub.min, r.min.y), v2(r.min.x + w*sub.max, r.max.y)); }
Rng2 rng2_subrng_y01(Rng2 r, Rng1 sub)    { f32 w = rng2_dim(r).y; return Rng2(v2(r.min.x, r.min.y + w*sub.min), v2(r.min.x, r.max.y + w*sub.max)); }

Rng2 rng2_align_dim_at_center(Rng2 r, v2 size) { v2 c = rng2_center(r); v2 half = size/2; return Rng2(c - half, c + half); }
// NOTE: define prefix, postfix, skip, chop operations?

///////////////////////////////////
// Dim3
Rng3 rng3_shift(Rng3 r, v3 x)    { return Rng3(r.min + x, r.max + x); }
Rng3 rng3_pad(Rng3 r, f32 x)     { return Rng3(r.min - v3_splat(x), r.max + v3_splat(x)); }
v3 rng3_center(Rng3 r)           { return v3((r.min.x + r.max.x)/2, (r.min.y + r.max.y)/2, (r.min.z + r.max.z)/2); }
b32 rng3_contains(Rng3 r, v3 x)  { return (r.min.x <= x.x && x.x < r.max.x && r.min.y <= x.y && x.y < r.max.y && r.min.z <= x.z && x.z < r.max.z); }
v3 rng3_dim(Rng3 r)              { return v3(r.max.x - r.min.x, r.max.y - r.min.y, r.max.z - r.min.z); }
Rng3 rng3_union(Rng3 a, Rng3 b) {
  return Rng3(v3(Min(a.min.x, b.min.x), Min(a.min.y, b.min.y), Min(a.min.z, b.min.z)),
              v3(Max(a.max.x, b.max.x), Max(a.max.y, b.max.y), Max(a.max.z, b.max.z)));
}
Rng3 rng3_intersect(Rng3 a, Rng3 b) {
  return Rng3(v3(Max(a.min.x, b.min.x), Max(a.min.y, b.min.y), Max(a.min.z, b.min.z)),
              v3(Min(a.max.x, b.max.x), Min(a.max.y, b.max.y), Min(a.max.z, b.max.z)));
}
v3 rng3_clamp(Rng3 r, v3 x) { return v3(Clamp(r.min.x, x.x, r.max.x), Clamp(r.min.y, x.y, r.max.y), Clamp(r.min.z, x.z, r.max.z)); }

Rng3 rng3_make(v3 min, v3 size)             { return Rng3(min, min+size); }
Rng3 rng3_make_centered(v3 pos, v3 halfdim) { return Rng3(pos - halfdim, pos + halfdim); }
Rng3 rng3_scale_centered(Rng3 r, f32 scale) { v3 halfdim = rng3_dim(r)/2; v3 c = rng3_center(r); return Rng3(c - halfdim*scale, c + halfdim*scale); }
Rng3 rng3_scale_centered(Rng3 r, v3 scale)  { v3 halfdim = rng3_dim(r)/2; v3 c = rng3_center(r); return Rng3(c - v3_hadamard(halfdim, scale), c + v3_hadamard(halfdim, scale)); }
Rng3 rng3_scale(Rng3 r, f32 scale)          { return Rng3(r.min*scale, r.max*scale); }
Rng3 rng3_scale(Rng3 r, v3 scale)           { return Rng3(v3_hadamard(r.min, scale), v3_hadamard(r.max, scale)); }

Rng2 layout_row(Rng2Cursor& c, Rng1 x, f32 h) {
  Rng2 r = {
    v2(x.min, c.pos.y),
    v2(x.max, c.pos.y + h)
  };
  c.pos.y += h;
  return r;
}

void layout_next(Rng2Cursor& c, f32 h) {
  c.pos.y += h;
}



