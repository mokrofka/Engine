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

void test() {
  
}
