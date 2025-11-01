#include "test.h"

void test_mem_pool() {
  Info("=== Running mem pool tests ===");

  Arena* arena = arena_alloc();
  struct A { u32 x, y, z; };

  MemPool p = mem_pool_create(arena, sizeof(A), alignof(A));

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

  arena_release(arena);

  Info("=== Test: mem pool works ===");
}

void test() {
  test_mem_pool();
}
