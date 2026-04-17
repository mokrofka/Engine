#include "lib.h"
#include "common.h"

///////////////////////////////////
// Allocators

const u32 TEST_SAMPLES = 100;
global i32 test_alignments[] = { 8, 16, 32, 64 };

intern void test_global_alloc() {
  Array<u8*, TEST_SAMPLES> arr = {};
  Allocator alloc = mem_get_global_allocator();
  Loop (i, TEST_SAMPLES) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr.add(mem_alloc(alloc, size, align));
    MemZero(arr[i], size);
  }
  Array<u32, TEST_SAMPLES> indices = {};
  Loop(i, TEST_SAMPLES) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, TEST_SAMPLES) {
    mem_free(alloc, arr[indices[i]]);
  }
  arr.clear();
  Loop (i, TEST_SAMPLES) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr.add(mem_alloc(alloc, size, align));
    MemZero(arr[i], size);
  }
  indices.clear();
  Loop(i, TEST_SAMPLES) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, TEST_SAMPLES) {
    mem_free(alloc, arr[indices[i]]);
  }
}

intern void test_arena_alloc() {
  Arena arena = arena_init();
  Array<u8*, TEST_SAMPLES> arr = {};
  Array<u32, TEST_SAMPLES> sizes = {};
  Array<u32, TEST_SAMPLES> values = {};
  Loop (i, TEST_SAMPLES) {
    u32 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr[i] = push_buffer(arena, size, align);
    sizes[i] = size;
    values[i] = rand_range_u32(0, 255);
    MemSet(arr[i], values[i], size);
  }
  Loop (i, TEST_SAMPLES) {
    u8* buf = arr[i];
    u32 size = sizes[i];
    u32 value = values[i];
    Loop (j, size) {
      Assert(buf[j] == value);
    }
  }
  arena_deinit(&arena);
}

intern void test_arena_list_alloc() {
  Scratch scratch;
  ArenaList arena(scratch);
  Array<u8*, TEST_SAMPLES> arr = {};
  Array<u32, TEST_SAMPLES> sizes = {};
  Array<u32, TEST_SAMPLES> values = {};
  Loop (i, TEST_SAMPLES) {
    u32 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr[i] = push_buffer(arena, size, align);
    sizes[i] = size;
    values[i] = rand_range_u32(0, 255);
    MemSet(arr[i], values[i], size);
  }
  Loop (i, TEST_SAMPLES) {
    u8* buf = arr[i];
    u32 size = sizes[i];
    u32 value = values[i];
    Loop (j, size) {
      Assert(buf[j] == value);
    }
  }
  arena.clear();
  Loop (i, TEST_SAMPLES) {
    u32 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr[i] = push_buffer(arena, size, align);
    sizes[i] = size;
    values[i] = rand_range_u32(0, 255);
    MemSet(arr[i], values[i], size);
  }
  Loop (i, TEST_SAMPLES) {
    u8* buf = arr[i];
    u32 size = sizes[i];
    u32 value = values[i];
    Loop (j, size) {
      Assert(buf[j] == value);
    }
  }
  arena.clear();
}

intern void test_seglist_alloc() {
  Scratch scratch;
  AllocSegList alloc(scratch);
  Array<u8*, TEST_SAMPLES> arr = {};
  Loop (i, TEST_SAMPLES) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr.add(mem_alloc(alloc, size, align));
    MemZero(arr[i], size);
  }
  Array<u32, TEST_SAMPLES> indices = {};
  Loop(i, TEST_SAMPLES) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, TEST_SAMPLES) {
    mem_free(alloc, arr[indices[i]]);
  }
  arr.clear();
  Loop (i, TEST_SAMPLES) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr.add(mem_alloc(alloc, size, align));
    MemZero(arr[i], size);
  }
  indices.clear();
  Loop(i, TEST_SAMPLES) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, TEST_SAMPLES) {
    mem_free(alloc, arr[indices[i]]);
  }
}

intern void test_gpu_seglist_alloc() {
  Scratch scratch;
  GpuAllocSegList alloc = {.cap = MB(1)};
  alloc.init(scratch);
  Array<GpuMemHandler, TEST_SAMPLES> arr = {};
  Loop (i, TEST_SAMPLES) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr.add(alloc.alloc(size, align));
  }
  Array<u32, TEST_SAMPLES> indices = {};
  Loop(i, TEST_SAMPLES) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, TEST_SAMPLES) {
    alloc.free(arr[indices[i]]);
  }
  arr.clear();
  Loop (i, TEST_SAMPLES) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = test_alignments[rand_range_u32(0, 3)];
    arr.add(alloc.alloc(size, align));
  }
  indices.clear();
  Loop(i, TEST_SAMPLES) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, TEST_SAMPLES) {
    alloc.free(arr[indices[i]]);
  }
}

