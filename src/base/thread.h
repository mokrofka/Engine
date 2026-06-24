#pragma once
#include "os/os_core.h"
#include "containers.h"

#define MAX_TASKS 1024

MakeId(CounterId)
struct TaskId { CounterId counter_id; };

struct Task {
  ThreadEntryPointFn* func;
  void* ctx;
  CounterId counter_id;
  b32 async;
};

TaskId thread_task_push(Task t);
void thread_pool_init(u32 num_threads);
void thread_wait_task(TaskId task_id);
void thread_wait_for();

