#include "geometry_sys.h"

#include "render/r_frontend.h"

struct GeometrySysState {
  GeometrySysConfig config;
  // SparseSet sparse_set;
};

GeometrySysState* state;

void geometry_sys_init(Arena* arena, GeometrySysConfig config) {
  state = push_struct(arena, GeometrySysState);
  state->config = config;
  
  // state->sparse_set.data = push_array(arena, Geometry, config.max_geometry_count);
  // state->sparse_set.entity_to_index = push_array(arena, u32, config.max_geometry_count);
  // state->sparse_set.index_to_entity = push_array(arena, u32, config.max_geometry_count);
  // state->sparse_set.size = 0;
  // state->sparse_set.capacity = config.max_geometry_count;
  // state->sparse_set.element_size = sizeof(Geometry);
}

Geometry* geometry_create(GeometryConfig config) {
  Geometry* geom = mem_alloc_struct(Geometry);
  local u32 id = 0;
  geom->id = id++;
  geom->name = config.name;
  geom->vertex_size = config.vertex_size;
  geom->vertex_count = config.vertex_count;
  geom->vertices = config.vertices;
  
  geom->index_size = config.index_size;
  geom->index_count = config.index_count;
  geom->indices = config.indices;
  
  r_create_geometry(geom);
  return geom;
}
