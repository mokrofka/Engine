#include "maths.h"

#include "platform/platform.h"

#include <stdlib.h>
#include <math.h>

global b8 rand_seeded = false;

f32 Sin(f32 x) {
  return sinf(x);
}

f32 Cos(f32 x) {
  return cos(x);
}

f32 Tan(f32 x) {
  return tan(x);
}

f32 Acos(f32 x) {
  return acosf(x);
}

f32 Sqrt(f32 x) {
  return sqrtf(x);
}

i32 random() {
  if (!rand_seeded) {
    srand(u32(platform_get_absolute_time()));
    rand_seeded = true;
  }
  return rand();
}

i32 random_in_range(i32 min, i32 max) {
  if (!rand_seeded) {
    srand(u32(platform_get_absolute_time()));
    rand_seeded = true;
  }
  return (rand() % (max - min + 1)) + min;
}

f32 frandom() {
  return (f32)random() / (f32)RAND_MAX;
}

f32 frandom_in_range(f32 min, f32 max) {
  return min + ((f32)random() / ((f32)RAND_MAX / (max - min)));
}
