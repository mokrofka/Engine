#pragma once
#include "lib.h"

typedef u32 Entity;
typedef u32 ComponentType;
typedef u32 Signature;

#define MaxEntities KB(20)
#define MaxComponents 32
#define MaxSystems 32
#define MaxEcsHashes KB(5)

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

struct ECS_state {
  // Entity
  u32 entity_count;
  Entity entities[MaxEntities];
  b8 is_entities_alive[MaxEntities];
  Signature signatures[MaxEntities];
  Signature tags[MaxEntities];
  String64 entity_names[MaxEntities];
  
  // ComponentManager
  u32 component_count;
  struct ComponentArray* component_arrays[MaxComponents];
  
  // SystemManager
  u32 system_count;
  u32 entity_system_masks[MaxEntities];
  Signature system_signatures[MaxSystems];
  struct BaseSystem* systems[MaxSystems];

  // hashed string to id
  u32 hashed_id_to_component_id[MaxEcsHashes];
  u32 hashed_id_to_system_id[MaxEcsHashes];

  // queue
  u32 component_queue_count;
  u32 component_queue_sizes[100];
  u32 component_queue_hash_ids[100];

  u32 system_queue_count;
  struct {
    u32 component_count;
    u32 components_hash_id[10];
  } system_queue_component[100];
  u32 system_queue_hash_ids[100];

  u32 tag_queue_count;
  u32 tag_queue_hash_ids[100];
};

KAPI extern ECS_state ecs;

//////////////////////////////////////////////////////
// Entity

inline String64 _entity_create_default_name() {
  Scratch scratch;
  String entity_name = push_strf(scratch, "entity_%i", ecs.entity_count);
  String64 result;
  str_copy(result, entity_name);
  return result;
};

// inline Entity entity_create(String64 entity_name = _entity_create_default_name()) {
//   Assert(ecs.entity_count < MaxEntities);

//   Entity id = ecs.entities[ecs.entity_count];
//   ecs.is_entities_alive[id] = true;
//   ecs.entity_names[id] = entity_name;
//   ++ecs.entity_count;
//   return id;
// }

inline void entity_destroy_id(Entity entity) {
  Assert(ecs.is_entities_alive[entity]);

  ecs.is_entities_alive[entity] = false;
  ecs.entities[--ecs.entity_count] = entity;
}

inline void entity_set_signature(Entity entity, Signature signature) {
  Assert(entity < MaxEntities);
  Assert(ecs.is_entities_alive[entity]);

  ecs.signatures[entity] = signature;
}

inline Signature entity_get_signature(Entity entity) {
  Assert(entity < MaxEntities);
  Assert(ecs.is_entities_alive[entity]);

  return ecs.signatures[entity];
}

inline u32 entity_get_system_mask(Entity entity) {
  Assert(entity < MaxEntities);
  Assert(ecs.is_entities_alive[entity]);

  return ecs.entity_system_masks[entity];
}

INLINE constexpr u32 _component_get_id_internal(String component_name) {
  u32 hashed_id = hash_name_at_compile(component_name);
  return ecs.hashed_id_to_component_id[hashed_id];
}
#define component_get_id(T) \
  _component_get_id_internal(str_lit(Stringify(T)))

constexpr u32 _system_get_id_internal(String system_name) {
  u32 hashed_id = hash_name_at_compile(system_name);
  return ecs.hashed_id_to_system_id[hashed_id];
}
#define system_get_id(T) \
  _system_get_id_internal(str_lit(Stringify(T)))

#define entity_has_component_id(entity, T) ((entity_get_signature(entity) & Bit(T)) == Bit(T))
#define entity_has_component(entity, T) ((entity_get_signature(entity) & Bit(component_get_id(T))) == Bit(component_get_id(T)))
//////////////////////////////////////////////////////
// ComponentArray

struct ComponentArray {
  void* component_array;
  u32 entity_to_index[MaxEntities];  
  u32 index_to_entity[MaxEntities];  
  u32 size;
  u32 element_size;
  
