#include "lib.h"

typedef u32 Entity;

#define MaxEntities KB(20)
#define MaxEcsHashes KB(5)
#define MaxComponents 32
#define ComponentSize KB(1)

INLINE constexpr u64 hash_name_at_compile(String name) {
  // A multipler to use when generating a hash. Prime to hopefully avoid collisions
  #define multiplier 97
  
  u8* us = name.str;
  u64 hash = 0;
  
  Loop (i, name.size) {
    hash = hash * multiplier + *us;
    ++us;
  }
  
  // Mod it against the size of the table
  hash %= MaxEcsHashes;
  
  return hash;
}

// struct Column {
//   u8* data;
//   u32 element_size;
//   u32 count;
// };

struct Archetype {
  u32 id;
  u32 type;
  u32 component_count;
  struct Column* components[10];
  u32 add_count;
  u32 remove_count;
  Archetype* archetype_add[10];
  Archetype* archetype_remove[10];
  String component_names[10];
};

struct Record {
  u32 archetype_id;
  u32 row;
};

struct ECS_state {
  // Entity
  u32 entity_count;
  Entity entities[MaxEntities];
  b8 is_entities_alive[MaxEntities];
  String64 entities_name[MaxEntities];

  // Archetype
  Record entities_records[MaxEntities];
  u32 archetype_count;
  Archetype archetypes[10];
  u32 type_to_archetype[10];

  u32 component_index_to_archetype_column[10][10];

  u32 hashed_id_to_component_id[MaxEcsHashes];
  u32 hashed_id_to_component_name[MaxEcsHashes];
  u32 components_size[MaxComponents];

  u32 component_count;
  u32 component_queue_count;
  u32 component_queue_sizes[100];
  u32 component_queue_hash_ids[100];
};

global ECS_state ecs;

struct Column {
  u8* data;
  u32 count;
  u32 element_size;
  u32 index_to_entity[MaxEntities];  
  String component_name;
  
  inline void insert_data(Entity entity, void* component) {
    Assert(ecs.is_entities_alive);

    index_to_entity[count] = entity;
    MemCopy(Offset(data, element_size*count++), component, element_size);
  }
  inline void add(Entity entity) {
    Assert(ecs.is_entities_alive);

    index_to_entity[count] = entity;
    AllocMemZero(Offset(data, element_size*count++), element_size);
  }
  inline void remove_data(Entity entity) {
    Assert(ecs.is_entities_alive[entity]);

    Record record_of_removed = ecs.entities_records[entity];
    Record* record_of_moved = &ecs.entities_records[index_to_entity[record_of_removed.row]];
    record_of_moved->row = record_of_removed.row;

    u32 index_of_last_element = count - 1;
    void* dst = Offset(data, element_size*record_of_removed.row);
    void* src = Offset(data, element_size*index_of_last_element);
    MemCopy(dst, src, element_size);

    Entity entity_of_last_element = index_to_entity[index_of_last_element];
    index_to_entity[record_of_removed.row] = entity_of_last_element;

    --count;
  }
};

// utils

inline b32 type_has_component(u32 type, u32 component_id) {
  return ((Bit(type) & Bit(component_id)) == Bit(component_id));
}

INLINE constexpr u32 _component_get_id(String component_name) {
  u32 hashed_id = hash_name_at_compile(component_name);
  return ecs.hashed_id_to_component_id[hashed_id];
}
#define component_get_id(T) \
  _component_get_id(str_lit(Stringify(T)))

// entity

inline Entity entity_create() {
  Entity id = ecs.entities[ecs.entity_count];
  ecs.entities_records[id].archetype_id = 0;
  ecs.is_entities_alive[id] = true;
  ++ecs.entity_count;
  return id;
}

inline void entity_destroy_id(Entity entity) {
  Assert(ecs.is_entities_alive[entity]);
  ecs.is_entities_alive[entity] = false;
  ecs.entities[--ecs.entity_count] = entity;
}

inline void move_entity(Archetype* archetype, u32 row, Archetype* next_archetype) {

}

inline void* _component_get(Entity entity, u32 component_id) {
  Record* record = &ecs.entities_records[entity];
  Archetype* archetype = &ecs.archetypes[record->archetype_id];

  u32 column = ecs.component_index_to_archetype_column[component_id][archetype->id];

  u32 element_size = archetype->components[column]->element_size;
  return Offset(archetype->components[column]->data, record->row*element_size);
}
#define component_get(entity, T) \
  (T*)_component_get(entity, component_get_id(T))

inline void _component_add(Entity e, String component_name) {
  u32 component_id = _component_get_id(component_name);
  Record* record = &ecs.entities_records[e];
  Archetype* archetype = &ecs.archetypes[record->archetype_id];

  Archetype* next_archetype = archetype->archetype_add[component_id];
  if (next_archetype == null) {
    u32 type = archetype->type | Bit(component_id);

    Archetype* new_archetype = &ecs.archetypes[ecs.archetype_count++];
    new_archetype->type = type;
    while (type) {
      u32 component_id = LowestBit(type);

      new_archetype->components[new_archetype->component_count] = mem_alloc_struct(Column);
      *new_archetype->components[new_archetype->component_count++] = {
        .data = mem_alloc(ComponentSize),
        .element_size = ecs.components_size[component_id],
        .component_name = component_name,
      };

      ClearBit(type, LowestBit(type));
    }
  }

  move_entity(archetype, record->row, next_archetype);
}
#define component_add(entity, T) \
  _component_add(entity, str_lit(Stringify(T)))

// component registration

inline void new_component_enqueue(String component_name, u32 component_size) {
  u32 hashed_id = hash_name_at_compile(component_name);
  ecs.component_queue_hash_ids[ecs.component_queue_count] = hashed_id;
  ecs.hashed_id_to_component_name[hashed_id] = component_name;
  ecs.component_queue_sizes[ecs.component_queue_count] = component_size;
  ++ecs.component_queue_count;
}

inline void new_component_queue_register() {
  Loop (i, ecs.component_queue_count) {
    u32 hashed_id = ecs.component_queue_hash_ids[i];
    u32 component_id = ecs.component_count;
    u32 element_size = ecs.component_queue_sizes[i];

    ecs.hashed_id_to_component_id[hashed_id] = component_id;
    ecs.components_size[ecs.component_count] = element_size;
    ++ecs.component_count;
  }
}

#define Component(T)                                       \
  struct Glue(__, T) {                                     \
    Glue(__, T)() {                                        \
      new_component_enqueue(str_lit(Stringify(T)), sizeof(T)); \
    }                                                      \
  };                                                       \
  static Glue(__, T) Glue(__variable, T);
