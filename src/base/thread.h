#pragma once
#include "os/os_core.h"

#define MAX_TASKS 1024

struct Task {
  ThreadEntryPointFn* func;
  void* arg;
};

struct TaskQueue {
  Task tasks[MAX_TASKS];
  u32 head;
  u32 tail;
  u32 count;
  u32 remaining_tasks;
  Mutex mutex;
  CondVar cond_not_empty;
  CondVar cond_not_full;
  CondVar finished;
};

struct ThreadPool {
  Thread threads[16];
  u32 num_threads;
  TaskQueue queue;
};

void task_queue_init();
void task_queue_push(Task t);
Task task_queue_pop();
void thread_worker(void* arg);
void thread_pool_init(u32 num_threads);
void thread_wait_for();
