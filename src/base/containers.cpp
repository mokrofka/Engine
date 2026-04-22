#include "containers.h"

////////////////////////////////////////////////////////////////////////
// SparseSet

SparseSetIndex::SparseSetIndex(Allocator alloc_) { *this = {}; alloc = alloc_; }
void SparseSetIndex::init(Allocator alloc_) { *this = {}; alloc = alloc_; }
void SparseSetIndex::deinit() { if (sparse) { mem_free(alloc, sparse); mem_free(alloc, dense); } }
u32* SparseSetIndex::begin() { return dense; }
u32* SparseSetIndex::end()   { return dense + count; }

void SparseSetIndex::add(u32 id) {
  if (count >= cap) {
    grow();
  }
  if (id >= sparse_count) {
    grow_max_index(id);
  }
  sparse[id] = count;
  dense[count] = id;
  ++count;
}

void SparseSetIndex::remove(u32 id) {
  u32 idx_removed = sparse[id];
  u32 idx_last = count - 1;
  u32 last_entity = dense[idx_last];
  sparse[last_entity] = idx_removed;
  dense[idx_removed] = last_entity;
  --count;
}

void SparseSetIndex::grow() {
  if (dense) {
    u32 old_cap = cap;
    cap *= DEFAULT_RESIZE_FACTOR;
    dense = mem_realloc_array(alloc, dense, old_cap, cap);
  }
  else {
    cap = DEFAULT_CAPACITY;
    dense = push_array(alloc, u32, cap);
    sparse_count = cap;
    sparse = push_array(alloc, u32, sparse_count);
  }
}

void SparseSetIndex::grow_max_index(u32 id) {
  u32 modifier = CeilIntDiv(id+1, sparse_count);
  u32 old_sparse_count = sparse_count;
  sparse_count *= modifier;
  sparse = mem_realloc_array(alloc, sparse, old_sparse_count, sparse_count);
}

////////////////////////////////////////////////////////////////////////
// HandlerArray

DarrayIndexHandler::DarrayIndexHandler(Allocator alloc_) { *this = {}; alloc = alloc_; }
void DarrayIndexHandler::init(Allocator alloc_) { *this = {}; alloc = alloc_; }
void DarrayIndexHandler::deinit() { if (sparse) mem_free(alloc, sparse); }
u32* DarrayIndexHandler::begin() { return dense; }
u32* DarrayIndexHandler::end()   { return dense + count; }

u32 DarrayIndexHandler::add() {
  if (count >= cap) {
    grow();
  }
  u32 id = count++;
  sparse[id] = id;
  dense[id] = id;
  return id;
}

void DarrayIndexHandler::remove(u32 id) {
  DebugDo(Assert(sparse[id] != INVALID_ID));
  u32 idx_removed = sparse[id];
  u32 idx_last = count - 1;
  u32 last_entity = dense[idx_last];
  sparse[last_entity] = idx_removed;
  dense[idx_removed] = last_entity;
  --count;
  DebugDo(sparse[id] = INVALID_ID);
}

void DarrayIndexHandler::grow() {
  if (sparse) {
    u32 cap_old = cap;
    cap *= DEFAULT_RESIZE_FACTOR;
    SoA_Field fields[] = {
      SoA_push_field(&sparse, u32),
      SoA_push_field(&dense, u32),
    };
    mem_realloc_soa(alloc, cap_old, cap, ArraySlice(fields));
  }
  else {
    cap = DEFAULT_CAPACITY;
    SoA_Field fields[] = {
      SoA_push_field(&sparse, u32),
      SoA_push_field(&dense, u32),
    };
    mem_alloc_soa(alloc, cap, ArraySlice(fields));
  }
}

////////////////////////////////////////////////////////////////////////
// IdPool

IdPool::IdPool(Allocator alloc) { *this = {}; allocator = alloc; }
void IdPool::init(Allocator alloc) { *this = {}; allocator = alloc; }
void IdPool::clear() { count = 0; }

u32 IdPool::alloc() {
  if (count+1 >= cap) {
    if (ids) {
      u32 old_cap = cap;
      cap *= DEFAULT_RESIZE_FACTOR;
      ids = mem_realloc_array(allocator, ids, old_cap, cap);
      generations = mem_realloc_array_zero(allocator, generations, old_cap, cap);
      for (u32 i = old_cap; i < cap; ++i) {
        ids[i] = i;
      }
    } else {
      cap = DEFAULT_CAPACITY;
      ids = push_array(allocator, u32, cap);
      generations = push_array_zero(allocator, u32, cap);
      for (u32 i = 0; i < cap; ++i) {
        ids[i] = i;
      }
    }
  }
#if BUILD_DEBUG
  u32 generation = generations[count];
  u32 idx = count++;
  u32 result = (generation << INDEX_BITS) | idx;
  return result;
#else
  return idx[count++];
#endif
}

void IdPool::free(u32 id) {
  u32 idx = id & INDEX_MASK;
  u32 generation = id >> INDEX_BITS;
  Assert(generations[idx]++ == generation);
  ids[--count] = idx;
}

void StaticIdPool::init(Allocator alloc, u32 cap_) {
  *this = {};
  cap = cap_;
  ids = push_array(alloc, u32, cap);
#if BUILD_DEBUG
  generations = push_array_zero(alloc, u32, cap);
#endif
  Loop (i, cap) {
    ids[i] = i;
  }
}

u32 StaticIdPool::alloc() {
  Assert(count+1 <= cap);
#if BUILD_DEBUG
  u32 generation = generations[count];
  u32 idx = count++;
  u32 result = (generation << INDEX_BITS) | idx;
  return result;
#else
  return idx[count++];
#endif
}

void StaticIdPool::free(u32 id) {
  u32 idx = id & INDEX_MASK;
  u32 generation = id >> INDEX_BITS;
  Assert(generations[idx]++ == generation);
  ids[--count] = idx;
}

// struct Entity {
//   v3 pos;
//   f32 health;
//   f32 vel;
//   ModelHandle model;
//   AnimationHandle anim;
//   ...
// };

// #define MaxId 100
// Entity entities[MaxId];
// typedef u32 EntityId;
// EntityId id_count;
// EntityId ids[MaxId];
// void id_init() {
//   Loop (i, MaxId) {
//     ids[i] = i;
//   }
// }
// EntityId id_alloc() {
//   return ids[id_count++];
// }
// void id_free(EntityId id) {
//   ids[--id_count] = id;
// }
// void game_init() {
//   EntityId id = id_alloc();
//   Entity& e = entities[id];
//   ...
// }


