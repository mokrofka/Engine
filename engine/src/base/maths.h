#pragma once
#include "defines.h"
#include "math_types.h"

#define PI            3.14159265358f
#define Two_PI        6.28318530718f
#define Half_PI       0.5f  * PI
#define Quarter_PI    0.25f * PI
#define OneOver_PI    1.0f  / PI
#define OneOverTwo_PI 1.0f  / Two_PI

#define SqrtTwo          1.41421356237f
#define SqrtThree        1.73205080756f
#define SqrtOneOverTwo   0.70710678118f
#define SqrtOneOverThree 0.57735026918f

#define Euler     2.71828182846f
#define GoldBig   1.61803398875f
#define GoldSmall 0.61803398875f

#define FloatEpsilon   1e-5
#define MachineEpsilon 1.1920929e-7f

#define DegToRad_Multiplier PI / 180.0f
#define RadToDeg_Multiplier 180.0f / PI

INLINE f32 Sin(f32 a) {
  return __builtin_sinf(a);
}

INLINE f32 Cos(f32 a) {
  return __builtin_cosf(a);
}

INLINE f32 Tan(f32 a) {
  return __builtin_tanf(a);
}

INLINE f32 Acos(f32 a) {
  return __builtin_acos(a);
}

INLINE f32 Sqrt(f32 a) {
  return __builtin_sqrtf(a);
}

INLINE f32 Pow(f32 a, f32 b) {
  return __builtin_powf(a, b);
}

INLINE f32 Floor(f32 a) {
  return __builtin_floorf(a);
}

INLINE f32 Round(f32 a) {
  return __builtin_roundf(a);
}

INLINE f32 lerp(f32 a, f32 t, f32 b) {
  return (1 - t) * a + t * b;
}

INLINE v4 rgba_from_u32(u32 hex) {
  v4 result = v4(((hex & 0xff000000) >> 24) / 255.f,
                 ((hex & 0x00ff0000) >> 16) / 255.f,
                 ((hex & 0x0000ff00) >> 8) / 255.f,
                 ((hex & 0x000000ff) >> 0) / 255.f);
  return result;
}

INLINE u32 u32_from_rgba(v4 rgba) {
  u32 result = 0;
  result |= ((u32)((u8)(rgba.x*255.f))) << 24;
  result |= ((u32)((u8)(rgba.y*255.f))) << 16;
  result |= ((u32)((u8)(rgba.z*255.f))) <<  8;
  result |= ((u32)((u8)(rgba.w*255.f))) <<  0;
  return result;
}

INLINE f32 deg_to_rad(f32 degrees) {
  return degrees * DegToRad_Multiplier;
}

INLINE f32 rad_to_deg(f32 radians) {
  return radians * RadToDeg_Multiplier;
}

INLINE u32 xorshift32(u32* seed) {
  u32 x = *seed;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return *seed = x;
}

thread_local global u32 g_seed = 1;
INLINE u32 rand_u32() {
  u32 x = xorshift32(&g_seed);
  return x;
}

INLINE u32 rand_in_range_u32(u32 min, u32 max) {
  u32 x = xorshift32(&g_seed);
  return (x % (max - min + 1)) + min;
}

INLINE i32 rand_i32() {
  u32 x = xorshift32(&g_seed);
  return x;
}

INLINE i32 rand_in_range_i32(i32 min, i32 max) {
  u32 x = xorshift32(&g_seed);
  return min + (i32)(x % (u32)(max - min + 1));
}

INLINE f32 rand_f32_01() {
  return (f32)rand_u32() / (f32)U32_MAX;
}

INLINE f32 rand_f32_11() {
  return rand_f32_01() * 2.f - 1.f;
}

INLINE f32 rand_f32() {
  f32 normalized = (f32)rand_u32() / (f32)U32_MAX;
  return normalized * 2 * U16_MAX - U16_MAX;
}

INLINE f32 rand_in_range_f32(f32 min, f32 max) {
  return min + (max - min) * rand_f32_01();
}

