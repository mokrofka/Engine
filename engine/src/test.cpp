#include "test.h"
#include <cstdio>
#include <cstdlib>

struct Time {
  f32 start;
  f32 end;
  f32 elapsed;
  Time() {
    start = os_now_seconds();
  }
  ~Time() {
    end = os_now_seconds();
    elapsed = end - start;
    Info("time took: %f", elapsed);
  }
};

void run_free_list_tests() {
  Scratch scratch;
  FreeList fl = free_list_create(scratch, KB(128));
  
  printf("---- Basic allocation and free ----\n");

  void* a = free_list_alloc(fl, 16, 8);
  void* b = free_list_alloc(fl, 32, 8);
  void* c = free_list_alloc(fl, 48, 8);

  Assert(fl.used > 0);

  // Free in reverse
  free_list_free(fl, c);
  free_list_free(fl, b);
  free_list_free(fl, a);

  Assert(fl.used == 0);
  Assert(fl.head == (FreeListNode*)fl.data);
  Assert(fl.head->block_size == fl.size);
  printf("PASS: head reset and memory coalesced\n");

  printf("---- Allocation reuse ----\n");

  void* d = free_list_alloc(fl, 64, 16);
  free_list_free(fl, d);

  void* e = free_list_alloc(fl, 64, 16);
  Assert(d == e); // Should reuse the same memory
  free_list_free(fl, e);
  printf("PASS: memory reused correctly\n");

  printf("---- Alignment test ----\n");

  void* aligned = free_list_alloc(fl, 8, 64);
  Assert(((PtrInt)aligned % 64) == 0);
  printf("PASS: alignment respected\n");

  free_list_free(fl, aligned);

  printf("---- Full buffer test ----\n");

  void* big = free_list_alloc(fl, fl.size - sizeof(FreeListAllocationHeader), 8);
  Assert(big != null);

  free_list_free(fl, big);
  Assert(fl.head == (FreeListNode*)fl.data);
  Assert(fl.head->block_size == fl.size);
  printf("PASS: entire buffer can be allocated and freed\n");

  printf("---- Coalescing multiple blocks ----\n");

  void* x1 = free_list_alloc(fl, 16, 8);
  void* x2 = free_list_alloc(fl, 16, 8);
  void* x3 = free_list_alloc(fl, 16, 8);

  free_list_free(fl, x2);
  free_list_free(fl, x3);
  free_list_free(fl, x1);

  Assert(fl.head->block_size == fl.size);
  Assert(fl.head == (FreeListNode*)fl.data);
  printf("PASS: coalescing multiple adjacent blocks\n");

  printf("All tests passed \n");
}

void run_advanced_free_list_tests() {
  Scratch scratch;
  FreeList fl = free_list_create(scratch, KB(256));
  printf("======== Advanced Free List Tests ========\n");

  const int max_allocs = 64;
  void* allocs[max_allocs] = {};
  u64 sizes[max_allocs] = {};
  u64 alignments[max_allocs] = {};

  // ---- Random allocation sizes with varied alignments ----
  printf("---- Random allocation patterns ----\n");

  srand(42); // Stable results
  for (int i = 0; i < max_allocs; ++i) {
    u64 size = 8 + (rand() % 64);
    u64 alignment = 1ULL << (3 + (rand() % 5)); // 8, 16, 32, 64, 128
    void* ptr = free_list_alloc(fl, size, alignment);

    if (!ptr) break;

    allocs[i] = ptr;
    sizes[i] = size;
    alignments[i] = alignment;

    Assert(((PtrInt)ptr % alignment) == 0);
  }

  printf("PASS: all allocations aligned and within buffer\n");

  // ---- Free in reverse, ensure full memory is reclaimed ----
  for (int i = max_allocs - 1; i >= 0; --i) {
    if (allocs[i]) {
      free_list_free(fl, allocs[i]);
    }
  }

  Assert(fl.used == 0);
  Assert(fl.head == (FreeListNode*)fl.data);
  Assert(fl.head->block_size == fl.size);
  printf("PASS: memory fully reclaimed after reverse free\n");

  // ---- Fragmentation test: allocate, free every 2nd ----
  printf("---- Fragmentation pattern ----\n");
  for (int i = 0; i < max_allocs; ++i) {
    u64 size = 16;
    allocs[i] = free_list_alloc(fl, size, 8);
  }

  for (int i = 0; i < max_allocs; i += 2) {
    if (allocs[i]) free_list_free(fl, allocs[i]);
  }

  u64 used_before = fl.used;

  // Allocate something small enough to fit into one of the freed gaps
  void* filler = free_list_alloc(fl, 16, 8);
  Assert(filler != null);
  printf("PASS: fragmentation reclaimed by new alloc\n");

  free_list_free(fl, filler);

  for (int i = 1; i < max_allocs; i += 2) {
    if (allocs[i]) free_list_free(fl, allocs[i]);
  }

  Assert(fl.used == 0);
  Assert(fl.head == (FreeListNode*)fl.data);
  Assert(fl.head->block_size == fl.size);
  printf("PASS: fragmented memory fully coalesced\n");

  // ---- Allocate and free entire buffer multiple times ----
  printf("---- Reuse full buffer multiple times ----\n");
  for (int i = 0; i < 10; ++i) {
    void* p = free_list_alloc(fl, fl.size - sizeof(FreeListAllocationHeader), 8);
    Assert(p != null);
    free_list_free(fl, p);
    Assert(fl.used == 0);
  }

  printf("PASS: multiple full reuse cycles\n");

  printf("======== All advanced tests passed ✅ ========\n");
}

void test() {
  // run_free_list_tests();
  // run_advanced_free_list_tests();
}
