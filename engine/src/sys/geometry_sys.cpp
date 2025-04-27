#include "geometry_sys.h"

#include "render/r_frontend.h"
#include "sys/material_sys.h"

struct GeometryRef {
  u64 reference_count;
  Geometry geometry;
  b8 auto_release;
};

struct GeometrySysState {
  Arena* arena;
  GeometrySysConfig config;
  Geometry default_geometry;
  GeometryRef* registered_geometries;
};

global GeometrySysState* state;

void create_default_geometry();
void create_geometry(GeometryConfig config, Geometry* g);
void destroy_geometry(Geometry* g);

void geometry_sys_init(Arena* arena, GeometrySysConfig config) {
  if (config.max_geometry_count == 0) {
    Error("geometry_system_initialize - config.max_geometry_count must be > 0"_);
  }
  
  state = push_struct(arena, GeometrySysState);
  u64 mem_reserved = MB(1);
  state->arena = arena_alloc(arena, mem_reserved);
  state->registered_geometries = push_array(arena, GeometryRef, config.max_geometry_count);
  state->config = config;

  // Invalidate all geometries in the array.
  u32 count = state->config.max_geometry_count;
  Loop (i, count) {
    state->registered_geometries[i].geometry.id = INVALID_ID;
    state->registered_geometries[i].geometry.internal_id = INVALID_ID;
    state->registered_geometries[i].geometry.generation = INVALID_ID;
  }

  create_default_geometry();
}

void geometry_sys_shutdown() {
  // NOTE: nothing to do here.
}

Geometry* geometry_system_acquire_by_id(u32 id) {
  if (id != INVALID_ID && state->registered_geometries[id].geometry.id != INVALID_ID) {
    state->registered_geometries[id].reference_count++;
    return &state->registered_geometries[id].geometry;
  }

  // NOTE: Should return default geometry instead?
  Error("geometry_system_acquire_by_id cannot load invalid geometry id. Returning nullptr"_);
  return 0;
}

Geometry* geometry_sys_acquire_from_config(GeometryConfig config, b8 auto_release) {
  Geometry* g = 0;
  Loop (i, state->config.max_geometry_count) {
    if (state->registered_geometries[i].geometry.id == INVALID_ID) {
      // Found empty slot.
      state->registered_geometries[i].auto_release = auto_release;
      state->registered_geometries[i].reference_count = 1;
      g = &state->registered_geometries[i].geometry;
      g->id = i;
      break;
    }
  }

  if (!g) {
    Error("Unable to obtain free slot for geometry. Adjust configuration to allow more space. Returning nullptr"_);
    return 0;
  }

  create_geometry(config, g);

  return g;
}

void geometry_system_release(Geometry* geometry) {
  if (geometry && geometry->id != INVALID_ID) {
    GeometryRef* ref = &state->registered_geometries[geometry->id];

    // Take a copy of the id;
    u32 id = geometry->id;
    if (ref->geometry.id == geometry->id) {
      if (ref->reference_count > 0) {
        --ref->reference_count;
      }

      // Also blanks out the geometry id.
      if (ref->reference_count < 1 && ref->auto_release) {
        destroy_geometry(&ref->geometry);
        ref->reference_count = 0;
        ref->auto_release = false;
      }
    } else {
      Error("Geometry id mismatch. Check registration logic, as this should never occur"_);
    }
    return;
  }

  Warn("geometry_system_acquire_by_id cannot release invalid geometry id. Nothing was done"_);
}

Geometry* geometry_sys_get_default() {
  if (state) {
    return &state->default_geometry;
  }

  Error("geometry_system_get_default called before system was initialized. Returning nullptr"_);
  return 0;
}

void create_geometry(GeometryConfig config, Geometry* g) {
  // Send the geometry off to the renderer to be uploaded to the GPU.
  r_create_geometry(g, config.vertex_count, config.vertices, config.index_count, config.indices);
    // Invalidate the entry.
    // state->registered_geometries[g->id].reference_count = 0;
    // state->registered_geometries[g->id].auto_release = false;
    // g->id = INVALID_ID;
    // g->generation = INVALID_ID;
    // g->internal_id = INVALID_ID;

  // Acquire the material
  if (config.material_name64.size > 0) {
    g->material = material_system_acquire(config.material_name64);
    if (!g->material) {
      g->material = material_sys_get_default();
    }
  }
}

void destroy_geometry(Geometry* g) {
  r_destroy_geometry(g);
  g->internal_id = INVALID_ID;
  g->generation = INVALID_ID;
  g->id = INVALID_ID;
  
  g->name64.size = 0;

  // Release the material.
  if (g->material && g->material->name64.size > 0) {
    material_sys_release(g->material->name64);
    g->material = 0;
  }
}

