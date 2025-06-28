#include "entity.h"

struct EntityState {
  u32 entity_count;
  u32 entities[MaxEntities];
  b8 is_entities_alive[MaxEntities];
};

EntityState st;

void entity_init() {
  Loop (i, MaxEntities) {
    st.entities[i] = i;
  }
  st.entity_count = 0;
}

u32 entity_create() {
  u32 entity_id = st.entities[st.entity_count++];
  st.is_entities_alive[entity_id] = true;
  return entity_id;
}

void entity_destroy(u32 entity) {
  b32 is_alive = st.is_entities_alive[entity];
  Assert(st.is_entities_alive[entity]);
  st.is_entities_alive[entity] = false;
  st.entities[--st.entity_count] = entity;
}