  inline void insert_data(Entity entity, void* component) {
    Assert(ecs.is_entities_alive);

    u32 new_index = size;
    entity_to_index[entity] = new_index;
    index_to_entity[new_index] = entity;
    void* dst = (u8*)(component_array) + element_size*new_index;
    MemCopy(dst, component, element_size);
    ++size;
  }
  inline void add(Entity entity) {
    Assert(ecs.is_entities_alive);

    u32 new_index = size;
    entity_to_index[entity] = new_index;
    index_to_entity[new_index] = entity;
    void* dst = (u8*)(component_array) + element_size*new_index;
    MemZero(dst, element_size);
    ++size;
  }
  inline void remove_data(Entity entity) {
    Assert(ecs.is_entities_alive[entity]);

    u32 index_of_removed_entity = entity_to_index[entity];
    u32 index_of_last_element = size - 1;
    void* dst_of_removed_entity = (u8*)(component_array) + element_size*index_of_removed_entity;
    void* src_of_last_element = (u8*)(component_array) + element_size*index_of_last_element;
    MemCopy(dst_of_removed_entity, src_of_last_element, element_size);

    Entity entity_of_last_element = index_to_entity[index_of_last_element];
    entity_to_index[entity_of_last_element] = index_of_removed_entity;
    index_to_entity[index_of_removed_entity] = entity_of_last_element;

#ifdef Debug
    entity_to_index[entity] = INVALID_ID;
    index_to_entity[index_of_last_element] = INVALID_ID;
#endif

    --size;
  }
  inline void* get_data(Entity entity) {
    Assert(ecs.is_entities_alive[entity]);
    return (u8*)component_array + entity_to_index[entity] * element_size;
  }
};

//////////////////////////////////////////////////////
// ComponentManager 

inline void _component_register_internal(String component_name, u32 element_size) {
  u32 hashed_id = hash_name_at_compile(component_name);
  Assert(!ecs.hashed_id_to_component_id[hashed_id]);
  u32 component_id = ecs.component_count;
  ecs.hashed_id_to_component_id[hashed_id] = ecs.component_count++;
  Assert(!ecs.component_arrays[component_id]);

  Assign(ecs.component_arrays[component_id], mem_alloc(sizeof(ComponentArray)));
  ComponentArray* array = ecs.component_arrays[component_id];
  array->size = 0;
  array->element_size = element_size;
  array->component_array = mem_alloc(MaxEntities * element_size);
}
#define component_register(T) \
  _component_register_internal(str_lit(Stringify(T)), sizeof(T));

inline ComponentArray* component_get_array(u32 component_id) {
  Assert(ecs.component_arrays[component_id]);
  return ecs.component_arrays[component_id];
}

inline void _component_add_internal(Entity entity, u32 component_id) {
  Assert(ecs.component_arrays[component_id]);
  Assert(ecs.is_entities_alive[entity]);
  Assert(!entity_has_component_id(entity, component_id));
  ecs.component_arrays[component_id]->add(entity);
}

inline void _component_set_internal(Entity entity, u32 component_id, void* component) {
  Assert(ecs.component_arrays[component_id]);
  Assert(ecs.is_entities_alive[entity]);
  Assert(!entity_has_component_id(entity, component_id));
  ecs.component_arrays[component_id]->insert_data(entity, component);
}

inline void _component_remove_internal(Entity entity, u32 component_id) {
  Assert(ecs.component_arrays[component_id]);
  Assert(ecs.is_entities_alive[entity]);
  Assert(entity_has_component_id(entity, component_id));
  ecs.component_arrays[component_id]->remove_data(entity);
}

inline void* _component_get_internal(Entity entity, u32 index) {
  Assert(ecs.component_arrays[index]);
  Assert(ecs.is_entities_alive[entity]);
  Assert(entity_has_component_id(entity, index));
  return ecs.component_arrays[index]->get_data(entity);
}
#define entity_get_component(entity, T) \
  (T*)_component_get_internal(entity, component_get_id(T))

inline void components_entity_destroy(Entity entity) {
  Assert(ecs.is_entities_alive[entity]);
  Signature mask = entity_get_signature(entity);
  while (mask) {
    u32 index = __builtin_ctz(mask); // index of lowest set bit
    Assert(ecs.component_arrays[index]);
    ClearBit(mask, index);
    
    ComponentArray* component = ecs.component_arrays[index];
    component->remove_data(entity);
  }
  entity_set_signature(entity, 0);
}

//////////////////////////////////////////////////////
// System
inline void* _system_get_internal(u32 system_id) {
  return ecs.systems[system_id];
}

#define system_get(T) \
  (BaseSystem*)_system_get_internal(system_get_id(T))

