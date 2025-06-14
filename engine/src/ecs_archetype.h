#include "lib.h"

typedef u32 Entity;

#define MaxEntities KB(20)
#define MaxEcsHashes KB(5)
#define MaxComponents 32
#define ComponentSize KB(1)

INLINE constexpr u64 hash_name_at_compile(String name) {
  #define multiplier 97
  
  u8* us = name.str;
  u64 hash = 0;
  
  Loop (i, name.size) {
    hash = hash * multiplier + *us;
    ++us;
  }
  
  hash %= MaxEcsHashes;
  
  return hash;
}

struct Archetype {
  u32 id;
  u32 tier;
  u32 type;
  u32 entity_count;
  u32 component_count;
  u32 add_count;
  u32 remove_count;
  struct Column* components[10];
  Archetype* archetype_add[10];
  Archetype* archetype_remove[10];
  String component_names[10];
};

struct Record {
  u32 archetype_id;
  u32 row;
};

struct Query {
  u32 archetype_version;
  u32 archetype_count;
  u32 archetypes_id[10];
  u32 type;
  b8 is_initialized;
};

struct QueryIter {
  u32 archetype_count;
  u32 current;
  u32* archetypes_id;
  u32 count;
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
  u32 archetype_tiers_count[10];
  u32 archetype_tiers[10][100];
  u32 archetype_version;

  u32 component_count;
  u32 components_size[MaxComponents];
  u32 hashed_id_to_component_id[MaxEcsHashes];

  u32 component_queue_count;
  u32 component_queue_sizes[100];
  u32 component_queue_hash_ids[100];

  u32 query_count;
  Query queries[100];
  u32 hashed_id_to_query_id[MaxEcsHashes];
};

global ECS_state ecs;

struct Column {
  u8 data[MaxEntities];
  u32 count;
  u32 element_size;
  u32 index_to_entity[MaxEntities];  
  String component_name;
  
  inline void insert_data(Entity entity, void* component) {
    Assert(ecs.is_entities_alive);

    index_to_entity[count] = entity;
    MemCopy(Offset(data, element_size*count), component, element_size);
    ++count;
  }
  inline void add(Entity entity) {
    Assert(ecs.is_entities_alive);

    index_to_entity[count] = entity;
    AllocMemZero(Offset(data, element_size*count), element_size);
    ++count;
  }
  inline void remove_data(Entity entity) {
    Assert(ecs.is_entities_alive[entity]);

    Record record_of_removed = ecs.entities_records[entity];
    Record* record_of_moved = &ecs.entities_records[index_to_entity[record_of_removed.row]];
    record_of_moved->row = record_of_removed.row;

    u32 index_of_last_element = count - 1;
    u8* dst = Offset(data, element_size*record_of_removed.row);
    u8* src = Offset(data, element_size*index_of_last_element);
    MemCopy(dst, src, element_size);

    Entity entity_of_last_element = index_to_entity[index_of_last_element];
    index_to_entity[record_of_removed.row] = entity_of_last_element;

    --count;
    AllocMemZero(src, element_size);
  }
};

// utils

inline b32 type_has_component(u32 type, u32 component_id) {
  return ((type & Bit(component_id)) == Bit(component_id));
}

