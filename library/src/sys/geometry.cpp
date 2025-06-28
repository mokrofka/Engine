#include "geometry.h"

#include "render/r_frontend.h"

struct GeometrySysState {
  Arena* arena;
  u32 geom_count;
  HashMap hashmap;
};

GeometrySysState st;

#define MaxGeometryCount 256
void geometry_init() {
  st.arena = mem_arena_alloc(KB(1));
  st.hashmap = hashmap_create(st.arena, sizeof(u32), MaxGeometryCount);
  u32 invalid_id = INVALID_ID;
  hashmap_fill(st.hashmap, &invalid_id);
}

u32 geometry_create(Geometry& geometry) {
  hashmap_set(st.hashmap, geometry.name, &st.geom_count);
  vk_r_geometry_create(geometry);
  return st.geom_count++;
}

u32 geometry_get(String name) {
  u32 id;
  hashmap_get(st.hashmap, name, &id);
  Assert(id != INVALID_ID);
  return id;
}