///////////////////////////////////
// Containters

intern void test_object_pool() {
  Scratch scratch;
  struct A {
    u32 a;
    u32 b;
  };
  ObjectPool<A> pool(scratch);
  Array<A, TEST_SAMPLES> values = {};
  Array<Handle<A>, TEST_SAMPLES> handlers = {};
  Loop (i, TEST_SAMPLES) {
    values[i].a = rand_range_u32(0, TEST_SAMPLES);
    values[i].b = rand_range_u32(0, TEST_SAMPLES);
  };
  Loop (i, TEST_SAMPLES) {
    handlers[i] = pool.add(values[i]);
  }
  Loop (i, TEST_SAMPLES) {
    Assert(MemMatchStruct(&values[i], &pool.get(handlers[i])));
  }
  Array<u32, TEST_SAMPLES> indices = {};
  Loop(i, TEST_SAMPLES) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, TEST_SAMPLES) {
    pool.remove(handlers[i]);
  }
  indices.clear();
  Loop (i, TEST_SAMPLES) {
    values[i].a = rand_range_u32(0, TEST_SAMPLES);
    values[i].b = rand_range_u32(0, TEST_SAMPLES);
  };
  Loop (i, TEST_SAMPLES) {
    handlers[i] = pool.add(values[i]);
  }
  Loop (i, TEST_SAMPLES) {
    Assert(MemMatchStruct(&values[i], &pool.get(handlers[i])));
  }
  Loop(i, TEST_SAMPLES) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, TEST_SAMPLES) {
    pool.remove(handlers[i]);
  }
}

intern void test_handle_darray() {
  Scratch scratch;
  struct A {
    u32 a;
    u32 b;
  };
  DarrayHandler<A> arr = {};
  Array<A, TEST_SAMPLES> values = {};
  Array<Handle<A>, TEST_SAMPLES> handlers = {};
  Loop (i, TEST_SAMPLES) {
    values[i].a = rand_range_u32(0, TEST_SAMPLES);
    values[i].b = rand_range_u32(0, TEST_SAMPLES);
  };
  Loop (i, TEST_SAMPLES) {
    handlers[i] = arr.add(values[i]);
  }
  Loop (i, TEST_SAMPLES) {
    Assert(MemMatchStruct(&values[i], &arr.get(handlers[i])));
  }
  Array<u32, TEST_SAMPLES> indices = {};
  Loop(i, TEST_SAMPLES) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, TEST_SAMPLES) {
    arr.remove(handlers[i]);
  }
  indices.clear();
  Loop (i, TEST_SAMPLES) {
    values[i].a = rand_range_u32(0, TEST_SAMPLES);
    values[i].b = rand_range_u32(0, TEST_SAMPLES);
  };
  Loop (i, TEST_SAMPLES) {
    handlers[i] = arr.add(values[i]);
  }
  Loop (i, TEST_SAMPLES) {
    Assert(MemMatchStruct(&values[i], &arr.get(handlers[i])));
  }
  Loop(i, TEST_SAMPLES) indices.add(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, TEST_SAMPLES) {
    arr.remove(handlers[i]);
  }
}

intern void test_id_pool() {

}

///////////////////////////////////
// Profiler

intern void profiler_bar() {
  TimeFunction;
  {
  TimeBlock("block in bar");
  os_sleep_ms(1);
  }
  os_sleep_ms(2);
}

intern void profiler_der() {
  TimeFunction;
  os_sleep_ms(10);
}

intern void profiler_die(i32 i) {
  TimeFunction;
  os_sleep_ms(1);
  if (--i) {
    profiler_die(i);
  }
}

intern void profile_print_time_elapsed(u64 total_tsc_elapsed, ProfileAnchor anchor) {
  Scratch scratch;
  String label_c = push_str_copy(scratch, anchor.label);
  f64 percent = 100.0 * ((f64)anchor.tsc_elapsed_exclusive / (f64)total_tsc_elapsed);
  print("  %s[%u64]: %u64 (%.2f%%)", label_c.str, anchor.hit_count, anchor.tsc_elapsed_exclusive, percent);
  if (anchor.tsc_elapsed_inclusive != anchor.tsc_elapsed_exclusive) {
    f64 percent_with_children = 100.0 * ((f64)anchor.tsc_elapsed_inclusive / (f64)total_tsc_elapsed);
    print(", %.2f%% w/children", percent_with_children);
  }
  print(")\n");
}

