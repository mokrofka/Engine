#include "lib.h"

GlobalVar ProfState profiler_st;

void prof_init(Allocator arena) {
	var& g = profiler_st;
	g.arena = arena_make(.lock = true);
	g.gpa = alloc_make(g.arena, .lock = true);
	LoopArray(i, g.prof_threads) {
		var& prof_thread = g.prof_threads[i];
		prof_thread.events[0] = array_make<ProfEvent>(g.gpa);
		prof_thread.events[1] = array_make<ProfEvent>(g.gpa);
		prof_thread.delayed_anchors = array_make<ProfAnchor>(g.gpa);
		prof_thread.launch_anchors = array_make<ProfAnchor>(g.gpa);
		for(var& anchors : prof_thread.recorded_anchors) anchors = array_make<ProfAnchor>(g.gpa);
	}
}

_ProfBlock::_ProfBlock(String label_, String func_, ProfType type_) {
	var& g = profiler_st;
	ProfThread& prof_thread = g.prof_threads[tctx_id()];
	u32 label_idx = hash(label_) % ArrayCount(g.strs);
	u32 func_idx = hash(func_) % ArrayCount(g.strs);
	if(!bit_array_get(g.strs_exist, label_idx)) {
		g.strs[label_idx] = push_str_copy(g.arena, label_);
		bit_array_set(g.strs_exist, label_idx);
	}
	if(!bit_array_get(g.strs_exist, func_idx)) {
		g.strs[func_idx] = push_str_copy(g.arena, func_);
		bit_array_set(g.strs_exist, func_idx);
	}
	label = g.strs[label_idx];
	func = g.strs[func_idx];
	type = type_;
	ProfEvent event = {
		.type = ProfEventType_Push,
		.prof_type = type_,
		.tsc = cpu_now(),
		.label = label,
		.func = func,
	};
	array_push(prof_thread.events[atomic_load(&g.current_write)], event);
}

_ProfBlock::~_ProfBlock() {
	var& g = profiler_st;
	ProfThread& prof_thread = g.prof_threads[tctx_id()];
	ProfEvent event = {
		.type = ProfEventType_Pop,
		.prof_type = type,
		.tsc = cpu_now(),
		.label = label,
		.func = func,
	};
	array_push(prof_thread.events[atomic_load(&g.current_write)], event);
}

void prof_begin() {
	var& g = profiler_st;
	for(var& prof_thread : g.prof_threads) {
		Assert(prof_thread.events[0].count < 3000);
		Assert(prof_thread.events[1].count < 3000);
	}
	for(var& prof_thread : g.prof_threads) array_clear(prof_thread.events[g.current_write]);
	g.current_frame_time.tsc_start = cpu_now();
}

void prof_end() {
	Scratch scratch;
	var& g = profiler_st;
	g.current_frame_time.tsc_end = cpu_now();
	if(g.paused) return;
	g.frames_times[current_frame % ArrayCount(g.frames_times)] = {
		g.current_frame_time.tsc_start,
		g.current_frame_time.tsc_end,
	};
	u32 read_buf = atomic_xor(&g.current_write, 1);
	for(var& prof_thread : g.prof_threads) {
		var& anchors = prof_thread.recorded_anchors[current_frame % ArrayCount(g.frames_times)];
		array_clear(anchors);
		u32 depth = 0;
		var stack = array_make<u32>(scratch);

		///////////////////////////////////
		// Process events
		for(var event : prof_thread.events[read_buf]) switch(event.type) {
			case ProfEventType_Push: {
				ProfAnchor anchor = {
					.type = event.prof_type,
					.label = event.label,
					.func = event.func,
					.depth = depth,
					.tsc_start = event.tsc,
				};
				depth++;

				// In prev frame was push event
				if(prof_thread.delayed_anchors.count) {
					array_push(prof_thread.delayed_anchors, anchor);
				} else {
					u32 idx = array_push(anchors, anchor);
					array_push(stack, idx);
				}
			}break;
			case ProfEventType_Pop: {
				// In prev frame was push event
				if(prof_thread.delayed_anchors.count) {
					ProfAnchor old_anchor = array_pop(prof_thread.delayed_anchors);
					u32 idx = array_push(anchors, old_anchor);
					array_push(stack, idx);
					depth++;
				}

				// FIXME: shouldn't happen 
				if(stack.count == 0) {
					continue;
				}
				u32 anchor_idx = array_pop(stack);
				ProfAnchor& anchor = anchors[anchor_idx];
				anchor.tsc_end = event.tsc;
				u64 elapsed = anchor.tsc_end - anchor.tsc_start;
				if(stack.count) {
					anchors[array_back(stack)].tsc_elapsed_excl -= elapsed;
				}
				anchor.tsc_elapsed_excl += elapsed;
				depth--;
			}break;
		}

		// We save long block time to handle it in next frames
		Loop(i, stack.count) {
			array_push(prof_thread.delayed_anchors, anchors[anchors.count - stack.count + i]);
		}
	}
}

