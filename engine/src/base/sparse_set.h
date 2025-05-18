#include "lib.h"

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
  u32 size;
  u32 capacity;
  u32 element_size;
  
  inline void insert_data(u32 id) {
    // Put new entry at end and update the maps
    u32 new_index = size;
    entity_to_index[id] = new_index;
    entities[new_index] = id;
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
  u32 size;
  
  inline void add(u32 entity) {
    // Put new entry at end and update the maps
    u32 new_index = size;
    entity_to_index[entity] = new_index;
    entities[new_index] = entity;
    ++size;
  }
  inline void remove(u32 entity) {
    // Copy element at end into deleted element's place to maintain density
    u32 index_of_removed_entity = entity_to_index[entity];
    u32 index_of_last_element = size - 1;

    // Update map to point to moved spot
    u32 entity_of_last_element = entities[index_of_last_element];
    entity_to_index[entity_of_last_element] = index_of_removed_entity;
    entities[index_of_removed_entity] = entity_of_last_element;

    entity_to_index[entity] = INVALID_ID;
    entities[index_of_last_element] = INVALID_ID;

    --size;
  }
};