intern void profiler_print() {
  u64 cpu_freq = cpu_frequency();
  u64 total_cpu_elapsed = profiler_get_tsc_elapsed();
  if (cpu_freq) {
    print("\nTotal time: %0.4fms (CPU freq %lu)\n", 1000.0 * (f64)total_cpu_elapsed / (f64)cpu_freq, cpu_freq);
  }
  Slice<ProfileAnchor> anchors = profiler_get_anchors();
  Loop (anchor_idx, anchors.count) {
    ProfileAnchor anchor = anchors[anchor_idx];
    if (anchor.tsc_elapsed_inclusive) {
      profile_print_time_elapsed(total_cpu_elapsed, anchor);
    }
  }
}

intern void profiler_test() {
  profiler_begin();
  profiler_bar();
  profiler_bar();
  profiler_der();
  profiler_die(10);
}

struct Transform1 {
  v3 pos;
  // v3 arr[2];
};

void task_queue_push(Task t);
u32 task_queue_count();

struct TransformTask {
  Transform1* transforms;
  u32* idx_arr;       // optional for random access
  u64 count;
  v3* out_sum;        // where the thread accumulates
  b32 use_random;
};

void transform_task_func(void* arg) {
  TransformTask* t = (TransformTask*)arg;
  v3 sum = {0, 0, 0};
  if (t->use_random) {
    for (u64 i = 0; i < t->count; ++i) {
      sum += t->transforms[t->idx_arr[i]].pos;
    }
  }
  else {
    for (u64 i = 0; i < t->count; ++i) {
      sum += t->transforms[i].pos;
    }
  }
  // store result (safe if each thread writes to its own slot)
  *t->out_sum = sum;
}