INLINE b32 rand_b32() {
  return rand_u32()%2;
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Vector_2


INLINE v2 v2_zero() {
  return v2(0.0f, 0.0f);
}

INLINE v2 v2_one() {
  return v2(1.0f, 1.0f);
}

INLINE v2 v2_up() {
  return v2(0.0f, 1.0f);
}

INLINE v2 v2_down() {
  return v2(0.0f, -1.0f);
}

INLINE v2 v2_left() {
  return v2(-1.0f, 0.0f);
}

INLINE v2 v2_right() {
  return v2(1.0f, 0.0f);
}

INLINE v2 operator+(v2 a, v2 b) {
  return v2(a.x + b.x, a.y + b.y);
}

INLINE v2 operator-(v2 a, v2 b) {
  return v2(a.x - b.x, a.y - b.y);
}

INLINE v2 operator*(v2 a, f32 scalar) {
  return v2(a.x * scalar, a.y * scalar);
}

INLINE v2 operator*(f32 scalar, v2 a) {
  return v2(a.x * scalar, a.y * scalar);
}

INLINE v2 operator/(v2 a, f32 scalar) {
  return v2(a.x / scalar, a.y / scalar);
}

INLINE v2& operator+=(v2& a, v2 b) {
  a = a + b;
  return a;
}

INLINE v2& operator-=(v2& a, v2 b) {
  a = a - b;
  return a;
}

INLINE v2& operator*=(v2& a, f32 scalar) {
  a = a * scalar;
  return a;
}

INLINE v2& operator/=(v2& a, f32 scalar) {
  a = a / scalar;
  return a;
}

INLINE v2 operator-(const v2& a) {
    return v2(-a.x, -a.y);
}

INLINE f32 v2_length_squared(v2 a) {
  return Sqr(a.x) + Sqr(a.y);
}

INLINE f32 v2_length(v2 a) {
  return Sqrt(v2_length_squared(a));
}

INLINE void v2_normalize(v2* a) {
  f32 length = v2_length(*a);
  a->x /= length;
  a->y /= length;
}

INLINE v2 v2_normal(v2 a) {
  f32 length = v2_length(a);
  a.x /= length;
  a.y /= length;
  return a;
}

INLINE b32 v2_compare(v2 a, v2 b, f32 tolerance) {
  return Abs(a.x - b.x) <= tolerance &&
         Abs(a.y - b.y) <= tolerance;
}

INLINE f32 v2_distance(v2 a, v2 b) {
  v2 d = v2(a.x - b.x, a.y - b.y);
  return v2_length(d);
}

INLINE f32 v2_dot(v2 a, v2 b) {
  return a.x * b.x + a.y * b.y;
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Vector_3


INLINE v4 v3_to_v4(v3 a, f32 w) {
  return v4(a.x, a.y, a.z, w);
}

INLINE v3 v3_zero() {
  return v3(0.0f, 0.0f, 0.0f);
}

INLINE v3 v3_one() {
  return v3(1.0f, 1.0f, 1.0f);
}

INLINE v3 v3_up() {
  return v3(0.0f, 1.0f, 0.0f);
}

INLINE v3 v3_down() {
  return v3(0.0f, -1.0f, 0.0f);
}

INLINE v3 v3_left() {
  return v3(-1.0f, 0.0f, 0.0f);
}

INLINE v3 v3_right() {
  return v3(1.0f, 0.0f, 0.0f);
}

INLINE v3 v3_forward() {
  return v3(0.0f, 0.0f, -1.0f);
}

INLINE v3 v3_back() {
  return v3(0.0f, 0.0f, 1.0f);
}

INLINE v3 operator+(v3 a, v3 b) {
  return v3(a.x + b.x, a.y + b.y, a.z + b.z);
}

INLINE v3 operator-(v3 a, v3 b) {
  return v3(a.x - b.x, a.y - b.y, a.z - b.z);
}

INLINE v3 operator*(v3 a, f32 scalar) {
  return v3(a.x * scalar, a.y * scalar, a.z * scalar);
}

INLINE v3 operator*(f32 scalar, v3 a) {
  return v3(a.x * scalar, a.y * scalar, a.z * scalar);
}

INLINE v3 operator/(v3 a, f32 scalar) {
  return v3(a.x / scalar, a.y / scalar, a.z / scalar);
}

INLINE v3 operator/(f32 scalar, v3 a) {
  return v3(a.x / scalar, a.y / scalar, a.z / scalar);
}

INLINE v3& operator+=(v3& a, v3 b) {
  a = a + b;
  return a;
}

INLINE v3& operator-=(v3& a, v3 b) {
  a = a - b;
  return a;
}

INLINE v3& operator*=(v3& a, f32 scalar) {
  a = a * scalar;
  return a;
}

INLINE v3& operator/=(v3& a, f32 scalar) {
  a = a / scalar;
  return a;
}

INLINE v3 operator-(const v3& a) {
    return v3(-a.x, -a.y, -a.z);
}

INLINE f32 v3_length_squared(v3 a) {
  return Sqr(a.x) + Sqr(a.y) + Sqr(a.z);
}

INLINE f32 v3_length(v3 a) {
  return Sqrt(v3_length_squared(a));
}

INLINE void v3_normalize(v3* a) {
  f32 length = v3_length(*a);
  a->x /= length;
  a->y /= length;
  a->z /= length;
}

INLINE v3 v3_normal(v3 a) {
  f32 length = v3_length(a);
  a.x /= length;
  a.y /= length;
  a.z /= length;
  return a;
}

INLINE b32 v3_compare(v3 a, v3 b, f32 tolerance) {
  return Abs(a.x - b.x) <= tolerance &&
         Abs(a.y - b.y) <= tolerance &&
         Abs(a.z - b.z) <= tolerance;
}

INLINE b32 operator==(v3 a, v3 b) {
  return Abs(a.x - b.x) <= FloatEpsilon &&
         Abs(a.y - b.y) <= FloatEpsilon &&
         Abs(a.z - b.z) <= FloatEpsilon;
}

INLINE b32 operator!=(v3 a, v3 b) {
  return !(a == b);
}

INLINE f32 v3_distance(v3 a, v3 b) {
  v3 d = v3(a.x - b.x, a.y - b.y, a.z - b.z);
  return v3_length(d);
}

INLINE f32 v3_dot(v3 a, v3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

INLINE v3 v3_cross(v3 a, v3 b) {
  return v3(
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x
  );
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Vector_4


INLINE v3 v4_to_v3(v4 a) {
  return v3(a.x, a.y, a.z);
}

INLINE v4 v4_zero() {
  return v4(0.0f, 0.0f, 0.0f, 0.0f);
}

INLINE v4 v4_one() {
  return v4(1.0f, 1.0f, 1.0f, 1.0f);
}

INLINE v4 operator+(v4 a, v4 b) {
  return v4(a.x + b.x, a.y + b.y, a.z + b.z, a.z + b.z);
}

INLINE v4 operator-(v4 a, v4 b) {
  return v4(a.x - b.x, a.y - b.y, a.z - b.z, a.z - b.z);
}

INLINE v4& operator+=(v4& a, v4 b) {
  a = a + b;
  return a;
}

INLINE v4& operator-=(v4& a, v4 b) {
  a = a - b;
  return a;
}

INLINE v4 operator-(const v4& a) {
    return v4(-a.x, -a.y, -a.z, -a.w);
}

INLINE f32 v4_length_squared(v4 a) {
  return Sqr(a.x) + Sqr(a.y) + Sqr(a.z) + Sqr(a.w);
}

INLINE f32 v4_length(v4 a) {
  return Sqrt(v4_length_squared(a));
}

INLINE void v4_normalize(v4* a) {
  f32 length = v4_length(*a);
  a->x /= length;
  a->y /= length;
  a->z /= length;
}

INLINE v4 v4_normal(v4 a) {
  f32 length = v4_length(a);
  a.x /= length;
  a.y /= length;
  a.z /= length;
  a.w /= length;
  return a;
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Matrix4


INLINE mat4 mat4_identity() {
  mat4 matrix = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1};
  return matrix;
}

inline mat4 operator*(mat4 matrix_0, mat4 matrix_1) {
  mat4 result = {};
  Loop (row, 4) {
    Loop (col, 4) {
      result.data[row * 4 + col] =
          matrix_1.data[row * 4 + 0] * matrix_0.data[0 * 4 + col] +
          matrix_1.data[row * 4 + 1] * matrix_0.data[1 * 4 + col] +
          matrix_1.data[row * 4 + 2] * matrix_0.data[2 * 4 + col] +
          matrix_1.data[row * 4 + 3] * matrix_0.data[3 * 4 + col];
    }
  }
  return result;
}

INLINE mat4& operator*=(mat4& a, mat4 b) {
  a = b * a;
  return a;
}

inline mat4 mat4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 Near, f32 Far) {
  mat4 mat = {};
  mat.data[0*4 + 0] = 2.0f / (right - left);
  mat.data[1*4 + 1] = 2.0f / (bottom - top);
  mat.data[2*4 + 2] = 1.0f / (Far - Near);

  mat.data[3*4 + 2] = -Near / (Far - Near);

  mat.data[3*4 + 3] = 1.0f;

  return mat;
}

inline mat4 mat4_perspective1(f32 fov_radians, f32 aspect_ratio, f32 Near, f32 Far) {
  mat4 mat = {};
  mat.data[0*4 + 0] = 1.0f / (Tan(fov_radians/2.0f) * aspect_ratio);
  mat.data[1*4 + 1] = 1.0f / Tan(fov_radians/2.0f);
  mat.data[2*4 + 2] = Far / (Far - Near);
  mat.data[2*4 + 3] = 1.0f;
  mat.data[3*4 + 2] = (-Far * Near) / (Far - Near);
  
  return mat;
}

inline mat4 mat4_perspective(f32 fov_radians, f32 aspect_ratio, f32 Near, f32 Far) {
  mat4 mat = {};
  mat.data[0*4 + 0] = 1.0f / (Tan(fov_radians / 2.0f) * aspect_ratio);
  mat.data[1*4 + 1] = 1.0f / Tan(fov_radians / 2.0f);
  mat.data[2*4 + 2] = -(Far + Near) / (Far - Near); // Flip sign
  mat.data[2*4 + 3] = -1.0f; // Flip sign
  mat.data[3*4 + 2] = (-2.0f * Far * Near) / (Far - Near); // Flip sign
  
  return mat;
}

inline mat4 mat4_look_at(v3 position, v3 target, v3 up) {
  v3 z = v3_normal(target - position);
  v3 x = v3_normal(v3_cross(up, z));
  v3 y = v3_cross(z, x);
  
  mat4 camera_view = {
    x.x, y.x, z.x, 0.0f,
    x.y, y.y, z.y, 0.0f,
    x.z, y.z, z.z, 0.0f,
   -v3_dot(x, position), -v3_dot(y, position), -v3_dot(z, position), 1.0f
};

  return camera_view;
}

inline mat4 mat4_transposed(mat4 matrix) {
  mat4 out_matrix = mat4_identity();
  out_matrix.data[0] = matrix.data[0];
  out_matrix.data[1] = matrix.data[4];
  out_matrix.data[2] = matrix.data[8];
  out_matrix.data[3] = matrix.data[12];
  out_matrix.data[4] = matrix.data[1];
  out_matrix.data[5] = matrix.data[5];
  out_matrix.data[6] = matrix.data[9];
  out_matrix.data[7] = matrix.data[13];
  out_matrix.data[8] = matrix.data[2];
  out_matrix.data[9] = matrix.data[6];
  out_matrix.data[10] = matrix.data[10];
  out_matrix.data[11] = matrix.data[14];
  out_matrix.data[12] = matrix.data[3];
  out_matrix.data[13] = matrix.data[7];
  out_matrix.data[14] = matrix.data[11];
  out_matrix.data[15] = matrix.data[15];
  return out_matrix;
}

inline mat4 mat4_inverse(mat4 matrix) {
  const f32* m = matrix.data;

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
  f32* o = out_matrix.data;

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

INLINE mat4 mat4_translation(v3 position) {
  mat4 mat = {
      1,          0,          0,          0,
      0,          1,          0,          0,
      0,          0,          1,          0,
      position.x, position.y, position.z, 1};
  return mat;
}

INLINE mat4 mat4_scale(v3 scale) {
  mat4 mat = {
    scale.x,0,      0,      0,
    0,      scale.y,0,      0,
    0,      0,      scale.z,0,
    0,      0,      0,      1};
  return mat;
}

INLINE mat4 mat4_euler_x(f32 angle_radians) {
  f32 cos = Cos(angle_radians);
  f32 sin = Sin(angle_radians);
  mat4 mat = {
    1,    0,     0,    0,
    0,  cos,   sin,    0,
    0, -sin,   cos,    0,
    0,    0,     0,    1,
  };
  return mat;
}

INLINE mat4 mat4_euler_y(f32 angle_radians) {
  f32 cos = Cos(angle_radians);
  f32 sin = Sin(angle_radians);
  mat4 mat = {
    cos, 0,-sin, 0,
    0,   1, 0,   0,
    sin, 0, cos, 0,
    0,   0, 0,   1
  };
  return mat;
}

INLINE mat4 mat4_euler_z(f32 angle_radians) {
  f32 cos = Cos(angle_radians);
  f32 sin = Sin(angle_radians);
  mat4 mat = {
    cos,  sin, 0, 0,
   -sin,  cos, 0, 0,
    0,    0,   1, 0,
    0,    0,   0, 1
  };
  return mat;
}

INLINE mat4 mat4_euler_xyz(f32 x_radians, f32 y_radians, f32 z_radians) {
  mat4 rx = mat4_euler_x(x_radians);
  mat4 ry = mat4_euler_y(y_radians);
  mat4 rz = mat4_euler_z(z_radians);
  mat4 mat = rx * ry * rz;
  return mat;
}

INLINE v3 mat4_forward(mat4 matrix) {
  v3 forward;
  forward.x = -matrix.data[2];
  forward.y = -matrix.data[6];
  forward.z = -matrix.data[10];
  v3_normalize(&forward);
  return forward;
}

INLINE v3 mat4_backward(mat4 matrix) {
  v3 backward;
  backward.x = matrix.data[2];
  backward.y = matrix.data[6];
  backward.z = matrix.data[10];
  v3_normalize(&backward);
  return backward;
}

INLINE v3 mat4_up(mat4 matrix) {
  v3 up;
  up.x = matrix.data[1];
  up.y = matrix.data[5];
  up.z = matrix.data[9];
  v3_normalize(&up);
  return up;
}

INLINE v3 mat4_down(mat4 matrix) {
  v3 down;
  down.x = -matrix.data[1];
  down.y = -matrix.data[5];
  down.z = -matrix.data[9];
  v3_normalize(&down);
  return down;
}

INLINE v3 mat4_left(mat4 matrix) {
  v3 right;
  right.x = -matrix.data[0];
  right.y = -matrix.data[4];
  right.z = -matrix.data[8];
  v3_normalize(&right);
  return right;
}

INLINE v3 mat4_right(mat4 matrix) {
  v3 left;
  left.x = matrix.data[0];
  left.y = matrix.data[4];
  left.z = matrix.data[8];
  v3_normalize(&left);
  return left;
}

INLINE quat quat_identity() {
  return quat{0, 0, 0, 1.0f};
}

INLINE f32 quat_normal(quat q) {
  return Sqrt(
      q.x * q.x +
      q.y * q.y +
      q.z * q.z +
      q.w * q.w);
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Quaternions


INLINE quat quat_normalize(quat q) {
  f32 normal = quat_normal(q);
  return quat{
      q.x / normal,
      q.y / normal,
      q.z / normal,
      q.w / normal};
}

INLINE quat quat_conjugate(quat q) {
  return quat{
      -q.x,
      -q.y,
      -q.z,
      q.w};
}

INLINE quat quat_inverse(quat q) {
  return quat_normalize(quat_conjugate(q));
}

inline quat quat_mul(quat q_0, quat q_1) {
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

INLINE f32 quat_dot(quat q_0, quat q_1) {
  return q_0.x * q_1.x +
         q_0.y * q_1.y +
         q_0.z * q_1.z +
         q_0.w * q_1.w;
}

inline mat4 quat_to_mat4(quat q) {
  mat4 out_matrix = mat4_identity();

  // https://stackoverflow.com/questions/1556260/convert-quaternion-rotation-to-rotation-matrix

  quat n = quat_normalize(q);

  out_matrix.data[0] = 1.0f - 2.0f * n.y * n.y - 2.0f * n.z * n.z;
  out_matrix.data[1] = 2.0f * n.x * n.y - 2.0f * n.z * n.w;
  out_matrix.data[2] = 2.0f * n.x * n.z + 2.0f * n.y * n.w;

  out_matrix.data[4] = 2.0f * n.x * n.y + 2.0f * n.z * n.w;
  out_matrix.data[5] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.z * n.z;
  out_matrix.data[6] = 2.0f * n.y * n.z - 2.0f * n.x * n.w;

  out_matrix.data[8] = 2.0f * n.x * n.z - 2.0f * n.y * n.w;
  out_matrix.data[9] = 2.0f * n.y * n.z + 2.0f * n.x * n.w;
  out_matrix.data[10] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.y * n.y;

  return out_matrix;
}

// Calculates a rotation matrix based on the quaternion and the passed in center point.
inline mat4 quat_to_rotation_matrix(quat q, v3 center) {
  mat4 out_matrix;

  f32* o = out_matrix.data;
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

inline quat quat_from_axis_angle(v3 axis, f32 angle, b32 normalize) {
  const f32 half_angle = 0.5f * angle;
  f32 s = Sin(half_angle);
  f32 c = Cos(half_angle);

  quat q = quat{s * axis.x, s * axis.y, s * axis.z, c};
  if (normalize) {
    return quat_normalize(q);
  }
  return q;
}

inline quat quat_slerp(quat q_0, quat q_1, f32 percentage) {
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
