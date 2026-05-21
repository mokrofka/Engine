#include "thread.h"
#include "thread_ctx.h"
#include "profiler.h"

global ThreadPool thread_pool;

void thread_task_push(Task t) {
  // ProfFunc;
  ThreadPool& g = thread_pool;
  TaskQueue& queue = g.queue;
  os_mutex_take(queue.mutex);
  while (queue.count == MAX_TASKS) {
    os_cond_var_wait(queue.cond_not_full, queue.mutex);
  }
  ring_write_struct(queue.ring, &t);
  // queue.tasks[queue.tail] = t;
  // queue.tail = ModPow2(queue.tail + 1, MAX_TASKS);
  ++queue.count;
  ++queue.remaining_tasks;
  os_cond_var_signal(queue.cond_not_empty);
  os_mutex_drop(queue.mutex);
}

intern Task thread_task_pop() {
  // ProfFunc;
  ThreadPool& g = thread_pool;
  TaskQueue& queue = g.queue;
  {
    os_mutex_take(queue.mutex);
  }
  while (queue.count == 0) {
    ProfBlock("sleep", ProfType_Sleep);
    os_cond_var_wait(queue.cond_not_empty, queue.mutex);
  }
  if (queue.count == MAX_TASKS) {
    os_cond_var_signal(queue.cond_not_full);
  }
  // Task t = queue.tasks[queue.head];
  // queue.head = ModPow2(queue.head + 1, MAX_TASKS);

  Task t = {};
  ring_read_struct(queue.ring, &t);
  --queue.count;
  os_mutex_drop(queue.mutex);
  return t;
}

intern void thread_worker(void* arg) {
  TaskQueue& queue = thread_pool.queue;
  tctx_init();
  while (true) {
    Task t = thread_task_pop();
    ProfBlock("working", ProfType_Worker);
    t.func(t.arg);
    os_mutex_take(queue.mutex);
    --queue.remaining_tasks;
    if (queue.remaining_tasks == 0) {
      os_cond_var_signal(queue.finished);
    }
    os_mutex_drop(queue.mutex);
  }
}

void thread_pool_init(u32 num_threads) {
  ProfFunc;
  ThreadPool& g = thread_pool;
  g.arena = arena_make();
  g.num_threads = num_threads;
  TaskQueue& q = g.queue;
  q.tasks = push_array(g.arena, Task, MAX_TASKS);
  q.ring.base = (u8*)q.tasks;
  q.ring.size = MAX_TASKS * sizeof(Task);
  q.mutex = os_mutex_alloc();
  q.cond_not_empty = os_cond_var_alloc();
  q.cond_not_full = os_cond_var_alloc();
  q.finished = os_cond_var_alloc();
  Loop (i, num_threads) {
    g.threads[i] = os_thread_launch(thread_worker, null);
  }
}

void thread_wait_for() {
  // ProfBlock("wait for workers", ProfileType_Sleep);
  TaskQueue& queue = thread_pool.queue;
  os_mutex_take(queue.mutex);
  if (queue.remaining_tasks > 0) {
    os_cond_var_wait(queue.finished, queue.mutex);
  }
  os_mutex_drop(queue.mutex);
}
