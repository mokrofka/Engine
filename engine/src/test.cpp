#include "test.h"
#include <stdlib.h>

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

void foo(void* p);

void test_allocator() {
  u32 count = KB(1);
  u32 count1 = 200;
  u32 size = MB(100);
  if (1) {
    {
      Time time;
      Loop (j, count1) {
        Loop (i, count) {
          u32 temp = rand_u32() % MB(size);
          void* mem = malloc(temp);
          foo(mem);
          free(mem);
        }
      }
    }
  } else {
    {
      Time time;
      Loop (j, count1) {
        Loop (i, count) {
          u32 temp = rand_u32() % MB(size);    
          void* mem = mem_alloc(temp);
          foo(mem);
          mem_free(mem);
        }
      }
    }
  }
}

#include <vulkan/vulkan.h>
void test() {

}
