#pragma once
#include "lib.h"

#define MaxEntities 20000

// struct SparseSet {
//   u8* data;
//   u32 entity_to_index[MaxEntities];
//   u32 entities[MaxEntities];
//   u32 count;
//   u32 element_size;
  
//   inline void insert_data(u32 id, void* component) {
//     entity_to_index[id] = count;
//     entities[count] = id;
//     MemCopy(Offset(data, element_size*count), component, element_size);
//     ++count;
//   }
//   inline void add(u32 id) {
//     entity_to_index[id] = count;
//     entities[count] = id;
//     FillAlloc(Offset(data, element_size*count), element_size);
//     ++count;
//   }
//   inline void remove_data(u32 id) {
//     u32 index_of_removed_entity = entity_to_index[id];
//     u32 index_of_last_element = count - 1;

//     u8* dst = Offset(data, element_size*index_of_removed_entity);
//     u8* src = Offset(data, element_size*index_of_last_element);
//     MemCopy(dst, src, element_size);

//     u32 last_entity = entities[index_of_last_element];

//     entity_to_index[last_entity] = index_of_removed_entity;
//     entities[index_of_removed_entity] = last_entity;

//     entity_to_index[id] = INVALID_ID;
//     entities[index_of_last_element] = INVALID_ID;

//     --count;
//   }
//   inline void* get_data(u32 id) {
//     return Offset(data, entity_to_index[id]*element_size);
//   }
// };

struct SparseSetStatic {
  u8 data[KB(1)];
  u32 entity_to_index[MaxEntities];
  u32 entities[MaxEntities];
  u32 count;
  u32 capacity;
  u32 element_size;
  
  inline void insert_data(u32 id, void* component) {
    u32 new_index = count;
    entity_to_index[id] = new_index;
    entities[new_index] = id;
    void* dst = (u8*)(data) + element_size * new_index;
    MemCopy(dst, component, element_size);
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
    void* dst_of_removed_entity = (u8*)(data) + element_size * index_of_removed_entity;
    void* src_of_last_element = (u8*)(data) + element_size * index_of_last_element;
    MemCopy(dst_of_removed_entity, src_of_last_element, element_size);

    // Update map to point to moved spot
    u32 entity_of_last_element = entities[index_of_last_element];
    entity_to_index[entity_of_last_element] = index_of_removed_entity;
    entities[index_of_removed_entity] = entity_of_last_element;

    entity_to_index[id] = INVALID_ID;
    entities[index_of_last_element] = INVALID_ID;

    --count;
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

struct SparseSetE {
  u32 entity_to_index[MaxEntities];  
  u32 entities[MaxEntities];  
  u32 count;
  u32 operator[](u32 i) { return entities[i]; }
  
  inline void add(u32 entity) {
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