struct BaseSystem {
  u32 entity_to_index[MaxEntities];  
  Entity entities[MaxEntities];  
  u32 entity_count;
  inline void entity_add(Entity entity) {
    Assert(ecs.is_entities_alive[entity]);
    u32 index = entity_count++;
    entity_to_index[entity] = index;
    entities[index] = entity;
  }
  inline void entity_remove(Entity entity) {
  Assert(ecs.is_entities_alive[entity]);
    u32 index = entity_to_index[entity];
    if (index == INVALID_ID)
      return;

    u32 last_index = entity_count - 1;
    Entity last_entity = entities[last_index];

    entities[index] = last_entity;
    entity_to_index[last_entity] = index;

    entity_to_index[entity] = INVALID_ID;
    --entity_count;
  }
};

inline void* _system_register_internal(String system_name, Signature signature) {
  u32 hashed_id = hash_name_at_compile(system_name);
  Assert(!ecs.hashed_id_to_system_id[hashed_id]);
  u32 system_id = ecs.system_count;
  ecs.hashed_id_to_system_id[system_id] = ecs.system_count++;

  Assign(ecs.systems[system_id], mem_alloc(sizeof(BaseSystem)));
  BaseSystem* system = ecs.systems[system_id];
  ecs.system_signatures[system_id] = signature;
  system->entity_count = 0;
  Loop (i, MaxEntities) { 
    system->entity_to_index[i] = INVALID_ID;
    system->entities[i] = INVALID_ID;
  }
  return system;
}
#define system_register(T, signature) \
  (T*)_system_register_internal(str_lit(Stringify(T)), signature)

inline void _set_system_signature_internal(Signature signature, u32 index) {
  Assert(ecs.systems[index]);
  ecs.system_signatures[index] = signature;
}
#define set_system_signature(T, signature) \
  _set_system_signature_internal(signature, system_get_id(T))

inline void system_entity_destroyed(Entity entity) {
  Assert(ecs.is_entities_alive[entity]);
  u32 mask = entity_get_system_mask(entity);
  while (mask) {
    u32 index = __builtin_ctz(mask); // index of lowest set bit
    Assert(ecs.systems[index]);
    ClearBit(mask, index);
    
    BaseSystem* system = ecs.systems[index];
    system->entity_remove(entity);
  }
}

inline void entity_signature_changed(Entity entity, Signature entity_signature) {
  Loop (i, ecs.system_count) {
    Signature system_signature = ecs.system_signatures[i];
    BaseSystem* system = ecs.systems[i];

    if ((entity_signature & system_signature) == system_signature) {
      ecs.entity_system_masks[entity] |= Bit(i);
      system->entity_add(entity);
    } else {
      ClearBit(ecs.entity_system_masks[entity], Bit(i));
      system->entity_remove(entity);
    }
  }
}

// inline void entity_destroy(Entity entity) {
//   components_entity_destroy(entity);
//   system_entity_destroyed(entity);
//   entity_destroy_id(entity);
// }

inline void __component_add(Entity entity, u32 component_id) {
  _component_add_internal(entity, component_id);
  Signature signature = entity_get_signature(entity);
  SetBit(signature, component_id);
  entity_set_signature(entity, signature);
  entity_signature_changed(entity, signature);
}
inline void __component_set(Entity entity, u32 component_id, void* component) {
  _component_set_internal(entity, component_id, component);
  Signature signature = entity_get_signature(entity);
  SetBit(signature, component_id);
  entity_set_signature(entity, signature);
  entity_signature_changed(entity, signature);
}
inline void __tag_add(Entity entity, u32 component_id) {
  Signature signature = entity_get_signature(entity);
  SetBit(signature, component_id);
  entity_set_signature(entity, signature);
  entity_signature_changed(entity, signature);
}

#define component_add(entity, T)                  \
  {                                               \
    __component_add(entity, component_get_id(T)); \
  }
#define component_set(entity, T, ...)                    \
  {                                                      \
    auto temp = __VA_ARGS__;                             \
    __component_set(entity, component_get_id(T), &temp); \
  }

#define tag_add(entity, T)                  \
  {                                         \
    __tag_add(entity, component_get_id(T)); \
  }

#define component_remove(entity, T)                          \
  {                                                          \
    _component_remove_internal(entity, component_get_id(T)); \
    Signature signature = entity_get_signature(entity);      \
    ClearBit(signature, component_get_id(T));                \
    entity_set_signature(entity, signature);                 \
    entity_signature_changed(entity, signature);             \
  }

inline void component_enqueue(String component_name, u32 component_size) {
  ecs.component_queue_hash_ids[ecs.component_queue_count] = hash_name_at_compile(component_name);
  ecs.component_queue_sizes[ecs.component_queue_count] = component_size;
  ++ecs.component_queue_count;
}

