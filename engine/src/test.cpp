#include "test.h"

struct Time {
  f32 start;
  f32 end;
  f32 elapsed;
  Time() {
    start = os_now_seconds(); 
  }
  
  ~Time() {
    end = os_now_seconds(); 
    Info("%f", end - start);
  }
};

void test_pool_alloc_and_free() {
  Scratch scratch;
  Pool pool = pool_create(scratch, 4, 16); // 4 chunks, each 16 bytes
  

  void* ptrs[4] = {};
  for (int i = 0; i < 4; ++i) {
    ptrs[i] = pool_alloc(pool);
    Assert(ptrs[i] != nullptr);
    MemSet(ptrs[i], 0xAB, 16); // touch memory
  }

  // Free one and allocate again
  pool_free(pool, ptrs[2]);
  pool_free(pool, ptrs[2]);
  u8* a = pool_alloc(pool);
  u8* b = pool_alloc(pool);
  u8* c = pool_alloc(pool);
  u8* d = pool_alloc(pool);
  u8* e = pool_alloc(pool);
  u8* r = pool_alloc(pool);
  pool_free(pool, ptrs[2]);
  pool_free(pool, ptrs[2]);
  pool_free(pool, ptrs[2]);
  void* reused = pool_alloc(pool);
  Assert(reused == ptrs[2]); // allocator should reuse freed memory

  pool_free_all(pool);
  void* after_reset = pool_alloc(pool);
  Assert(after_reset != nullptr);
}

void test() {
  Scratch scratch;
  test_pool_alloc_and_free();
  Pool pool = pool_create(scratch, 4, 8);
  u8* mem = pool_alloc(pool);
  *mem = 1;
  pool_free(pool, mem);

}
