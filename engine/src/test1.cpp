#include "lib.h"

void foo(void* mem) {

}

void test_lambda(void (*f)(i32 a), i32 a) {
  f(a);
}
