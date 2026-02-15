#include "containers.h"

////////////////////////////////////////////////////////////////////////
// SparseSet

u32* SparseSetIndex::begin() { return dense; }
u32* SparseSetIndex::end()   { return dense + count; }
SparseSetIndex::SparseSetIndex(Allocator alloc_) { *this = {}; alloc = alloc_; }
void SparseSetIndex::init(Allocator alloc_) { *this = {}; alloc = alloc_; }

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
    dense = mem_alloc_array<u32>(alloc, cap);
    sparse_count = cap;
    sparse = mem_alloc_array<u32>(alloc, sparse_count);
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

u32* DarrayIndexHandler::begin() { return entities; }
u32* DarrayIndexHandler::end()   { return entities + count; }
DarrayIndexHandler::DarrayIndexHandler(Allocator alloc_) { *this = {}; alloc = alloc_; }
void DarrayIndexHandler::init(Allocator alloc_) { *this = {}; alloc = alloc_; }

u32 DarrayIndexHandler::add() {
  if (count >= cap) {
    grow();
  }
  u32 id = count++;
  entity_to_index[id] = id;
  entities[id] = id;
  return id;
}

void DarrayIndexHandler::remove(u32 id) {
  DebugDo(Assert(entity_to_index[id] != INVALID_ID));
  u32 idx_removed = entity_to_index[id];
  u32 idx_last = count - 1;
  u32 last_entity = entities[idx_last];
  entity_to_index[last_entity] = idx_removed;
  entities[idx_removed] = last_entity;
  --count;
  DebugDo(entity_to_index[id] = INVALID_ID);
}

void DarrayIndexHandler::grow() {
  if (entity_to_index) {
    u32 cap_old = cap;
    cap *= DEFAULT_RESIZE_FACTOR;
    SoA_Field fields[] = {
      SoA_push_field(&entity_to_index, u32),
      SoA_push_field(&entities, u32),
    };
    mem_realloc_soa(alloc, entity_to_index, cap_old, cap, fields, ArrayCount(fields));
  }
  else {
    cap = DEFAULT_CAPACITY;
    SoA_Field fields[] = {
      SoA_push_field(&entity_to_index, u32),
      SoA_push_field(&entities, u32),
    };
    mem_alloc_soa(alloc, cap, fields, ArrayCount(fields));
  }
}

////////////////////////////////////////////////////////////////////////
// IdPool

u32 id_pool_alloc(IdPool& p) {
  if (p.array.count > 0) {
    return p.array.pop();
  }
  u32 id = p.next_idx++;
  return id;
}

void id_pool_free(IdPool& p, u32 id) {
  Assert(id < p.next_idx);
  Assert(!p.array.exists(id));
  p.array.add(id);
}