inline void component_queue_register() {
  Loop (i, ecs.component_queue_count) {
    u32 hashed_id = ecs.component_queue_hash_ids[i];
    u32 component_id = ecs.component_count;
    u32 element_size = ecs.component_queue_sizes[i];

    ecs.hashed_id_to_component_id[hashed_id] = ecs.component_count++;

    Assign(ecs.component_arrays[component_id], mem_alloc(sizeof(ComponentArray)));
    ComponentArray* array = ecs.component_arrays[component_id];
    array->size = 0;
    array->element_size = element_size;
    array->component_array = mem_alloc(MaxEntities * element_size);
  }
}

inline void system_enqueue(String system_name, String dependency) {
  Signature system_signature = 0;

  u32 component_count = 0;
  Range range = {};
  Loop (i, dependency.size + 1) {
    if (char_is_space(dependency.str[i]) || dependency.str[i] == '\0') {
      String s = str_substr(dependency, range);
      range.offset += range.size + 1;

      u32 component_id = hash_name_at_compile(s);
      ecs.system_queue_component[ecs.system_queue_count].components_hash_id[component_count++] = component_id;
      ecs.system_queue_component[ecs.system_queue_count].component_count = component_count;
      if (dependency.str[i] == '\0') {
        break;
      }
    }
    ++range.size;
  }

  ecs.system_queue_hash_ids[ecs.system_queue_count] = hash_name_at_compile(system_name);
  ++ecs.system_queue_count;
}

inline void system_queue_register() {
  Loop (i, ecs.system_queue_count) {
    u32 hashed_id = ecs.system_queue_hash_ids[i];
    u32 system_id = ecs.system_count;
    u32 signature = 0;
    Loop (j, ecs.system_queue_component[i].component_count) {
      u32 component_hashed_id = ecs.system_queue_component[i].components_hash_id[j];
      Assert(ecs.hashed_id_to_component_id[component_hashed_id] != INVALID_ID && "component isn't registered");
      signature |= Bit(ecs.hashed_id_to_component_id[component_hashed_id]);
    }

    ecs.hashed_id_to_system_id[hashed_id] = ecs.system_count++;

    Assign(ecs.systems[system_id], mem_alloc(sizeof(BaseSystem)));
    BaseSystem* system = ecs.systems[system_id];

    ecs.system_signatures[system_id] = signature;
    system->entity_count = 0;
    Loop (i, MaxEntities) { 
      system->entity_to_index[i] = INVALID_ID;
      system->entities[i] = INVALID_ID;
    }
  }
}

inline void tag_enqueue(String tag_name) {
  ecs.tag_queue_hash_ids[ecs.component_queue_count] = hash_name_at_compile(tag_name);
  ++ecs.tag_queue_count;
}

inline void tag_queue_register() {
  Loop (i, ecs.tag_queue_count) {
    u32 hashed_id = ecs.tag_queue_hash_ids[i];
    ecs.hashed_id_to_component_id[hashed_id] = ecs.component_count++;
  }
}

inline void ecs_init() {
  Loop (i, MaxEntities) {
    ecs.entities[i] = i;
  }
  ecs.entity_count = 0;
  Loop (i, MaxEcsHashes) {
    ecs.hashed_id_to_component_id[i] = INVALID_ID;
  }
  component_queue_register();
  tag_queue_register();
  system_queue_register();
}

#define Component(T)                                       \
  struct Glue(__, T) {                                     \
    Glue(__, T)() {                                        \
      component_enqueue(str_lit(Stringify(T)), sizeof(T)); \
    }                                                      \
  };                                                       \
  static Glue(__, T) Glue(__variable, T);

