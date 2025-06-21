#include "geometry.h"

#include "render/r_frontend.h"

struct GeometrySysState {
  u32 geom_count;
  HashMap hashmap;
};

GeometrySysState st;

#define MaxGeometryCount 1024
void geometry_init(Arena* arena) {
  st.hashmap = hashmap_create(arena, sizeof(u32), MaxGeometryCount);
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
