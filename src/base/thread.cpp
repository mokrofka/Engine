#include "base_impl.h"

struct WaitGroupSlot {
  u32 count;
  u32 gen;
};

struct ThreadPool {
  Arena arena;
  Thread threads[Thread_NumWorkers];

  Queue<Task, Thread_MaxTasks> tasks[TaskPriority_COUNT];
  u32 wg_ring_idx;
  WaitGroupSlot wg_slots[Thread_MaxCounters];

  u32 remaining_tasks;
  Mutex mutex;
  CondVar cond_not_empty;
  CondVar cond_not_full;
  CondVar finished;

  u8 ctx_buffer[KB(4)];
  u32 ctx_cursor;
};

global ThreadPool thread_pool;

WaitGroup thread_wg_make(u32 count) {
  var& g = thread_pool;
  u32 idx = atomic_inc(&g.wg_ring_idx) % Thread_MaxCounters;
  WaitGroupSlot& slot = g.wg_slots[idx];
  ++slot.gen;
  slot.count = count;
  return WaitGroup(idx, slot.gen);
}

void thread_wg_add(WaitGroup wg, u32 n) {
  var& g = thread_pool;
  Assert(g.wg_slots[wg.idx].gen == wg.gen);
  atomic_add(&g.wg_slots[wg.idx].count, n);
}

intern void thread_wg_decrement(WaitGroup wg) { atomic_dec(&thread_pool.wg_slots[wg.idx].count); }
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
  LockScope(g.mutex);
  while (g.tasks[desc.priority].count == Thread_MaxTasks) {
    os_cond_var_wait(g.cond_not_full, g.mutex);
  }
  if (g.tasks[desc.priority].count <= Thread_NumWorkers) {
    os_cond_var_wake_one(g.cond_not_empty);
  }
  queue_push(g.tasks[desc.priority], t);
  ++g.remaining_tasks;
  return wg;
}
WaitGroup thread_push(void* ctx, TaskFn* fn, TaskPriority prio) { return thread_push({ctx, fn, prio}); }

WaitGroup thread_push_batch(Slice<TaskDesc> tasks) {
  var& g = thread_pool;
  WaitGroup wg = thread_wg_make(tasks.count);
  LockScope(g.mutex);
  while (g.tasks[TaskPriority_High].count+tasks.count >= Thread_MaxTasks || g.tasks[TaskPriority_Low].count+tasks.count >= Thread_MaxTasks) {
    os_cond_var_wait(g.cond_not_full, g.mutex);
  }
  if (i32(g.tasks[TaskPriority_High].count-tasks.count) <= i32(Thread_NumWorkers) || i32(g.tasks[TaskPriority_Low].count-tasks.count) <= i32(Thread_NumWorkers)) {
    os_cond_var_wake_all(g.cond_not_empty);
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
  LockScope(g.mutex);
  while (g.tasks[TaskPriority_High].count == 0 && g.tasks[TaskPriority_Low].count == 0) {
    ProfBlock("sleep", ProfType_Sleep);
    os_cond_var_wait(g.cond_not_empty, g.mutex);
  }
  if (g.tasks[TaskPriority_High].count == Thread_MaxTasks && g.tasks[TaskPriority_Low].count == Thread_MaxTasks) {
    os_cond_var_wake_one(g.cond_not_full);
  }
  Task t = {};
  if (g.tasks[TaskPriority_High].count) {
    t = queue_pop(g.tasks[TaskPriority_High]);
  } else {
    t = queue_pop(g.tasks[TaskPriority_Low]);
  }
  return t;
}

intern ResultOk<Task> thread_try_pop() {
  var& g = thread_pool;
  LockScope(g.mutex);
  if (g.tasks[TaskPriority_High].count == 0 && g.tasks[TaskPriority_Low].count == 0) {
    return {};
  }
  if (g.tasks[TaskPriority_High].count == Thread_MaxTasks && g.tasks[TaskPriority_Low].count == Thread_MaxTasks) {
    os_cond_var_wake_one(g.cond_not_full);
  }
  Task t = {};
  if (g.tasks[TaskPriority_High].count) {
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
      os_cond_var_wake_one(g.finished);
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
      continue;
    }
    ProfBlock("Working", ProfType_Worker);
    t.fn(t.ctx);
    thread_wg_decrement(t.wg);
    if (atomic_dec(&g.remaining_tasks) == 1) {
      os_cond_var_wake_one(g.finished);
    }
  }
}

void thread_wait_remanings() {
  var& g = thread_pool;
  LockScope(g.mutex);
  if (g.remaining_tasks > 0) {
    os_cond_var_wait(g.finished, g.mutex);
  }
}

u8* _thread_push_ctx(u64 size, u64 align) {
  var& g = thread_pool;
  g.ctx_cursor = AlignUp(g.ctx_cursor, align) % sizeof(g.ctx_buffer);
  if (g.ctx_cursor+size > sizeof(g.ctx_buffer)) {
    g.ctx_cursor = 0;
  }
  u8* res = &g.ctx_buffer[g.ctx_cursor];
  g.ctx_cursor += size;
  MemZero(res, size);
  return res;
}

void thread_pool_init() {
  ProfFunc;
  var& g = thread_pool;
  g.arena = arena_make();
  g.mutex = os_mutex_make();
  g.cond_not_empty = os_cond_var_make();
  g.cond_not_full = os_cond_var_make();
  g.finished = os_cond_var_make();
  Loop (i, Thread_NumWorkers) {
    g.threads[i] = os_thread_make(thread_worker, null);
  }
}

