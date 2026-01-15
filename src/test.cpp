#include "test.h"

i32 alignments[] = { 8, 16, 32, 64 };

void global_alloc_test() {
  Array<u8*, 100> arr;

  Loop (i, 100) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = alignments[rand_range_u32(0, 3)];
    arr.append(global_alloc(size, align));
    MemZero(arr[i], size);
  }
  Array<u32, 100> indices;
  Loop(i, 100) indices.append(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    global_free(arr[indices[i]]);
  }

  arr.clear();
  Loop (i, 100) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = alignments[rand_range_u32(0, 3)];
    arr.append(global_alloc(size, align));
    MemZero(arr[i], size);
  }
  indices.clear();
  Loop(i, 100) indices.append(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    global_free(arr[indices[i]]);
  }
}

void arena_alloc_test() {
  Arena arena = arena_init();
  Array<u8*, 100> arr;
  Array<u32, 100> sizes;
  Array<u32, 100> values;
  Loop (i, 100) {
    u32 size = rand_range_u32(8, KB(1));
    u64 align = alignments[rand_range_u32(0, 3)];
    arr[i] = push_buffer(arena, size, align);
    sizes[i] = size;
    values[i] = rand_range_u32(0, 255);
    MemSet(arr[i], values[i], size);
  }
  Loop (i, 100) {
    u8* buff = arr[i];
    u32 size = sizes[i];
    u32 value = values[i];
    Loop (j, size) {
      Assert(buff[j] == value);
    }
  }
  arena_deinit(&arena);
}

void arena_list_alloc_test() {
  Scratch scratch;
  ArenaList arena(scratch);

  Array<u8*, 100> arr;
  Array<u32, 100> sizes;
  Array<u32, 100> values;
  Loop (i, 100) {
    u32 size = rand_range_u32(8, KB(1));
    u64 align = alignments[rand_range_u32(0, 3)];
    arr[i] = push_buffer(arena, size, align);
    sizes[i] = size;
    values[i] = rand_range_u32(0, 255);
    MemSet(arr[i], values[i], size);
  }
  Loop (i, 100) {
    u8* buff = arr[i];
    u32 size = sizes[i];
    u32 value = values[i];
    Loop (j, size) {
      Assert(buff[j] == value);
    }
  }

  arena.clear();

  Loop (i, 100) {
    u32 size = rand_range_u32(8, KB(1));
    u64 align = alignments[rand_range_u32(0, 3)];
    arr[i] = push_buffer(arena, size, align);
    sizes[i] = size;
    values[i] = rand_range_u32(0, 255);
    MemSet(arr[i], values[i], size);
  }
  Loop (i, 100) {
    u8* buff = arr[i];
    u32 size = sizes[i];
    u32 value = values[i];
    Loop (j, size) {
      Assert(buff[j] == value);
    }
  }

  arena.clear();
}

void seglist_alloc_test() {
  Scratch scratch;
  AllocSegList alloc(scratch);
  Array<u8*, 100> arr;

  Loop (i, 100) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = alignments[rand_range_u32(0, 3)];
    arr.append(mem_alloc(alloc, size, align));
    MemZero(arr[i], size);
  }
  Array<u32, 100> indices;
  Loop(i, 100) indices.append(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    mem_free(alloc, arr[indices[i]]);
  }

  arr.clear();
  Loop (i, 100) {
    u64 size = rand_range_u32(8, KB(1));
    u64 align = alignments[rand_range_u32(0, 3)];
    arr.append(mem_alloc(alloc, size, align));
    MemZero(arr[i], size);
  }
  indices.clear();
  Loop(i, 100) indices.append(i);
  for (u32 i = indices.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(indices[i], indices[j]);
  }
  Loop (i, 100) {
    mem_free(alloc, arr[indices[i]]);
  }
}

void test() {
  global_alloc_test();
  arena_alloc_test();
  arena_list_alloc_test();
  seglist_alloc_test();
}
