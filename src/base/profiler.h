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
	// u64 hit_count;
	String label;
	String func;
	u32 depth;
	u64 tsc_start;
	u64 tsc_end;
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
	DArray<ProfEvent> events[2];
	DArray<ProfAnchor> recorded_anchors[ProfRecordHistoryNum];
	DArray<ProfAnchor> launch_anchors;
	DArray<ProfAnchor> delayed_anchors;
};

struct ProfState {
	Arena arena;
	Alloc gpa;
	ProfFrameTime current_frame_time;
	ProfFrameTime frames_times[ProfRecordHistoryNum];
	ProfFrameTime launch_time;
	ProfThread prof_threads[Thread_NumWorkers+1];
	u32 current_write;
	b32 paused;
};

extern ProfState profiler_st;

void prof_init(Allocator arena);
void prof_begin();
void prof_end();
ProfFrame prof_get_prev_frame();
void prof_launch_begin();
void prof_launch_end();

#if PROFILE_BUILD
	#define ProfBlock(Name, ...) _ProfBlock Glue(__profiler_block, __LINE__)(Name, __func__, ##__VA_ARGS__)
	#define ProfFunc ProfBlock(__func__)
#else
	#define ProfBlock(Name)
	#define ProfFunc
#endif
