#include "profiler.h"

global ProfilerState profiler_st;

void profiler_init(Allocator arena) {
  ProfilerState& g = profiler_st;
  for EachElement (i, g.prof_threads) {
    ProfileThread& prof_thread = g.prof_threads[i];
    String str = push_strf(arena, "profiler_st thread %u arena", i);
    prof_thread.arena = arena_init_named(str);
    prof_thread.gpa.init(prof_thread.arena);
    prof_thread.events[0].init(prof_thread.gpa);
    prof_thread.events[1].init(prof_thread.gpa);
    prof_thread.long_anchors.init(prof_thread.gpa);
    prof_thread.launch_anchors.init(prof_thread.gpa);
    for EachElement(j, g.frames_times) {
      prof_thread.recorded_anchors[j].init(prof_thread.gpa);
    }
  }
}

ProfilerState& profiler_get() { return profiler_st; }

ProfileBlock::ProfileBlock(String label_, String func_, ProfileType type) {
  ProfilerState& g = profiler_st;
  ProfileThread& prof_thread = profiler_get_prof_thread();
  label = label_;
  func = func_;
  ProfileEvent event = {
    .type = ProfileEventType_Push,
    .prof_type = type,
    .tsc = cpu_timer_now(),
    .label = label_,
    .func = func_,
  };
  prof_thread.events[g.current_buf].add(event);
}

ProfileBlock::~ProfileBlock() {
  ProfilerState& g = profiler_st;
  ProfileThread& prof_thread = profiler_get_prof_thread();
  ProfileEvent event = {
    .type = ProfileEventType_Pop,
    .tsc = cpu_timer_now(),
    .label = label,
    .func = func,
  };
  prof_thread.events[g.current_buf].add(event);
}

void profiler_begin(u32 current_frame) {
  ProfilerState& g = profiler_st;
  ProfileFrameTime& frame_time = g.current_frame_time;
  // before first frame threads pushes sleep events
  if (current_frame != 0) {
    for EachElement(i, g.prof_threads) {
      ProfileThread& prof_thread = g.prof_threads[i];
      prof_thread.events[g.current_buf].clear();
    }
  }
  frame_time.tsc_start = cpu_timer_now();
}

void profiler_end(u32 current_frame) {
  Scratch scratch;
  ProfilerState& g = profiler_st;

  ProfileFrameTime& frame_time = g.current_frame_time;
  frame_time.tsc_end = cpu_timer_now();
  if (!g.paused) {
    ProfileFrameTime& write_frame_time = g.frames_times[current_frame % ArrayCount(g.frames_times)];
    write_frame_time.tsc_start = frame_time.tsc_start;
    write_frame_time.tsc_end = frame_time.tsc_end;
  }

  u32 read_buf = atomic_u32_xor(&g.current_buf, 1);

  for EachElement(j, g.prof_threads) {
    ProfileThread& prof_thread = g.prof_threads[j];
    Darray<ProfileAnchor> anchors(scratch);
    u32 depth = 0;
    Darray<u32> stack(scratch);

    ///////////////////////////////////
    // Process events
    Loop (i, prof_thread.events[read_buf].count) {
      ProfileEvent event = prof_thread.events[read_buf][i];
      switch (event.type) {
        case ProfileEventType_Push: {
          ProfileAnchor anchor = {
            .type = event.prof_type,
            .label = event.label,
            .func = event.func,
            .tsc_start = event.tsc,
            .depth = depth,
          };
          anchors.add(anchor);
          stack.add(anchors.count-1);
          ++depth;
        } break;
        case ProfileEventType_Pop: {
          u32 anchor_idx = 0;
          // In some time back block time was longer than frame
          if (prof_thread.long_anchors.count) {
            ProfileAnchor old_anchor = prof_thread.long_anchors.pop();
            anchors.add(old_anchor);
            stack.add(anchors.count-1);
            ++depth;
          }

          anchor_idx = stack.pop();
          ProfileAnchor& anchor = anchors[anchor_idx];
          anchor.tsc_end = event.tsc;
          u64 elapsed = anchor.tsc_end - anchor.tsc_start;
          if (stack.count) {
            u32 parent_idx = stack.back();
            ProfileAnchor& anchor_parent = anchors[parent_idx];
            anchor_parent.tsc_elapsed_exclusive -= elapsed;
          }
          anchor.tsc_elapsed_inclusive += elapsed;
          anchor.tsc_elapsed_exclusive += elapsed;
          anchor.was_poped = true;
          --depth;
        } break;
      }
    }

    // We save long block time to handle it in next frames
    if (stack.count) {
      Loop (i, stack.count) {
        prof_thread.long_anchors.add(anchors[anchors.count - stack.count + i]);
      }
    }

    ///////////////////////////////////
    // Record anchors
    if (!g.paused) {
      var& write_anchors = prof_thread.recorded_anchors[current_frame % ArrayCount(g.frames_times)];
      write_anchors.reserve(anchors.count);
      MemCopyArray(write_anchors.data, anchors.data, anchors.count);
      write_anchors.count = anchors.count;
    }
  }
}