inline b32 type_is_subset_of(u32 subset, u32 superset) {
  return (subset & superset) == subset;
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

void new_ecs_init() {
  Loop (i, MaxEntities) {
    ecs.entities[i] = i;
  }
  ecs.archetype_count = 1;
  ecs.query_count = 1;
}

inline void* _component_get(Entity entity, u32 component_id) {
  Record* record = &ecs.entities_records[entity];
  Archetype* archetype = &ecs.archetypes[record->archetype_id];
  Assert(archetype->components[component_id] && "entity doesn't have such component")
  u32 element_size = archetype->components[component_id]->element_size;
  return Offset(archetype->components[component_id]->data, record->row*element_size);
}
#define component_get(entity, T) \
  (T*)_component_get(entity, component_get_id(T))

inline void move_entity(Archetype* old_archetype, Entity e, u32 component_id) {
  Archetype* next_archetype = old_archetype->archetype_add[component_id];

  u32 old_type = old_archetype->type;
  while (old_type) {
    u32 component_id = LowestBit(old_type);
    void* component = _component_get(e, component_id);
    old_archetype->components[component_id]->remove_data(e);
    next_archetype->components[component_id]->insert_data(e, component);

    ClearBit(old_type, LowestBit(old_type));
  }
  --old_archetype->entity_count;
  ++next_archetype->entity_count;

  next_archetype->components[component_id]->add(e);
  Record* new_record = &ecs.entities_records[e];
  new_record->archetype_id = next_archetype->id;
  new_record->row = next_archetype->entity_count - 1;
}

inline void _component_add(Entity e, String component_name) {
  u32 component_id = _component_get_id(component_name);
  Record* record = &ecs.entities_records[e];
  Archetype* archetype = &ecs.archetypes[record->archetype_id];
  Assert(!type_has_component(archetype->type, component_id) && "entity already has component");

  Archetype** next_archetype = &archetype->archetype_add[component_id];
  if (*next_archetype == null) {
    ++ecs.archetype_version;
    u32 type = archetype->type;
    u32 tier = 0;

    // Create archetype with previous components
    Archetype* new_archetype = &ecs.archetypes[ecs.archetype_count];
    while (type) {
      u32 component_id = LowestBit(type);

      new_archetype->component_names[component_id] = archetype->component_names[component_id];
      new_archetype->components[component_id] = mem_alloc_struct(Column);
      *new_archetype->components[component_id] = {
        .element_size = ecs.components_size[component_id],
        .component_name = archetype->component_names[component_id],
      };

      ClearBit(type, LowestBit(type));
      ++new_archetype->component_count;
      ++tier;
    }

    // Add new component
    {
      new_archetype->component_names[component_id] = component_name;
      new_archetype->components[component_id] = mem_alloc_struct(Column);
      *new_archetype->components[component_id] = {
        .element_size = ecs.components_size[component_id],
        .component_name = component_name,
      };
    }

    new_archetype->id = ecs.archetype_count++;
    new_archetype->type = archetype->type | Bit(component_id);
    *next_archetype = new_archetype;
    new_archetype->archetype_remove[component_id] = archetype;
    ++tier;

    new_archetype->tier = tier;
    ecs.archetype_tiers[tier][ecs.archetype_tiers_count[tier]++] = new_archetype->id;

    Loop (i, ecs.archetype_tiers_count[tier + 1]) {
      u32 archetype_id = ecs.archetype_tiers[tier + 1][i];
      Archetype* next_archetype = &ecs.archetypes[archetype_id];
      if (type_is_subset_of(new_archetype->type, next_archetype->type)) {
        u32 type = LowestBit(next_archetype->type & ~new_archetype->type);
        new_archetype->archetype_add[type] = next_archetype;
        next_archetype->archetype_remove[type] = new_archetype;
      }
    }

    Loop (i, ecs.archetype_tiers_count[tier - 1]) {
      u32 archetype_id = ecs.archetype_tiers[tier - 1][i];
      Archetype* prev_archetype = &ecs.archetypes[archetype_id];
      if (type_is_subset_of(prev_archetype->type, new_archetype->type)) {
        u32 type = LowestBit(new_archetype->type & ~prev_archetype->type);
        new_archetype->archetype_remove[type] = prev_archetype;
        prev_archetype->archetype_add[type] = new_archetype;
      }
    }
  }
  
  move_entity(archetype, e, component_id);
}
#define component_add(entity, T) \
  _component_add(entity, str_lit(Stringify(T)))

// component registration

inline void new_component_enqueue(String component_name, u32 component_size) {
  u32 hashed_id = hash_name_at_compile(component_name);
  ecs.component_queue_hash_ids[ecs.component_queue_count] = hashed_id;
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

#define Component(T)                                           \
  struct Glue(__, T) {                                         \
    Glue(__, T)() {                                            \
      new_component_enqueue(str_lit(Stringify(T)), sizeof(T)); \
    }                                                          \
  };                                                           \
  static Glue(__, T) Glue(__variable, T);

enum QuerySection {
  QuerySection_Required,
  QuerySection_Optional,
  QuerySection_Excluded,
  QuerySection_OptionalExcluded,
};

inline Query* _query_get(String text) {
  u32 hash_query_id = hash_name_at_compile(text);
  Query* q = &ecs.queries[ecs.hashed_id_to_query_id[hash_query_id]];
  if (q->archetype_version != ecs.archetype_version) {
    if (!q->is_initialized) {
      ecs.hashed_id_to_query_id[hash_query_id] = ecs.query_count++;
      q = &ecs.queries[ecs.hashed_id_to_query_id[hash_query_id]];
      q->is_initialized = true;
      u32 start = 0;
      QuerySection section = QuerySection_Required;

      Loop (i, text.size + 1) {
        u8 c = text.str[i];

        if (c == ' ' || c == ',' || c == '|' || c == '\0') {
          if (i > start) {
            String word = { &text.str[start], i - start };
            Info("%s", word);

            switch (section) {
              case QuerySection_Required: {
                q->type |= Bit(ecs.hashed_id_to_component_id[hash_name_at_compile(word)]);
              } break;
              case QuerySection_Optional: {

              } break;
              case QuerySection_Excluded: {

              } break;
              case QuerySection_OptionalExcluded: {

              } break;
            }
          }

          switch (c) {
            case ',': {
              if (section == QuerySection_Required) {
                section = QuerySection_Optional;
              } else {
                section = QuerySection_OptionalExcluded;
              }
            } break;
            case '|': {
              section = QuerySection_Excluded;
            } break;
          }

          start = i + 1;
        }
      }
    }

    q->archetype_version = ecs.archetype_version;

    q->archetype_count = 0;
    Loop (i, 10) {
      q->archetypes_id[i] = INVALID_ID;
    }
    for (i32 i = 1; i < ecs.archetype_count; ++i) {
      // if (type_is_subset_of(ecs.archetypes[i].type, q->type)) {
      if (type_is_subset_of(q->type, ecs.archetypes[i].type)) {
        q->archetypes_id[q->archetype_count++] = ecs.archetypes[i].id;
      }
    } 

  }

  return q;
}

inline QueryIter query_iter(Query* query) {
  QueryIter it = {
    .archetype_count = query->archetype_count,
    .current = 0,
    .archetypes_id = query->archetypes_id,
  };
  return it;
}

inline b32 query_next(QueryIter& it) {
  if (it.archetypes_id[it.current] != INVALID_ID) {
    it.count = ecs.archetypes[it.archetypes_id[it.current]].entity_count;
    ++it.current;
    return true;
  }
  return false;
}

#define query_get(...) \
  _query_get(str_lit(#__VA_ARGS__))

inline void* _it_component_get(QueryIter& it, u32 component_id) {
  Archetype* archetype = &ecs.archetypes[it.archetypes_id[it.current-1]];
  u32 element_size = archetype->components[component_id]->element_size;
  return archetype->components[component_id]->data;
}

#define it_component_get(it, T) \
  (T*)_it_component_get(it, component_get_id(T))
