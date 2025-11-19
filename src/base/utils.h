#pragma once
#include "lib.h"

struct Timer {
  f32 passed;
  f32 interval;
};

inline Timer timer_create(f32 interval) {
  Timer timer = {
    .interval = interval,
  };
  return timer;
}

inline b32 timer_tick(Timer& t, f32 dt) {
  t.passed += dt;
  if (t.passed >= t.interval) {
    t.passed = 0;
    return true;
  }
  return false;
}


