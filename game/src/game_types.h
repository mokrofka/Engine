#include "lib.h"
#include "entity.h"
#include "render/r_types.h"

struct Camera {
  mat4 view;
  mat4 projection;
  b8 view_dirty;
  v3 position;
  v3 direction;
  f32 yaw;
  f32 pitch;
  f32 fov;
};

struct Entity {
  u32 id;
  v3 pos;
  v3 scale;
  v3 rot;
  v3 color;
  PointLight point_light;
  DirLight dir_light;
  SpotLight spot_light;
};

struct SparseSetEntity {
  Entity data[MaxEntities];
  u32 entity_to_index[MaxEntities];
  u32 entities[MaxEntities];
  u32 count;
  u32 capacity;
  
  inline void insert_data(Entity e) {
    u32 new_index = count;
    entity_to_index[e.id] = new_index;
    entities[new_index] = e.id;
    data[new_index] = e;
    ++count;
  }
  inline void add(u32 id) {
    // Put new entry at end and update the maps
    u32 new_index = count;
    entity_to_index[id] = new_index;
    entities[new_index] = id;
    ++count;
  }
  inline void remove_data(u32 id) {
    // Copy element at end into deleted element's place to maintain density
    u32 index_of_removed_entity = entity_to_index[id];
    u32 index_of_last_element = count - 1;
    data[index_of_removed_entity] = data[index_of_last_element];

    // Update map to point to moved spot
    u32 entity_of_last_element = entities[index_of_last_element];
    entity_to_index[entity_of_last_element] = index_of_removed_entity;
    entities[index_of_removed_entity] = entity_of_last_element;

    entity_to_index[id] = INVALID_ID;
    entities[index_of_last_element] = INVALID_ID;

    --count;
  }
  inline Entity& get_data(u32 id) {
    return data[entity_to_index[id]];
  }
};

struct GameState {
  Arena* arena;
  
  Camera camera;
  
  SparseSetEntity cubes;
  SparseSetEntity entities;
  SparseSetEntity point_lights;
  SparseSetEntity dir_lights;
  SparseSetEntity spot_lights;

  struct ShaderGlobalState* shader_global_state;
  u32 grid_id;
  u32 triangled_id;

  b8 is_mouse_move;
};

extern GameState* st;
