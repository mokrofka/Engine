#pragma once
#include "os/os_core.h"
#include "containers.h"

#define MAX_TASKS 1024

struct TaskId { u32 counter_id; };

struct Task {
  ThreadEntryPointFn* func;
  void* ctx;
  u32 counter_id;
  b32 async;
};

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
  ObjectPool<u32> counters;
};

TaskId thread_task_push(Task t, b32 async = false);
void thread_pool_init(u32 num_threads);
void thread_wait_task(TaskId task_id);
void thread_wait_for();