ProfFrame prof_get_prev_frame() {
	var& g = profiler_st;
	var& prof_thread = g.prof_threads[tctx_id()];
	ProfFrame result = {
		.frame_time = g.frames_times[(current_frame-1) % ArrayCount(g.frames_times)],
		.anchors = slice(prof_thread.recorded_anchors[(current_frame-1) % ArrayCount(g.frames_times)]),
	};
	return result;
}

void prof_launch_begin() {
	ProfState& g = profiler_st;
	g.current_frame_time.tsc_start = cpu_now();
}

void prof_launch_end() {
	Scratch scratch;
	ProfState& g = profiler_st;

	ProfFrameTime& frame_time = g.current_frame_time;
	frame_time.tsc_end = cpu_now();
	g.launch_time = frame_time;

	LoopArray(j, g.prof_threads) {
		ProfThread& prof_thread = g.prof_threads[j];
		var anchors = array_make<ProfAnchor>(scratch);
		u32 depth = 0;
		var stack = array_make<u32>(scratch);

		///////////////////////////////////
		// Process events
		Loop(i, prof_thread.events[0].count) {
			ProfEvent event = prof_thread.events[0][i];
			switch(event.type) {
				case ProfEventType_Push: {
					ProfAnchor anchor = {
						.type = event.prof_type,
						.label = event.label,
						.func = event.func,
						.tsc_start = event.tsc,
						.depth = depth,
					};
					array_push(anchors, anchor);
					array_push(stack, anchors.count-1);
					depth++;
				}break;
				case ProfEventType_Pop: {
					u32 anchor_idx = 0;
					// In some time back block time was longer than frame
					if(prof_thread.delayed_anchors.count) {
						ProfAnchor old_anchor = array_pop(prof_thread.delayed_anchors);
						array_push(anchors, old_anchor);
						array_push(stack, anchors.count-1);
						depth++;
					}

					anchor_idx = array_pop(stack);
					ProfAnchor& anchor = anchors[anchor_idx];
					anchor.tsc_end = event.tsc;
					u64 elapsed = anchor.tsc_end - anchor.tsc_start;
					if(stack.count) {
						u32 parent_idx = array_back(stack);
						ProfAnchor& anchor_parent = anchors[parent_idx];
						anchor_parent.tsc_elapsed_excl -= elapsed;
					}
					anchor.tsc_elapsed_excl += elapsed;
					--depth;
				}break;
			}
		}

		// We save long block time to handle it in next frames
		if(stack.count) Loop(i, stack.count) {
			array_push(prof_thread.delayed_anchors, anchors[anchors.count - stack.count + i]);
			array_push(prof_thread.launch_anchors, anchors[anchors.count - stack.count + i]);
		}

		///////////////////////////////////
		// Record anchors
		var& write_anchors = prof_thread.recorded_anchors[0];
		var& launch_anchors = prof_thread.launch_anchors;
		array_reserve(write_anchors, anchors.count);
		MemCopyArray(write_anchors.data, anchors.data, anchors.count);
		write_anchors.count = anchors.count;
		array_reserve(launch_anchors, anchors.count);
		MemCopyArray(launch_anchors.data, anchors.data, anchors.count);
		launch_anchors.count = anchors.count;
	}



}
