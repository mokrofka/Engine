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

IdPool::IdPool(Allocator alloc) { *this = {}; array.init(alloc); }
void IdPool::init(Allocator alloc) { *this = {}; array.init(alloc); }
void IdPool::clear() { next_idx = 0; array.clear(); }

u32 IdPool::alloc() {
  if (array.count > 0) {
    return array.pop();
  }
  u32 id = next_idx++;
  return id;
}

void IdPool::free(u32 id) {
  Assert(id < next_idx);
  Assert(!array.exists(id));
  array.add(id);
}

// TODO: use old implementation?
#define MaxId 100
u32 id_count;
u32 ids[MaxId];

void id_init() {
  Loop (i, MaxId) {
    ids[i] = i;
  }
}

u32 id_make() {
  return ids[id_count++];
}

void id_remove(u32 id) {
  ids[--id_count] = id;
}
