#include "test1.h"
void test_copy(void* a, void* b, u32 size) {
  MemCopy(a, b, sizeof(v3));
}
