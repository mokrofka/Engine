#include "com.h"

struct Ding {
  f32 x;
};

struct ThingsState {
  Ding dings[KB(1)];
  Thing things[KB(1)];
};

#if InternalState
ThingsState things_state;
#else
extern ThingsState things_state;

void intern intern_things_do() {
  // g_things_st 
}

void things_do() {

}

#endif

struct AnotherState {
  
};

#if InternalState
AnotherState g_another_st;
#else
extern AnotherState g_another_st;

AnotherState* pg_another_st;

void another_st_reload(void* data) { pg_another_st = (AnotherState*)data; }

void another_do() {
  
}

#endif

