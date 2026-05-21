#pragma once
#include "os/os_core.h"

#define MAX_TASKS 1024

struct Task {
  ThreadEntryPointFn* func;
  void* arg;
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
};

void thread_task_push(Task t);
void thread_pool_init(u32 num_threads);
void thread_wait_for();
