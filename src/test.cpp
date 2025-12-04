#include "test.h"

void test_mem_pool() {
  Info("=== Running mem pool tests ===");

  Scratch scratch;
  struct A { u32 x, y, z; };

  MemPool p = mem_pool_create(scratch, sizeof(A), alignof(A));

  // Allocate several chunks
  A* a0; Assign(a0, mem_pool_alloc(p));
  A* a1; Assign(a1, mem_pool_alloc(p));
  A* a2; Assign(a2, mem_pool_alloc(p));

  test_true(a0 != nullptr);
  test_true(a1 != nullptr);
  test_true(a2 != nullptr);

  // Check distinctness
  test_true(a0 != a1);
  test_true(a1 != a2);
  test_true(a0 != a2);

  // Check that count updated
  test_equal(p.count, 3);

  // Free one and ensure it’s reused
  mem_pool_free(p, a1);
  test_equal(p.count, 2);

  A* reused; Assign(reused, mem_pool_alloc(p));
  test_equal(p.count, 3);
  test_equal((u8*)reused, (u8*)a1);  // Should reuse the freed slot

  // Force pool grow
  u64 old_cap = p.cap;
  for (u64 i = p.count; i < old_cap + 1; ++i) {
    (void)mem_pool_alloc(p);
  }
  test_true(p.cap > old_cap);

  // Free all
  mem_pool_free_all(p);
  test_equal(p.count, 0);
  test_true(p.head != nullptr);

  Info("=== Test: mem pool works ===");
}


#include <vector>
#include <random>
#include <algorithm>
#include <cstdio>

void test_general_allocator() {
  Scratch scratch;
  AllocSegList allocator(scratch);

  const int ITER = 20000;      // total operations
  const int MAX_ACTIVE = 5000; // number of live allocations

  std::vector<void*> ptrs;
  ptrs.reserve(MAX_ACTIVE);

  std::mt19937 rng(123456); // deterministic
  std::uniform_int_distribution<int> size_dist(1, 5000);
  std::uniform_int_distribution<int> index_dist(0, MAX_ACTIVE - 1);

  for (int i = 0; i < ITER; i++) {

    // 50% alloc, 50% free (if any live pointers exist)
    bool do_alloc = (ptrs.empty()) || (rng() & 1);

    if (do_alloc) {
      // generate random allocation size
      int size = size_dist(rng);
      void* p = seglist_alloc(allocator, size);
      if (!p) {
        printf("ALLOC FAILED at iter %d\n", i);
        return;
      }

      // fill memory with a recognizable pattern
      MemSet(p, (i & 255), size);

      ptrs.push_back(p);
    } else {
      // free a random existing pointer
      int idx = index_dist(rng) % ptrs.size();
      void* p = ptrs[idx];

      seglist_free(allocator, p);

      // remove from vector
      ptrs[idx] = ptrs.back();
      ptrs.pop_back();
    }

    // Occasionally shuffle the vector to make access patterns messy
    if ((i % 2000) == 0)
      std::shuffle(ptrs.begin(), ptrs.end(), rng);
  }

  // free remaining
  for (void* p : ptrs)
    seglist_free(allocator, p);

  Info("=== Test: mem general allocator works ===");
}

void test() {
  test_mem_pool();
  test_general_allocator();
}
