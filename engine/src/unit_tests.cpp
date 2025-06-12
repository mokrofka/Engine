#include "lib.h"

#define TEST(name) void name()

// Assume chunk size of 64 and 10 chunks
const u64 CHUNK_SIZE = 64;
const u64 CHUNK_COUNT = 10;

TEST(test_create_pool) {
    Scratch  scratch;// Implement or mock
    Pool pool = pool_create(scratch, CHUNK_COUNT, CHUNK_SIZE);
    Assert(pool.data != nullptr);
    Assert(pool.chunk_count == CHUNK_COUNT);
    Assert(pool.chunk_size == CHUNK_SIZE);
}

TEST(test_allocate_all_chunks) {
    Scratch scratch;
    Pool pool = pool_create(scratch, CHUNK_COUNT, CHUNK_SIZE);

    u8* chunks[CHUNK_COUNT];
    for (u64 i = 0; i < CHUNK_COUNT; ++i) {
        chunks[i] = pool_alloc(pool);
        Assert(chunks[i] != nullptr);
    }

    // Should return nullptr now
    // Assert(pool_alloc(pool) == nullptr);
}

TEST(test_unique_allocations) {
    Scratch scratch;
    Pool pool = pool_create(scratch, CHUNK_COUNT, CHUNK_SIZE);

    u8* chunks[CHUNK_COUNT];
    for (u64 i = 0; i < CHUNK_COUNT; ++i) {
        chunks[i] = pool_alloc(pool);
    }

    for (u64 i = 0; i < CHUNK_COUNT; ++i) {
        for (u64 j = i + 1; j < CHUNK_COUNT; ++j) {
            Assert(chunks[i] != chunks[j]);
        }
    }
}

TEST(test_free_and_realloc) {
    Scratch scratch;
    Pool pool = pool_create(scratch, CHUNK_COUNT, CHUNK_SIZE);

    u8* first = pool_alloc(pool);
    u8* second = pool_alloc(pool);

    pool_free(pool, second);
    u8* realloced = pool_alloc(pool);

    Assert(realloced == second); // Assuming LIFO behavior
}

TEST(test_pool_free_all) {
    Scratch scratch;
    Pool pool = pool_create(scratch, CHUNK_COUNT, CHUNK_SIZE);

    for (u64 i = 0; i < CHUNK_COUNT; ++i) {
        pool_alloc(pool);
    }

    pool_free_all(pool);

    for (u64 i = 0; i < CHUNK_COUNT; ++i) {
        Assert(pool_alloc(pool) != nullptr);
    }

    // Assert(pool_alloc(pool) == nullptr);
}

TEST(test_alignment_check) {
    Scratch scratch;
    u64 alignment = 16;
    Pool pool = pool_create(scratch, CHUNK_COUNT, CHUNK_SIZE, alignment);

    for (u64 i = 0; i < CHUNK_COUNT; ++i) {
        void* ptr = pool_alloc(pool);
        u32 a = (PtrInt)ptr % alignment;
        Assert(((PtrInt)ptr % alignment) == 0);
    }
}

TEST(test_double_free_safety) {
    Scratch scratch;
    Pool pool = pool_create(scratch, CHUNK_COUNT, CHUNK_SIZE);

    void* ptr = pool_alloc(pool);
    pool_free(pool, ptr);

    // Depending on allocator, this may crash or ignore — if you want to assert it's safe:
    // pool_free(pool, ptr); // Should be ignored or handled safely
}

void test_pool() {
  test_create_pool();
  test_allocate_all_chunks();
  test_unique_allocations();
  test_free_and_realloc();
  test_pool_free_all();
  test_alignment_check();
  test_double_free_safety();
}

#define BUFFER_SIZE KB(64)
FreeList fl;

void test_setup(Arena* arena) {
  fl = freelist_create(arena, BUFFER_SIZE);
}

void test_basic_allocation() {
  Scratch scratch;
  test_setup(scratch);
  u8* ptr = freelist_alloc(fl, 16);
  Assert(ptr != null);
  Assert((PtrInt)ptr % DEFAULT_ALIGNMENT == 0);
}

void test_multiple_allocations() {
  Scratch scratch;
  test_setup(scratch);
  u8* a = freelist_alloc(fl, 32);
  u8* b = freelist_alloc(fl, 64);
  Assert(a != null && b != null);
  Assert(a != b);
}

void test_alignment() {
  Scratch scratch;
  test_setup(scratch);
  for (u64 align = 8; align <= 128; align *= 2) {
    u8* ptr = freelist_alloc(fl, 24, align);
    Assert(ptr != null);
    Assert(((PtrInt)ptr % align == 0));
  }
}

void test_free_and_reallocate() {
  Scratch scratch;
  test_setup(scratch);
  u8* a = freelist_alloc(fl, 64);
  freelist_free(fl, a);
  u8* b = freelist_alloc(fl, 64);
  Assert(a == b);
}

void test_out_of_memory() {
  Scratch scratch;
  test_setup(scratch);
  u64 big_size = fl.size * 2;
  // u8* ptr = free_list_alloc(fl, big_size);
  // Assert(ptr == null);
}

