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

#include "ecs_archetype.h"

struct Position {
  f32 x,y;
};

struct Velocity {
  f32 x,y;
};

Component(Position)
Component(Velocity)

void test() {
  Scratch scratch;
  new_component_queue_register();

  Entity e = entity_create();
  component_add(e, Position);
  component_add(e, Velocity);
  Position* pos = component_get(e, Position);
  pos->x = 10;
  u8* buff = mem_alloc(1);
  mem_free(buff);

  Position* pos1 = component_get(e, Position);
  Info("%f", pos1->x);
}

