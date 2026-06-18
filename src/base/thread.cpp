#include "thread.h"
#include "thread_ctx.h"
#include "profiler.h"

struct TaskQueue {
  Task* tasks;
  RingBuffer ring;
  u32 count;
  u32 remaining_tasks;
  Mutex mutex;
  CondVar cond_not_empty;
  CondVar cond_not_full;
  CondVar finished;
};

struct ThreadPool {
  Arena arena;
  Thread threads[16];
  u32 num_threads;
  TaskQueue queue;
  Pool<u32, MAX_TASKS, CounterId> counters;
};

global ThreadPool thread_pool;

TaskId thread_task_push(Task t, b32 async) {
  ThreadPool& g = thread_pool;
  TaskQueue& q = thread_pool.queue;
  LockScope(q.mutex);
  while (q.count == MAX_TASKS) {
    os_cond_var_wait(q.cond_not_full, q.mutex);
  }
  os_cond_var_signal(q.cond_not_empty);
  t.async = async;
  if (!t.async) {
    t.counter_id = pool_push(g.counters, (u32)1);
  }
  ring_write_struct(q.ring, &t);
  ++q.count;
  ++q.remaining_tasks;
  TaskId res = {t.counter_id};
  return res;
}

intern Task thread_task_pop() {
  TaskQueue& q = thread_pool.queue;
  LockScope(q.mutex);
  while (q.count == 0) {
    ProfBlock("sleep", ProfType_Sleep);
    os_cond_var_wait(q.cond_not_empty, q.mutex);
  }
  if (q.count == MAX_TASKS) {
    os_cond_var_signal(q.cond_not_full);
  }
  Task t = {};
  ring_read_struct(q.ring, &t);
  --q.count;
  return t;
}

intern Result<Task> thread_task_try_pop() {
  TaskQueue& q = thread_pool.queue;
  LockScope(q.mutex);
  if (q.count == 0) {
    return Err();
  }
  if (q.count == MAX_TASKS) {
    os_cond_var_signal(q.cond_not_full);
  }
  Task t;
  ring_read_struct(q.ring, &t);
  --q.count;
  return t;
}

intern void thread_worker(void* ctx) {
  ThreadPool& g = thread_pool;
  TaskQueue& q = thread_pool.queue;
  tctx_init();
  while (true) {
    Task t = thread_task_pop();
    ProfBlock("working", ProfType_Worker);
    t.func(t.ctx);
    if (!t.async) {
      atomic_sub(&pool_get(g.counters, t.counter_id), 1);
    }
    LockScope(q.mutex);
    --q.remaining_tasks;
    if (q.remaining_tasks == 0) {
      os_cond_var_signal(q.finished);
    }
  }
}

void thread_pool_init(u32 num_threads) {
  ProfFunc;
  ThreadPool& g = thread_pool;
  g.arena = arena_make();
  g.num_threads = num_threads;
  TaskQueue& q = g.queue;
  q.tasks = push_array(g.arena, Task, MAX_TASKS);
  q.ring = ring_make(q.tasks, MAX_TASKS * sizeof(Task));
  q.mutex = os_mutex_alloc();
  q.cond_not_empty = os_cond_var_alloc();
  q.cond_not_full = os_cond_var_alloc();
  q.finished = os_cond_var_alloc();
  Loop (i, num_threads) {
    g.threads[i] = os_thread_launch(thread_worker, null);
  }
}

void thread_wait_task(TaskId task_id) {
  ProfFunc;
  ThreadPool& g = thread_pool;
  TaskQueue& q = thread_pool.queue;
  while (atomic_load(&pool_get(g.counters, task_id.counter_id)) > 0) {
    Result t_r = thread_task_try_pop();
    if (t_r.err) {
      os_sleep_ms(1);
      continue;
    }
    Task t = t_r;
    ProfBlock("Working", ProfType_Worker);
    t.func(t.ctx);
    if (!t.async) {
      atomic_sub(&pool_get(g.counters, t.counter_id), 1);
    }
    LockScope(q.mutex);
    --q.remaining_tasks;
    if (q.remaining_tasks == 0) {
      os_cond_var_signal(q.finished);
    }
  }
  pool_remove(g.counters, task_id.counter_id);
}

void thread_wait_for() {
  TaskQueue& q = thread_pool.queue;
  LockScope(q.mutex);
  if (q.remaining_tasks > 0) {
    os_cond_var_wait(q.finished, q.mutex);
  }
}
