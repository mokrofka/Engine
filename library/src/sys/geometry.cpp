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
  st.arena = mem_arena_alloc(KB(100));
  st.hashmap = hashmap_create(st.arena, sizeof(u32), MaxGeometryCount);
  u32 invalid_id = INVALID_ID;
  hashmap_fill(st.hashmap, &invalid_id);

  st.hashmap = hashmap_create(st.arena, sizeof(Geometry), MaxGeometryCount);
  Geometry geom = {
    .id = INVALID_ID,
  };
  hashmap_fill(st.hashmap, &geom);
}

void geometry_create(Geometry geometry) {
  geometry.id = st.geom_count++;
  hashmap_set(st.hashmap, geometry.name, &geometry);
  vk_r_geometry_create(geometry);
}

Geometry& geometry_get(String name) {
  Geometry* geom; Assign(geom, hashmap_get(st.hashmap, name));
  Assert(geom->id != INVALID_ID);
  return *geom;
}
