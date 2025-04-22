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
    elapsed = end - start;
    Info("time took: %f", elapsed);
  }
};

void test() {
  
}
