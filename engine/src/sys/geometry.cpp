#include "geometry.h"

#include "render/r_frontend.h"

#define MaxGeometries 1024
Geometry geoms[MaxGeometries];

struct GeometrySysState {
  GeometrySysConfig config;
  u32 geom_count;
  HashMap hashmap;
};

GeometrySysState* st;

void geometry_sys_init(Arena* arena, GeometrySysConfig config) {
  st = push_struct(arena, GeometrySysState);
  st->config = config;
  
  st->hashmap = hashmap_create(arena, sizeof(u32), config.max_geometry_count);

}

u32 geometry_create(Geometry geometry) {
  hashmap_set(st->hashmap, geometry.name, &st->geom_count);
  vk_r_geometry_create(&geometry);
  return st->geom_count++;
}

u32 geometry_get(String name) {
  u32 id;
  hashmap_get(st->hashmap, name, &id);
  return id;
}

void geometry_destroy(u32 id) {

}
