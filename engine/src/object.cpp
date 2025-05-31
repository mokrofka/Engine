#include "object.h"
#include "render/r_frontend.h"

// u32 entity_count;
// u32 entities[MaxEntities];
// b8 is_entities_alive[MaxEntities];

// void entity_init() {
//   Loop (i, MaxEntities) {
//     entities[i] = i;
//   }
//   entity_count = 0;
// }

// u32 entity_create() {
//   Assert(entity_count < MaxEntities);
//   is_entities_alive[entity_count] = true;
//   return entities[entity_count++];
// }

// void entity_destroy(u32 entity) {
//   Assert(is_entities_alive[entity]);
//   is_entities_alive[entity] = false;
//   entities[--entity_count] = entity;
// }

void entity_make_renderable(u32 id, u32 mesh_id, u32 shader_id) {
  vk_make_renderable(id, mesh_id, shader_id);
}

void entity_remove_renderable(u32 id) {
  vk_remove_renderable(id);
}
