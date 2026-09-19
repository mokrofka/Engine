#include "lib.h"

struct WaitGroupSlot {
	u32 count;
	Futex is_done;
	u32 waiting_count;
};

struct ThreadPool {
	Arena arena;
	Thread threads[Thread_NumWorkers];

	QueueMPMC<Task, Thread_MaxTasks> tasks[TaskPriority_COUNT];
	u32 wg_write;
	WaitGroupSlot wg_slots[Thread_MaxCounters];

	Semaphore tasks_available;
	u32 working_num;
	u32 remaining_tasks;
	Futex is_waiting_remaning_tasks;

	Mutex task_mutex;
	Mutex ctx_mutex;

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
void thread_decrement_wg(WaitGroup wg) {
	var& g = thread_pool;
	var& slot = g.wg_slots[wg.idx];
	if(atomic_dec(&slot.count) == 1) {
		atomic_store(&slot.is_done, 1);
		if(atomic_load(&slot.waiting_count) > 0) {
			os_futex_wake(slot.is_done, U32_MAX);
		}
	}
}

WaitGroup thread_push(TaskDesc desc) {
	var& g = thread_pool;
	WaitGroup wg = thread_wg_make(1);
	Task t = {
		.fn = desc.fn,
		.ctx = desc.ctx,
		.wg = wg,
		.priority = desc.priority,
	};
	{
		// LockScope(g.task_mutex);
		queue_push(g.tasks[desc.priority], t);
	}
	if(atomic_load(&g.working_num) < Thread_NumWorkers) {
		os_sem_post(g.tasks_available);
	} else {
		atomic_inc(&g.tasks_available.futex);
	}
	atomic_inc(&g.remaining_tasks);
	return wg;
}
WaitGroup thread_push(void* ctx, TaskFn* fn, TaskPriority prio) { return thread_push({ctx, fn, prio}); }

WaitGroup thread_push_batch(Slice<TaskDesc> tasks) {
	var& g = thread_pool;
	u32 count_by_prio[TaskPriority_COUNT] = {};
	Loop(i, tasks.count) count_by_prio[tasks[i].priority]++;
	WaitGroup wg = thread_wg_make(tasks.count);
	Loop(i, tasks.count) {
		Task t = {.fn = tasks[i].fn, .ctx = tasks[i].ctx, .wg = wg, .priority = tasks[i].priority};
		queue_push(g.tasks[tasks[i].priority], t);
	}
	atomic_add(&g.tasks_available.futex, tasks.count);
	atomic_add(&g.remaining_tasks, tasks.count);
	if(atomic_load(&g.working_num) < tasks.count) {
		os_futex_wake(g.tasks_available.futex, U32_MAX);
	}
	return wg;
}

Task thread_pop_locked() {
	var& g = thread_pool;
	var res = queue_pop(g.tasks[TaskPriority_High]);
	if(!res.ok) res = queue_pop(g.tasks[TaskPriority_Low]);
	Assert(res.ok);
	return res.value;
}

Task thread_pop() {
	var& g = thread_pool;
	ProfBlock("sleep", ProfType_Sleep);
	atomic_dec(&g.working_num);
	os_sem_wait(thread_pool.tasks_available);
	atomic_inc(&g.working_num);
	return thread_pop_locked();
}

ResultOk<Task> thread_try_pop() {
	var& g = thread_pool;
	if(os_sem_try_wait(g.tasks_available)) return {thread_pop_locked(), true};
	return {};
}

void thread_worker(void* ctx) {
	var& g = thread_pool;
	tctx_init();
	For {
		Task t = thread_pop();
		ProfBlock("working", t.priority == TaskPriority_High ? ProfType_Worker : ProfType_Async);
		t.fn(t.ctx);
		if(atomic_dec(&g.remaining_tasks) == 1) {
			if(atomic_load(&g.is_waiting_remaning_tasks) == 1) {
				os_futex_wake(g.is_waiting_remaning_tasks, 1);
			}
		}
		thread_decrement_wg(t.wg);
	}
}

void thread_wg_wait(WaitGroup wg) {
	ProfFunc;
	var& g = thread_pool;
	var& slot = g.wg_slots[wg.idx];
	while(atomic_load(&slot.is_done) == 0) {
		var[t, ok] = thread_try_pop();
		if(ok) {
			ProfBlock("Working", ProfType_Worker);
			t.fn(t.ctx);
			atomic_dec(&g.remaining_tasks);
			thread_decrement_wg(t.wg);
			continue;
		}
		atomic_inc(&slot.waiting_count);
		os_futex_wait(slot.is_done, 0);
	}
}

void thread_wait_remanings() {
	var& g = thread_pool;
	atomic_store(&g.is_waiting_remaning_tasks, 1);
	if(atomic_load(&g.remaining_tasks)) {
		os_futex_wait(g.is_waiting_remaning_tasks, 1);
	}
	atomic_store(&g.is_waiting_remaning_tasks, 0);
}

u8* _thread_push_ctx(u64 size, u64 align) {
	var& g = thread_pool;
	os_mutex_lock(g.ctx_mutex);
	g.ctx_write = AlignUp(g.ctx_write, align) % sizeof(g.ctx_buffer);
	if(g.ctx_write + size > sizeof(g.ctx_buffer)) {
		g.ctx_write = 0;
	}
	u8* res = &g.ctx_buffer[g.ctx_write];
	g.ctx_write += size;
	os_mutex_unlock(g.ctx_mutex);
	MemZero(res, size);
	return res;
}

void thread_pool_init() {
	ProfFunc;
	var& g = thread_pool;
	g.arena = arena_make();
	g.working_num = Thread_NumWorkers;
	for(var& q : g.tasks) {
		q = queue_mpmc_make<Task, Thread_MaxTasks>();
	}
	Loop(i, Thread_NumWorkers) {
		g.threads[i] = os_thread_make(thread_worker, null);
	}
}