void test_bandwidth() {
  Scratch scratch;
  thread_pool_init(4);

  u64 cpu_frequ = cpu_frequency();
  Info("%.2f Ghz/s cpu frequency", (f64)cpu_frequ / Billion(1));
  #define IterationNum (MB(10))
  #define NUM_THREADS 4
  #define IterationChunk (IterationNum / NUM_THREADS)
  #define SIZE (IterationNum * sizeof(Transform1))
  #define USEFUL_SIZE (IterationNum * sizeof(v3))
  Info("%.2f mb Memory size", (f64)SIZE / MB(1));

  u8* idx_buf = os_reserve(IterationNum*sizeof(u32));
  os_commit(idx_buf, IterationNum*sizeof(u32));
  u8* buf = os_reserve(SIZE);
  os_commit(buf, SIZE);

  u32* idx_arr = (u32*)idx_buf;
  Loop(i, IterationNum) {
    idx_arr[i] = i;
  }
  for (u32 i = IterationNum - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(idx_arr[i], idx_arr[j]);
  }
  Transform1* transforms = (Transform1*)buf;
  Loop (i, IterationNum) {
    transforms[i].pos = v3_rand_range(v3_scale(-10), v3_scale(10));
  }

#if 0
  Loop (i, 1) {
    u64 now = cpu_timer_now();
    v3 vec_res = {};
    Loop (i, IterationNum) {
      vec_res += transforms[i].pos;
    }
    u64 end = cpu_timer_now();
    f64 seconds = f64(end-now) / cpu_frequ;
    f64 bandwidth_gb = SIZE / seconds / GB(1);
    f64 useful_bandwidth_gb = USEFUL_SIZE / seconds / GB(1);
    Info("%.2f GB/s", bandwidth_gb);
    Info("%.2f useful GB/s", useful_bandwidth_gb);
    Info("%f took sec", seconds);
    Info("%f %f %f\n\n", vec_res.x, vec_res.y, vec_res.z);
  }
  Info("------------- RANDOM ACCESS");
  Loop (i, 1) {
    u64 now = cpu_timer_now();
    v3 vec_res = {};
    Loop (i, IterationNum) {
      vec_res += transforms[idx_arr[i]].pos;
    }
    u64 end = cpu_timer_now();
    f64 seconds = f64(end-now) / cpu_frequ;
    f64 bandwidth_gb = SIZE / seconds / GB(1);
    f64 useful_bandwidth_gb = USEFUL_SIZE / seconds / GB(1);
    Info("%.2f GB/s", bandwidth_gb);
    Info("%.2f useful GB/s", useful_bandwidth_gb);
    Info("%f took sec", seconds);
    Info("%f %f %f\n\n", vec_res.x, vec_res.y, vec_res.z);
  }
#else

  Loop (k, 2) {
    Info("---------------- LINEAR ACCESS");
    Loop (m, 2) {
      u64 now = cpu_timer_now();
      v3 thread_sums[NUM_THREADS] = {};
      TransformTask trans_tasks[NUM_THREADS] = {};
      Task tasks[NUM_THREADS] = {};
      Loop (i, NUM_THREADS) {
        trans_tasks[i] = {
          .transforms = transforms + i*IterationChunk,
          .idx_arr = idx_arr + i*IterationChunk,
          .count = IterationChunk,
          .out_sum = &thread_sums[i],
          .use_random = false,
        };
        tasks[i] = {
          .func = transform_task_func,
          .arg = &trans_tasks[i],
        };
        task_queue_push(tasks[i]);
      }
      while (true) {
        if (task_queue_count() == 0) {
          break;
        }
        os_sleep_ms(5);
      }
      u64 end = cpu_timer_now();
      f64 seconds = f64(end-now) / cpu_frequ;
      f64 bandwidth_gb = SIZE / seconds / GB(1);
      f64 useful_bandwidth_gb = USEFUL_SIZE / seconds / GB(1);
      Info("%.2f GB/s", bandwidth_gb);
      Info("%.2f useful GB/s", useful_bandwidth_gb);
      Info("%f took sec", seconds);
      v3 vec_res = {};
      Loop (i, NUM_THREADS) {
        vec_res += thread_sums[i];
      }
      Info("%f %f %f\n\n", vec_res.x, vec_res.y, vec_res.z);
    }

    Info("---------------- RANDOM ACCESS");
    Loop (m, 2) {
      u64 now = cpu_timer_now();
      v3 thread_sums[NUM_THREADS] = {};
      TransformTask trans_tasks[NUM_THREADS] = {};
      Task tasks[NUM_THREADS] = {};
      Loop (i, NUM_THREADS) {
        trans_tasks[i] = {
          .transforms = transforms,
          .idx_arr = idx_arr + i*IterationChunk,
          .count = IterationChunk,
          .out_sum = &thread_sums[i],
          .use_random = true,
        };
        tasks[i] = {
          .func = transform_task_func,
          .arg = &trans_tasks[i],
        };
        task_queue_push(tasks[i]);
      }
      while (true) {
        if (task_queue_count() == 0) {
          break;
        }
      }
      u64 end = cpu_timer_now();
      f64 seconds = f64(end-now) / cpu_frequ;
      f64 bandwidth_gb = SIZE / seconds / GB(1);
      f64 useful_bandwidth_gb = USEFUL_SIZE / seconds / GB(1);
      Info("%.2f GB/s", bandwidth_gb);
      Info("%.2f useful GB/s", useful_bandwidth_gb);
      Info("%f took sec", seconds);
      v3 vec_res = {};
      Loop (i, NUM_THREADS) {
        vec_res += thread_sums[i];
      }
      Info("%f %f %f\n\n", vec_res.x, vec_res.y, vec_res.z);
    }
  }


#endif

  os_exit(0);
  
  // u8* 

  // i32 arr[] = {5, 2, 9, 1, 5, 6};
  // i32 n = sizeof(arr)/sizeof(arr[0]);
  // for (var i : arr) {
  //   print("%i ", i);
  // }
  // quick_sort(arr, 0, n - 1);
  // print("\n");
  // for (var i : arr) {
  //   print("%i ", i);
  // }

  // RingBuffer ring = {
  //   .base = push_buffer(scratch, KB(1)),
  //   .size = KB(1),
  // };
  // u64 size = 128;
  // u8* buf0 = push_buffer(scratch, size);
  // Loop (i, size) { buf0[i] = 0; }
  // u8* buf1 = push_buffer(scratch, size);
  // ring_write(ring, buf0, size);
  // ring_read(ring, buf1, size);
  // Loop (i, size) { Info("%i", buf1[i]); }

  // ring_read(ring, Slice(buf0, size));

}


intern void test() {
  test_global_alloc();
  test_arena_alloc();
  test_arena_list_alloc();
  test_seglist_alloc();
  test_gpu_seglist_alloc();
  test_object_pool();
  test_handle_darray();
  test_id_pool();
}