ProfileFrame profiler_get_prev_frame(u32 current_frame) {
  ProfilerState& g = profiler_st;
  ProfileThread& prof_thread = profiler_get_prof_thread();
  ProfileFrame result = {
    .frame_time = g.frames_times[(current_frame-1) % ArrayCount(g.frames_times)],
    .anchors = prof_thread.recorded_anchors[(current_frame-1) % ArrayCount(g.frames_times)].slice(),
  };
  return result;
}

ProfileThread& profiler_get_prof_thread() {
  ProfilerState& g = profiler_st;
  return g.prof_threads[tctx_get_id()];
}

void profiler_launch_begin() {
  ProfilerState& g = profiler_st;
  g.current_frame_time.tsc_start = cpu_timer_now();
}

void profiler_launch_end() {
  Scratch scratch;
  ProfilerState& g = profiler_st;

  ProfileFrameTime& frame_time = g.current_frame_time;
  frame_time.tsc_end = cpu_timer_now();

  for EachElement(j, g.prof_threads) {
    ProfileThread& prof_thread = g.prof_threads[j];
    Darray<ProfileAnchor> anchors(scratch);
    u32 depth = 0;
    Darray<u32> stack(scratch);

    ///////////////////////////////////
    // Process events
    Loop (i, prof_thread.events[0].count) {
      ProfileEvent event = prof_thread.events[0][i];
      switch (event.type) {
        case ProfileEventType_Push: {
          ProfileAnchor anchor = {
            .type = event.prof_type,
            .label = event.label,
            .func = event.func,
            .tsc_start = event.tsc,
            .depth = depth,
          };
          anchors.add(anchor);
          stack.add(anchors.count-1);
          ++depth;
        } break;
        case ProfileEventType_Pop: {
          u32 anchor_idx = 0;
          // In some time back block time was longer than frame
          if (prof_thread.long_anchors.count) {
            ProfileAnchor old_anchor = prof_thread.long_anchors.pop();
            anchors.add(old_anchor);
            stack.add(anchors.count-1);
            ++depth;
          }

          anchor_idx = stack.pop();
          ProfileAnchor& anchor = anchors[anchor_idx];
          anchor.tsc_end = event.tsc;
          u64 elapsed = anchor.tsc_end - anchor.tsc_start;
          if (stack.count) {
            u32 parent_idx = stack.back();
            ProfileAnchor& anchor_parent = anchors[parent_idx];
            anchor_parent.tsc_elapsed_exclusive -= elapsed;
          }
          anchor.tsc_elapsed_inclusive += elapsed;
          anchor.tsc_elapsed_exclusive += elapsed;
          anchor.was_poped = true;
          --depth;
        } break;
      }
    }

    // We save long block time to handle it in next frames
    if (stack.count) {
      Loop (i, stack.count) {
        prof_thread.long_anchors.add(anchors[anchors.count - stack.count + i]);
      }
    }

    ///////////////////////////////////
    // Record anchors
    var& write_anchors = prof_thread.recorded_anchors[0];
    var& launch_anchors = prof_thread.launch_anchors;
    write_anchors.reserve(anchors.count);
    MemCopyArray(write_anchors.data, anchors.data, anchors.count);
    write_anchors.count = anchors.count;
    launch_anchors.reserve(anchors.count);
    MemCopyArray(launch_anchors.data, anchors.data, anchors.count);
    launch_anchors.count = anchors.count;
  }



}
