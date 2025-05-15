#include "geometry.h"

#include "render/r_frontend.h"

#define MaxGeometries 1024
u32 geom_count;
Geometry geoms[MaxGeometries];

struct GeometrySysState {
  GeometrySysConfig config;
};

GeometrySysState* state;

void geometry_sys_init(Arena* arena, GeometrySysConfig config) {
  state = push_struct(arena, GeometrySysState);
}

u32 geometry_create(Geometry geometry) {
  vk_r_geometry_create(&geometry);
  return geom_count++;
}

void geometry_destroy(u32 id) {

}
