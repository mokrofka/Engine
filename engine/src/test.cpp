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

struct Pos {
  f32 x,y;
};

struct Vel {
  f32 x,y;
};

struct Scale {
  f32 scale;
};

Component(Pos)
Component(Vel)
Component(Scale)

void test() {
  Scratch scratch;
  new_ecs_init();
  new_component_queue_register();

  Entity e = entity_create();
  component_add(e, Pos);
  Pos* pos = component_get(e, Pos);
  pos->x = 1;
  pos->y = 2;
  Info("%f %f", pos->x, pos->y);

  component_add(e, Vel);
  Vel* vel = component_get(e, Vel);
  vel->x = 10;
  vel->y = 11;
  Info("%f %f", vel->x, vel->y);

  Entity e0 = entity_create();
  component_add(e0, Pos);
  Pos* pos0 = component_get(e0, Pos);
  pos0->x = 1;
  pos0->y = 2;
  Info("%f %f", pos0->x, pos0->y);

  component_add(e0, Vel);
  Vel* vel0 = component_get(e0, Vel);
  vel0->x = 10;
  vel0->y = 11;
  Info("%f %f", vel0->x, vel0->y);

  component_add(e, Scale);
  Scale* s = component_get(e, Scale);
  s->scale = 20;
  Info("%f", s->scale);

  // Entity 1
  Entity e1 = entity_create();
  component_add(e1, Vel);
  Vel* vel1 = component_get(e, Vel);
  vel1->x = 10;
  vel1->y = 11;
  Info("%f %f", vel1->x, vel1->y);

  component_add(e1, Pos);
  Pos* pos1 = component_get(e, Pos);
  pos1->x = 10;
  pos1->y = 11;
  Info("%f %f", pos1->x, pos1->y);

  Query* query = query_get(Pos Vel, Orc | , Died);


}

