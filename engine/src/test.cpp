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

// #include "ecs_archetype.h"

// struct Pos {
//   f32 x,y;
// };

// struct Vel {
//   f32 x,y;
// };

// struct Scale {
//   f32 scale;
// };

// // Component(Pos)
// // Component(Vel)
// // Component(Scale)
// // Tag(Light)

// void test_ecs() {
//   Scratch scratch;

//   new_ecs_init();
//   new_component_queue_register();
//   new_tag_queue_register();

//   Entity e = entity_create();
//   component_add(e, Pos);
//   component_add(e, Light);
//   Pos* pos = component_get(e, Pos);
//   pos->x = 1;
//   pos->y = 2;
//   pos = component_get(e, Pos);
//   Info("%f %f", pos->x, pos->y);

//   component_add(e, Vel);
//   Vel* vel = component_get(e, Vel);
//   vel->x = 3;
//   vel->y = 4;
//   vel = component_get(e, Vel);
//   Info("%f %f", vel->x, vel->y);

//   Entity e0 = entity_create();
//   component_add(e0, Pos);
//   Pos* pos0 = component_get(e0, Pos);
//   pos0->x = 10;
//   pos0->y = 11;
//   pos0 = component_get(e0, Pos);
//   Info("%f %f", pos0->x, pos0->y);

//   component_add(e0, Vel);
//   Vel* vel0 = component_get(e0, Vel);
//   vel0->x = 12;
//   vel0->y = 13;
//   vel0 = component_get(e0, Vel);
//   Info("%f %f", vel0->x, vel0->y);

//   component_add(e0, Scale);
//   Scale* s = component_get(e0, Scale);
//   s->scale = 14;
//   s = component_get(e0, Scale);
//   Info("%f", s->scale);

//   // Entity 1
//   Entity e1 = entity_create();
//   component_add(e1, Vel);
//   Vel* vel1 = component_get(e1, Vel);
//   vel1->x = 20;
//   vel1->y = 21;
//   vel1 = component_get(e1, Vel);
//   Info("%f %f", vel1->x, vel1->y);

//   component_add(e1, Pos);
//   Pos* pos1 = component_get(e1, Pos);
//   pos1->x = 22;
//   pos1->y = 23;
//   pos1 = component_get(e1, Pos);
//   Info("%f %f", pos1->x, pos1->y);

//   Loop (i, 2) {
//     Query* query = query_get(Pos Vel);
//     QueryIter it = query_iter(query);
//     while (query_next(it)) {
//       Pos* pos = it_component_get(it, Pos);
//       Vel* vel = it_component_get(it, Vel);
//       Loop (i, it.count) {
//         Info("pos %f %f", pos[i].x, pos[i].y);
//         Info("vel %f %f", vel[i].x, vel[i].y);
//       }
//     }
//     Info("/////////////////////");
//   }

// }

