#pragma once
#include "lib.h"

typedef u32 Entity;
typedef u32 ComponentType;
typedef u32 Signature;

#define MaxEntities 1024
#define MaxComponents 32
#define MaxSystems 32

struct ECS_state {
  // Entity
  u32 entity_count;
  Entity entities[MaxEntities];
  Signature signatures[MaxEntities];
  
  // ComponentManager
  u32 component_count;
  ComponentType next_component_type;
  u32 next_system_type;
  struct ComponentArray* component_arrays[MaxComponents];
  
  // SystemManager
  u32 system_count;
  u32 entity_system_masks[MaxEntities];
  Signature system_signatures[MaxSystems];
  struct System* systems[MaxSystems];
};

// TODO put into .cpp
ECS_state ecs_state;

//////////////////////////////////////////////////////
// Entity

void ecs_init() {
  Loop (i, MaxEntities) {
    ecs_state.entities[i] = i;
  }
  ecs_state.entity_count = 0;
}

inline Entity entity_create() {
  Assert(ecs_state.entity_count < MaxEntities);
  return ecs_state.entities[ecs_state.entity_count++];
}

inline void _entity_destroy(Entity entity) {
  ecs_state.entities[--ecs_state.entity_count] = entity;
}

inline void entity_set_signature(Entity entity, Signature signature) {
  Assert(entity < MaxEntities && "Entity out of range.");

  // Put this entity's signature into the array
  ecs_state.signatures[entity] = signature;
}

inline Signature entity_get_signature(Entity entity) {
  Assert(entity < MaxEntities && "Entity out of range.");

  // Get this entity's signature from the array
  return ecs_state.signatures[entity];
}

//////////////////////////////////////////////////////
// ComponentArray

template <typename T>
ComponentType get_component_type_ID() {
  local ComponentType type_ID = ecs_state.next_component_type++;
  return type_ID;
}
template <typename T>
ComponentType get_system_type_ID() {
  local u32 system_ID = ecs_state.next_system_type++;
  return system_ID;
}

struct ComponentArray {
  void* component_array;
  u32 entity_to_index[MaxEntities];  
  u32 index_to_entity[MaxEntities];  
  u32 size;
  u32 element_size;
  
  inline void insert_data(Entity entity, void* component) {
    // Put new entry at end and update the maps
    u32 new_index = size;
    entity_to_index[entity] = new_index;
    index_to_entity[new_index] = entity;
    void* dst = (u8*)(component_array) + element_size * new_index;
    MemCopy(dst, component, element_size);
    ++size;
  }
  inline void remove_data(Entity entity) {
    // Copy element at end into deleted element's place to maintain density
    u32 index_of_removed_entity = entity_to_index[entity];
    u32 index_of_last_element = size - 1;
    void* dst_of_removed_entity = (u8*)(component_array) + element_size * index_of_removed_entity;
    void* src_of_last_element = (u8*)(component_array) + element_size * index_of_last_element;
    MemCopy(dst_of_removed_entity, src_of_last_element, element_size);

    // Update map to point to moved spot
    Entity entity_of_last_element = index_to_entity[index_of_last_element];
    entity_to_index[entity_of_last_element] = index_of_removed_entity;
    index_to_entity[index_of_removed_entity] = entity_of_last_element;

    entity_to_index[entity] = INVALID_ID;
    index_to_entity[index_of_last_element] = INVALID_ID;

    --size;
  }
  inline void* get_data(Entity entity) {
    return (u8*)component_array + entity_to_index[entity] * element_size;
  }
};

//////////////////////////////////////////////////////
// ComponentManager 
inline void _component_register(u32 index, u32 element_size) {
  Assert(!ecs_state.component_arrays[index]);
  ecs_state.component_arrays[index] = mem_alloc_struct(ComponentArray);
  ComponentArray* array = ecs_state.component_arrays[index];
  array->size = 0;
  array->element_size = element_size;
  array->component_array = mem_alloc(MaxEntities * element_size);
}

inline ComponentArray* get_component_array(u32 index) {
  return ecs_state.component_arrays[index];
}

inline void _component_add(Entity entity, u32 index, void* component) {
  ecs_state.component_arrays[index]->insert_data(entity, component);
}

inline void _component_remove(Entity entity, u32 index) {
  ecs_state.component_arrays[index]->remove_data(entity);
}

inline void* _component_get(Entity entity, u32 index) {
  return ecs_state.component_arrays[index]->get_data(entity);
}

inline void components_entity_destroy(Entity entity) {
  Signature mask = entity_get_signature(entity);
  while (mask) {
    u32 index = __builtin_ctz(mask); // index of lowest set bit
    ClearBit(mask, index);
    
    ComponentArray* component = ecs_state.component_arrays[index];
    component->remove_data(entity);
  }
}