void create_default_geometry() {
  Vertex3D verts[4] = {};

  f32 f = 10.0f;

  verts[0].position.x = -0.5 * f; // 0    3
  verts[0].position.y = -0.5 * f; //
  verts[0].texcoord.x = 0.0f;     //
  verts[0].texcoord.y = 0.0f;     // 2    1

  verts[1].position.y = 0.5 * f;
  verts[1].position.x = 0.5 * f;
  verts[1].texcoord.x = 1.0f;
  verts[1].texcoord.y = 1.0f;

  verts[2].position.x = -0.5 * f;
  verts[2].position.y = 0.5 * f;
  verts[2].texcoord.x = 0.0f;
  verts[2].texcoord.y = 1.0f;

  verts[3].position.x = 0.5 * f;
  verts[3].position.y = -0.5 * f;
  verts[3].texcoord.x = 1.0f;
  verts[3].texcoord.y = 0.0f;

  u32 indices[6] = {0, 1, 2, 0, 3, 1};

  // Send the geometry off to the renderer to be uploaded to the GPU.
  r_create_geometry(&state->default_geometry, 4, verts, 6, indices);

  // Acquire the default material.
  state->default_geometry.material = material_sys_get_default();
}

GeometryConfig geometry_sys_generate_plane_config(f32 width, f32 height, u32 x_segment_count, u32 y_segment_count, f32 tile_x, f32 tile_y, String name, String material_name) {
  if (width == 0) {
    Warn("Width must be nonzero. Defaulting to one"_);
    width = 1.0f;
  }
  if (height == 0) {
    Warn("Height must be nonzero. Defaulting to one"_);
    height = 1.0f;
  }
  if (x_segment_count < 1) {
    Warn("x_segment_count must be a positive number. Defaulting to one"_);
    x_segment_count = 1;
  }
  if (y_segment_count < 1) {
    Warn("y_segment_count must be a positive number. Defaulting to one"_);
    y_segment_count = 1;
  }

  if (tile_x == 0) {
    Warn("tile_x must be nonzero. Defaulting to one"_);
    tile_x = 1.0f;
  }
  if (tile_y == 0) {
    Warn("tile_y must be nonzero. Defaulting to one"_);
    tile_y = 1.0f;
  }

  GeometryConfig config;
  config.vertex_count = x_segment_count * y_segment_count * 4; // 4 verts per segment
  // config.vertices = kallocate(sizeof(vertex_3d) * config.vertex_count, MEMORY_TAG_ARRAY);
  config.vertices = push_array(state->arena, Vertex3D, config.vertex_count);
  config.index_count = x_segment_count * y_segment_count * 6; // 6 indices per segment
  // config.indices = kallocate(sizeof(u32) * config.index_count, MEMORY_TAG_ARRAY);
  config.indices = push_array(state->arena, u32, config.index_count);

  // TODO: This generates extra vertices, but we can always deduplicate them later.
  f32 seg_width = width / x_segment_count;
  f32 seg_height = height / y_segment_count;
  f32 half_width = width * 0.5f;
  f32 half_height = height * 0.5f;
  Loop (y, y_segment_count) {
    Loop (x, x_segment_count) {
      // Generate vertices
      f32 min_x = (x * seg_width) - half_width;
      f32 min_y = (y * seg_height) - half_height;
      f32 max_x = min_x + seg_width;
      f32 max_y = min_y + seg_height;
      f32 min_uvx = (x / (f32)x_segment_count) * tile_x;
      f32 min_uvy = (y / (f32)y_segment_count) * tile_y;
      f32 max_uvx = ((x + 1) / (f32)x_segment_count) * tile_x;
      f32 max_uvy = ((y + 1) / (f32)y_segment_count) * tile_y;

      u32 v_offset = ((y * x_segment_count) + x) * 4;
      Vertex3D* v0 = &config.vertices[v_offset + 0];
      Vertex3D* v1 = &config.vertices[v_offset + 1];
      Vertex3D* v2 = &config.vertices[v_offset + 2];
      Vertex3D* v3 = &config.vertices[v_offset + 3];

      v0->position.x = min_x;
      v0->position.y = min_y;
      v0->texcoord.x = min_uvx;
      v0->texcoord.y = min_uvy;

      v1->position.x = max_x;
      v1->position.y = max_y;
      v1->texcoord.x = max_uvx;
      v1->texcoord.y = max_uvy;

      v2->position.x = min_x;
      v2->position.y = max_y;
      v2->texcoord.x = min_uvx;
      v2->texcoord.y = max_uvy;

      v3->position.x = max_x;
      v3->position.y = min_y;
      v3->texcoord.x = max_uvx;
      v3->texcoord.y = min_uvy;

      // Generate indices
      u32 i_offset = ((y * x_segment_count) + x) * 6;
      config.indices[i_offset + 0] = v_offset + 0;
      config.indices[i_offset + 1] = v_offset + 1;
      config.indices[i_offset + 2] = v_offset + 2;
      config.indices[i_offset + 3] = v_offset + 0;
      config.indices[i_offset + 4] = v_offset + 3;
      config.indices[i_offset + 5] = v_offset + 1;
    }
  }

  if (name.str && name.size > 0) {
    str_copy(config.name64, name);
  } else {
    str_copy(config.name64, DefaultGeometryName);
  }

  if (material_name.str && material_name.size > 0) {
    str_copy(config.material_name64, material_name);
  } else {
    str_copy(config.material_name64, DefaultMaterialName);
  }

  return config;
}
