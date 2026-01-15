#pragma once
#include "lib.h"

struct Timer {
  f32 passed;
  f32 interval;
};

inline Timer timer_init(f32 interval) {
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

inline u64 cpu_frequency() {
  u64 os_freq = os_timer_frequency();
  u64 cpu_start = CpuTimerNow();
  u64 os_start = os_timer_now();

  u64 milliseconds = 1;
  u64 os_end = 0;
  u64 os_elapsed = 0;
  u64 os_wait_time = os_freq * milliseconds / 1000;
  while (os_elapsed < os_wait_time) {
    os_end = os_timer_now();
    os_elapsed = os_end - os_start;
  }

  u64 cpu_end = CpuTimerNow();
  u64 cpu_elapsed = cpu_end - cpu_start;
  u64 cpu_freq = 0;
  if (cpu_elapsed) {
    cpu_freq = os_freq * cpu_elapsed / os_elapsed;
  }

  return cpu_freq;
}

struct ProfileAnchor {
  u64 TSC_elapsed;
  u64 TSC_elapsed_children;
  u64 TSC_elapsed_at_root;
  u64 hit_count;
  String label;
};

struct Profiler {
  Array<ProfileAnchor*, 4096> anchors;
  u64 start_TSC;
  u64 end_TSC;
};
KAPI extern Profiler global_profiler;
KAPI extern ProfileAnchor* global_profiler_parent;

struct ProfileBlock {
  ProfileAnchor* anchor;
  ProfileAnchor* parent;
  u64 start_TSC;
  u64 old_TSC_elapsed_at_root;

  ProfileBlock(String label_, ProfileAnchor& anchor_) {
    if (!anchor_.label.size) {
      global_profiler.anchors.append(&anchor_);
    }
    parent = global_profiler_parent;

    anchor = &anchor_;
    anchor->label = label_;
    global_profiler_parent = anchor;
    old_TSC_elapsed_at_root = anchor->TSC_elapsed_at_root;

    start_TSC = CpuTimerNow();
  }

  ~ProfileBlock() {
    u64 elapsed = CpuTimerNow() - start_TSC;
    global_profiler_parent = parent;
    
    if (parent) {
      parent->TSC_elapsed_children += elapsed;
    }
    anchor->TSC_elapsed_at_root = old_TSC_elapsed_at_root + elapsed;
    anchor->TSC_elapsed += elapsed;
    ++anchor->hit_count;
  }
};

#define TimeFunction TimeBlock(__func__)
#define TimeBlock(name) \
static ProfileAnchor Glue(_anchor, __LINE__); ProfileBlock Glue(_block, __LINE__)(name, Glue(_anchor, __LINE__))

inline void print_time_elapsed(u64 total_TSC_elapsed, ProfileAnchor* anchor) {
  u64 elapsed = anchor->TSC_elapsed - anchor->TSC_elapsed_children;
  f64 percent = 100.0 * ((f64)elapsed / (f64)total_TSC_elapsed);
  print("  %s[%u64]: %u64 (%.2f%%)", anchor->label, anchor->hit_count, elapsed, percent);
  if (anchor->TSC_elapsed_at_root != elapsed) {
    f64 percent_with_children = 100.0 * ((f64)anchor->TSC_elapsed / (f64)total_TSC_elapsed);
    print(", %.2f%% w/children", percent_with_children);
  }
  println("");
}

inline void begin_profile() {
  global_profiler.start_TSC = CpuTimerNow();
}

inline void end_and_print_profile() {
  global_profiler.end_TSC = CpuTimerNow();
  u64 cpu_freq = cpu_frequency();

  u64 total_CPU_elapsed = global_profiler.end_TSC - global_profiler.start_TSC;

  if (cpu_freq) {
    Info("Total time: %.4fms (CPU freq %u64)", 1000.0 * (f64)total_CPU_elapsed / (f64)cpu_freq, cpu_freq);
  }

  for (ProfileAnchor* x : global_profiler.anchors) {
    print_time_elapsed(total_CPU_elapsed, x);
  }
}
