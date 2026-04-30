#pragma once
#include "base.h"
#include "containers.h"
#include "thread_ctx.h"

enum ProfileType {
  ProfileType_Work,
  ProfileType_Sleep,
};

struct ProfileAnchor {
  ProfileType type;
  u64 tsc_elapsed_exclusive; // without children
  u64 tsc_elapsed_inclusive; // with children
  // u64 hit_count;
  String label;
  String func;
  u32 depth;
  u64 tsc_start;
  u64 tsc_end;
  u32 current;
  b32 was_poped;
};

enum ProfileEventType {
  ProfileEventType_Push,
  ProfileEventType_Pop,
};

struct ProfileEvent {
  ProfileEventType type;
  ProfileType prof_type;
  u64 tsc;
  String label;
  String func;
};

struct ProfileBlock {
  String label;
  String func;
  ProfileBlock(String label_, String func, ProfileType type = ProfileType_Work);
  ~ProfileBlock();
};

struct ProfileFrameTime {
  u64 tsc_start;
  u64 tsc_end;
};

struct ProfileFrame {
  ProfileFrameTime frame_time;
  Slice<ProfileAnchor> anchors;
};

struct ProfileThread {
  Arena arena;
  AllocSegList gpa;
  Darray<ProfileEvent> events[2];
  Darray<ProfileAnchor> recorded_anchors[120];
  Darray<ProfileAnchor> launch_anchors;
  Darray<ProfileAnchor> long_anchors;
};

enum ProfileTabActive {
  ProfileTabActive_Root,
  ProfileTabActive_Frames,
  ProfileTabActive_Time,
  ProfileTabActive_Memory,
};

struct ProfilerState {
  ProfileFrameTime current_frame_time;
  ProfileFrameTime frames_times[120];
  ProfileThread prof_threads[THREAD_COUNT+1];
  u32 current_buf;

  f32 frame_avg_time;
  f32 frame_min_time;
  f32 frame_max_time;

  b32 paused;

  ProfileTabActive active_tab;
};

void profiler_init(Allocator arena);
ProfilerState& profiler_get();
void profiler_begin(u32 current_frame);
void profiler_end(u32 current_frame);
ProfileFrame profiler_get_prev_frame(u32 current_frame);
ProfileThread& profiler_get_prof_thread();
void profiler_launch_begin();
void profiler_launch_end();

#if PROFILE_BUILD
  #define TimeBlock(Name, ...) ProfileBlock Glue(__profiler_block, __LINE__)(Name, __func__, ##__VA_ARGS__)
  #define TimeFunction TimeBlock(__func__)
#else
  #define TimeBlock(Name)
  #define TimeFunction
#endif
