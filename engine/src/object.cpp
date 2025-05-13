#include "object.h"
#include "render/r_frontend.h"

#define MaxEntities 1024
u32 entity_count;
u32 entities[MaxEntities];
b8 is_entities_alive[MaxEntities];

void ecs_init() {
  Loop (i, MaxEntities) {
    entities[i] = i;
  }
  entity_count = 0;
}

u32 entity_create() {
  Assert(entity_count < MaxEntities);
  is_entities_alive[entity_count] = true;
  return entities[entity_count++];
}

void entity_destroy(u32 entity) {
  Assert(is_entities_alive[entity]);
  is_entities_alive[entity] = false;
  entities[--entity_count] = entity;
}

void object_sys_init() {
  ecs_init();
}

Object object_create(u32 mesh_id, u32 shader_id) {
  Object obj = {};
  obj.id = entity_create();
  obj.mesh_id = mesh_id;
  obj.shader_id = shader_id;
  return obj;
}

void object_make_renderable(Object obj) {
  r_make_renderable(obj.id, obj.mesh_id, obj.shader_id);
}