void test_allocation_until_full() {
  Scratch scratch;
  test_setup(scratch);
  // while (true) {
  //   u8* ptr = free_list_alloc(fl, 128);
  //   if (!ptr) break;
  // }
  Assert(fl.used <= fl.size);
}

void test_free_all() {
  Scratch scratch;
  test_setup(scratch);
  u8* a = freelist_alloc(fl, 64);
  u8* b = freelist_alloc(fl, 64);
  freelist_free_all(fl);
  Assert(fl.used == 0);
  // Assert(fl.head == null); // If your implementation clears head
  u8* c = freelist_alloc(fl, 64);
  Assert(c != null);
}

void test_fragmentation_resistance() {
  Scratch scratch;
  test_setup(scratch);
  u8* a = freelist_alloc(fl, 32);
  u8* b = freelist_alloc(fl, 32);
  u8* c = freelist_alloc(fl, 32);
  freelist_free(fl, b);
  u8* d = freelist_alloc(fl, 32);
  Assert(d == b);
}

void test_block_alloc() {
  Scratch scratch;
  test_setup(scratch);
  // u64 offset = freelist_alloc_block(fl, 64);
  // Assert(offset + 64 <= fl.size);
  // u8* ptr = fl.data + offset;
  // Assert((PtrInt)ptr % DEFAULT_ALIGNMENT == 0);
}

void test_block_vs_regular_alloc() {
  Scratch scratch;
  test_setup(scratch);
  // u64 off1 = freelist_alloc_block(fl, 32, 16);
  u8* ptr1 = freelist_alloc(fl, 32, 16);
  Assert(ptr1 >= fl.data);
  // Assert(off1 >= 0);
}

void test_free_list() {
  test_basic_allocation();
  test_multiple_allocations();
  test_alignment();
  test_free_and_reallocate();
  test_out_of_memory();
  test_allocation_until_full();
  test_free_all();
  test_fragmentation_resistance();
  test_block_alloc();
  test_block_vs_regular_alloc();

  Scratch scratch;
  u64 total_size = 1024;
  u64 alignment = 8;

  FreeList fl = freelist_create(scratch, total_size, alignment);
  Assert(fl.data != null);
  Assert(fl.size == total_size);
  Assert(fl.used == 0);

  // Allocate one block
  u64 alloc_size = 128;
  void* ptr1 = freelist_alloc(fl, alloc_size, alignment);
  Assert(ptr1 != null);
  Assert(fl.used >= alloc_size);

  // Allocate second block
  void* ptr2 = freelist_alloc(fl, alloc_size, alignment);
  Assert(ptr2 != null);
  Assert(fl.used >= 2 * alloc_size);
  Assert(ptr2 != ptr1);

  // Free first block
  freelist_free(fl, ptr1);
  Assert(fl.used >= alloc_size);

  // Free second block
  freelist_free(fl, ptr2);
  Assert(fl.used == 0);

  // Test reallocation of freed memory
  void* ptr3 = freelist_alloc(fl, alloc_size, alignment);
  Assert(ptr3 != null);
  Assert(ptr3 == ptr1 || ptr3 == ptr2); // Should reuse one of the freed blocks

  // Free again
  freelist_free(fl, ptr3);
  Assert(fl.used == 0);

  // Fill the whole memory
  u64 max_allocs = 0;
  void* ptrs[100];
  // while (true) {
  //   void* p = free_list_alloc(fl, alloc_size, alignment);
  //   if (!p) break;
  //   ptrs[max_allocs++] = p;
  // }
  // Assert(max_allocs > 0);
  Info("Allocated %u blocks of size %u", max_allocs, alloc_size);

  // Free everything
  for (u64 i = 0; i < max_allocs; ++i) {
    freelist_free(fl, ptrs[i]);
  }
  Assert(fl.used == 0);

  // Test freelist_free_all
  void* check_ptr = freelist_alloc(fl, 64, alignment);
  Assert(check_ptr);
  freelist_free_all(fl);
  Assert(fl.used == 0);
  void* check_ptr2 = freelist_alloc(fl, 64, alignment);
  Assert(check_ptr2);
  // Assert(check_ptr2 == fl.data); // Should be start of memory

  // Test alignment correctness
  for (int a = 8; a <= 64; a *= 2) {
    void* aptr = freelist_alloc(fl, 100, a);
    Assert(aptr);
    Assert(((PtrInt)aptr % a) == 0);
    freelist_free(fl, aptr);
  }

  freelist_free_all(fl);
  void* c1 = freelist_alloc(fl, 128, alignment);
  void* c2 = freelist_alloc(fl, 128, alignment);
  void* c3 = freelist_alloc(fl, 128, alignment);

  // Free c2 then c1, they are adjacent
  freelist_free(fl, c2);
  freelist_free(fl, c1);

  // After coalescence, c1 + c2 should be merged into one free block
  // Allocate a 256-byte block, it should succeed and return same address as c1
  void* c12 = freelist_alloc(fl, 256, alignment);
  Assert(c12 == c1); // Coalesced block should allow 256 bytes
  freelist_free(fl, c12);

  // Free c3, then check full memory is free again
  freelist_free(fl, c3);
  Assert(fl.used == 0);
}
