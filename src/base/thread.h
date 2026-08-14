#pragma once
#include "os/os_core.h"

const u32 Thread_MaxTasks = 1024;
const u32 Thread_MaxCounters = 1024;
const u32 Thread_NumWorkers = 2;

MakeId(WaitGroup);

enum TaskPriority {
  TaskPriority_High, // physics, gameplay-critical
  TaskPriority_Low,    // background IO, asset streaming
  TaskPriority_COUNT,
};

typedef void TaskFn(void* ctx);

struct TaskDesc {
  void* ctx;
  TaskFn* fn;
  TaskPriority priority;
};

struct Task {
  TaskFn* fn;
  void* ctx;
  WaitGroup wg;
  TaskPriority priority;
};

typedef void (*ParallelForFn)(void* ctx, u32 base, u32 count);

struct ParallelForCtx {
  void* ctx;
  ParallelForFn fn;
  u32 next_idx;
  u32 count;
  u32 chunk_size;
};

WaitGroup thread_wg_make(u32 count);

WaitGroup thread_push(TaskDesc desc);
WaitGroup thread_push_batch(Slice<TaskDesc> tasks);

void thread_wg_wait(WaitGroup wg);
void thread_wait_remanings();
u8* _thread_push_ctx(u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
#define thread_push_ctx(T) *(T*)_thread_push_ctx(sizeof(T), alignof(T))
void thread_pool_init();

template<typename T, typename F> void thread_parallel_for(u32 chunk_size, Slice<T> s, F fn) {
  if (s.count == 0) return;
  Scratch scratch;
  u32 task_count = CeilIntDiv(s.count, chunk_size);
  var tasks = push_slice_zero(scratch, TaskDesc, task_count);
  u32 task_id = 0;
  for (u32 i = 0; i < s.count; i += chunk_size) {
    struct TaskCtx {
      Slice<T> s;
      F fn;
    };
    var& ctx = thread_push_ctx(TaskCtx);
    ctx.s = slice_n(s, i, Min(chunk_size, s.size - i));
    ctx.fn = fn;
    tasks[task_id].ctx = &ctx;
    tasks[task_id].fn = [](void* ctx) {
      TaskCtx data = *(TaskCtx*)ctx;
      data.fn(data.s);
    };
    ++task_id;
  }
  WaitGroup wg = thread_push_batch(tasks);
  thread_wg_wait(wg);
}
