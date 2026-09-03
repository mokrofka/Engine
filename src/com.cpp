#include "com.h"

#include "gfx.cpp"
#include "render.cpp"
#include "tokenizer.cpp"

#include "generated.h"

R_Vertex cube_vertices[] = {
  // Front face (0, 0, 1)
  {.pos = v3(-1.00, -1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3( 1.00, -1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3( 1.00,  1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3( 1.00,  1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3(-1.00, -1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(0.0f, 0.0f)},

  // Back face (0, 0, -1)
  {.pos = v3( 1.00, -1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3(-1.00, -1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3(-1.00,  1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3( 1.00,  1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3( 1.00, -1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(0.0f, 0.0f)},

  // Left face (-1, 0, 0)
  {.pos = v3(-1.00, -1.00, -1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3(-1.00, -1.00,  1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3(-1.00,  1.00,  1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00,  1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00, -1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3(-1.00, -1.00, -1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},

  // Right face (1, 0, 0)
  {.pos = v3(1.00, -1.00,  1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3(1.00, -1.00, -1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3(1.00,  1.00, -1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(1.00,  1.00, -1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(1.00,  1.00,  1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3(1.00, -1.00,  1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},

  // Bottom face (0, -1, 0)
  {.pos = v3(-1.00, -1.00, -1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3( 1.00, -1.00, -1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3( 1.00, -1.00,  1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3( 1.00, -1.00,  1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3(-1.00, -1.00,  1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3(-1.00, -1.00, -1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(0.0f, 1.0f)},

  // Top face (0, 1, 0)
  {.pos = v3(-1.00,  1.00,  1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3( 1.00,  1.00,  1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3( 1.00,  1.00, -1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3( 1.00,  1.00, -1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00, -1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00,  1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},
};

R_Vertex triangle_vertices[] = {
  {.pos = v3( 0.0,   0.5, 0), .uv = v2(0.5, 0), .color = v4(1,0,0,1)},
  {.pos = v3(-0.5,  -0.5, 0), .uv = v2(0.0, 1), .color = v4(0,1,0,1)},
  {.pos = v3( 0.5,  -0.5, 0), .uv = v2(1.0, 1), .color = v4(0,0,1,1)},
};

R_Vertex axis_vertices[] = {
  {.pos = v3(),      .color = v4(1,0,0,1)},
  {.pos = v3(1,0,0), .color = v4(1,0,0,1)},
  {.pos = v3(),      .color = v4(0,1,0,1)},
  {.pos = v3(0,1,0), .color = v4(0,1,0,1)},
  {.pos = v3(),      .color = v4(0,0,1,1)},
  {.pos = v3(0,0,1), .color = v4(0,0,1,1)},
};

global String meshes_strs[] = {
#define X(name) [Glue(Mesh_, name)] = Stringify(name),
  MESH_LIST
#undef X
};

global String textures_strs[] = {
#define X(name) [Glue(Texture_, name)] = Stringify(name),
  TEXTURE_LIST
#undef X
};

global String materials_strs[] = {
#define X(name) [Glue(Material_, name)] = Stringify(name),
  MATERIAL_LIST
#undef X
};

Extern GlobalState* st;

///////////////////////////////////
// Allocators

const u32 TEST_SAMPLES = 100;
global i32 test_alignments[] = { 8, 16, 32, 64 };

intern void test_arena_alloc() {
  Arena arena = arena_make();
  Array<u8*, TEST_SAMPLES> arr = {};
  Array<u32, TEST_SAMPLES> sizes = {};
  Array<u32, TEST_SAMPLES> values = {};
  Loop (i, TEST_SAMPLES) {
    u32 size = rand_u32_rng(8, KB(1));
    u64 align = ArrayRand(test_alignments);
    arr[i] = push_buffer(arena, size, align);
    sizes[i] = size;
    values[i] = rand_u32_rng(0, 255);
    MemSet(arr[i], values[i], size);
  }
  Loop (i, TEST_SAMPLES) {
    u8* buf = arr[i];
    u32 size = sizes[i];
    u32 value = values[i];
    Loop (j, size) {
      AssertAlways(buf[j] == value);
    }
  }
  arena_destroy(arena);
}

intern void test_arena_list_alloc() {
  Scratch scratch;
  ArenaList arena(scratch);
  Array<u8*, TEST_SAMPLES> arr = {};
  Array<u32, TEST_SAMPLES> sizes = {};
  Array<u32, TEST_SAMPLES> values = {};

  Loop (i, TEST_SAMPLES) {
    u32 size = rand_u32_rng(8, KB(1));
    u64 align = ArrayRand(test_alignments);
    arr[i] = push_buffer(arena, size, align);
    sizes[i] = size;
    values[i] = rand_u32_rng(0, 255);
    MemSet(arr[i], values[i], size);
  }
  Loop (i, TEST_SAMPLES) {
    u8* buf = arr[i];
    u32 size = sizes[i];
    u32 value = values[i];
    Loop (j, size) {
      AssertAlways(buf[j] == value);
    }
  }
  alloc_arena_list_clear(arena);

  Loop (i, TEST_SAMPLES) {
    u32 size = rand_u32_rng(8, KB(1));
    u64 align = ArrayRand(test_alignments);
    arr[i] = push_buffer(arena, size, align);
    sizes[i] = size;
    values[i] = rand_u32_rng(0, 255);
    MemSet(arr[i], values[i], size);
  }
  Loop (i, TEST_SAMPLES) {
    u8* buf = arr[i];
    u32 size = sizes[i];
    u32 value = values[i];
    Loop (j, size) {
      AssertAlways(buf[j] == value);
    }
  }
  alloc_arena_list_clear(arena);
}

intern void test_seglist_alloc() {
  Arena arena = arena_make();
  Alloc alloc = alloc_make(arena);
  defer(alloc_destroy(alloc));

  struct Mem {
    u8* data;
    u64 size;
  };
  Array<Mem, TEST_SAMPLES> arr = {};

  Loop (i, TEST_SAMPLES) {
    u64 size = rand_u32_rng(8, KB(1));
    u64 align = ArrayRand(test_alignments);
    array_push(arr, {mem_alloc(alloc, size, align), size});
    MemZero(arr[i].data, size);
  }
  Array<u32, TEST_SAMPLES> indices = {};
  Loop(i, TEST_SAMPLES) array_push(indices, i);
  rand_shuffle(slice(indices));
  Loop (i, TEST_SAMPLES) {
    mem_free(alloc, arr[indices[i]].data, arr[indices[i]].size);
  }

  array_clear(arr);
  Loop (i, TEST_SAMPLES) {
    u64 size = rand_u32_rng(8, KB(1));
    u64 align = ArrayRand(test_alignments);
    array_push(arr, {mem_alloc(alloc, size, align), size});
    MemZero(arr[i].data, size);
  }
  array_clear(indices);
  Loop(i, TEST_SAMPLES) array_push(indices, i);
  rand_shuffle(slice(indices));
  Loop (i, TEST_SAMPLES) {
    mem_free(alloc, arr[indices[i]].data, arr[indices[i]].size);
  }
  arena_destroy(arena);
}

intern void test_gpu_seglist_alloc() {
  Scratch scratch;
  GpuAllocSegList alloc = {.cap = MB(1)};
  alloc = gpu_alloc_seglist_make(scratch);
  Array<GpuMemId, TEST_SAMPLES> arr = {};

  Loop (i, TEST_SAMPLES) {
    u64 size = rand_u32_rng(8, KB(1));
    u64 align = ArrayRand(test_alignments);
    array_push(arr, gpu_alloc_seglist_alloc(alloc, size, align));
  }
  Array<u32, TEST_SAMPLES> indices = {};
  Loop(i, TEST_SAMPLES) array_push(indices, i);
  rand_shuffle(slice(indices));
  Loop (i, TEST_SAMPLES) {
    gpu_alloc_seglist_free(alloc, arr[indices[i]]);
  }

  array_clear(arr);
  Loop (i, TEST_SAMPLES) {
    u64 size = rand_u32_rng(8, KB(1));
    u64 align = ArrayRand(test_alignments);
    array_push(arr, gpu_alloc_seglist_alloc(alloc, size, align));
  }
  array_clear(indices);
  Loop(i, TEST_SAMPLES) array_push(indices, i);
  rand_shuffle(slice(indices));
  Loop (i, TEST_SAMPLES) {
    gpu_alloc_seglist_free(alloc, arr[indices[i]]);
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
  var pool = pool_make(A, OpaqueId, scratch);
  Array<A, TEST_SAMPLES> values = {};
  Array<OpaqueId, TEST_SAMPLES> handlers = {};

  Loop (i, TEST_SAMPLES) {
    values[i].a = rand_u32_rng(0, TEST_SAMPLES);
    values[i].b = rand_u32_rng(0, TEST_SAMPLES);
  };
  Loop (i, TEST_SAMPLES) {
    handlers[i] = pool_push(pool, values[i]);
  }
  Loop (i, TEST_SAMPLES) {
    AssertAlways(MemMatchStruct(&values[i], &pool_get(pool, handlers[i])));
  }
  Array<u32, TEST_SAMPLES> indices = {};
  Loop(i, TEST_SAMPLES) array_push(indices, i);
  rand_shuffle(slice(indices));
  Loop (i, TEST_SAMPLES) {
    pool_remove(pool, handlers[i]);
  }

  array_clear(indices);
  Loop (i, TEST_SAMPLES) {
    values[i].a = rand_u32_rng(0, TEST_SAMPLES);
    values[i].b = rand_u32_rng(0, TEST_SAMPLES);
  };
  Loop (i, TEST_SAMPLES) {
    handlers[i] = pool_push(pool, values[i]);
  }
  Loop (i, TEST_SAMPLES) {
    AssertAlways(MemMatchStruct(&values[i], &pool_get(pool, handlers[i])));
  }
  Loop(i, TEST_SAMPLES) array_push(indices, i);
  rand_shuffle(slice(indices));
  Loop (i, TEST_SAMPLES) {
    pool_remove(pool, handlers[i]);
  }
}

intern void test_object_pool_linklist() {
  Scratch scratch;
  struct A {
    u32 a;
    u32 b;
  };
  var pool = pool_linklist_make(A, OpaqueId, scratch);
  Array<A, TEST_SAMPLES> values = {};
  Array<OpaqueId, TEST_SAMPLES> handlers = {};

  Loop (i, TEST_SAMPLES) {
    values[i].a = rand_u32_rng(0, TEST_SAMPLES);
    values[i].b = rand_u32_rng(0, TEST_SAMPLES);
  };
  Loop (i, TEST_SAMPLES) {
    handlers[i] = pool_push(pool, values[i]);
  }
  Loop (i, TEST_SAMPLES) {
    AssertAlways(MemMatchStruct(&values[i], &pool_get(pool, handlers[i])));
  }
  Array<u32, TEST_SAMPLES> indices = {};
  Loop(i, TEST_SAMPLES) array_push(indices, i);
  rand_shuffle(slice(indices));

  u32 i = 0;
  LoopIter (it, pool_begin(pool)) {
    A elem = *it;
    AssertAlways(elem.a == values[i].a && elem.a == values[i].a);
    ++i;
  }
  Loop (i, TEST_SAMPLES) {
    pool_remove(pool, handlers[i]);
  }

  array_clear(indices);
  Loop (i, TEST_SAMPLES) {
    values[i].a = rand_u32_rng(0, TEST_SAMPLES);
    values[i].b = rand_u32_rng(0, TEST_SAMPLES);
  };
  Loop (i, TEST_SAMPLES) {
    handlers[i] = pool_push(pool, values[i]);
  }
  Loop (i, TEST_SAMPLES) {
    AssertAlways(MemMatchStruct(&values[i], &pool_get(pool, handlers[i])));
  }
  Loop(i, TEST_SAMPLES) array_push(indices, i);
  rand_shuffle(slice(indices));
  i = 0;
  
  LoopIter (it, pool_begin(pool)) {
    A elem = *it;
    AssertAlways(elem.a == values[i].a && elem.a == values[i].a);
    ++i;
  }
  Loop (i, TEST_SAMPLES) {
    pool_remove(pool, handlers[i]);
  }
}

intern void test_handle_darray() {
  Scratch scratch;
  struct A {
    u32 a;
    u32 b;
  };
  var arr = array_handler_make(A, OpaqueId, scratch);
  Array<A, TEST_SAMPLES> values = {};
  Array<OpaqueId, TEST_SAMPLES> handlers = {};

  Loop (i, TEST_SAMPLES) {
    values[i].a = rand_u32_rng(0, TEST_SAMPLES);
    values[i].b = rand_u32_rng(0, TEST_SAMPLES);
  };
  Loop (i, TEST_SAMPLES) {
    handlers[i] = array_handler_push(arr, values[i]);
  }
  Loop (i, TEST_SAMPLES) {
    AssertAlways(MemMatchStruct(&values[i], &array_handler_get(arr, handlers[i])));
  }
  Array<u32, TEST_SAMPLES> indices = {};
  Loop (i, TEST_SAMPLES) array_push(indices, i);
  rand_shuffle(slice(indices));
  Loop (i, TEST_SAMPLES) {
    array_handler_remove(arr, handlers[indices[i]]);
  }

  array_clear(indices);
  Loop (i, TEST_SAMPLES) {
    values[i].a = rand_u32_rng(0, TEST_SAMPLES);
    values[i].b = rand_u32_rng(0, TEST_SAMPLES);
  };
  Loop (i, TEST_SAMPLES) {
    handlers[i] = array_handler_push(arr, values[i]);
  }
  Loop (i, TEST_SAMPLES) {
    AssertAlways(MemMatchStruct(&values[i], &array_handler_get(arr, handlers[i])));
  }
  Loop(i, TEST_SAMPLES) array_push(indices, i);
  rand_shuffle(slice(indices));
  Loop (i, TEST_SAMPLES) {
    array_handler_remove(arr, handlers[indices[i]]);
  }
}

intern void test_id_pool() {
  Scratch scratch;
  {
    IdPool id_pool = id_pool_make(scratch);
    Array<u32, TEST_SAMPLES> arr = {};
    Loop (i, TEST_SAMPLES) {
      u32 id = id_pool_push(id_pool);
      array_push(arr, id);
    }
    rand_shuffle(slice(arr));
    Loop (i, arr.count) {
      id_pool_remove(id_pool, arr[i]);
    }
    Array<u32, TEST_SAMPLES> new_arr = {};
    Loop (i, arr.count) {
      u32 id = id_pool_push(id_pool);
      array_push(new_arr, id);
    }
    Loop (i, arr.count) {
      b32 exists = false;
      Loop (j, arr.count) {
        if (id_idx(arr[i]) == id_idx(new_arr[j])) {
          AssertAlways(exists == false);
          exists = true;
        }
      }
      AssertAlways(exists);
    }
    Loop (i, arr.count) {
      id_pool_remove(id_pool, new_arr[i]);
    }
  }

  {
    StaticIdPool<TEST_SAMPLES> static_id_pool = {};
    id_pool_init(static_id_pool);
    Array<u32, TEST_SAMPLES> arr = {};
    Loop (i, TEST_SAMPLES) {
      u32 id = id_pool_push(static_id_pool);
      array_push(arr, id);
    }
    rand_shuffle(slice(arr));
    Loop (i, arr.count) {
      id_pool_remove(static_id_pool, arr[i]);
    }
    Array<u32, TEST_SAMPLES> new_arr = {};
    Loop (i, arr.count) {
      u32 id = id_pool_push(static_id_pool);
      array_push(new_arr, id);
    }
    Loop (i, arr.count) {
      b32 exists = false;
      Loop (j, arr.count) {
        if (id_idx(arr[i]) == id_idx(new_arr[j])) {
          AssertAlways(exists == false);
          exists = true;
        }
      }
      AssertAlways(exists);
    }
    Loop (i, arr.count) {
      id_pool_remove(static_id_pool, new_arr[i]);
    }
  }
}

///////////////////////////////////
// Profiler

intern void test_profiler_bar() {
  ProfFunc;
  {
  ProfBlock("block in bar");
  os_sleep_ms(1);
  }
  os_sleep_ms(2);
}

intern void test_profiler_der() {
  ProfFunc;
  os_sleep_ms(10);
}

intern void test_profiler_die(i32 i) {
  ProfFunc;
  os_sleep_ms(1);
  if (--i) {
    test_profiler_die(i);
  }
}

// intern void test_profile_print_time_elapsed(u64 total_tsc_elapsed, ProfileAnchor anchor) {
//   Scratch scratch;
//   String label_c = push_str_copy(scratch, anchor.label);
//   f64 percent = 100.0 * ((f64)anchor.tsc_elapsed_exclusive / (f64)total_tsc_elapsed);
//   print("  %s[%u64]: %u64 (%.2f%%)", label_c.str, anchor.hit_count, anchor.tsc_elapsed_exclusive, percent);
//   if (anchor.tsc_elapsed_inclusive != anchor.tsc_elapsed_exclusive) {
//     f64 percent_with_children = 100.0 * ((f64)anchor.tsc_elapsed_inclusive / (f64)total_tsc_elapsed);
//     print(", %.2f%% w/children", percent_with_children);
//   }
//   print(")\n");
// }

// intern void test_profiler_print() {
  // u64 cpu_freq = cpu_frequency();
  // u64 total_cpu_elapsed = g_st->profiler.prev_tsc_elapsed;
  // if (cpu_freq) {
  //   print("\nTotal time: %0.4fms (CPU freq %lu)\n", 1000.0 * (f64)total_cpu_elapsed / (f64)cpu_freq, cpu_freq);
  // }
  // Slice<ProfileAnchor> anchors = profiler_get_current_frame_anchors();
  // Loop (anchor_idx, anchors.count) {
  //   ProfileAnchor anchor = anchors[anchor_idx];
  //   if (anchor.tsc_elapsed_inclusive) {
  //     test_profile_print_time_elapsed(total_cpu_elapsed, anchor);
  //   }
  // }
// }

intern void profiler_test() {
  // profiler_begin();
  test_profiler_bar();
  test_profiler_bar();
  test_profiler_der();
  test_profiler_die(10);
}

////////////////////////////////////////////////////////////////////////
// sort

void test_sort() {
  Scratch scratch;
  {
    Info("insert");
    i32 arr[] = {0, -4, -3, 7, 3};
    sort_insert(slice(arr), [](var a, var b) { return a < b; });
    LoopArray (i, arr) {
      print("%i ", arr[i]);
    }
    print("\n");
  }
  {
    Info("quick");
    i32 arr[] = {0, -4, -3, 7, 3};
    sort_quick(slice(arr), [](var a, var b) { return a < b;});
    LoopArray (i, arr) {
      print("%i ", arr[i]);
    }
    print("\n");
  }
  {
    Scratch scratch;
    Info("merge");
    i32 arr[] = {0, -4, -3, 7, 3};
    sort_merge(scratch, slice(arr), [](var a, var b) { return a < b;});
    LoopArray (i, arr) {
      print("%i ", arr[i]);
    }
    print("\n");
  }
  {
    Scratch scratch;
    Info("radix");
    i32 arr[] = {0, -4, -3, 7, 3};
    Slice<SortEntry> entries = push_slice(scratch, SortEntry, ArrayCount(arr));
    LoopArray (i, arr) {
      entries[i] = {sort_i32_key_to_u32(arr[i]), (u32)i};
    }
    sort_radix(scratch, entries);
    LoopArray (i, arr) {
      print("%i ", arr[entries[i].idx]);
    }
    print("\n");
  }

  u32 counts[] = {32, 64, 128, 512, KB(1), KB(10), KB(100), MB(1)};
  Slice<u32> arrs[ArrayCount(counts)];
  LoopArray (i, counts) {
    arrs[i] = push_slice(scratch, u32, counts[i]);
    Loop (j, counts[i]) {
      arrs[i][j] = rand_i32();
    }
  }
  print("\n/////////////////\n");
  Info("\nQuick test:");
  Loop (i, 2) {
    LoopArray (i, counts) {
      Info("Count %i", counts[i]);
      rand_shuffle(arrs[i]);
      {
        u64 s = cpu_now();
        sort_quick(arrs[i], [](var a, var b) {return a < b;});
        Info("%fms", tsc_to_ms(cpu_now()-s));
      }
    }
  }
  Info("\nmerge test:");
  Loop (i, 2) {
    LoopArray (i, counts) {
      Scratch scratch;
      Info("Count %i", counts[i]);
      rand_shuffle(arrs[i]);
      {
        u64 s = cpu_now();
        sort_merge(scratch, arrs[i], [](var a, var b) {return a < b;});
        Info("%fms", tsc_to_ms(cpu_now()-s));
      }
    }
  }
  Info("\nradix test:");
  Loop (i, 2) {
    LoopArray (i, counts) {
      Scratch scratch;
      Info("Count %i", counts[i]);
      rand_shuffle(arrs[i]);
      Slice<SortEntry> entries = push_slice(scratch, SortEntry, counts[i]);
      Loop (j, counts[i]) {
        entries[j] = {arrs[i][j], (u32)j};
      }
      {
        u64 s = cpu_now();
        Scratch scratch;
        sort_radix(scratch, entries);
        Info("%fms", tsc_to_ms(cpu_now()-s));
      }
    }
  }
}

void test_alloc() {
  Scratch scratch;
  Alloc alloc = alloc_make(scratch);
  defer(alloc_destroy(alloc));
  struct AllocCtx {
    u8* data;
    u64 size;
  };
  Array<AllocCtx, TEST_SAMPLES> arr = {};

  Loop (i, TEST_SAMPLES) {
    u64 size = rand_u32_rng(8, KB(1));
    u64 align = ArrayRand(test_alignments);
    array_push(arr, {mem_alloc(alloc, size, align), size});
    MemZero(arr[i].data, size);
  }
  Array<u32, TEST_SAMPLES> indices = {};
  Loop(i, TEST_SAMPLES) array_push(indices, i);
  rand_shuffle(slice(indices));
  Loop (i, TEST_SAMPLES) {
    mem_free(alloc, arr[indices[i]].data, arr[indices[i]].size);
  }

  array_clear(arr);
  Loop (i, TEST_SAMPLES) {
    u64 size = rand_u32_rng(8, KB(1));
    u64 align = ArrayRand(test_alignments);
    array_push(arr, {mem_alloc(alloc, size, align), size});
    MemZero(arr[i].data, size);
  }
  array_clear(indices);
  Loop(i, TEST_SAMPLES) array_push(indices, i);
  rand_shuffle(slice(indices));
  Loop (i, TEST_SAMPLES) {
    mem_free(alloc, arr[indices[i]].data, arr[indices[i]].size);
  }
}

void test_js() {
  String d = R"(
  {
    "name": "cube",
    "visible": true,
    "pos": [1.0, 2.0, 3.0],
    "material": {
      "color": [1.0, 0.0, 0.0],
      "roughness": 0.5
    }
  }
  )";

  Scratch scratch;

  {
    struct {
      String name;
      b32 visible;
      v3 pos;
      struct {
        v3 color;
        f32 roughness;
      } material;
    } res = {};

    JsParser p = js_parse_make(scratch, d);
    JsObj root = js_parse(&p).obj;
    Loop (i, root.fields.count) {
      var [k, v] = root.fields[i];
      if (str_match(k, "name")) {
        res.name = push_str_copy(scratch, v->str);
      }
      else if (str_match(k, "visible")) {
        res.visible = v->boolean;
      }
      else if (str_match(k, "pos")) {
        Loop (i, v->array.count) {
          res.pos.v[i] = v->array[i]->number;
        }
      }
      else if (str_match(k, "material")) {
        JsObj material = v->obj;
        Loop (i, material.fields.count) {
          var [k, v] = material.fields[i];
          if (str_match(k, "color")) {
            Loop (i, v->array.count) {
              res.material.color.v[i] = v->array[i]->number;
            }
          } else if (str_match(k, "roughness")) {
            res.material.roughness = v->number;
          }
        }
      }
    }
  }

  {
    JsParser p = js_parse_make(scratch, d);
    JsObj root = js_parse(&p).obj;
    String name = js_get_str(root, "name");
    Info("%s", name);
    b32 visible = js_get_bool(root, "visible");
    Info("%u", visible);
    Slice pos = js_get_array(root, "pos");
    Loop (i, pos.count) {
      Info("%f", pos[i]->number);
    }
    {
      JsObj material = js_get_obj(root, "material");
      Slice color = js_get_array(material, "color");
      Loop (i, color.count) {
        Info("%f", color[i]->number);
      }
      f64 roughness = js_get_number(material, "roughness");
      Info("%f", roughness);
    }
  }

  {
    JsParser p = js_parse_make(scratch, d);
    JsVal root = js_parse(&p);
    JsVal name = js_get_val(root, "name");
    Info("%s", name.str);
    JsVal visible = js_get_val(root, "visible");
    Info("%u", visible.boolean);
    JsVal pos = js_get_val(root, "pos");
    Loop (i, pos.array.count) {
      Info("%f", pos.array[i]->number);
    }
    JsVal material = js_get_val(root, "material");
    JsVal color = js_get_val(material, "color");
    Loop (i, color.array.count) {
      Info("%f", color.array[i]->number);
    }
    JsVal roughness = js_get_val(material, "roughness");
    Info("%f", roughness.number);
  }
}

void test_co1(Coroutine* co, f32 dt) {
  co_begin(co);

  Info("child");
  co_wait(co, 1, dt);
  co_yield(co);
  Info("child waits");
  co_wait(co, 1, dt);

  co_end(co);
}

void test_co(Coroutine* co, f32 dt) {
  f32 wait_time = 1;
  var& a = co_var(co, u32);
  co_begin(co);

  Info("start");
  co_wait(co, wait_time, dt);
  co_yield(co);

  Info("waited 0, %i", a++);
  co_wait(co, wait_time, dt);
  co_call(co, test_co1(co, dt));
  Info("waited 1, %i", a);
  co_wait(co, wait_time, dt);
  Info("waited 2, %i", a);

  co_end(co);
}

void test() {
  ProfFunc;
  // test_sort();
  // os_exit(0);
  test_alloc();
  test_arena_alloc();
  test_arena_list_alloc();
  test_seglist_alloc();
  test_gpu_seglist_alloc();
  test_object_pool();
  test_object_pool_linklist();
  test_handle_darray();
  test_id_pool();
}

f64 tsc_to_ms(u64 tsc) { return (f64)tsc/cpu_frequency()*1000; }
// f32 time_dt { return st->dt; }
// f32 time_now { return st->time; }
b32 time_on_interval(f64 time, f32 delta, f32 interval, f32 offset) {
	u32 last = (time - offset - delta) / interval;
	u32 next = (time - offset) / interval;
	return last < next;
}
u32 time_on_interval_steps(f64 time, f32 delta, f32 interval, f32 offset) {
  u32 last = (time - offset - delta) / interval;
  u32 next = (time - offset) / interval;
  return next - last;
}
f64 time_next_interval(f64 time, f32 interval, f32 offset) {
  u32 next = (time - offset) / interval + 1;
  return offset + next * interval;
}
f64 time_prev_interval(f64 time, f32 interval, f32 offset) {
  u32 prev = (time - offset) / interval;
  return offset + prev * interval;
}
b32 time_on_time(f64 time, f64 timestamp, f64 dt)                  { return time >= timestamp && (time - dt) < timestamp; }
b32 time_on_between_interval(f64 time, f32 interval, f32 offset)   { return Mod(time - offset, interval*2) >= interval; }
f32 time_percent(f64 time, f64 start, f64 duration)                { return (time - start) / duration; }
f32 time_lerp_delta(f32 current, f32 target, f32 rate, f32 delta)  { return target + (current - target) * Exp(-rate * delta); }
b32 time_on_frame_interval(u32 frame, u32 n, u32 offset = 0)       { return (frame + offset) % n == 0; }
f64 time_saw_wave(f64 time, f32 interval, f32 offset) {
  f64 t = (time - offset) / interval;
  return t - Floor(t);
}
f32 time_sine_wave(f64 time, f32 period) {
  return Sin(time / period * 2.0f * PI);
}
f32 time_smooth_wave(f64 time, f32 period) {
  f32 p = time_saw_wave(time, period, 0);
  return 0.5f - 0.5f * Cos(p * 2.0f * PI);
}
f32 time_triangle_wave(f64 time, f32 period) {
  f32 p = time_saw_wave(time, period, 0);
  return (1.0f - Abs(2.0f * p - 1.0f));
}
b32 time_pulse_wave(f64 time, f32 period, f32 duration, f32 offset) {
  return Mod(time - offset, period) < duration;
}
u32 time_frame(f64 time, f32 frame_duration, u32 frame_count) {
  return u32(time / frame_duration) % frame_count;
}

b32 time_elapsed(f64 time, f32 start, f32 duration) { 
  return time > start && start + duration > time;
}
b32 time_between(f64 time, f32 start, f32 end) {
  return rng1_contains({start, end}, time);
}

b32 time_on_interval(f32 interval, f32 offset)         { return time_on_interval(time_now, time_dt, interval, offset); }
b32 time_on_between_interval(f32 interval, f32 offset) { return time_on_between_interval(time_now, interval, offset); }
f64 time_since(f64 timestamp) { return time_now - timestamp; }
f64 time_until(f64 timestamp) { return timestamp - time_now; }

#define GEN_ID (__LINE__)

void ui_draw_rect(Rng2 rect, v4 color) {
  r_draw_rect(rect, color);
}

b32 ui_button(u32 id, v2 pos) {
  var& g = st->ui;
  v2 button_size = v2(64, 48);
  v2 active_off = v2(2,2);
  v2 shadow_off = v2(8,8);
  if (rng2_contains(rng2_make(pos, button_size), os_mouse_pos())) {
    g.hotitem = id;
    if (g.activeitem == 0 && g.mouse_down) {
      g.activeitem = id;
    }
  }

  if (g.kbditem == 0) {
    g.kbditem = id;
  }
  if (g.kbditem == id) {
    ui_draw_rect(rng2_make(pos-v2(6,6), v2(84,68)), ColorRed);
  }

  ui_draw_rect(rng2_make(pos + shadow_off, button_size), ColorBlack);
  if (g.hotitem == id) {
    if (g.activeitem == id) {
      ui_draw_rect(rng2_make(pos + active_off, button_size), ColorWhite);
    } else {
      ui_draw_rect(rng2_make(pos, button_size), ColorWhite);
    }
  } else {
    ui_draw_rect(rng2_make(pos, button_size), ColorGrey);
  }

  if (g.kbditem == id) {;
    if (g.tab) {
      g.kbditem = 0;
      if (os_key_modifiers() & OS_Modifier_Shift) {
        g.kbditem = g.last_widget;
      }
      g.tab = false;
    }
    if (g.enter) {
      g.enter = false;
      return true;
    }
  }
  g.last_widget = id;

  if (!g.mouse_down && g.hotitem == id && g.activeitem == id) {
    return true;
  }
  return false;
}

b32 ui_slider(u32 id, v2 pos, i32 max, i32& value) {
  var& g = st->ui;

  i32 track_height = 256;
  i32 knob_size = 16;
  i32 padding = 8;

  i32 ypos = remap(value, max, track_height - knob_size - padding);

  if (rng2_contains(rng2_make(pos+v2(padding), v2(knob_size,track_height-padding)), os_mouse_pos())) {
    g.hotitem = id;
    if (g.activeitem == 0 && g.mouse_down) {
      g.activeitem = id;
    }
  }

  if (g.kbditem == 0) {
    g.kbditem = id;
  }
  if (g.kbditem == id) {
    ui_draw_rect(rng2_make(pos-v2(4,4), v2(40,280)), ColorRed);
  }

  ui_draw_rect(rng2_make(pos, v2(knob_size*2,track_height)), rgba_from_u32(0x777777));
  if (g.activeitem == id || g.hotitem == id) {
    ui_draw_rect(rng2_make(pos + v2(padding) + v2(0,ypos), v2(knob_size)), ColorWhite);
  } else {
    ui_draw_rect(rng2_make(pos + v2(padding) + v2(0,ypos), v2(knob_size)), rgba_from_u32(0xaaaaaa));
  }

  if (g.kbditem == id) {;
    if (g.tab) {
      g.kbditem = 0;
      if (os_key_modifiers() & OS_Modifier_Shift) {
        g.kbditem = g.last_widget;
      }
      g.tab = false;
    }
    if (g.up) {
      if (value > 0) {
        --value;
        return true;
      }
      g.up = false;
    }
    if (g.down) {
      if (value < max) {
        ++value;
        return true;
      }
      g.down = false;
    }
  }
  g.last_widget = id;

  if (g.activeitem == id) {
    i32 mousepos = Clamp(0, os_mouse_pos().y - (pos.y + padding), track_height-1);
    i32 v = remap(mousepos, track_height-1, max);
    if (v != value) {
      value = v;
      return 1;
    }
  }

  return 0;
}

void ui_begin() {
  var& g = st->ui;
  g.mouse_down = os_mouse_is_button_down(MouseButton_Left);
  g.hotitem = 0;
  g.enter = os_key_is_down(Key_Enter);
  g.tab = os_key_is_down(Key_Tab);
  g.down = os_key_is_down(Key_Down);
  g.up = os_key_is_down(Key_Up);
}

void ui_end() {
  var& g = st->ui;
  if (g.activeitem == 0 && g.mouse_down) {
    g.activeitem = -1;
  } else {
    g.activeitem = 0;
  }
  if (g.tab) {
    g.kbditem = 0;
  }
  g.enter = false;
  g.tab = false;
  g.down = false;
  g.up = false;
}

ImGui_DrawList imgui_get_window_drawlist() {
  ImGui_DrawList res = {
    .draw = ImGui::GetWindowDrawList(),
  };
  return res;
}
void imgui_draw_rect(ImGui_DrawList draw, Rng2 rect, v4 col, f32 rounding, ImDrawFlags flags, f32 thickness) {
  draw.draw->AddRect(rect.min, rect.max, u32_from_rgba(col), rounding, flags, thickness);
}
void imgui_draw_rect_filled(ImGui_DrawList draw, Rng2 rect, v4 col, f32 rounding, ImDrawFlags flags) {
  draw.draw->AddRectFilled(rect.min, rect.max, u32_from_rgba(col));
}
void imgui_draw_push_clip_rect(ImGui_DrawList draw, Rng2 rect) {
  draw.draw->PushClipRect(rect.min, rect.max, true);
}
void imgui_draw_pop_clip_rect(ImGui_DrawList draw) {
  draw.draw->PopClipRect();
}
void imgui_draw_line(ImGui_DrawList draw, v2 p0, v2 p1, v4 col, f32 thickness) {
  draw.draw->AddLine(p0, p1, u32_from_rgba(col));
}
void imgui_draw_text(ImGui_DrawList draw, v2 pos, v4 col, String fmt, ...) {
  Scratch scratch;
  VaList args;
  va_start(args, fmt);
  String formateted = push_strfv(scratch, fmt, args);
  va_end(args);
  draw.draw->AddText(pos, u32_from_rgba(col), (char*)formateted.str, (char*)(formateted.str + formateted.size));
}
void imgui_draw_text(ImGui_DrawList draw, ImFont* font, f32 font_size, v2 pos, v4 col, String fmt, ...) {
  Scratch scratch;
  VaList args;
  va_start(args, fmt);
  String formateted = push_strfv(scratch, fmt, args);
  va_end(args);
  draw.draw->AddText(font, font_size, pos, u32_from_rgba(col), (char*)formateted.str, (char*)(formateted.str + formateted.size));
}
void imgui_text(String fmt, ...) {
  Scratch scratch;
  VaList args;
  va_start(args, fmt);
  String formateted = push_strfv(scratch, fmt, args);
  va_end(args);
  ImGui::TextUnformatted((char*)formateted.str);
}
v2 imgui_calc_text_size(String str) {
  return ImGui::CalcTextSize((char*)str.str, (char*)str.str+str.size);
}
void imgui_begin_tab_item(String str) {
  Scratch scratch;
  String str_c = push_str_copy(scratch, str);
  ImGui::BeginTabItem((char*)str_c.str);
}

Rng2 debug_window_get_rect(DebugWindow win) {
  if (win.fullscreen) {
    return rng2_make(v2(), v2_of_v2u(os_window_size()));
  } else {
    return rng2_make(win.pos, win.size);
  }
}

void debug_window_apply_state(DebugWindow& win) {
  if (win.toggle_fullscreen) {
    if (!win.fullscreen) {
      win.fullscreen = true;
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
      win.flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    } else {
      win.fullscreen = false;
      ImGui::SetNextWindowPos(win.pos);
      ImGui::SetNextWindowSize(win.size);
      win.flags = NoFlags;
    }
    win.toggle_fullscreen = false;
  }
}

void debug_window_track_state(DebugWindow& win) {
  if (!win.fullscreen) {
    win.pos = ImGui::GetWindowPos();
    win.size = ImGui::GetWindowSize();
  }
}

void debug_window_toggle_fullscreen(DebugWindow& win) {
  win.toggle_fullscreen = 1;
}

void debug_init() {
  DebugState& g = st->debug;
  g.prof_win = {
    .root_scroll_state = scroll_state_make(1),
    .frames_scroll_state = scroll_state_make(1),
    .launch_time_scroll_state = scroll_state_make(1),
    .mem_scroll_state = scroll_state_make(1),
    .win.open = false,
  };
  g.imgui_demo_open = false;
  imgui_init();
  ImGuiIO& io = ImGui::GetIO();
  g.font = io.Fonts->AddFontDefault();
  g.prof_win.colors = {
    .work = ColorGreyDark,
    .sleep = ColorGreenUi,
    .job = ColorOrangeUi,
    .async = ColorBlueUi,
    .current_frame = ColorGrey3,
    .frame_ok = ColorGreen,
    .frame_warn = ColorYellow,
    .frame_bad = ColorRed,
    .mem_used = ColorGreenUi,
    .mem_committed = ColorBlueUi,
    .mem_cap = ColorRedUi,
  };
}

void debug_update() {
  DebugState& g = st->debug;
  if (key_pressed(Key_F1)) g.prof_win.win.open = !g.prof_win.win.open;
  if (key_pressed(Key_F2)) g.imgui_demo_open = !g.imgui_demo_open;
  if (key_pressed(Key_F3)) g.game_win.open = !g.game_win.open;

  if (g.imgui_demo_open) ImGui::ShowDemoWindow();

  debug_prof_view();
  debug_game();
}

void debug_game() {
  Scratch scratch;
  var& g = *st;
  DebugWindow& win = st->debug.game_win;
  if (win.open) {
    debug_window_apply_state(win);
    ImGui::Begin("Game");
    if (ImGui::IsWindowHovered()) {
      if (key_pressed(Key_V)) {
        debug_window_toggle_fullscreen(win);
      }
    }

    var list = imgui_get_window_drawlist();
    imgui_draw_rect_filled(list, rng2_make(v2(0), v2(100)), ColorWhite);
    ImGui::Text("entities: %u", g.entities_count);
    {
      ImGui::Text("Camera:");
      imgui_text(push_str_copy(scratch, dumb_struct(scratch, slice(members_of_Camera), &g.cam)));
      ImGui::Separator();
    }
    {
      Thing& e = get_thing(g.axis_attached_to_cam_id);
      imgui_text(push_str_copy(scratch, dumb_struct(scratch, slice(members_of_Entity), &e, e.flags)));
    }

    if (ImGui::Button("save state")) {
      save_game_state();
    }
    if (ImGui::Button("load state")) {
      load_game_state();
    }
    if (ImGui::Button("clear moving cubes")) {
      Loop (i, g.moving_cubes.count) {
        ThingId e =  g.moving_cubes[i];
        destroy_thing(e);
      }
      array_clear(g.moving_cubes);
    }
    ImGui::SliderFloat3("target pos", g.pos_target.v, -10, 10);
    ImGui::DragFloat3("cam pos", g.cam.pos.v);
    ImGui::End();
  }
}

void debug_prof_view() {
  ProfFunc;
  Scratch scratch;
  DebugState& debug = st->debug;
  ProfState& prof = prof_get();
  ProfWindow& prof_win = st->debug.prof_win;

  // Avg, min, max
  u64 tsc_elapsed_sum = 0;
  u64 tsc_elapsed_max = 0;
  u64 tsc_elapsed_min = U32_MAX;
  LoopArray (i, prof.frames_times) {
    ProfFrameTime frame = prof.frames_times[i];
    u64 elapsed = frame.tsc_end - frame.tsc_start;
    tsc_elapsed_sum += elapsed;
    tsc_elapsed_max = Max(tsc_elapsed_max, elapsed);
    tsc_elapsed_min = Min(tsc_elapsed_min, elapsed);
  }
  prof_win.frame_avg_time = tsc_to_ms(tsc_elapsed_sum / ProfRecordHistoryNum);
  prof_win.frame_max_time = tsc_to_ms(tsc_elapsed_max);
  prof_win.frame_min_time = tsc_to_ms(tsc_elapsed_min);

  ProfFrame prev_frame = prof_get_prev_frame(st->current_frame);
  var anchors = prev_frame.anchors;
  u64 tsc_start = prev_frame.frame_time.tsc_start;
  u64 tsc_end = prev_frame.frame_time.tsc_end;
  u64 tsc_elapsed = tsc_end - tsc_start;
  ProfColors colors = prof_win.colors;

  if (key_pressed(Key_H)) {
    ImGui::SetNextWindowFocus(); 
  }

  if (prof_win.win.open) {
    f32 thread_name_text_size = 20;
    f32 time_bar_text_size = 15;

    debug_window_apply_state(prof_win.win);

    if (ImGui::Begin("Profiler", null, prof_win.win.flags)) {
      debug_window_track_state(prof_win.win);

      Rng2 win_rect = debug_window_get_rect(prof_win.win);
      ImGui::PushClipRect(win_rect.min, win_rect.max, false);

      if (key_pressed(Key_1)) prof_win.future_active_tab = ProfileTabActive_Root;
      if (key_pressed(Key_2)) prof_win.future_active_tab = ProfileTabActive_Frames;
      if (key_pressed(Key_3)) prof_win.future_active_tab = ProfileTabActive_Time;
      if (key_pressed(Key_4)) prof_win.future_active_tab = ProfileTabActive_LaunchTime;
      if (key_pressed(Key_5)) prof_win.future_active_tab = ProfileTabActive_Memory;
      if (key_pressed(Key_P)) prof.paused = !prof.paused;
      if (ImGui::IsWindowHovered()) {
        if (key_pressed(Key_V)) {
          debug_window_toggle_fullscreen(prof_win.win);
        }
      }

      if (ImGui::BeginTabBar("MyTabBar")) {
        ImGui_DrawList draw = imgui_get_window_drawlist();
        v2 cursor_pos = ImGui::GetCursorScreenPos();
        v2 mouse_pos = os_mouse_pos();
        v2 win_pos = ImGui::GetWindowPos();
        v2 avail_size = ImGui::GetWindowSize();
        avail_size.x -= (cursor_pos - win_pos).x * 2;

        enum UI_ItemType {
          UI_ItemType_Bar,
          UI_ItemType_NextThread,
        };
        struct UI_Item {
          UI_ItemType type;
          Rng2 rect;
          ProfAnchor anchor;
        };

        var draw_frame_graph = [&](Slice<Slice<ProfAnchor>> slices, ProfFrameTime time, f32 width_off, ScrollState scroll_state, b32 wrap = false) {
          Scratch scratch;
          var items = array_make(UI_Item, scratch);
          Loop (i, slices.count) {
            var anchors = slices[i];

            ///////////////////////////////////
            // Build rect layout
            {
              u64 tsc_start = time.tsc_start;
              u64 tsc_end = time.tsc_end;
              u64 tsc_elapsed = tsc_end - tsc_start;
              Loop(i, anchors.count) {
                ProfAnchor anchor = anchors[i];
                u64 var_tsc_elapsed_incl = anchor.tsc_elapsed_incl;
                u64 var_tsc_start = anchor.tsc_start;
                
                // Handle async anchors
                if (wrap) {
                  if (!anchor.was_poped) {
                    var_tsc_elapsed_incl = tsc_end - anchor.tsc_start;
                    anchor.tsc_elapsed_incl = var_tsc_elapsed_incl;
                  }
                  if (anchor.tsc_start < tsc_start) {
                    var_tsc_start = tsc_start;
                    anchor.tsc_start = var_tsc_start;
                  }
                }
                else {
                  if (!anchor.was_poped) {
                    break;
                  }
                }

                f32 height = 30;
                f32 height_off = anchor.depth * height;
                f32 width = (f64)var_tsc_elapsed_incl / tsc_elapsed * avail_size.x;
                f32 width_off = remap(var_tsc_start, tsc_start, tsc_end, 0, avail_size.x);
                Rng2 rect = rng2_make(v2(width_off, height_off), v2(width, height));
                UI_Item item = {
                  .type = UI_ItemType_Bar,
                  .rect = rect,
                  .anchor = anchor,
                };
                array_push(items, item);
              }
            }
            array_push(items, {.type = UI_ItemType_NextThread});
          }

          ///////////////////////////////////
          // Anchors and thread offsets
          f32 height_off = 0;
          f32 thread_height_off = 200;
          Loop (i, items.count) {
            UI_Item& item = items[i];
            switch (item.type) {
              case UI_ItemType_Bar: {
                item.rect = rng2_shift(item.rect, v2(width_off, height_off));
              } break;
              case UI_ItemType_NextThread: {
                height_off += thread_height_off;
              } break;
            }
          }

          // Scroll
          Loop (i, items.count) {
            Rng2& rect = items[i].rect;
            rect = rng2_shift(rect, cursor_pos);
            rect = rng2_scale(rect, scroll_state.scale);
            rect = rng2_shift(rect, scroll_state.offset);
          }

          ///////////////////////////////////
          // Drawing
          Loop (i, items.count) {
            UI_Item item = items[i];
            switch (item.type) {
              default:{}break;
              case UI_ItemType_Bar: {
                v4 color = {};
                String str = {};
                ProfAnchor anchor = item.anchor;
                Rng2 rect = item.rect;
                switch (anchor.type) {
                  case ProfType_Default: {
                    color = colors.work;
                    str = "work";
                  } break;
                  case ProfType_Sleep: {
                    color = colors.sleep;
                    str = "sleep";
                  } break;
                  case ProfType_Worker: {
                    color = colors.job;
                    str = "job";
                  } break;
                  case ProfType_Async: {
                    color = colors.async;
                    str = "async";
                  } break;
                }
                imgui_draw_rect_filled(draw, rect, color);
                imgui_draw_rect(draw, rect, ColorGreyLight);
                if (rng2_contains(rect, mouse_pos)) {
                  ImGui::BeginTooltip();
                  imgui_text("Label: %s", anchor.label);
                  imgui_text("Percent: %f%%", rng2_dim(rect).x / avail_size.x * 100);
                  imgui_text("Time: %fms", tsc_to_ms(anchor.tsc_elapsed_incl));
                  imgui_text("Time exclusive: %fms", tsc_to_ms(anchor.tsc_elapsed_excl));
                  imgui_text("Type: %s", str);
                  ImGui::EndTooltip();
                }

                // Text
                {
                  String str = push_strf(scratch, "%s %.3f", anchor.label, tsc_to_ms(anchor.tsc_elapsed_incl));
                  v2 text_size = imgui_calc_text_size(str);
                  if (rng2_dim(rect).x < 30.1 || scroll_state.scale.y < 0.3) {
                    continue;
                  }
                  v2 text_pos = {};
                  if (text_size.x > rng2_dim(rect).x) {
                    text_pos.x = rect.min.x;
                    text_pos.y = rect.min.y + (rng2_dim(rect).y - text_size.y) * 0.5;
                  } else {
                    text_pos = rng2_align_dim_at_center(rect, text_size).min;
                  }
                  imgui_draw_push_clip_rect(draw, rect);
                  imgui_draw_text(draw, debug.font, time_bar_text_size, text_pos, ColorWhite, str);
                  imgui_draw_pop_clip_rect(draw);
                }
              } break;
            }
          }
        };

        imgui_text("%.1ffps %.1fms CPU %.1fGhz, Recording: %s", 1000 / tsc_to_ms(tsc_elapsed), tsc_to_ms(tsc_elapsed), (f64)cpu_frequency() / Billion(1), prof.paused ? S("off") : S("on"));
        imgui_text("avg %.1fms, max %.1f, min %.1f", prof_win.frame_avg_time, prof_win.frame_max_time, prof_win.frame_min_time);
        f32 info_height = 60;
        cursor_pos.y += info_height;

        ///////////////////////////////////
        // Draw thread names
        var draw_threads = [&](ScrollState scroll_state) {
          f32 thread_height = 200;
          f32 thread_height_offset = 0;
          f32 text_off_above = -40;
          {
            String str = push_strf(scratch, "Main thread");
            v2 text_pos = (v2(0, text_off_above) + cursor_pos);
            text_pos.y *= scroll_state.scale.y;
            text_pos.y += scroll_state.offset.y;
            imgui_draw_text(draw, debug.font, thread_name_text_size, text_pos, ColorWhite, str);
          }
          {
            Loop (i, Thread_NumWorkers) {
              thread_height_offset += thread_height;
              String str = push_strf(scratch, "Worker %i", i);
              v2 text_pos = v2(0, thread_height_offset + text_off_above) + cursor_pos;
              text_pos.y *= scroll_state.scale.y;
              text_pos.y += scroll_state.offset.y;
              imgui_draw_text(draw, debug.font, thread_name_text_size, text_pos, ColorWhite, str);
            }
            thread_height_offset = 0;
          }
        };

        ///////////////////////////////////
        // Tab mouse click
        var tab_mouse_click_handle = [&](String name, ProfTabActive tab) {
          Scratch scratch;
          String str = push_strf(scratch, name);
          imgui_begin_tab_item(str);
          Rng2 tab_rect = Rng2(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
          // if (str_match(name, "root")) {
          //   Debug("root");
          //   Info("min: %f %f", tab_rect.min.x, tab_rect.min.y);
          //   Info("max: %f %f", tab_rect.max.x, tab_rect.max.y);
          // }
          // if (str_match(name, "frames")) {
          //   Debug("frames");
          //   Info("min: %f %f", tab_rect.min.x, tab_rect.min.y);
          //   Info("max: %f %f", tab_rect.max.x, tab_rect.max.y);
          // }
          // if (str_match(name, "time")) {
          //   Debug("time");
          //   Info("min: %f %f", tab_rect.min.x, tab_rect.min.y);
          //   Info("max: %f %f", tab_rect.max.x, tab_rect.max.y);
          // }
          // if (str_match(name, "launch")) {
          //   Debug("launch");
          //   Info("min: %f %f", tab_rect.min.x, tab_rect.min.y);
          //   Info("max: %f %f", tab_rect.max.x, tab_rect.max.y);
          // }
          // if (str_match(name, "memory")) {
          //   Debug("memory");
          //   Info("min: %f %f", tab_rect.min.x, tab_rect.min.y);
          //   Info("max: %f %f", tab_rect.max.x, tab_rect.max.y);
          // }
          if (os_mouse_is_button_pressed(MouseButton_Left)) {
            if (rng2_contains(tab_rect, mouse_pos)) {
              switch (tab) {
                case ProfileTabActive_Root: prof_win.future_active_tab = ProfileTabActive_Root; break;
                case ProfileTabActive_Frames: prof_win.future_active_tab = ProfileTabActive_Frames; break;
                case ProfileTabActive_Time: prof_win.future_active_tab = ProfileTabActive_Time; break;
                case ProfileTabActive_LaunchTime: prof_win.future_active_tab = ProfileTabActive_LaunchTime; break;
                case ProfileTabActive_Memory: prof_win.future_active_tab = ProfileTabActive_Memory; break;
              }
            }
          }
        };

        ///////////////////////////////////
        // Tabs
        tab_mouse_click_handle("root", ProfileTabActive_Root);
        tab_mouse_click_handle("frames", ProfileTabActive_Frames);
        tab_mouse_click_handle("time", ProfileTabActive_Time);
        tab_mouse_click_handle("launch", ProfileTabActive_LaunchTime);
        tab_mouse_click_handle("memory", ProfileTabActive_Memory);
        switch (prof_win.active_tab) {
          case ProfileTabActive_Root: {
            ScrollState& scroll_state = prof_win.root_scroll_state;
            if (ImGui::IsWindowHovered()) {
              scroll_state_update(scroll_state, ScrollType_PowClamp);
            }
            cursor_pos.y += 30;
            draw_threads(scroll_state);
            u32 idx = (st->current_frame-1) % ArrayCount(prof.frames_times);
            Slice<ProfAnchor> slices[ArrayCount(prof.prof_threads)] = {};
            LoopArray (i, prof.prof_threads) {
              slices[i] = slice(prof.prof_threads[i].recorded_anchors[idx]);
            }
            ProfFrameTime time = prof.frames_times[idx];
            draw_frame_graph(slice(slices), time, 0, scroll_state);
            ImGui::EndTabItem();
          } break;
          case ProfileTabActive_Frames: {
            ScrollState& scroll_state = prof_win.frames_scroll_state;
            if (ImGui::IsWindowHovered()) {
              scroll_state_update(scroll_state, ScrollType_PowClamp);
            }
            f32 width_size = avail_size.x;

            ///////////////////////////////////
            // Little bars
            Loop (i, ArrayCount(prof.frames_times)) {
              ProfFrameTime frame_time = prof.frames_times[i];
              f32 max_height = 40;
              f32 max_ms = 30;
              f64 frame_ms = tsc_to_ms(frame_time.tsc_end - frame_time.tsc_start);
              f64 height = max_height / (max_ms / frame_ms);
              v2 size = v2(avail_size.x / ArrayCount(prof.frames_times), height);
              v2 min = cursor_pos + v2(i*size.x, -height + max_height);
              Rng2 rect = rng2_make(min, size);
              if (rng2_contains(rect, mouse_pos)) {
                ImGui::BeginTooltip();
                ImGui::Text("frame: %i", i);
                ImGui::EndTooltip();
                if (os_mouse_is_button_pressed(MouseButton_Left)) {
                  prof_win.frames_scroll_state.offset.x = -width_size * i;
                  prof_win.frames_scroll_state.scale = v2(1);
                }
              }
              if (i == st->current_frame % ArrayCount(prof.frames_times)) {
                imgui_draw_rect_filled(draw, rect, colors.current_frame);
              }
              else {
                v4 color = colors.frame_ok;
                if (rng1_contains(Rng1(17, 21), frame_ms)) {
                  color = colors.frame_warn;
                } else if (frame_ms > 20) {
                  color = colors.frame_bad;
                }
                imgui_draw_rect_filled(draw, rect, color);
                imgui_draw_rect(draw, rect, v4_set_w(ColorGrey0, 0.3));
              }
            }
            cursor_pos.y += 80;

            ///////////////////////////////////
            // Draw lines and current rect
            {
              f32 width_offset = 0;
              Loop (i, ArrayCount(prof.frames_times)) {
                f32 line_height = 1000;
                f32 thick = 1;
                v2 base = cursor_pos + v2(width_offset, 0);
                v2 p0 = base + v2(0, -line_height / 2);
                v2 p1 = base + v2(0, line_height);
                v2 p2 = base + v2(width_size, 0);
                v2 p3 = base + v2(width_size, 0) + v2(0, line_height);
                p0 = p0 * scroll_state.scale.x + scroll_state.offset;
                p1 = p1 * scroll_state.scale.x + scroll_state.offset;
                p2 = p2 * scroll_state.scale.x + scroll_state.offset;
                p3 = p3 * scroll_state.scale.x + scroll_state.offset;
                imgui_draw_line(draw, p0, p1, ColorGrey3, thick);
                if (i == st->current_frame % ArrayCount(prof.frames_times)) {
                  imgui_draw_rect_filled(draw, Rng2(p0, p3), v4(0.4,0.4,0.4,0.4));
                }
                width_offset += width_size;
              }
            }

            draw_threads(scroll_state);

            ///////////////////////////////////
            // Draw graph per thread
            LoopArray (j, prof.frames_times) {
              Slice<ProfAnchor> slices[ArrayCount(prof.prof_threads)] = {};
              LoopArray (i, prof.prof_threads) {
                slices[i] = slice(prof.prof_threads[i].recorded_anchors[j]);
              }
              ProfFrameTime time = prof.frames_times[j];
              draw_frame_graph(slice(slices), time, j * width_size, scroll_state);
            }
            ImGui::EndTabItem();
          } break;
          case ProfileTabActive_Time: {
            var sorted_anchors = slice_clone(scratch, anchors);
            sort_insert(sorted_anchors, [](ProfAnchor a, ProfAnchor b) { return a.tsc_elapsed_excl > b.tsc_elapsed_excl; });

            Loop (i, anchors.count) {
              ImGui::PushID(i);
              ProfAnchor anchor = sorted_anchors[i];
              f64 width_exclusive_percent = (f64)anchor.tsc_elapsed_excl / tsc_elapsed;
              f32 width_exclusive = avail_size.x * 0.8;
              f32 height = 30;
              width_exclusive *= width_exclusive_percent;

              v2 offset = v2(0,  i * height) + cursor_pos;
              v2 size = v2(width_exclusive, height);
              Rng2 rect = Rng2(offset, size + offset);

              imgui_draw_rect_filled(draw, rect, ColorGreyDark);
              imgui_draw_rect(draw, rect, ColorGreyLight);

              String name_str = push_strf(scratch, "%s", anchor.label);
              String ms_str = push_strf(scratch, "%.3fms", (f64)anchor.tsc_elapsed_excl / cpu_frequency() * 1000);
              v2 name_offset = v2(0, height * i) + cursor_pos;
              v2 ms_offset = v2(avail_size.x * 0.82, height * i) + cursor_pos;

              imgui_draw_text(draw, name_offset, ColorWhite, name_str);
              imgui_draw_text(draw, ms_offset, ColorWhite, ms_str);

              ImGui::PopID();
            }
            ImGui::EndTabItem();
            } break;
          case ProfileTabActive_LaunchTime: {
            ScrollState& scroll_state = prof_win.launch_time_scroll_state;
            if (ImGui::IsWindowHovered()) {
              scroll_state_update(scroll_state, ScrollType_PowClamp);
            }

            draw_threads(scroll_state);

            Slice<ProfAnchor> slices[ArrayCount(prof.prof_threads)] = {};
            LoopArray (i, prof.prof_threads) {
              slices[i] = slice(prof.prof_threads[i].launch_anchors);
            }
            ProfFrameTime time = prof.launch_time;
            draw_frame_graph(slice(slices), time, 0, scroll_state, true);
            ImGui::EndTabItem();
          } break;
          case ProfileTabActive_Memory: {
            ScrollState& scroll_state = prof_win.mem_scroll_state;
            if (ImGui::IsWindowHovered()) {
              scroll_state_update(scroll_state);
            }
            enum UI_ItemType {
              UI_ItemType_MemUsage,
              UI_ItemType_MemLevel,
              UI_ItemType_Arena,
              UI_ItemType_Child,
            };
            struct UI_Item {
              UI_ItemType type;
              Rng2 rect;
              u32 depth;
              AllocatorInfo* info;
              u32 mem_level;
            };
            var items = array_make(UI_Item, scratch);
            AllocatorInfoList infos = mem_track_info();
            var infos_sorted = sort_list_insert(scratch, infos.first, [](var a, var b) { return a->pos > b->pos; });
            f64 mem_usage = 0;
            Loop (i, infos_sorted.count) {
              AllocatorInfo* x = infos_sorted[i];
              mem_usage += x->cap;
            }
            f32 mem_levels[] = {KB(1), KB(10), KB(100), MB(1), MB(10), MB(100), GB(1)};

            ///////////////////////////////////
            // Layout
            {
              f32 row_h = 30;
              Rng2Cursor curs = {};
              // mem usage
              {
                UI_Item item = {
                  .type = UI_ItemType_MemUsage,
                  .rect = layout_row(curs, Rng1(0, avail_size.x), row_h),
                };
                array_push(items, item);
              }

              b32 level_drawn[ArrayCount(mem_levels)] = {};
              Loop (i, infos_sorted.count) {
                var& info = *infos_sorted[i];
    
                //  Mem level
                u32 mem_level = 0;
                LoopArray (i, mem_levels) {
                  if (info.pos < mem_levels[i]) {
                    mem_level = i;
                    break;
                  }
                }
                if (!level_drawn[mem_level]) {
                  level_drawn[mem_level] = true;
                  UI_Item item = {
                    .type = UI_ItemType_MemLevel,
                    .rect = layout_row(curs, Rng1(0, avail_size.x), row_h),
                    .mem_level = mem_level,
                  };
                  array_push(items, item);
                }
    
                // Arena
                {
                  UI_Item item = {
                    .type = UI_ItemType_Arena,
                    .rect = layout_row(curs, Rng1(0, avail_size.x), row_h),
                    .info = &info,
                    .mem_level = mem_level,
                  };
                  array_push(items, item);
                }
    
                // Children
                u32 depth = 1;
                struct StackEntry {
                  AllocatorInfo* node;
                  u32 depth;
                };
                var stack = array_make(StackEntry, scratch);
                Slice sorted_children = sort_list_insert(scratch, info.first, [](var a, var b) { return a->pos > b->pos; });
                LoopReverse (i, sorted_children.count) {
                  array_push(stack, {sorted_children[i], 1});
                }
                while (stack.count) {
                  StackEntry entry = array_pop(stack);
                  var child = entry.node;
                  UI_Item item = {
                    .type = UI_ItemType_Child,
                    .depth = entry.depth,
                    .info = child,
                    .mem_level = mem_level,
                  };
                  item.rect = Rng2(
                    v2(depth * 10, curs.pos.y),
                    v2(avail_size.x, curs.pos.y + row_h * 0.6)
                  );
                  array_push(items, item);
                  layout_next(curs, row_h * 0.6);
                  if (child->first) {
                    ++depth;
                    Slice sorted_children = sort_list_insert(scratch, child->first, [](var a, var b) { return a->pos > b->pos; });
                    LoopReverse (i, sorted_children.count) {
                      array_push(stack, {sorted_children[i], entry.depth + 1});
                    }
                  }
                }
              }
            }

            Loop (i, items.count) {
              UI_Item& item = items[i];
              item.rect = rng2_shift(item.rect, cursor_pos);
              item.rect = rng2_scale(item.rect, scroll_state.scale);
              item.rect = rng2_shift(item.rect, scroll_state.offset);
            }

            Rng2 rounding_edge = rng2_shift(rng2_scale(rng2_make(cursor_pos, avail_size), v2(scroll_state.scale)), v2(scroll_state.offset));
            rounding_edge = rng2_pad(rounding_edge, 10);
            imgui_draw_rect(draw, rounding_edge, ColorGreyLight);

            ///////////////////////////////////
            // Drawing
            Loop (i, items.count) {
              UI_Item item = items[i];
              AllocatorInfo& info = *item.info;

              switch (item.type) {
                case UI_ItemType_MemUsage: {
                  MemFormatSize mem_fmt = mem_format_size(mem_usage);
                  MemFormatSize os_commited_fmt = mem_format_size(os_commited_size());
                  MemFormatSize os_address_reserved_fmt = mem_format_size(os_reserved_size());
                  String mem_usage_str = push_strf(scratch, "os commited: %.2f%s, address reserved %.2f%s, mem usage: %.2f%s", 
                    os_commited_fmt.size, os_commited_fmt.format,
                    os_address_reserved_fmt.size, os_address_reserved_fmt.format,
                    mem_fmt.size, mem_fmt.format);
                  imgui_draw_text(draw, cursor_pos, ColorWhite, mem_usage_str);
                } break;
                case UI_ItemType_MemLevel: {
                  MemFormatSize mem_fmt = mem_format_size(mem_levels[item.mem_level]);
                  String str = push_strf(scratch, "%.0f%s", mem_fmt.size, mem_fmt.format);
                  v2 text_size = imgui_calc_text_size(str);
                  Rng2 rect = item.rect;
                  Rng2 text_rect = rng2_align_dim_at_center(rect, text_size);
                  Rng2 pad_text_rect = rng2_pad(text_rect, 5);
                  imgui_draw_rect_filled(draw, pad_text_rect, v4(0.3, 3.5, 0.5, 0.5));
                  imgui_draw_text(draw, text_rect.min, ColorWhite, str);
                } break;
                case UI_ItemType_Arena: {
                  f32 t_w = rng2_dim(item.rect).x;
                  f32 t_pos = info.pos / mem_levels[item.mem_level];
                  // f32 t_cap = info.cap / mem_levels[item.mem_level];
                  f32 t_excl = info.children_size / mem_levels[item.mem_level];
                  f32 w_pos = t_w * t_pos;
                  // f32 w_cap = t_w * t_cap;
                  f32 w_excl = t_w * t_excl;
                  Rng2 excl_rect = rng2_subrng_x(item.rect, Rng1(0, w_excl));
                  Rng2 incl_rect = rng2_subrng_x(item.rect, Rng1(w_excl, w_pos));

                  imgui_draw_rect_filled(draw, excl_rect, ColorGreenUi);
                  imgui_draw_rect(draw, excl_rect, ColorGreyLight);
                  imgui_draw_rect_filled(draw, incl_rect, ColorBlueUi);
                  imgui_draw_rect(draw, incl_rect, ColorGreyLight);

                  MemFormatSize pos = mem_format_size(info.pos);
                  MemFormatSize pos_exclusive = mem_format_size(info.children_size);
                  MemFormatSize cmt = mem_format_size(info.cap);

                  String name_str = push_strf(scratch, "%s", info.name);
                  String mem_str = push_strf(scratch, "%.2f%s pos, %.2f%s cmt", pos.size, pos.format, cmt.size, cmt.format);

                  imgui_draw_text(draw, excl_rect.min, ColorWhite, name_str);
                  imgui_draw_text(draw, rng2_subrng_x01(item.rect, Rng1(0.2, 1)).min, ColorWhite, mem_str);
                  
                  if (rng2_contains(rng2_union(incl_rect, excl_rect), mouse_pos)) {
                    ImGui::BeginTooltip();
                    imgui_text(push_strf(scratch, "inclusive: %.2f%s", pos.size, pos.format));
                    imgui_text(push_strf(scratch, "exclusive: %.2f%s", pos_exclusive.size, pos_exclusive.format));
                    ImGui::EndTooltip();
                  }
                } break;
                case UI_ItemType_Child: {
                  f32 t_w = rng2_dim(item.rect).x;
                  f32 t_pos = info.pos / mem_levels[item.mem_level];
                  f32 t_cap = info.cap / mem_levels[item.mem_level];
                  // f32 t_excl = info.exclusive_pos / mem_levels[item.mem_level];
                  f32 w_pos = t_w * t_pos;
                  f32 w_cap = t_w * t_cap;
                  // f32 w_excl = t_w * t_excl;
                  Rng2 child_rect = rng2_subrng_x(item.rect, Rng1(0, w_pos));
                  Rng2 child_rect_cap = rng2_subrng_x(item.rect, Rng1(w_pos, w_cap));

                  // pos
                  imgui_draw_rect_filled(draw, child_rect, ColorGreyDark);
                  imgui_draw_rect(draw, child_rect, ColorGreyLight);
                  
                  // cap
                  imgui_draw_rect_filled(draw, child_rect_cap, ColorRedUi);
                  imgui_draw_rect(draw, child_rect_cap, ColorGrey);

                  MemFormatSize pos = mem_format_size(info.pos);
                  MemFormatSize cap = mem_format_size(info.cap);
                  String child_name_str = push_strf(scratch, "%s", info.name);
                  String child_meta_str = push_strf(scratch, "%.2f%s pos, %.2f%s cap, alloc count: %u, free count: %u, current alloc count: %u", pos.size, pos.format, cap.size, cap.format, info.allocs_count, info.frees_count, info.allocs_count-info.frees_count);
                  imgui_draw_text(draw, child_rect.min, ColorWhite, child_name_str);
                  imgui_draw_text(draw, rng2_subrng_x01(child_rect, Rng1(0.3, 1)).min, ColorWhite, child_meta_str);
                } break;
              }
            }
            ImGui::EndTabItem();
          } break;
        }
        ImGui::EndTabBar();
      }

      ImGui::PopClipRect();
    } ImGui::End();
  }

  prof_win.active_tab = prof_win.future_active_tab;
}

R_MeshDesc load_obj(Allocator arena, String name) {
  Scratch scratch(arena);
  var positions = array_make(v3, scratch);
  var normals = array_make(v3, scratch);
  var uvs = array_make(v2, scratch);
  var indexes = array_make(v3u, scratch);

  // String str = R"(
  //   v  -4.4   14 4.1
  //   v   1.4  -14 4.1
  //   vt -4.4   14
  //   vt  1.4  -14
  //   vn -4.4   14 4.1
  //   vn  1.4  -14 4.1
  //   f 10/4/1 1/2/3 11/22/33
  //   f 20/2/2 91/42/13 141/22/33
  // )";

  struct WordLexer {
    String str;
    u32 cursor;
  };
  var word_lexer_next = [](WordLexer& l) {
    while (l.cursor < l.str.size && char_is_ws(l.str.str[l.cursor])) {
      ++l.cursor;
    }
    u32 word_base = l.cursor;
    while (l.cursor < l.str.size && !char_is_ws(l.str.str[l.cursor])) {
      ++l.cursor;
    }
    String res = str_make(l.str.str + word_base, l.cursor - word_base);
    return res;
  };
  WordLexer l = {os_file_path_read_all_str(scratch, name)};
  for (String word = {}; (word = word_lexer_next(l)).size;) {
    // Info("%s", word);
    if (word.str[0] == 'v' && word.str[1] == ' ') {
      v3 pos = {
        f32_from_str(word_lexer_next(l)),
        f32_from_str(word_lexer_next(l)),
        f32_from_str(word_lexer_next(l)),
      };
      array_push(positions, pos);
      // Info("%f %f %f", pos.x, pos.y, pos.z);
    } else if (word.str[0] == 'v' && word.str[1] == 'n') {
      v3 norm = {
        f32_from_str(word_lexer_next(l)),
        f32_from_str(word_lexer_next(l)),
        f32_from_str(word_lexer_next(l)),
      };
      array_push(normals, norm);
      // Info("%f %f %f", norm.x, norm.y, norm.z);
    } else if (word.str[0] == 'v' && word.str[1] == 't') {
      v2 uv = {
        f32_from_str(word_lexer_next(l)),
        f32_from_str(word_lexer_next(l)),
      };
      array_push(uvs, uv);
      // Info("%f %f", uv.x, uv.y);
    } else if (word.str[0] == 'f' && word.str[1] == ' ') {
      Loop (i, 3) {
        v3u raw = {};
        String word = word_lexer_next(l);
        u32 cursor = 0;
        String r = {};
        Loop (i, 3) {
          u32 num_base = cursor;
          while (cursor < word.size && char_is_digit(word.str[cursor])) {
            ++cursor;
          }
          r = str_make(word.str+num_base, cursor - num_base);
          raw.v[i] = u64_from_str(r) - 1;
          ++cursor;
        }
        // Info("%u %u %u", raw.x, raw.y, raw.z);
        v3u v = {raw.x, raw.z, raw.y};
        array_push(indexes, v);
      }
    }
  }

  var vertices = array_make(R_Vertex, arena);
  var final_indices = array_make(u32, arena);
  var map = map_make(v3u, u32, scratch);
  Loop (i, indexes.count) {
    v3u idx = indexes[i];
    var [value, ok] = map_get(map, idx);
    if (ok) {
      array_push(final_indices, value);
    } else {
      R_Vertex v = {
        positions[idx.x],
        normals[idx.y],
        uvs[idx.z],
      };
      u32 new_index = vertices.count;
      array_push(vertices, v);
      array_push(final_indices, new_index);
      map_set(map, idx, new_index);
    } 
  }
  R_MeshDesc mesh = {
    .vertices = slice(vertices),
    .indices = slice(final_indices),
  };
  return mesh;
}

#define Gltf_i8  5120
#define Gltf_u8  5121
#define Gltf_i16 5122
#define Gltf_u16 5123
#define Gltf_u32 5125
#define Gltf_f32 5126

R_MeshDesc load_gltf(Allocator arena, String path, b32 is_glb) {
  Scratch scratch(arena);

  String json = {};
  Slice<u8> cursor = {};
  if (is_glb) {
    struct FileHeader {
      u32 magic;
      u32 version;
      u32 size;
    };
    struct ChunkHeader {
      u32 chunk_length;
      u32 chunk_type;
    };
    cursor = os_file_path_read_all(scratch, path);

    FileHeader* header = (FileHeader*)cursor.data;
    Assert(str_match(str_make((u8*)&header->magic, 4), "glTF"));
    cursor = slice_skip(cursor, sizeof(FileHeader));

    ChunkHeader* json_chunk = (ChunkHeader*)cursor.data;
    Assert(str_match(str_make((u8*)&json_chunk->chunk_type, 4), "JSON"));
    cursor = slice_skip(cursor, sizeof(ChunkHeader));
    json = str_make(slice_prefix(cursor, json_chunk->chunk_length));
    cursor = slice_skip(cursor, json_chunk->chunk_length);

    ChunkHeader* bin_chunk = (ChunkHeader*)cursor.data;
    Assert(str_match(str_make((u8*)&bin_chunk->chunk_type, 3), "BIN"));
    cursor = slice_skip(cursor, sizeof(ChunkHeader));

  } else {
    json = os_file_path_read_all_str(scratch, path);
  }

  struct Accessor {
    u32 buffer_view;
    u32 count;
  };

  struct {
    u32 pos_attribute;
    u32 norm_attribute;
    u32 texcoord_attribute;
    u32 indices_attribute;
    Accessor pos_accessor;
    Accessor norm_accessor;
    Accessor texcoord_accessor;
    Accessor indices_accessor;
    Region pos_buffer_view;
    Region norm_buffer_view;
    Region texcoord_buffer_view;
    Region indices_buffer_view;
  } gltf;

  JsParser p = js_parse_make(scratch, json);
  JsObj root = js_parse(&p).obj;
  {
    Slice meshes = js_get_array(root, "meshes");
    JsObj mesh = meshes[0]->obj;
    {
      Slice primitives = js_get_array(mesh, "primitives");
      JsObj first = primitives[0]->obj;
      {
        JsObj attributes = js_get_obj(first, "attributes");
        gltf.pos_attribute = js_get_number(attributes, "POSITION");
        gltf.norm_attribute = js_get_number(attributes, "NORMAL");
        gltf.texcoord_attribute = js_get_number(attributes, "TEXCOORD_0");
      }
      gltf.indices_attribute = js_get_number(first, "indices");
    }
  }
  {
    Slice accessors = js_get_array(root, "accessors");
    JsObj pos_accessor = accessors[gltf.pos_attribute]->obj;
    gltf.pos_accessor.buffer_view = js_get_number(pos_accessor, "bufferView");
    gltf.pos_accessor.count = js_get_number(pos_accessor, "count");
    JsObj norm_accessor = accessors[gltf.norm_attribute]->obj;
    gltf.norm_accessor.buffer_view = js_get_number(norm_accessor, "bufferView");
    gltf.norm_accessor.count = js_get_number(norm_accessor, "count");
    JsObj texcoord_accessor = accessors[gltf.texcoord_attribute]->obj;
    gltf.texcoord_accessor.buffer_view = js_get_number(texcoord_accessor, "bufferView");
    gltf.texcoord_accessor.count = js_get_number(texcoord_accessor, "count");
    JsObj indices_accessor = accessors[gltf.indices_attribute]->obj;
    gltf.indices_accessor.buffer_view = js_get_number(indices_accessor, "bufferView");
    gltf.indices_accessor.count = js_get_number(indices_accessor, "count");
  }
  {
    Slice buffer_views = js_get_array(root, "bufferViews");
    JsObj pos_buffer_view = buffer_views[gltf.pos_accessor.buffer_view]->obj;
    gltf.pos_buffer_view.size = js_get_number(pos_buffer_view, "byteLength");
    gltf.pos_buffer_view.offset = js_get_number(pos_buffer_view, "byteOffset");
    JsObj norm_buffer_view = buffer_views[gltf.norm_accessor.buffer_view]->obj;
    gltf.norm_buffer_view.size = js_get_number(norm_buffer_view, "byteLength");
    gltf.norm_buffer_view.offset = js_get_number(norm_buffer_view, "byteOffset");
    JsObj texcoord_buffer_view = buffer_views[gltf.texcoord_accessor.buffer_view]->obj;
    gltf.texcoord_buffer_view.size = js_get_number(texcoord_buffer_view, "byteLength");
    gltf.texcoord_buffer_view.offset = js_get_number(texcoord_buffer_view, "byteOffset");
    JsObj indices_buffer_view = buffer_views[gltf.indices_accessor.buffer_view]->obj;
    gltf.indices_buffer_view.size = js_get_number(indices_buffer_view, "byteLength");
    gltf.indices_buffer_view.offset = js_get_number(indices_buffer_view, "byteOffset");
  }
  String file_path = str_chop_last_dot(path);
  String file_path_bin = push_strf(scratch, "%s.bin", file_path);
  Slice data = is_glb ? cursor : os_file_path_read_all(scratch, file_path_bin);
  Slice pos = slice_reinterpret<v3>(slice_n(data, gltf.pos_buffer_view.offset, gltf.pos_buffer_view.size));
  Slice norm = slice_reinterpret<v3>(slice_n(data, gltf.norm_buffer_view.offset, gltf.norm_buffer_view.size));
  Slice texcoord = slice_reinterpret<v2>(slice_n(data, gltf.texcoord_buffer_view.offset, gltf.texcoord_buffer_view.size));
  Slice indices_u16 = slice_reinterpret<u16>(slice_n(data, gltf.indices_buffer_view.offset, gltf.indices_buffer_view.size));

  Slice vertices = push_slice(arena, R_Vertex, gltf.pos_accessor.count);
  Slice indices = push_slice(arena, u32, gltf.indices_accessor.count);
  Loop (i, indices.count) {
    indices[i] = indices_u16[i];
  }
  Loop (i, vertices.count) {
    vertices[i] = {
      .pos = pos[i],
      .norm = norm[i],
      .uv = texcoord[i],
    };
  }
  R_MeshDesc mesh = {
    .vertices = vertices,
    .indices = indices,
  };
  return mesh;
}

JsParser js_parse_make(Allocator arena, String str) {
  JsParser res = {
    .arena = arena,
    .str = str,
  };
  return res;
}
u8 js_peek(JsParser* p) {
  if (p->cursor >= p->str.size) return 0;
  return p->str.str[p->cursor];
}
b32 js_at_end(JsParser* p) {
  return p->cursor >= p->str.size;
}
void js_advance(JsParser* p) {
  ++p->cursor;
}
void js_skip_ws(JsParser* p) {
  while (char_is_ws(js_peek(p))) {
    js_advance(p);
  }
}
String js_parse_str(JsParser* p) {
  Assert(js_peek(p) == '\"');
  js_advance(p);
  u32 start = p->cursor;
  while (js_peek(p) != '\"' && !js_at_end(p)) {
    js_advance(p);
  }
  Assert(js_peek(p) == '\"'); 
  String res = str_substr(p->str, Rng1u(start, p->cursor));
  js_advance(p);
  return res;
}
f64 js_parse_number(JsParser* p) {
  u32 start = p->cursor;
  // while (char_is_number_cont(js_peek(p))) {
  //   js_advance(p);
  // }
  if (js_peek(p) == '-')
    js_advance(p);
  while (char_is_digit(js_peek(p)))
    js_advance(p);
  if (js_peek(p) == '.') {
    js_advance(p);
    while (char_is_digit(js_peek(p)))
      js_advance(p);
  }
  if (js_peek(p) == 'e' || js_peek(p) == 'E') {
    js_advance(p);
    if (js_peek(p) == '+' || js_peek(p) == '-')
      js_advance(p);
    while (char_is_digit(js_peek(p)))
      js_advance(p);
  }
  String str = str_substr(p->str, Rng1u(start, p->cursor));
  f64 res = f64_from_str(str);
  return res;
}
JsVal js_parse(JsParser* p) {
  js_skip_ws(p);
  switch (js_peek(p)) {
    default:   return { .type = JsType_Number, .number = js_parse_number(p) };
    case '\"': return { .type = JsType_Str, .str = js_parse_str(p), };
    case 't': p->cursor += 4; return { .type = JsType_Bool, .boolean = true, };
    case 'f': p->cursor += 5; return { .type = JsType_Bool, .boolean = false, };
    case 'n': p->cursor += 4; return { .type = JsType_Null, };
    case '[': {
      js_advance(p);
      var arr = array_make(JsVal*, p->arena);
      while (true) {
        js_skip_ws(p);
        if (js_peek(p) == ']') {
          js_advance(p);
          break;
        }
        JsVal* val = push_struct(p->arena, JsVal);
        *val = js_parse(p);
        // switch (val->type) {
        //   case JsType_Null: Info("null"); break;
        //   case JsType_Bool: Info("bool: %u", val->boolean); break;
        //   case JsType_Number: Info("num: %f", val->number); break;
        //   case JsType_Str: Info("str: %s", val->str); break;
        //   case JsType_Array: Info("array"); break;
        //   case JsType_Obj:  Info("obj"); break;
        // }
        array_push(arr, val);
        js_skip_ws(p);
        if (js_peek(p) == ',') {
          js_advance(p);
        }
      }
      JsVal res = {
        .type = JsType_Array,
        .array = slice(arr),
      };
      return res;
    } break;
    case '{': {
      js_advance(p);
      var fields = array_make(JsField, p->arena);
      while (true) {
        js_skip_ws(p);
        if (js_peek(p) == '}') {
          js_advance(p);
          break;
        }
        String key = js_parse_str(p);
        js_skip_ws(p);
        Assert(js_peek(p) == ':');
        js_advance(p);
        JsVal* val = push_struct(p->arena, JsVal);
        *val = js_parse(p);
        // switch (val->type) {
        //   case JsType_Null: Info("null"); break;
        //   case JsType_Bool: Info("bool: %u", val->boolean); break;
        //   case JsType_Number: Info("num: %f", val->number); break;
        //   case JsType_Str: Info("str: %s", val->str); break;
        //   case JsType_Array: Info("array"); break;
        //   case JsType_Obj:  Info("obj"); break;
        // }
        array_push(fields, {.key = key, .val = val});
        js_skip_ws(p);
        if (js_peek(p) == ',') {
          js_advance(p);
        }
      }
      JsVal res = {
        .type = JsType_Obj,
        .obj = slice(fields),
      };
      return res;
    } break;
  }
}

JsVal js_get_val(JsVal val, String key) {
  Loop (i, val.obj.fields.count) {
    JsField& field = val.obj.fields[i];
    if (str_match(field.key, key)) {
      // Assert(field.val->type == JsType_Obj);
      return *field.val;
    }
  }
  return {};
}
b32 js_get_bool(JsObj obj, String key) {
  Loop (i, obj.fields.count) {
    JsField& field = obj.fields[i];
    if (str_match(field.key, key)) {
      Assert(field.val->type == JsType_Bool);
      return field.val->boolean;
    }
  }
  return {};
}
f64 js_get_number(JsObj obj, String key) {
  Loop (i, obj.fields.count) {
    JsField& field = obj.fields[i];
    if (str_match(field.key, key)) {
      Assert(field.val->type == JsType_Number);
      return field.val->number;
    }
  }
  return {};
}
String js_get_str(JsObj obj, String key) {
  Loop (i, obj.fields.count) {
    JsField& field = obj.fields[i];
    if (str_match(field.key, key)) {
      Assert(field.val->type == JsType_Str);
      return field.val->str;
    }
  }
  return {};
}
Slice<JsVal*> js_get_array(JsObj obj, String key) {
  Loop (i, obj.fields.count) {
    JsField& field = obj.fields[i];
    if (str_match(field.key, key)) {
      Assert(field.val->type == JsType_Array);
      return field.val->array;
    }
  }
  return {};
}
JsObj js_get_obj(JsObj obj, String key) {
  Loop (i, obj.fields.count) {
    JsField& field = obj.fields[i];
    if (str_match(field.key, key)) {
      Assert(field.val->type == JsType_Obj);
      return field.val->obj;
    }
  }
  return {};
}

b32 key_pressed(Key key) {
  if (os_key_is_pressed(key)) {
    if (!st->input.consumed[key]) return true;
  }
  return false;
}
b32 key_pressed_consume(Key key) {
  if (key_pressed(key)) {
    key_consume(key);
    return true;
  }
  return false;
}
b32 key_down(Key key) {
  if (os_key_is_down(key)) {
    if (!st->input.consumed[key]) return true;
  }
  return false;
}
b32 key_down_consume(Key key) {
  if (key_down(key)) {
    key_consume(key);
    return true;
  }
  return false;
}
void key_consume(Key key) {
  st->input.consumed[key] = true;
}

ScrollState scroll_state_make(f32 scale) {
  ScrollState res = {
    .scale_level = scale,
    .scale = v2(scale),
  };
  return res;
}

void scroll_state_update(ScrollState& s, ScrollType type) {
  f32 wheel = os_mouse_wheel();
  if (wheel) {
    if (os_key_is_down(Key_Ctrl)) {
      v2 mouse = os_mouse_pos();
      f32 sensity = 1.3;
      f32 zoom = (wheel > 0) ? sensity : 1.0f/sensity;
      switch (type) {
        case ScrollType_Default: {
          // we have: mouse == world * scale + offset;
          v2 world = (mouse - s.offset) / s.scale_level;
          s.scale_level *= zoom;
          s.offset = mouse - world * s.scale_level;
          s.scale = v2(s.scale_level);
        } break;
        case ScrollType_PowClamp: {
          v2 world = {
            (mouse.x - s.offset.x) / s.scale.x,
            (mouse.y - s.offset.y) / s.scale.y
          };
          s.scale_level *= zoom;
          s.scale_level = ClampBot(s.scale_level, 0.02);
          s.scale.y = Clamp(0.01, s.scale_level, 3);
          f32 t = Pow(s.scale_level + 1, 2);
          f32 ratio = t * 0.3;
          s.scale.x = s.scale.y * ratio;

          if (s.scale.y > 1) {
            f32 inv = 1.0f / s.scale_level;
            f32 target = inv / (1.0f + inv);
            s.scale.y = Lerp(1.0f, 0.3f, target);
          }

          s.offset.x = mouse.x - world.x * s.scale.x;
          s.offset.y = mouse.y - world.y * s.scale.y;

        } break;
      }
    }
    else {
      s.offset.y += wheel * 100.0f;
    }
  }

  f32 scroll_h = os_mouse_wheel_horizontal();
  if (scroll_h) {
    f32 sensity = 100;
    if (os_key_is_down(Key_Shift)) {
      sensity *= 3;
    }
    s.offset.x += scroll_h * sensity;
  }
}

void watch_add(String watch_name, WatchOp op) {
  WatchState& g = st->watch;
  FileProperties props = os_file_path_properties(watch_name);
  WatchFile file_watch = {
    .path = watch_name,
    .modified = props.modified,
    .op = op,
  };
  array_push(g.watches, file_watch);
}

void watch_directory_add(String watch_name, WatchOp op, OS_WatchFlags flags) {
  WatchState& g = st->watch;
  String dir_path = push_strf(g.arena, "%s", watch_name);
  OS_Watch watch = os_watch_open(flags);
  os_watch_attach(watch, dir_path);
  WatchDirectory dir_watch = {
    .path = dir_path,
    .watch = watch,
    .op = op,
  };
  array_push(g.directories, dir_watch);
}

void watch_update() {
  WatchState& g = st->watch;
  Scratch scratch;
  Loop (i, g.watches.count) {
    WatchFile& x = g.watches[i];
    FileProperties props = os_file_path_properties(x.path);
    if (props.modified > x.modified) {
      switch (x.op) {
        case WatchOp_NotifyHotreload: {
          st->should_hotreload = true;
        } break;
        InvalidDefaultCase;
      }
      x.modified = props.modified;
    }
  }
  Loop (i, g.directories.count) {
    WatchDirectory x = g.directories[i];
    Slice strs = os_watch_check(scratch, x.watch);
    Loop (i, strs.count) {
      String name = strs[i];
      switch (x.op) {
        case WatchOp_RecompileShader: {
          GlobalState& g = *st;
          Scratch scratch;
          String shader_filepath = push_strf(scratch, "%s/%s", g.shader_dir, name);
          String shader_compiled_filepath = push_strf(scratch, "%s/%s%s", g.shader_compiled_dir, str_chop_last_dot(name), String(".spv"));
          StringList list = {};
          str_list_push(scratch, list, "slangc");
          str_list_push(scratch, list, shader_filepath);
          str_list_push(scratch, list, "-target");
          str_list_push(scratch, list, "spirv");
          str_list_push(scratch, list, "-g");
          str_list_push(scratch, list, "-o");
          str_list_push(scratch, list, shader_compiled_filepath);
          os_process_make(list);
        } break;
        case WatchOp_ShaderReload: {
          GlobalState& g = *st;
          String shader_name = str_chop_last_dot(name);
          String shader_name_slang = push_strf(scratch, "%s.slang", shader_name);
          String shader_filepath = push_strf(scratch, "%s/%s", g.shader_dir, shader_name_slang);
          String shader_compiled_filepath = push_strf(scratch, "%s/%s", g.shader_compiled_dir, name);
          os_file_path_copy_mtime(shader_filepath, shader_compiled_filepath);
          r_shader_reload(shader_name);
        } break;
        InvalidDefaultCase;
      }
    }
  }
}

ThingDesc default_thing_desc() {
  ThingDesc res = {
    .scale = v3(1),
    .rot = quat_identity(),
  };
  return res;
}

R_MaterialProps default_material_props() {
  R_MaterialProps props = {
    .ambient = v3(1),
    .diffuse = v3(1),
    .specular = v3(1),
    .shininess = 1,
  };
  return props;
}

Thing& get_thing(ThingId id) { return pool_get(st->entities, id); }
R_MeshId get_mesh(MeshEnum mesh_enum) { return st->meshes_ids[mesh_enum]; }
R_TextureId get_texture(TextureEnum tex_enum) { return st->textures_ids[tex_enum]; }
R_MaterialId get_material(MaterialEnum id) { return st->materials_ids[id]; }
void mesh_set(MeshEnum mesh_enum, R_MeshId id) { 
  GlobalState& g = *st;
  g.meshes_ids[mesh_enum] = id;
  String str = push_str_copy(g.arena, meshes_strs[mesh_enum]);
  map_set(g.str_to_mesh, str, id);
  g.mesh_to_str[id.idx] = str;
}

void push_child_thing(ThingId parent, ThingId id) {
  var& g = *st;
  hdll_list_push_back(g.entities.data, get_thing(parent), id);
}

String dumb_struct(Allocator arena, Slice<MemberDefinition> members, void* ptr, EntityFlags flags) {
  Scratch scratch(arena);
  var string = dstr_make(arena);
  Loop (i, members.count) {
    MemberDefinition member = members[i];
    u8* member_ptr = Offset(ptr, member.offset);
    switch (members[i].type) {
      default:{} break;
      case MetaType_u32: {
        dstr_push(string, push_strf(scratch, "%s %u\n", member.name, *(u32*)member_ptr));
      } break;
      case MetaType_i32: {
        dstr_push(string, push_strf(scratch, "%s %u\n", member.name, *(i32*)member_ptr));
      } break;
      case MetaType_b32: {
        dstr_push(string, push_strf(scratch, "%s %u\n", member.name, *(b32*)member_ptr));
      } break;
      case MetaType_f32: {
        dstr_push(string, push_strf(scratch, "%s %f\n", member.name, *(f32*)member_ptr));
      } break;
      case MetaType_v2: {
        v2 v = *(v2*)member_ptr;
        dstr_push(string, push_strf(scratch, "%s %f %f\n", member.name, v.x, v.y));
      } break;
      case MetaType_v3: {
        v3 v = *(v3*)member_ptr;
        dstr_push(string, push_strf(scratch, "%s %f %f %f\n", member.name, v.x, v.y, v.z));
      } break;
      case MetaType_v4: {
        v4 v = *(v4*)member_ptr;
        dstr_push(string, push_strf(scratch, "%s %f %f %f %f\n", member.name, v.x, v.y, v.z, v.w));
      } break;
      case MetaType_Rng3: {
        Rng3 v = *(Rng3*)member_ptr;
        dstr_push(string, push_strf(scratch, "%s %f %f %f %f %f %f\n", member.name, v.min.x,v.min.y,v.min.z, v.max.x,v.max.y,v.max.z));
      } break;
      case MetaType_MeshId: {
        R_MeshId v = *(R_MeshId*)member_ptr;
        dstr_push(string, push_strf(scratch, "%s \"%s\"\n", member.name, st->mesh_to_str[v.idx]));
      } break;
      case MetaType_MaterialId: {
        R_MaterialId v = *(R_MaterialId*)member_ptr;
        dstr_push(string, push_strf(scratch, "%s \"%s\"\n", member.name, st->material_to_str[v.idx]));
      } break;
      case MetaType_String: {
        if (FlagHas(flags, EntityFlag_Referenced)) {
          String v = *(String*)member_ptr;
          dstr_push(string, push_strf(scratch, "%s \"%s\"\n", member.name, v));
        }
      } break;
      case MetaType_EntityFlags: {
        EntityFlags v = *(EntityFlags*)member_ptr;
        dstr_push(string, push_strf(scratch, "%s %u\n", member.name, v));
      } break;
    }
  }
  return string;
}

void dumb_struct_load(Slice<MemberDefinition> members, void* ptr, Parser* parser) {
  Parser& p = *parser;
  while (!tok_match(p, TokenType_CloseBrace)) {
    MemberDefinition member = {};
    Token ident_token = tok_advance(p);
    if (ident_token.type == TokenType_Identifier) {
      Loop (i, members.count) {
        if (str_match(members[i].name, ident_token.str)) {
          member = members[i];
          break;
        }
      }
    }
    u8* mem = Offset(ptr, member.offset);
    switch (member.type) {
      default:{} break;
      case MetaType_u32: {
        Token tok = tok_advance(p);
        *(u32*)mem = u32_from_str(tok.str);
      } break;
      case MetaType_i32: {
        *(u32*)mem = parse_i32(p);
      } break;
      case MetaType_b32: {
        Token tok = tok_advance(p);
        *(u32*)mem = u32_from_str(tok.str);
      } break;
      case MetaType_f32: {
        *(f32*)mem = parse_f32(p);
      } break;
      case MetaType_v2: {
        *(v2*)mem = v2(parse_f32(p), parse_f32(p));
      } break;
      case MetaType_v3: {
        *(v3*)mem = v3(parse_f32(p), parse_f32(p), parse_f32(p));
      } break;
      case MetaType_v4: {
        *(v4*)mem = v4(parse_f32(p), parse_f32(p), parse_f32(p), parse_f32(p));
      } break;
      case MetaType_Rng3: {
        *(Rng3*)mem = Rng3(v3(parse_f32(p), parse_f32(p), parse_f32(p)), v3(parse_f32(p), parse_f32(p), parse_f32(p)));
      } break;
      case MetaType_MeshId: {
        Token tok = tok_require(p, TokenType_String);
        var [mesh, ok] = map_get(st->str_to_mesh, tok.str);
        Assert(ok);
        *(R_MeshId*)mem = mesh;
      } break;
      case MetaType_MaterialId: {
        Token tok = tok_require(p, TokenType_String);
        var [material, ok] = map_get(st->str_to_material, tok.str);
        Assert(ok);
        *(R_MaterialId*)mem = material;
      } break;
      case MetaType_String: {
        Token tok = tok_require(p, TokenType_String);
        *(String*)mem = push_str_copy(st->arena, tok.str);
      } break;
      case MetaType_EntityFlags: {
        *(EntityFlags*)mem = parse_u32(p);
      } break;
    }
  }
}

void init() {
  Scratch scratch;
  var& g = *st;

  cpu_find_frequency();
  os_gfx_init();
  prof_init(g.arena);
  prof_launch_begin();

  {
    ProfBlock("init");
    thread_pool_init();
    test();

    g.gpa = alloc_make(g.arena);
    g.frame_arena = arena_make();
    g.asset_dir = push_strf(g.arena, "%s/%s", os_cur_directory(), String("../assets"));
    g.shader_dir = push_str_cat(g.arena, g.asset_dir, "/shaders");
    g.shader_compiled_dir = push_str_cat(g.arena, g.shader_dir, "/compiled");
    g.models_dir = push_str_cat(g.arena, g.asset_dir, "/models");
    g.textures_dir = push_str_cat(g.arena, g.asset_dir, "/textures");
    g.watch.arena = g.arena;
    r_shaders_compile(scratch);
    r_init();
    debug_init();
    init_game();
    watch_directory_add(g.shader_dir, WatchOp_RecompileShader);
    watch_directory_add(g.shader_compiled_dir, WatchOp_ShaderReload);

    // g.ui0 = ui_init();
  }
  prof_launch_end();
}

shared_function void update(HotReloadData* data) {
  Scratch scratch;
if (data->ctx == null) {
    Arena arena = arena_make(.name = "common arena");
    data->ctx = st = push_struct_zero(arena, GlobalState);
    st->arena = arena;
    {
      u64 start = cpu_now();
      init();
      Debug("init time: %fms", tsc_to_ms(cpu_now() - start));
    }
#if HOTRELOAD_BUILD
    watch_add(data->lib_path, WatchOp_NotifyHotreload);
#endif
  }
  if (!st) {
    st = (GlobalState*)data->ctx;
    st->should_hotreload = false;
  }

  var& g = *st;
  // ui_set_current_state(g.ui0);

  u64 target_fps = Billion(1) / 60;
  u64 prev_ns = os_now_ns();

  while (!os_window_should_close()) {
    // co_test(&g.co, get_dt());
    if (st->should_hotreload) {
      goto hotreload;
    }
    // if (time_on_interval(get_time(), get_dt(), 0.1, 0)) {
    //   Info("ye");
    // }

    prof_begin(g.current_frame);
    {
      ProfBlock("frame");
      os_pump_messages();
      u64 now_ns = os_now_ns();
      time_dt = f64(now_ns - prev_ns) / Billion(1);
      time_now += time_dt;
      prev_ns = now_ns;
      r_begin();
      // ui_begin();
      update_game();
      // ui_end();
      r_end();
      watch_update();

      u64 frame_duration = os_now_ns() - now_ns;
      if (frame_duration < target_fps) {
        u64 sleep_time = target_fps - frame_duration;
        ProfBlock("main sleep", ProfType_Sleep);
        os_sleep_ms(sleep_time / Million(1));
      }
    }
    prof_end(g.current_frame);
    ++g.current_frame;
    arena_clear(st->frame_arena);
    mem_track_end();
  }

  // vk_shutdown();
  // os_gfx_shutdown();
  os_exit(0);

  hotreload:
  thread_wait_remanings();
}

R_MeshDesc generate_sphere(Allocator arena) {
  u32 lat_steps = 10;
  u32 lon_steps = 10;
  u32 vert_count = lat_steps*lon_steps;
  u32 index_count = (lat_steps - 1) * lon_steps * 6;
  var vertices = push_slice(arena, R_Vertex, vert_count);
  var indices = push_slice(arena, u32, index_count);
  f32 lat_step_angle = PI / lat_steps;
  f32 lon_step_angle = 2*PI / lon_steps;
  for (u32 i = 0; i < lat_steps; ++i) {
    f32 lat_angle = -PI/2 + i*lat_step_angle;
    for (u32 j = 0; j < lon_steps; ++j) {
      f32 lon_angle = j * lon_step_angle;
      R_Vertex vert = {
        .pos.x = Cos(lat_angle) * Cos(lon_angle),
        .pos.y = Sin(lat_angle),
        .pos.z = Cos(lat_angle) * Sin(lon_angle),
        .uv.x = (f32)j / (lon_steps - 1),  // 0 → 1 across longitude
        .uv.y = (f32)i / (lat_steps - 1),  // 0 → 1 from bottom to top
      };
      vertices[i*lon_steps + j] = vert;
    }
  }
  var idx = [&](u32 i, u32 j) {
    return i * lon_steps + j;
  };
  u32 k = 0;
  for (u32 i = 0; i < lat_steps - 1; ++i) {
    for (u32 j = 0; j < lon_steps; ++j) {
      u32 next_j = (j + 1) % lon_steps; // wrap around
      u32 v0 = idx(i, j);
      u32 v1 = idx(i, next_j);
      u32 v2 = idx(i + 1, j);
      u32 v3 = idx(i + 1, next_j);
      indices[k++] = v0;
      indices[k++] = v2;
      indices[k++] = v1;
      indices[k++] = v1;
      indices[k++] = v2;
      indices[k++] = v3;
    }
  }
  // u32 north_pole_index = vert_count - 1; // last vertex
  // for (u32 j = 0; j < lon_steps; ++j) {
  //   u32 next_j = (j + 1) % lon_steps;
  //   indices[k++] = idx(lat_steps - 2, j); // last row before pole
  //   indices[k++] = north_pole_index;      // pole
  //   indices[k++] = idx(lat_steps - 2, next_j);
  // }
  R_MeshDesc mesh = {
    .vertices = vertices,
    .indices = indices,
  };
  return mesh;
}

R_MeshDesc generate_grid(Allocator arena, u32 size, f32 step) {
  var vertices = push_slice(arena, R_Vertex, size*4);
  v3 pos_offset = v3(-(i32)size/2, 0, -(i32)size/2);
  for (i32 i = 0; i < size; ++i) {
    vertices[i*2].pos = pos_offset + v3(0, 0, i*step);
    vertices[i*2+1].pos = pos_offset + v3(size*step, 0, i*step);
  }
  var vertical_vertices = slice_skip(vertices, size*2);
  for (i32 i = 0; i < size; ++i) {
    vertical_vertices[i*2].pos = pos_offset + v3(i*step, 0, 0);
    vertical_vertices[i*2+1].pos = pos_offset + v3(i*step, 0, size*step);
  }
  R_MeshDesc mesh = {
    .vertices = vertices,
  };
  return mesh;
}

// b32 ray_intersect_AABB(Ray ray, AABB aabb) {
//   v3 tMin = v3_hadamard_div(aabb.min - ray.origin, ray.dir);
//   v3 tMax = v3_hadamard_div(aabb.max - ray.origin, ray.dir);
//   v3 t1 = v3_less(tMin, tMax);
//   v3 t2 = v3_greater(tMin, tMax);
//   f32 tNear = Max3(t1.x, t1.y, t1.z);
//   f32 tFar = Min3(t2.x, t2.y, t2.z);
//   if (tNear > tFar) {
//     return false;
//   }
//   return true;
// };

// ThingId e_alloc_bare() {
//   var& g = *st;
//   ThingId e_id = pool_push(g.entities, {});
//   ++g.entities_count;
//   return e_id;
// }

// ThingId e_alloc(R_MeshId mesh_id, R_MaterialId material_id, EntityThing thing) {
//   var& g = *st;
//   Thing e = {
//     .name = thing.name,
//     .flags = thing.flags,
//     .pos = v3(),
//     .rot = quat_identity(),
//     .scale = v3(1),
//     .mesh = mesh_id,
//     .mat = material_id,
//     .aabb = Rng3(v3(-1), v3(1)),
//     .color = u32_from_rgba(ColorWhite),
//   };
//   ThingId e_id = pool_push(g.entities, e);
//   ++g.entities_count;
//   return e_id;
// }
// ThingId e_alloc(MeshEnum mesh_id, MaterialEnum material_id, EntityThing thing) { return e_alloc(get_mesh(mesh_id), get_material(material_id), thing); }

ThingId make_thing(ThingDesc desc) {
  var& g = *st;
  _DefSet(desc.mesh, Mesh_Cube);
  _DefSet(desc.mat, Material_Orange);
  _DefIfSet(desc.rot, v4_equal(desc.rot, v4()), quat_identity());
  _DefIfSet(desc.scale, v3_equal(desc.scale, v3()), v3(1));
  _DefSet(desc.color,  u32_from_rgba(ColorWhite));
  Thing e = {
    .pos = desc.pos,
    .rot = desc.rot,
    .scale = desc.scale,
    .mesh = get_mesh(desc.mesh),
    .mat = get_material(desc.mat),
    .aabb = Rng3(v3(-1), v3(1)),
    .color = desc.color,
  };
  ThingId id = pool_push(g.entities, e);
  ++g.entities_count;
  return id;
}

void destroy_thing(ThingId id) {
  var& g = *st;
  pool_remove(g.entities, id);
  --g.entities_count;
}

void select_obj() {
  var& g = *st;
  v3 dir = ray_from_screen(os_mouse_pos(), os_window_size(), g.cam.pos, st->view, st->projection).dir;
  var desc = default_thing_desc();
  desc.pos = g.cam.pos;
  desc.scale = v3(0.3);
  desc.vel = dir * 4;
  make_thing(desc);
  // e.pos() = st->cam.pos + v3_norm(m4x4_forward(st->cam.view));
}

void save_game_state() {
  Scratch scratch;
  var& g = *st;

  Dstring data = dstr_make(scratch);
  dstr_push(data, "Camera {\n");
  dstr_push(data, dumb_struct(scratch, slice(members_of_Camera), &g.cam));
  dstr_push(data, "}\n");

  {
    Thing e = get_thing(g.cube1);
    dstr_push(data, "e {\n");
    dstr_push(data, dumb_struct(scratch, slice(members_of_Entity), &e));
    dstr_push(data, "}\n");
  }
  {
    LoopIter (it, pool_begin(g.entities)) {
      Thing& e = *it;
      dstr_push(data, "Entity {\n");
      dstr_push(data, dumb_struct(scratch, slice(members_of_Entity), &e, e.flags));
      dstr_push(data, "}\n");
    }
  }

  OS_Handle file = os_file_open(push_strf(scratch, "%s/saved", os_cur_directory(), String("saved")), OS_AccessFlag_Write | OS_AccessFlag_Trunc);
  os_file_write(file, dstr_slice(data));
  os_file_close(file);
}

void load_game_state() {
  var& g = *st;
  Scratch scratch;
  Slice data = os_file_path_read_all(scratch, push_strf(scratch, "%s/saved", os_cur_directory(), String("saved")));
  Slice tokens = tokens_from_str(scratch, str_make(data.data, data.size));
  Parser p = parser_make(tokens);

  {
    LoopIter (it, pool_begin(g.entities)) {
      ThingId e_id = it.handle();
      destroy_thing(e_id);
    }
  }

  while (!tok_is_end(p)) {
    Token tok = tok_advance(p);
    switch (tok.type) {
      default:{} break;
      case TokenType_Identifier: {
        if (str_match(tok.str, "Camera")) {
          dumb_struct_load(slice(members_of_Camera), &g.cam, &p);
        } else if (str_match(tok.str, "Entity")) {
          ThingId id = make_thing({});
          Thing& e = get_thing(id);
          dumb_struct_load(slice(members_of_Entity), &e, &p);
          if (FlagHas(e.flags, EntityFlag_Referenced)) {
            if (str_match("monkey", e.name)) {
              g.monkey0 = id;
            } else if (str_match("axis_attached_to_cam", e.name)) {
              g.axis_attached_to_cam_id = id;
            } else if (str_match("rotating_cube", e.name)) {
              g.cube0 = id;
            } 
          }
        } else if (str_match(tok.str, "e")) {
          ThingId e_id = make_thing({});
          Thing& e = get_thing(e_id);
          dumb_struct_load(slice(members_of_Entity), &e, &p);
          g.cube1 = e_id;
        }
      }
    }
  }
}

void init_game() {
  ProfFunc;
  var& g = *st;
  Scratch scratch;
  g.arena = arena_make(.name = "game arena");
  g.gpa = alloc_make(g.arena);
  g.moving_cubes = array_make(ThingId, g.gpa);
  // g.font = r_font_load("arial.ttf", 512);

  R_MeshDesc triangle_mesh = {.vertices = slice(triangle_vertices)};
  mesh_set(Mesh_Triangle, r_make_mesh(triangle_mesh));
  R_MeshDesc grid_mesh = generate_grid(scratch, 100, 1);
  mesh_set(Mesh_Grid, r_make_mesh(grid_mesh));
  R_MeshDesc axis_mesh = {.vertices = slice(axis_vertices)};
  mesh_set(Mesh_Axis, r_make_mesh(axis_mesh));
  R_MeshDesc sphere = generate_sphere(scratch);
  mesh_set(Mesh_Sphere, r_make_mesh(sphere));

  {
    ProfBlock("cube");
    // R_Texture cubemap = r_texture_cube_load("night_cubemap");
    // R_TextureId cubemap = r_load_async_cubemap("night_cubemap");
    R_TextureId cubemap = r_make_texture({.name = "night_cubemap", .is_cube = true, .async = true});
    r_set_cubemap(cubemap);
  }

  {
    GlobalState& g = *st;
    var load_mesh = [&](MeshEnum enum_name, String name) {
      R_MeshId id = r_make_mesh({.name = name, .async = false});
      g.meshes_ids[enum_name] = id;
      String str = push_str_copy(g.arena, name);
      map_set(g.str_to_mesh, str, id);
      g.mesh_to_str[id.idx] = str;
    };
    load_mesh(Mesh_Cube, "cube_ok_uv.glb");
    load_mesh(Mesh_MonkeyGlb, "monkey.glb");
    load_mesh(Mesh_CubeGlft, "cube.gltf");
    load_mesh(Mesh_Barrack, "castle.gltf");
    // m_load(Mesh_Barrack, "castle.obj");
    // m_load(Mesh_GreeMan, "greenman.glb");

    var load_tex = [&](TextureEnum enum_name, String name) {
      R_TextureId id = r_make_texture({.name = name, .async = true});
      g.textures_ids[enum_name] = id;
      String str = push_str_copy(g.arena, name);
      map_set(g.str_to_texture, str, id);
      g.texture_to_str[id.idx] = str;
    };
    load_tex(Texture_Orange, "orange_lines_512.png");
    load_tex(Texture_Container, "container.jpg");
    load_tex(Texture_Barrack, "castle_diffuse.png");

    var default_state = r_make_pipeline_state({
      .depth = {
        .compare = Gfx_CompareOp_Less,
        .write_enabled = true,
      }
    });
    var default_state_line = r_make_pipeline_state({
      .primitive_type = Gfx_PrimitiveType_Line,
      .depth = {
        .compare = Gfx_CompareOp_Less,
        .write_enabled = true,
      }
    });

    var load_mat = [&](MaterialEnum enum_name, R_Material desc) {
      R_MaterialId id = r_material_make(desc);
      g.materials_ids[enum_name] = id;
      String str = push_str_copy(g.arena, materials_strs[enum_name]);
      map_set(g.str_to_material, str, id);
      g.material_to_str[id.idx] = str;
    };

    load_mat(Material_Orange, {
      .type = ShaderType::Texture,
      .batch = default_state,
      .props = default_material_props(),
      .base_color = get_texture(Texture_Orange),
    });
    load_mat(Material_Container, {
      .type = ShaderType::Texture,
      .batch = default_state,
      .props = default_material_props(),
      .base_color = get_texture(Texture_Container),
    });
    load_mat(Material_Axis, {
      .type = ShaderType::VertColor,
      .batch = default_state_line,
    });
    load_mat(Material_Line, {
      .type = ShaderType::E_Color,
      .batch = default_state_line,
    });
    load_mat(Material_Barrack, {
      .type = ShaderType::Texture,
      .batch = default_state,
      .props = default_material_props(),
      .base_color = get_texture(Texture_Barrack),
    });
  }

  ///////////////////////////////////
  // Camera
  {
    var& g = *st;
    Camera& cam = g.cam;
    cam = {
      .pos = v3(0,0,5),
      .yaw = 180,
      .fov = 45,
      .vel_friction = 7.7,
      .accel = 300,
    };
    cam.dir = {
      CosD(cam.yaw) * CosD(cam.pitch),
      SinD(cam.pitch),
      SinD(cam.yaw) * CosD(cam.pitch)
    };
    st->view = m4x4_look_at(cam.pos, cam.dir, v3_up());
  }

  g.cube0 = make_thing(default_thing_desc());
  var desc = default_thing_desc();
  desc.mesh = Mesh_MonkeyGlb;
  desc.mat = Material_Container;
  desc.pos.x = 10;
  desc.aabb = Rng3(v3(-1.2), v3(1.2));
  g.monkey0 = make_thing(desc);
  {
    var desc = default_thing_desc();
    desc.mesh = Mesh_Triangle;
    desc.mat = Material_Orange;
    desc.pos = v3(6);
    make_thing(desc);
  }
  {
    var desc = default_thing_desc();
    desc.mesh = Mesh_Grid;
    desc.mat = Material_Line;
    desc.color = u32_from_rgba(v4(0.6));
    desc.pos = v3(0,0,-5);
    make_thing(desc);
  }
  {
    var desc = default_thing_desc();
    desc.mesh = Mesh_Axis;
    desc.mat = Material_Axis;
    g.axis_attached_to_cam_id = make_thing(desc);
  }
#if 1
  Loop (i, 10) {
    // Handle<StaticEntity> e = entity_static_create(Mesh_Cube, Material_Orange);
    // u32 range = KB(1);
    // e.pos() = v3_rand_range(-v3_scale(range), v3_scale(range));
    MeshEnum meshes[] = {
      // Mesh_MonkeyGlb,
      // Mesh_Triangle,
      Mesh_Cube,
    };
    MaterialEnum materials[] = {
      Material_Orange,
      // Material_Container,
      // Material_Screen,
    };
    var desc = default_thing_desc();
    desc.mesh = meshes[rand_u32_rng(0, ArrayCount(meshes)-1)];
    desc.mat = materials[rand_u32_rng(0, ArrayCount(materials)-1)];
    u32 range = 100;
    desc.pos = v3_rand_rng(-v3(range), v3(range));;
    make_thing(desc);
  }
#endif

  {
    var desc = default_thing_desc();
    desc.mesh = Mesh_Sphere;
    desc.mat = Material_Container;
    desc.pos = v3(0,0,-10);
    make_thing(desc);
  }
  {
    // Handle<Entity> e = entity_create(Mesh_Castle, Shader_E_Texture, Material_Castle);
    // e.pos().z = -100;
  }
  {
    // Loop (i, KB(1)) {
    //   Handle<Entity> e = entity_create(Mesh_Cube, Material_Screen);
    //   u32 range = 100;
    //   e.pos() = v3_rand_range(-v3_scale(range), v3_scale(range));
    // }
  }
  {
    // Loop (i, KB(400)) {
    // Loop (i, MB(1)-KB(1)) {
    Loop (i, 0) {
      var desc = default_thing_desc();
      desc.mat = Material_Container;
      u32 range = KB(1);
      desc.pos = v3_rand_rng(-v3(range), v3(range));
      var id = make_thing(desc);
      array_push(g.moving_cubes, id);
    }
  }

  Loop (i, 0) {
    MeshEnum meshes[] = {
      Mesh_MonkeyGlb,
      Mesh_Triangle,
      Mesh_Cube,
    };
    MaterialEnum materials[] = {
      // Material_Orange,
      Material_Container,
      // Material_Screen,
    };
    var desc = default_thing_desc();
    desc.mesh = meshes[rand_u32_rng(0, ArrayCount(meshes)-1)];
    desc.mat = materials[rand_u32_rng(0, ArrayCount(materials)-1)];
    u32 range = 100;
    desc.pos = v3_rand_rng(-v3(range), v3(range));
    var id = make_thing(desc);
    array_push(g.moving_cubes, id);
  }
  {
    var desc = default_thing_desc();
    desc.mat = Material_Container;
    desc.pos = v3(10,10,20);
    g.cube1 = make_thing(desc);
  }

  {
    v4 quat = quat_axis_angle(v3(1,0,0), deg2rad(90));
    v4 quat1 = quat_axis_angle(v3(1,0,0), deg2rad(90));
    quat = quat_mul(quat, quat1);
    v3 v = v3(0, 1, 0);
    v = quat_rotate(quat, v);
    Info("%f %f %f", v.x,v.y,v.z);
  }

  {
    var desc = default_thing_desc();
    desc.mesh = Mesh_Barrack;
    desc.mat = Material_Barrack;
    desc.pos = v3(-3,3,-33);
    make_thing(desc);
  }
  {
    var desc = default_thing_desc();
    desc.mat = Material_Container;
    desc.pos = v3(3,3,10);
    g.cube_root = make_thing(desc);
    Loop (i, 4) {
      var desc = default_thing_desc();
      desc.mat = Material_Container;
      var child_id = make_thing(desc);
      push_child_thing(g.cube_root, child_id);
      Loop (i, 4) {
        var new_child_id = make_thing(desc);
        push_child_thing(child_id, new_child_id);
      }
    }
  }
  {
    var desc = default_thing_desc();
    desc.mesh = Mesh_MonkeyGlb;
    desc.mat = Material_Container;
    desc.pos = v3(5,5,0);
    g.monkey1 = make_thing(desc);
  }
  {
    var desc = default_thing_desc();
    desc.pos = v3(1);
    g.cube2 = make_thing(desc);
    desc.pos = v3(0,3,0);
    g.cube3 = make_thing(desc);
    desc.pos += v3(0,3,0);
    desc.mat = Material_Container;
    g.cube4 = make_thing(desc);
    get_thing(g.cube4).angle = Rad(-160);
    g.cube5 = make_thing(desc);
    get_thing(g.cube5).angle = Rad(160);
  }
}

void update_game() {
  ProfFunc;
  var& g = *st;

  ArrayZero(st->input.consumed);
  debug_update();

  // Test jobs
  {
    ProfBlock("push jobs");
    Loop (i, 2) {
      thread_push({.fn = [](void* ctx) {os_sleep_ms(rand_u32()%4);}, .priority = TaskPriority_Low});
    }
    WaitGroup id0 = thread_push({.fn = [](void* ctx) { os_sleep_ms(2); }});
    WaitGroup id1 = thread_push({.fn = [](void* ctx) { os_sleep_ms(2); }});
    WaitGroup id2 = thread_push({.fn = [](void* ctx) { os_sleep_ms(2); }});
    WaitGroup id3 = thread_push({.fn = [](void* ctx) { os_sleep_ms(2); }});
    TaskDesc tasks[] = {
      {.fn = [](void* ctx) { os_sleep_ms(2); }},
      {.fn = [](void* ctx) { os_sleep_ms(2); }},
      {.fn = [](void* ctx) { os_sleep_ms(2); }},
      {.fn = [](void* ctx) { os_sleep_ms(2); }},
    };
    WaitGroup batch = thread_push_batch(slice(tasks));
    thread_wg_wait(id0);
    thread_wg_wait(id1);
    thread_wg_wait(id2);
    thread_wg_wait(id3);
    thread_wg_wait(batch);
  }

  ///////////////////////////////////
  // Hotkeys
  if (os_key_is_down(Key_Escape)) {
    os_window_close();
  }
  if (os_key_is_pressed(Key_U)) {
    if (g.fps_camera) {
      os_cursor_unlock();
    } else {
      os_cursor_lock();
    }
    g.fps_camera = !g.fps_camera;
  }

  ///////////////////////////////////
  // Camera
  {
    Camera& cam = g.cam;
    v2 win_size = v2_of_v2u(os_window_size());
    m4x4& projection = st->projection;
    m4x4& view = st->view;
    projection = m4x4_perspective(deg2rad(cam.fov), win_size.x / win_size.y, 0.1f, 1000.0f);

    f32 rotation_speed = 180.0f * time_dt;
    if (os_key_is_down(Key_A)) {
      cam.yaw += rotation_speed;
    }
    if (os_key_is_down(Key_D)) {
      cam.yaw -= rotation_speed;
    }
    if (os_key_is_down(Key_R)) {
      cam.pitch += rotation_speed;
    }
    if (os_key_is_down(Key_F)) {
      cam.pitch -= rotation_speed;
    }
    if (g.fps_camera) {
      f32 rot_speed = 10;
      cam.pitch -= os_mouse_dt().y * time_dt * rot_speed;
      cam.yaw -= os_mouse_dt().x * time_dt * rot_speed;
    }
    f32 speed = 1;
    v3 mov = {};
    if (os_key_is_down(Key_W)) {
      mov += m4x4_forward(view);
    }
    if (os_key_is_down(Key_S)) {
      mov += m4x4_backward(view);
    }
    if (os_key_is_down(Key_Q)) {
      mov += m4x4_left(view);
    }
    if (os_key_is_down(Key_E)) {
      mov += m4x4_right(view);
    }
    if (os_key_is_down(Key_Space)) {
      mov.y += 1.0f;
    }
    if (os_key_is_down(Key_X)) {
      mov.y -= 1.0f;
    }
    if (os_key_is_down(Key_Shift)) {
      speed *= 10;
    }
    if (os_key_is_down(Key_LAlt)) {
      speed *= 0.1;
    }
    mov *= cam.accel * speed;
    f32 dt = time_dt;

    cam.vel += mov * dt;
    cam.pos += cam.vel * dt;
    cam.vel -= cam.vel * cam.vel_friction * dt;
    // cam.vel.y -= 9 * dt;

    ImGui::Begin("cam");
    // ImGui::DragFloat("accel", &cam.accel, 0, 0, 3000);
    ImGui::DragFloat("vel fric", &cam.vel_friction, 0.1, 0, 100);

    {
      var& cube2 = get_thing(g.cube2);
      var& cube3 = get_thing(g.cube3);
      var& cube4 = get_thing(g.cube4);
      // var& cube5 = get_thing(g.cube5);
      // local f32 deg = 0;
      // ImGui::DragFloat("angel", &deg);
      // ImGui::Text("cube5 - cub4 %f", Deg(cube5.angle - cube4.angle));
      // cube4.angle = Rad(deg);
      // ImGui::Text("cube4 wrap_pi %f", Deg(wrap_pi(cube4.angle)));
      // ImGui::Text("cube4 wrap_2pi %f", Deg(wrap_2pi(cube4.angle)));
      // ImGui::Text("wrap_pi(cube5 - cub4) %f", Deg(wrap_pi(cube5.angle - cube4.angle)));
      // ImGui::Text("wrap_2pi(cube5 - cub4) %f", Deg(wrap_2pi(cube5.angle - cube4.angle)));
      // cube3.pos = v3_lerp(cube3.pos, 0.50 * dt, cam.pos);
      // cube3.pos += (cam.pos - cube3.pos)*0.50 * dt;
      // a = (a, 0.01*dt, b);


      // f32 t = 1.0f - Pow(0.5f, dt);
      // f32 t = 1.0f - Exp(-111.9 * dt);
      // cube3.pos += (cam.pos - cube3.pos) * t;
      // cube3.pos.x += (cube2.pos.x - cube3.pos.x) * t;

      cube3.pos.x = exp_decay(cube3.pos.x, cube2.pos.x, 1.1f, dt);

      ImGui::DragFloat3("drag", cube4.pos.v, 0.1);
    }

    ImGui::End();

    // cam.pos -= v3(0,1,0) * 10 * time_dt;
    // cam.pitch = Clamp(-89.0f, cam.pitch, 89.0f);
    // cam.dir = {
    //   CosD(cam.yaw) * CosD(cam.pitch),
    //   SinD(cam.pitch),
    //   SinD(cam.yaw) * CosD(cam.pitch)
    // };
    // view = m4x4_look_at(cam.pos, cam.pos + cam.dir);
    v4 yaw = quat_axis_angle(v3_up(), deg2rad(cam.yaw));
    v4 pitch = quat_axis_angle(v3_right(), deg2rad(-cam.pitch));
    v4 quat = quat_mul(yaw, pitch);
    cam.dir = quat_forward(quat);
    view = m4x4_look_at(cam.pos, cam.pos + cam.dir, quat_up(quat));
  }

  // if (os_mouse_is_button_pressed(MouseButton_Left)) {
    // select_obj();
    // v3 dir = ray_from_screen();
    // Ray ray = ray_from_screen(os_mouse_get_pos(), os_get_window_size(), g.cam.pos, st->view, st->projection);
    // r_draw_line_persistent(ray.pos - v3(0,0.1,0), g.cam.pos + ray.dir*100, ColorWhite);
    // v3 max = st->cam.pos + v3_one();
    // v3 min = st->cam.pos - v3_one();
  // }

  // Playing stuff with moving/drawing things
  {
    Thing& cube0 = get_thing(g.cube0);
    Thing& monkey = get_thing(g.monkey0);
    monkey.pos.x += 0.1 * time_dt;
    cube0.pos.x = monkey.pos.x + Sin(time_dt) * 4;
    cube0.pos.z = monkey.pos.z + Cos(time_dt) * 4;
    cube0.pos.y = monkey.pos.z + Cos(time_dt) * 4;
    r_draw_cuboid(rng3_shift(monkey.aabb, monkey.pos), ColorWhite);
    Thing& cube1 = get_thing(g.cube1);
    // v2_rotate_relative(a, b, cosine, sine);
    // v2 pivot = v2(20, 10);
    // e.pos = v2_to_v3(v2_rotate_relative(v2_of_v3(e.pos), pivot, degtorad(20 * get_dt())), 0);
    // e.pos = v3_rotate_z(e.pos, degtorad(20) * get_dt());
    // e.pos = v3_rotate_y(e.pos, degtorad(20) * get_dt());
    // e.pos = v3_rotate_z(e.pos, degtorad(20) * get_dt());
    cube1.pos = v3_rotate_around_axis(cube1.pos, v3(1,1,1), deg2rad(60)*time_dt);

    {
      var& thing = get_thing(g.cube2);
      thing.scale = v3(1,1,3);
      // thing.pos.x = Lerp(-10, time_smooth_wave(time_now, 2), 10);
      thing.rot = quat_axis_angle(v3_up(), time_now);
    }

    {
      var& cube4 = get_thing(g.cube4);
      var& cube5 = get_thing(g.cube5);
      f32 dst = 10;
      cube4.pos = v2_to_v3(v2_from_angle(cube4.angle), 0) * dst;
      cube5.pos = v2_to_v3(v2_from_angle(cube5.angle), 0) * dst;
      // cube4.angle = angle_move_toward(cube4.angle, cube5.angle, 0.1, time_dt);
      
    }

    {
      Thing& e = get_thing(g.monkey1);
      // e.rot = quat_look_rotation(v3_back(), v3(0,1,0));
      // e.rot = quat_look_rotation(g.pos_target - e.pos, v3(0,1,0));
      e.rot = quat_axis_angle(v3_up(),time_now);
      r_draw_line(e.pos, e.pos + quat_right(e.rot)*2, ColorRed);
      r_draw_line(e.pos, e.pos + quat_up(e.rot)*2, ColorGreen);
      r_draw_line(e.pos, e.pos + quat_forward(e.rot)*2, ColorBlue);
      r_draw_mesh_trs(get_mesh(Mesh_Cube), get_material(Material_Container), g.pos_target, quat_identity(), v3(0.2));

      f32 line_len = 5;
      v3 pos = v3(0,0.1,0);
      r_draw_line(pos, line_len * v3_right(), ColorRed);
      r_draw_line(pos, line_len * v3_up(), ColorGreen);
      r_draw_line(pos, line_len * v3_forward(), ColorBlue);
    }

    {
      var& my = get_thing(g.cube_root);
      my.pos.z += time_dt * 1;
      u32 i = 1;
      LoopHNode (it, my.first, g.entities.data) {
        var& child = get_thing(it);
        child.pos = my.pos + v3(0,3,0)*i++;
        u32 j = 1;
        LoopHNode (i, child.first, g.entities.data) {
          var& new_child = get_thing(i);
          new_child.pos = child.pos + v3(3,0,0)*j;
          ++j;
        }
      }
    }
  }

  // Axis aligned at edge of screen
  {
    m4x4& view = st->view;
    v3 forward = m4x4_forward(view);
    v3 right   = m4x4_right(view);
    v3 up      = m4x4_up(view);
    f32 dist = 1.0f;
    f32 xoff = 0.3f;
    f32 yoff = 0.3f;
    Thing& axis = get_thing(g.axis_attached_to_cam_id);
    axis.pos = g.cam.pos + forward*dist + right*xoff + up*yoff;
    axis.scale = v3(0.1);
  }

  // Random creating
  Loop (i, 0) {
    MeshEnum meshes[] = {
      // Mesh_MonkeyGlb,
      // Mesh_Triangle,
      Mesh_Cube,
    };
    MaterialEnum materials[] = {
      Material_Orange,
      // Material_Container,
      // Material_Screen,
    };
    var desc = default_thing_desc();
    desc.mesh = meshes[rand_u32_rng(0, ArrayCount(meshes)-1)];
    desc.mat = materials[rand_u32_rng(0, ArrayCount(materials)-1)];
    u32 range = 100;
    desc.pos = v3_rand_rng(-v3(range), v3(range));;
    var id = make_thing(desc);
    array_push(g.moving_cubes, id);
  }

  // Moving cubes
  Loop (i, g.moving_cubes.count) {
    Thing& e = get_thing(g.moving_cubes[i]);
    e.pos += e.vel * time_dt;
    v3 center = {0, 0, 0};
    v3 dir = e.pos - center;
    v3 tangent = v3_norm(v3{-dir.z, 0, dir.x});
    e.vel += tangent * 2.0f * time_dt;
    e.vel += -dir * 0.5f * time_dt;
  }

  ///////////////////////////////////
  // Drawing things
  LoopIter (it, pool_begin(g.entities)) {
    ThingId e_id = it.handle();
    // Entity& e = pool_get(g.entities, e_id);
    // r_draw_mesh(e.mesh, e.mat, e.pos);
    r_draw_entity(e_id);
  }

  ///////////////////////////////////
  // Rect text
  {
    // r_draw_quad(rng2_make(v2(400), v2(400,400)), ColorCyan);
    // r_draw_rect_gradient(rng2_make(v2(300), v2(100)), {v4(1,0,0,1), v4(0,1,0,1), v4(0,0,1,1), v4()});
    // r_draw_rect(rng2_make(v2(600), v2(100)), ColorWhite);
    r_draw_texture(rng2_make(v2(800), v2(100)), get_texture(Texture_Orange));
    r_draw_text_ext(st->r.my_font, v2(300), "I'm a hobbit from Shire!", ColorOrange, 64);
    r_draw_rect(rng2_make(v2(100), v2(200)), ColorGreyDark);
    r_draw_text_ext(st->r.my_font, v2(100, 100+32), "I'm a button", ColorWhite, 32);
    // r_draw_text_ext(st->r.my_font, os_mouse_pos(), "I'm a button", ColorWhite, 32);
    // if (time_on_interval(0.3)) {
    //   Info("%f %f", os_mouse_pos().x, os_mouse_pos().y);
    // }
    // r_draw_rect_gradient(rng2_make(v2(500), v2(100)), {v4(1,1,1,0.0), v4(0,0,0,0.3), v4(1,1,1,0.3), v4()});
    Rng2 rect = rng2_make(v2(100), v2(100));
    r_draw_rect_rounded(rng2_pad(rect, 10), ColorGrey, 20, 8.9);
    if (rng2_contains(rect, os_mouse_pos())) {
      if (os_mouse_is_button_down(MouseButton_Left)) {
        r_draw_rect(rect, ColorBlack);
      } else {
        r_draw_rect(rect, ColorGrey);
      }
    } else {
      // r_draw_rect(rect, ColorGrey1);
    }
  }

  ///////////////////////////////////
  // UI
  {
    // static B32 dark_mode = false;
    // static F32 volume = 0.3f;
    // S32 click_count = 0;

    UI_Input input = {0};
    // input.mouse_pos = (UI_Vec2){ script[f].x, script[f].y };
    // input.mouse_down[0] = script[f].down;
    input.dt = 1.0f / 60.0f;

    // printf("================ frame %d: %s ================\n", f, script[f].note);
    // input.mouse_pos = bit_cast(UI_Vec2, os_mouse_pos());

    // ui_begin_frame(input, 800, 600);
    // {
    //   // UI_Parent()
    //   // ui_button(str8_lit("hello"));
    //   // ui_button(str8_lit("hello"));
    //   // ui_button(str8_lit("hello"));
    //   // ui_button(str8_lit("hello"));

    //   UI_Parent(ui_panel_begin(str8_lit("panel1"), UI_Axis2_Y)) {
    //     ui_label(str8_lit("label1"));
    //   }
    //   UI_Parent(ui_panel_begin(str8_lit("panel2"), UI_Axis2_Y)) {
    //     ui_label(str8_lit("label2"));
    //     UI_Parent(ui_panel_begin(str8_lit("panel3"), UI_Axis2_Y)) {
    //       ui_label(str8_lit("label3"));
    //     }
    //   }

    //   /* ui_panel_begin_sized() pushes itself as the current parent;
    //    * ui_panel_end() pops it. Fixed pixel width here (rather than
    //    * ui_size_children) is what lets the Volume slider below use
    //    * PercentOfParent safely -- see the note on ui_panel_begin(). */
    //   ui_panel_begin_sized(str8_lit("main_panel##mp"), UI_Axis2_Y, ui_size_px(200, 1), ui_size_children(1));
    //   {
    //     ui_label(str8_lit("Settings"));
    //     ui_spacer(ui_size_px(4, 1));

    //     UI_Signal btn = ui_button(str8_lit("Click me##btn1"));
    //     if (btn.clicked) click_count++;

    //     ui_checkbox(str8_lit("Dark mode##dm"), &dark_mode);

    //     ui_spacer(ui_size_px(4, 1));
    //     ui_slider(str8_lit("Volume##vol"), &volume, 0.0f, 1.0f);
    //   }
    //   ui_panel_end();
    // }
    // ui_end_frame();
    // Loop (i, ui_state->draw_cmd_count) {
    //   UI_DrawCmd cmd = ui_state->draw_cmds[i];
    //   Rng2 rect = Transmute(Rng2, cmd.rect);
    //   v4 color = Transmute(v4, cmd.color);
    //   String str = Transmute(String, cmd.text.str);
    //   switch (cmd.kind) {
    //     case UI_DrawCmd_Rect: {
    //       if (cmd.filled) {
    //         ui_draw_rect(rect, color);
    //       } else {
    //         r_draw_rect_outline(rect, 2, color);
    //       }
    //     } break;
    //     case UI_DrawCmd_Text: {
    //       // u32 a = 1;
    //       r_draw_text(v2(cmd.rect.x0, cmd.rect.y0), str, color);
    //     } break;
    //     default: break;
    //   }
    // }
  }
}

