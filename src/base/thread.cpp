#include "thread.h"
#include "thread_ctx.h"
#include "profiler.h"

global ThreadPool thread_pool;

void task_queue_init() {
  ThreadPool& g = thread_pool;
  TaskQueue& queue = g.queue;
  queue.mutex = os_mutex_alloc();
  queue.cond_not_empty = os_cond_var_alloc();
  queue.cond_not_full = os_cond_var_alloc();
  queue.finished = os_cond_var_alloc();
}

void task_queue_push(Task t) {
  ThreadPool& g = thread_pool;
  TaskQueue& queue = g.queue;
  os_mutex_take(queue.mutex);
  while (queue.count == MAX_TASKS) {
    os_cond_var_wait(queue.cond_not_full, queue.mutex);
  }
  queue.tasks[queue.tail] = t;
  queue.tail = ModPow2(queue.tail + 1, MAX_TASKS);
  ++queue.count;
  ++queue.remaining_tasks;
  os_cond_var_signal(queue.cond_not_empty);
  os_mutex_drop(queue.mutex);
}

Task task_queue_pop() {
  ThreadPool& g = thread_pool;
  TaskQueue& queue = g.queue;
  os_mutex_take(queue.mutex);
  while (queue.count == 0) {
    TimeBlock("sleep", ProfileType_Sleep);
    os_cond_var_wait(queue.cond_not_empty, queue.mutex);
  }
  if (queue.count == MAX_TASKS) {
    os_cond_var_signal(queue.cond_not_full);
  }
  Task t = queue.tasks[queue.head];
  queue.head = ModPow2(queue.head + 1, MAX_TASKS);
  --queue.count;
  os_mutex_drop(queue.mutex);
  return t;
}

void thread_worker(void* arg) {
  TaskQueue& queue = thread_pool.queue;
  tctx_init();
  while (true) {
    Task t = task_queue_pop();
    TimeBlock("doing job");
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
  TimeFunction;
  ThreadPool& g = thread_pool;
  g.num_threads = num_threads;
  task_queue_init();
  Loop (i, num_threads) {
    g.threads[i] = os_thread_launch(thread_worker, null);
  }
}

void thread_wait_for() {
  // TimeBlock("wait for workers", ProfileType_Sleep);
  TaskQueue& queue = thread_pool.queue;
  os_mutex_take(queue.mutex);
  if (queue.remaining_tasks > 0) {
    os_cond_var_wait(queue.finished, queue.mutex);
  }
  os_mutex_drop(queue.mutex);
}