//////////////////////////////////////////////////////
// System
struct System {
  // u32 system_mask[MASK_SIZE] = {0};
  u32 entity_to_index[MaxEntities];  
  Entity entities[MaxEntities];  
  u32 size;
  inline void entity_add(Entity entity) {
    u32 index = size++;
    entity_to_index[entity] = index;
    entities[index] = entity;
  }
  inline void entity_remove(Entity entity) {
    u32 index = entity_to_index[entity];
    if (index == INVALID_ID)
      return;

    u32 last_index = size - 1;
    Entity last_entity = entities[last_index];

    entities[index] = last_entity;
    entity_to_index[last_entity] = index;

    entity_to_index[entity] = INVALID_ID;
    --size;
  }
};

inline void* system_manager_register_system(u32 index, Signature signature) {
  ecs_state.systems[index] = mem_alloc_struct(System);
  ecs_state.system_signatures[index] = signature;
  ecs_state.systems[index]->size = 0;
  Loop (i, MaxEntities) { ecs_state.systems[index]->entity_to_index[i] = INVALID_ID; }
  ++ecs_state.system_count;
  return ecs_state.systems[index];
}

inline void system_manager_set_signature(Signature signature, u32 index) {
  ecs_state.system_signatures[index] = signature;
}

#define CountBitsSet(u32_int) __builtin_popcount(u32_int)

inline void system_manager_entity_destroyed(Entity entity) {
  LoopC (i, ecs_state.system_count) {
    System* system = ecs_state.systems[i];
    system->entity_remove(entity);
  }
}

inline void entity_signature_changed(Entity entity, Signature entity_signature) {
  LoopC (i, ecs_state.system_count) {
    Signature system_signature = ecs_state.system_signatures[i];
    System* system = ecs_state.systems[i];

    if ((entity_signature & system_signature) == system_signature) {
      system->entity_add(entity);
    } else {
      system->entity_remove(entity);
    }
  }
}

////////////////////////////////////////////////
// Interface
#define component_ID(T) \
  get_component_type_ID<T>()
  
inline void entity_destroy(Entity entity) {
  _entity_destroy(entity);
  components_entity_destroy(entity);
  system_manager_entity_destroyed(entity);
}

#define component_register(T) \
  _component_register(get_component_type_ID<T>(), sizeof(T));

#define component_add(entity, T, component)                    \
  {                                                            \
    auto temp = component;                                     \
    _component_add(entity, get_component_type_ID<T>(), &temp); \
    Signature signature = entity_get_signature(entity);        \
    SetBit(signature, component_ID(T));                        \
    entity_set_signature(entity, signature);                   \
    entity_signature_changed(entity, signature);               \
  }

#define component_remove(entity, T)                        \
  {                                                        \
    _component_remove(entity, get_component_type_ID<T>()); \
    Signature signature = entity_get_signature(entity);    \
    ClearBit(signature, component_ID(T));                  \
    entity_set_signature(entity, signature);               \
    entity_signature_changed(entity, signature);           \
  }

#define get_component(entity, T) \
  (T*)_component_get(entity, get_component_type_ID<T>())

#define system_register(T, signature) \
  (T*)system_manager_register_system(get_system_type_ID<T>(), signature)

#define set_system_signature(T, signature) \
  system_manager_set_signature(signature, get_component_type_ID<T>())

struct Gravity {
	f32 f;
};

struct Transform {
	v3 position;
};

struct Some {
  i32 a;
};

struct PhysicsSystem : System {
	void update() {
    LoopC (i, size) {
      Entity entity = entities[i];
      Transform* transform = get_component(entity, Transform);
      Gravity* gravity = get_component(entity, Gravity);
      
      transform->position.y += gravity->f;
    }
  }
};

void test() {
  component_register(Gravity);
  component_register(Transform);

  Signature signature = {};
  signature |= Bit(component_ID(Gravity)) | Bit(component_ID(Transform));
  PhysicsSystem* physics = system_register(PhysicsSystem, signature);
  
  Entity entities[MaxEntities];
  
  // Loop (i, ArrayCount(entities)) {
  //   entities[i] = entity_create();
  //   component_add(entities[i], Gravity, 0.5f);
  //   component_add(entities[i], Transform, v3(1,2,3));
  // }
  
  struct DoWork : System {
    void update() {
      Loop (i, size) {
        Entity e = entities[i];
        Transform* t = get_component(e, Transform);
        t->position *= 2;
      }
    }
  };
  
  Signature do_work_dependency = Bit(component_ID(Transform));
  DoWork* do_work = system_register(DoWork, do_work_dependency);
  Entity e = entity_create();
  component_add(e, Transform, v3(1,2,3));
  do_work->update();
  component_remove(e, Transform);
  do_work->update();
  
  physics->update();
  
  Loop (i, ArrayCount(entities)) {
    entity_destroy(entities[i]);
  }
}
