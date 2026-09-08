#include "base_impl.h"

struct WaitGroupSlot {
	u32 count;
	Futex is_done;
};

struct ThreadPool {
	Arena arena;
	Thread threads[Thread_NumWorkers];

	Queue<Task, Thread_MaxTasks> tasks[TaskPriority_COUNT];
	u32 wg_write;
	WaitGroupSlot wg_slots[Thread_MaxCounters];

	Futex tasks_available;
	Futex free_slots[TaskPriority_COUNT];
	Futex remaining_tasks;

	Mutex task_mutex;
	CondVar cond_not_empty;
	CondVar cond_not_full;
	CondVar finished;

	u8 ctx_buffer[KB(4)];
	u32 ctx_write;
};

global ThreadPool thread_pool;

WaitGroup thread_wg_make(u32 count) {
	var& g = thread_pool;
	u32 idx = atomic_inc(&g.wg_write) % Thread_MaxCounters;
	g.wg_slots[idx] = {.count = count};
	return WaitGroup(idx);
}
void thread_wg_add(WaitGroup wg, u32 n) {
	var& g = thread_pool;
	atomic_add(&g.wg_slots[wg.idx].count, n);
}
intern void thread_wg_decrement(WaitGroup wg) {
	var& g = thread_pool;
	if (atomic_dec(&g.wg_slots[wg.idx].count) == 1) {
		atomic_inc(&g.wg_slots[wg.idx].is_done);
	}
}
b32 thread_wg_is_finished(WaitGroup wg) { return atomic_load(&thread_pool.wg_slots[wg.idx].count) == 0; }

WaitGroup thread_push(TaskDesc desc) {
	var& g = thread_pool;
	WaitGroup wg = thread_wg_make(1);
	Task t = {
		.fn = desc.fn,
		.ctx = desc.ctx,
		.wg = wg,
		.priority = desc.priority,
	};
	LockScope(g.task_mutex);
	while (queue_count(g.tasks[desc.priority]) == Thread_MaxTasks) {
		os_cond_wait(g.cond_not_full, g.task_mutex);
	}
	if (queue_count(g.tasks[desc.priority]) <= Thread_NumWorkers) {
		os_cond_wake_one(g.cond_not_empty);
	}
	queue_push(g.tasks[desc.priority], t);
	g.remaining_tasks++;
	return wg;
}
WaitGroup thread_push(void* ctx, TaskFn* fn, TaskPriority prio) { return thread_push({ctx, fn, prio}); }

WaitGroup thread_push_batch(Slice<TaskDesc> tasks) {
	var& g = thread_pool;
	WaitGroup wg = thread_wg_make(tasks.count);
	LockScope(g.task_mutex);
	while (queue_count(g.tasks[TaskPriority_High])+tasks.count >= Thread_MaxTasks || queue_count(g.tasks[TaskPriority_Low])+tasks.count >= Thread_MaxTasks) {
		os_cond_wait(g.cond_not_full, g.task_mutex);
	}
	if (i32(queue_count(g.tasks[TaskPriority_High])-tasks.count) <= i32(Thread_NumWorkers) || i32(queue_count(g.tasks[TaskPriority_Low])-tasks.count) <= i32(Thread_NumWorkers)) {
		os_cond_wake_all(g.cond_not_empty);
	}
	Loop (i, tasks.count) {
		Task t = {
			.fn = tasks[i].fn,
			.ctx = tasks[i].ctx,
			.wg = wg,
			.priority = tasks[i].priority,
		};
		queue_push(g.tasks[tasks[i].priority], t);
	}
	g.remaining_tasks += tasks.count;
	return wg;
}

intern Task thread_pop() {
	var& g = thread_pool;
	LockScope(g.task_mutex);
	while (queue_count(g.tasks[TaskPriority_High]) == 0 && queue_count(g.tasks[TaskPriority_Low]) == 0) {
		ProfBlock("sleep", ProfType_Sleep);
		os_cond_wait(g.cond_not_empty, g.task_mutex);
	}
	if (queue_count(g.tasks[TaskPriority_High]) == Thread_MaxTasks && queue_count(g.tasks[TaskPriority_Low]) == Thread_MaxTasks) {
		os_cond_wake_one(g.cond_not_full);
	}
	Task t = {};
	if (queue_count(g.tasks[TaskPriority_High])) {
		t = queue_pop(g.tasks[TaskPriority_High]);
	} else {
		t = queue_pop(g.tasks[TaskPriority_Low]);
	}
	return t;
}

intern Task thread_pop2() {
	var& g = thread_pool;
}

intern ResultOk<Task> thread_try_pop() {
	var& g = thread_pool;
	LockScope(g.task_mutex);
	if (queue_count(g.tasks[TaskPriority_High]) == 0 && queue_count(g.tasks[TaskPriority_Low]) == 0) {
		return {};
	}
	if (queue_count(g.tasks[TaskPriority_High]) == Thread_MaxTasks && queue_count(g.tasks[TaskPriority_Low]) == Thread_MaxTasks) {
		os_cond_wake_one(g.cond_not_full);
	}
	Task t = {};
	if (queue_count(g.tasks[TaskPriority_High])) {
		t = queue_pop(g.tasks[TaskPriority_High]);
	} else {
		t = queue_pop(g.tasks[TaskPriority_Low]);
	}
	return {t, true};
}

intern void thread_worker(void* ctx) {
	var& g = thread_pool;
	tctx_init();
	while (true) {
		Task t = thread_pop();
		ProfBlock("working", t.priority == TaskPriority_High ? ProfType_Worker : ProfType_Async);
		t.fn(t.ctx);
		thread_wg_decrement(t.wg);
		if (atomic_dec(&g.remaining_tasks) == 1) {
			os_cond_wake_one(g.finished);
		}
	}
}

void thread_wg_wait(WaitGroup wg) {
	ProfFunc;
	var& g = thread_pool;

	while (!thread_wg_is_finished(wg)) {
		var [t, ok] = thread_try_pop();
		if (!ok) {
			os_sleep_ms(1);
			// os_futex_wait(g.wg_slots[wg.idx].is_done, 0);
			continue;
		}
		ProfBlock("Working", ProfType_Worker);
		t.fn(t.ctx);
		thread_wg_decrement(t.wg);
		if (atomic_dec(&g.remaining_tasks) == 1) {
			os_cond_wake_one(g.finished);
		}
	}
}

void thread_wait_remanings() {
	var& g = thread_pool;
	LockScope(g.task_mutex);
	if (g.remaining_tasks > 0) {
		os_cond_wait(g.finished, g.task_mutex);
	}
}

u8* _thread_push_ctx(u64 size, u64 align) {
	var& g = thread_pool;
	g.ctx_write = AlignUp(g.ctx_write, align) % sizeof(g.ctx_buffer);
	if (g.ctx_write+size > sizeof(g.ctx_buffer)) {
		g.ctx_write = 0;
	}
	u8* res = &g.ctx_buffer[g.ctx_write];
	g.ctx_write += size;
	MemZero(res, size);
	return res;
}

void thread_pool_init() {
	ProfFunc;
	var& g = thread_pool;
	g.arena = arena_make();
	// g.mutex = os_mutex_make();
	// g.cond_not_empty = os_cond_var_make();
	// g.cond_not_full = os_cond_var_make();
	// g.finished = os_cond_var_make();
	Loop (i, Thread_NumWorkers) {
		g.threads[i] = os_thread_make(thread_worker, null);
	}
}

