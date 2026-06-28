#pragma once
#include "base.h"
#include "containers.h"
#include "thread_ctx.h"
#include "thread.h"

const u32 ProfRecordHistoryNum = 120;

enum ProfType {
  ProfType_Default,
  ProfType_Sleep,
  ProfType_Worker,
  ProfType_Async,
};

struct ProfAnchor {
  ProfType type;
  u64 tsc_elapsed_excl; // without children
  u64 tsc_elapsed_incl; // with children
  // u64 hit_count;
  String label;
  String func;
  u32 depth;
  u64 tsc_start;
  u64 tsc_end;
  b32 was_poped;
};

enum ProfEventType {
  ProfEventType_Push,
  ProfEventType_Pop,
};

struct ProfEvent {
  ProfEventType type;
  ProfType prof_type;
  u64 tsc;
  String label;
  String func;
};

struct _ProfBlock {
  String label;
  String func;
  ProfType type;
  _ProfBlock(String label_, String func_, ProfType type_ = ProfType_Default);
  ~_ProfBlock();
};

struct ProfFrameTime {
  u64 tsc_start;
  u64 tsc_end;
};

struct ProfFrame {
  ProfFrameTime frame_time;
  Slice<ProfAnchor> anchors;
};

struct ProfThread {
  Arena arena;
  Alloc gpa;
  Darray<ProfEvent> events[2];
  Darray<ProfAnchor> recorded_anchors[ProfRecordHistoryNum];
  Darray<ProfAnchor> launch_anchors;
  Darray<ProfAnchor> long_anchors;
};

struct ProfState {
  ProfFrameTime current_frame_time;
  ProfFrameTime frames_times[ProfRecordHistoryNum];
  ProfFrameTime launch_time;
  ProfThread prof_threads[Thread_NumWorkers+1];
  u32 current_buf;
  b32 paused;
};

void prof_init(Allocator arena);
ProfState& prof_get();
void prof_begin(u32 current_frame);
void prof_end(u32 current_frame);
ProfFrame prof_get_prev_frame(u32 current_frame);
ProfThread& prof_get_prof_thread();
void prof_launch_begin();
void prof_launch_end();

#if PROFILE_BUILD
  #define ProfBlock(Name, ...) _ProfBlock Glue(__profiler_block, __LINE__)(Name, __func__, ##__VA_ARGS__)
  #define ProfFunc ProfBlock(__func__)
#else
  #define ProfBlock(Name)
  #define ProfFunc
#endif