#define System(T, ...)                                              \
  struct Glue(__, T) {                                              \
    Glue(__, T)() {                                                 \
      system_enqueue(str_lit(Stringify(T)), str_lit(#__VA_ARGS__)); \
    }                                                               \
  };                                                                \
  static Glue(__, T) Glue(__variable, T);

#define Tag(T)                                       \
  struct Glue(__, T) {                                     \
    Glue(__, T)() {                                        \
      tag_enqueue(str_lit(Stringify(T))); \
    }                                                      \
  };                                                       \
  static Glue(__, T) Glue(__variable, T);



struct SparseSet {
  void* data;
  u32* entity_to_index;
  u32* entities;
  u32 size;
  u32 capacity;
  u32 element_size;
  
  inline void insert_data(u32 id, void* component) {
    // Put new entry at end and update the maps
    u32 new_index = size;
    entity_to_index[id] = new_index;
    entities[new_index] = id;
    void* dst = (u8*)(data) + element_size * new_index;
    MemCopy(dst, component, element_size);
    ++size;
  }
  inline void remove_data(u32 id) {
    // Copy element at end into deleted element's place to maintain density
    u32 index_of_removed_entity = entity_to_index[id];
    u32 index_of_last_element = size - 1;
    void* dst_of_removed_entity = (u8*)(data) + element_size * index_of_removed_entity;
    void* src_of_last_element = (u8*)(data) + element_size * index_of_last_element;
    MemCopy(dst_of_removed_entity, src_of_last_element, element_size);

    // Update map to point to moved spot
    u32 entity_of_last_element = entities[index_of_last_element];
    entity_to_index[entity_of_last_element] = index_of_removed_entity;
    entities[index_of_removed_entity] = entity_of_last_element;

    entity_to_index[id] = INVALID_ID;
    entities[index_of_last_element] = INVALID_ID;

    --size;
  }
  inline void* get_data(u32 id) {
    return (u8*)data + entity_to_index[id] * element_size;
  }
};

struct SparseSetKeep {
  void* data;
  u32 entity_to_index[MaxEntities];
  u32 entities[MaxEntities];
  u32 entity_count;
  u32 capacity;
  u32 element_size;
  
  inline void insert_data(u32 id) {
    // Put new entry at end and update the maps
    u32 new_index = entity_count;
    entity_to_index[id] = new_index;
    entities[new_index] = id;
    ++entity_count;
  }
  inline void remove_data(u32 id) {
    // Copy element at end into deleted element's place to maintain density
    u32 index_of_removed_entity = entity_to_index[id];
    u32 index_of_last_element = entity_count - 1;
    void* dst_of_removed_entity = (u8*)(data) + element_size * index_of_removed_entity;
    void* src_of_last_element = (u8*)(data) + element_size * index_of_last_element;
    MemCopy(dst_of_removed_entity, src_of_last_element, element_size);

    // Update map to point to moved spot
    u32 entity_of_last_element = entities[index_of_last_element];
    entity_to_index[entity_of_last_element] = index_of_removed_entity;
    entities[index_of_removed_entity] = entity_of_last_element;

    entity_to_index[id] = INVALID_ID;
    entities[index_of_last_element] = INVALID_ID;

    --entity_count;
  }
  inline void* get_data(u32 id) {
    return (u8*)data + entity_to_index[id] * element_size;
  }
};

struct SparseSetIndex {
  u32 indexes[MaxEntities];
  u32 entity_to_index[MaxEntities];  
  u32 entities[MaxEntities];  
  u32 size;
  
  inline void insert_data(u32 entity, u32 id) {
    // Put new entry at end and update the maps
    u32 new_index = size;
    entity_to_index[entity] = new_index;
    entities[new_index] = entity;
    indexes[new_index] = id;
    ++size;
  }
  inline void remove_data(u32 entity) {
    // Copy element at end into deleted element's place to maintain density
    u32 index_of_removed_entity = entity_to_index[entity];
    u32 index_of_last_element = size - 1;
    indexes[index_of_removed_entity] = indexes[index_of_last_element];

    // Update map to point to moved spot
    u32 entity_of_last_element = entities[index_of_last_element];
    entity_to_index[entity_of_last_element] = index_of_removed_entity;
    entities[index_of_removed_entity] = entity_of_last_element;

    entity_to_index[entity] = INVALID_ID;
    entities[index_of_last_element] = INVALID_ID;

    --size;
  }
  inline u32 get_data(u32 entity) {
    u32 index = entity_to_index[entity];
    return indexes[index];
  }
};

struct SparseSetEntity {
  u32 entity_to_index[MaxEntities];  
  u32 entities[MaxEntities];  
  u32 count;
  
  inline void add(u32 entity) {
    // Put new entry at end and update the maps
    u32 new_index = count;
    entity_to_index[entity] = new_index;
    entities[new_index] = entity;
    ++count;
  }
  inline void remove(u32 entity) {
    // Copy element at end into deleted element's place to maintain density
    u32 index_of_removed_entity = entity_to_index[entity];
    u32 index_of_last_element = count - 1;

    // Update map to point to moved spot
    u32 entity_of_last_element = entities[index_of_last_element];
    entity_to_index[entity_of_last_element] = index_of_removed_entity;
    entities[index_of_removed_entity] = entity_of_last_element;

    entity_to_index[entity] = INVALID_ID;
    entities[index_of_last_element] = INVALID_ID;

    --count;
  }
};
