#pragma once
#include "base.h"
#include "logger.h"
#include "mem.h"
#include "maths.h"

const u32 DEFAULT_CAPACITY = 8;
const u32 DEFAULT_RESIZE_FACTOR = 2;

template<typename T>
struct Handle {
  u32 handle;
};

////////////////////////////////////////////////////////////////////////
// Generic interface

template <typename T>
struct InterfaceEqual {
  NO_DEBUG b32 operator()(T a, T b) { return equal(a,b); }
};
template <typename T>
struct InterfaceMemEqual {
  NO_DEBUG b32 operator()(T a, T b) { return MemMatchStruct(&a, &b); }
};

enum EqualMode {
  EqualDefault,
  EqualMem
};

////////////////////////////////////////////////////////////////////////
// Pool

template<typename T>
struct ObjectPool {
  u32 head;
  u32 cap;
  Allocator alloc;
  T* data;
  ObjectPool() = default;
  ObjectPool(Allocator alloc_) { *this = {}; alloc = alloc_; }
  void init(Allocator alloc_) { *this = {}; alloc = alloc_; }
  void deinit() { if (data) mem_free(alloc, data); }
  T& get(Handle<T> handle) {
    Assert(handle.handle < cap);
    return data[handle.handle];
  }
  Handle<T> add(T a) {
    if (head >= cap) {
      grow();
    }
    u32 result = head;
    head = *(u32*)&data[head];
    data[result] = a;
    Handle<T> handle = {result};
    return handle;
  }
  Handle<T> add() {
    if (head >= cap) {
      grow();
    }
    u32 result = head;
    head = *(u32*)&data[head];
    Handle<T> handle = {result};
    return handle;
  }
  void remove(Handle<T> handle) {
    *(u32*)&data[handle.handle] = head;
    head = handle.handle;
  }
  void grow() {
    if (data) {
      u32 cap_old = cap;
      cap *= DEFAULT_RESIZE_FACTOR;
      data = mem_realloc_array(alloc, data, cap_old, cap);
      head = cap_old;
      for (i32 i = cap_old; i < cap-1; ++i) {
        *(u32*)&data[i] = i+1;
      }
      *(u32*)&data[cap-1] = U32_MAX;
    } else {
      cap = DEFAULT_CAPACITY;
      data = mem_alloc_array<T>(alloc, cap);
      Loop (i, cap-1) {
        *(u32*)&data[i] = i+1;
      }
      *(u32*)&data[cap-1] = U32_MAX;
    }
  }
};

////////////////////////////////////////////////////////////////////////
// Array

template<typename T, i32 N>
struct Array {
  static constexpr i32 cap = N;
  u32 count;
  T data[N];
  Array() = default;
  Array(InitializerList<T> init) {
    Assert(init.size() <= cap);
    *this = {};
    count = init.size();
    u32 i = 0;
    for (T x : init) {
      data[i++] = x;
    }
  }
  T* begin() { return data; }
  T* end()   { return data + count; }
  NO_DEBUG T& operator[](u32 idx) {
    Assert(idx < cap);
    return data[idx];
  }
  void add(T a) { 
    Assert(count < cap);
    T& e = data[count];
    data[count++] = a;
  }
  void add(InitializerList<T> slice) {
    for (T x : slice) {
      add(x);
    }
  }
  void swap_remove(u32 idx) {
    Assert(idx < count);
    data[idx] = data[--count];
  }
  void clear() {
    count = 0;
  }
  T pop() {
    return data[--count];
  }
  template<typename Eq>
  b32 exists_impl(T a, Eq eq) {
    for (T x : data) {
      if (eq(x, a)) return true;
    }
    return false;
  }
  INLINE b32 exists(T a, bool(*fn)(T a, T b)) { return exists_impl(a, fn); }
  INLINE b32 exists(T a)                      { return exists_impl(a, InterfaceEqual<T>{}); }
  INLINE b32 exists(T a, EqualMode mode)      { return exists_impl(a, InterfaceMemEqual<T>{}); }
};

////////////////////////////////////////////////////////////////////////
// Darray

template <typename T>
struct Darray {
  u32 count;
  u32 cap;
  Allocator alloc;
  T* data;
  Darray() = default;
  Darray(Allocator alloc_) { *this = {}; alloc = alloc_; }
  void init(Allocator alloc_) { *this = {}; alloc = alloc_; }
  void deinit() { if (data) mem_free(alloc, data); }
  T* begin() { return data; }
  T* end()   { return data + count; }
  T& operator[](u32 idx) {
    Assert(idx < cap);
    return data[idx];
  }
  void add(T b) { 
    if (count >= cap) {
      if (data) {
        u32 old_cap = cap;
        cap *= DEFAULT_RESIZE_FACTOR;
        data = mem_realloc_array(alloc, data, old_cap, cap);
      } else {
        cap = DEFAULT_CAPACITY;
        data = mem_alloc_array<T>(alloc, cap);
      }
    }
    T& e = data[count];
    data[count++] = b;
  }
  void add(InitializerList<T> slice) {
    for (T x : slice) {
      add(x);
    }
  }
  void reserve(u32 min_cap) { 
    if (cap >= min_cap) return;
    u32 old_cap = cap;
    u32 new_cap = Max(old_cap*DEFAULT_RESIZE_FACTOR, min_cap);
    if (data) {
      data = mem_realloc_array(alloc, data, old_cap, new_cap);
    } else {
      data = mem_alloc_array<T>(alloc, new_cap);
    }
    cap = new_cap;
  }
  void swap_remove(u32 idx) {
    Assert(idx < count);
    data[idx] = data[--count];
  }
  void clear() { 
    count = 0; 
  }
  T pop() {
    return data[--count];
  }
  template<typename Eq>
  b32 exists_impl(T a, Eq eq) { 
    for (T x : *this) {
      if (eq(x, a)) return true;
    }
    return false;
  }
  b32 exists(T a, bool(*fn)(T a, T b)) { return exists_impl(a, fn); }
  b32 exists(T a)                      { return exists_impl(a, InterfaceEqual<T>{}); }
  b32 exists(T a, EqualMode mode)      { return exists_impl(a, InterfaceMemEqual<T>{}); }
  template<typename Equal>
  b32 exists_at_impl(T e, u32* index, Equal eq) { 
    Loop (i, count) {
      if (eq(data[i], e)) {
        *index = i;
        return true;
      }
    }
    return false;
  }
  b32 exists_at(T a, u32* idx, bool(*fn)(T a, T b)) { return exists_at_impl(a, idx, fn); }
  b32 exists_at(T a, u32* idx)                      { return exists_at_impl(a, idx, InterfaceEqual<T>{}); }
  b32 exists_at(T a, u32* idx, EqualMode mode)      { return exists_at_impl(a, idx, InterfaceMemEqual<T>{}); }
};

////////////////////////////////////////////////////////////////////////
// SparseSet

template <typename T>
struct SparseSet {
  u32 count;
  u32 cap;
  u32 sparse_count;
  Allocator alloc;
  u32* sparse;
  u32* dense;
  T* data;
  SparseSet() = default;
  SparseSet(Allocator alloc_) { *this = {}; alloc = alloc_; }
  void init(Allocator alloc_) { *this = {}; alloc = alloc_; }
  void deinit() { if (data) { mem_free(alloc, sparse); mem_free(alloc, dense); }; }
  T* begin() { return data; }
  T* end()   { return data + count; }
  T& get(u32 handle) {
    Assert(handle < cap);
    DebugDo(Assert(sparse[handle] != INVALID_ID));
    u32 idx = sparse[handle];
    return data[idx];
  }
  void add(u32 handle) {
    if (count >= cap) {
      grow();
    }
    if (handle >= sparse_count) {
      grow_max_index(handle);
    }
    sparse[handle] = count;
    dense[count] = handle;
    ++count;
  }
  void add(u32 handle, T element) {
    if (count >= cap) {
      grow();
    }
    if (handle >= sparse_count) {
      grow_max_index(handle);
    }
    sparse[handle] = count;
    dense[count] = handle;
    data[count] = element;
    ++count;
  }
  void remove(u32 handle) {
    DebugDo(Assert(sparse[handle] != INVALID_ID));
    u32 idx_removed = sparse[handle];
    u32 idx_last = count - 1;
    data[idx_removed] = data[idx_last];
    u32 last_entity = dense[idx_last];
    sparse[last_entity] = idx_removed;
    dense[idx_removed] = last_entity;
    --count;
    DebugDo(sparse[handle] = INVALID_ID);
  }
  void grow() {
    if (data) {
      u32 old_cap = cap;
      cap *= DEFAULT_RESIZE_FACTOR;
      SoA_Field fields[] = {
        SoA_push_field(&dense, u32),
        SoA_push_field(&data, T),
      };
      mem_realloc_soa(alloc, dense, old_cap, cap, fields, ArrayCount(fields));
    }
    else {
      cap = DEFAULT_CAPACITY;
      SoA_Field fields[] = {
        SoA_push_field(&dense, u32),
        SoA_push_field(&data, T),
      };
      mem_alloc_soa(alloc, cap, fields, ArrayCount(fields));
      sparse = mem_alloc_array<u32>(alloc, cap);
    }
  }
  void grow_max_index(u32 handle) {
    u32 modifier = CeilIntDiv(handle+1, sparse_count);
    u32 old_count = sparse_count;
    sparse_count *= modifier;
    sparse = mem_realloc_array(alloc, sparse, old_count, sparse_count);
  }
};

struct SparseSetIndex {
  u32 count;
  u32 cap;
  u32 sparse_count;
  Allocator alloc;
  u32* sparse;
  u32* dense;
  SparseSetIndex() = default;
  SparseSetIndex(Allocator alloc_);
  void init(Allocator alloc_);
  void deinit();
  u32* begin();
  u32* end();
  void add(u32 id);
  void remove(u32 id);
  void grow();
  void grow_max_index(u32 id);
};

////////////////////////////////////////////////////////////////////////
// HandlerArray

template<typename T, i32 N>
struct ArrayHandler {
  static constexpr i32 cap = N;
  u32 count;
  u32 entity_to_index[N];
  u32 entities[N];
  T data[N];
  T* begin() { return data; }
  T* end()   { return data + count; }
  T& get(Handle<T> handle) {
    Assert(handle < cap);
    DebugDo(Assert(entity_to_index[handle] != INVALID_ID));
    u32 index = entity_to_index[handle];
    return data[index];
  }
  Handle<T> add(T e) {
    u32 id = count++;
    entity_to_index[id] = id;
    entities[id] = id;
    data[id] = e;
    return id;
  }
  void remove(Handle<T> handle) {
    DebugDo(Assert(entity_to_index[handle] != INVALID_ID));
    u32 idx_removed = entity_to_index[handle];
    u32 idx_last = count - 1;
    data[idx_removed] = data[idx_last];
    u32 last_entity = entities[idx_last];
    entity_to_index[last_entity] = idx_removed;
    entities[idx_removed] = last_entity;
    --count;
    DebugDo(entity_to_index[handle] = INVALID_ID);
  }
};

template <typename T>
struct DarrayHandler {
  u32 count;
  u32 cap;
  Allocator alloc;
  u32* entity_to_index;
  u32* entities;
  T* data;
  DarrayHandler() = default;
  DarrayHandler(Allocator alloc_) { *this = {}; alloc = alloc_; }
  void init(Allocator alloc_) { *this = {}; alloc = alloc_; }
  void deinit() { if (entity_to_index) mem_free(alloc, entity_to_index); }
  T* begin() { return data; }
  T* end()   { return data + count; }
  T& get(Handle<T> handle) {
    Assert(handle.handle < cap);
    DebugDo(Assert(entity_to_index[handle.handle] != INVALID_ID));
    u32 index = entity_to_index[handle.handle];
    return data[index];
  }
  Handle<T> add(T e) {
    if (count >= cap) {
      grow();
    }
    u32 id = count++;
    entity_to_index[id] = id;
    entities[id] = id;
    data[id] = e;
    Handle<T> handle = {id};
    return handle;
  }
  void remove(Handle<T> handle) {
    DebugDo(Assert(entity_to_index[handle.handle] != INVALID_ID));
    u32 idx_removed = entity_to_index[handle.handle];
    u32 idx_last = count - 1;
    data[idx_removed] = data[idx_last];
    u32 last_entity = entities[idx_last];
    entity_to_index[last_entity] = idx_removed;
    entities[idx_removed] = last_entity;
    --count;
    DebugDo(entity_to_index[handle.handle] = INVALID_ID);
  }
  void grow() {
    if (data) {
      u32 cap_old = cap;
      cap *= DEFAULT_RESIZE_FACTOR;
      SoA_Field fields[] = {
        SoA_push_field(&entity_to_index, u32),
        SoA_push_field(&entities, u32),
        SoA_push_field(&data, T),
      };
      mem_realloc_soa(alloc, entity_to_index, cap_old, cap, fields, ArrayCount(fields));
    }
    else {
      cap = DEFAULT_CAPACITY;
      SoA_Field fields[] = {
        SoA_push_field(&entity_to_index, u32),
        SoA_push_field(&entities, u32),
        SoA_push_field(&data, T),
      };
      mem_alloc_soa(alloc, cap, fields, ArrayCount(fields));
    }
  }
  void clear() {
    count = 0;
  }
};

struct DarrayIndexHandler {
  u32 count;
  u32 cap;
  Allocator alloc;
  u32* entity_to_index;
  u32* entities;
  u32* begin();
  u32* end();
  DarrayIndexHandler() = default;
  DarrayIndexHandler(Allocator alloc_);
  void init(Allocator alloc_);
  void deinit();
  u32 add();
  void remove(u32 id);
  void grow();
};

////////////////////////////////////////////////////////////////////////
// IdPool

// TODO: make array that allocates as idpool

struct IdPool {
  u32 next_idx;
  Darray<u32> array;
  IdPool() = default;
  IdPool(Allocator alloc);
  void init(Allocator alloc);
  void clear();
  u32 alloc();
  void free(u32 id);
};

////////////////////////////////////////////////////////////////////////
// Hashmap

enum MapSlot : u8 {
  MapSlot_Empty,
  MapSlot_Occupied,
  MapSlot_Deleted
};

template<typename Key, typename T>
struct Map {
  static constexpr f32 LF = 0.8;
  u32 count;
  u32 cap;
  Allocator alloc;
  T* data;
  Key* keys;
  MapSlot* is_occupied;
  Map() = default;
  Map(Allocator alloc_) { *this = {}; alloc = alloc_; }
  void init(Allocator alloc_) { *this = {}; alloc = alloc_; }
  void add(Key key, T val) {
    if (count >= cap*LF) { grow(); }
    u64 hash_idx = hash(key);
    u64 index = ModPow2(hash_idx, cap);
    while (is_occupied[index] == MapSlot_Occupied) {
      index = ModPow2(index + 1, cap);
    }
    keys[index] = key;
    data[index] = val;
    is_occupied[index] = MapSlot_Occupied;
    ++count;
  }
  T* get(Key key) {
    if (!data) return null;
    u64 hash_idx = hash(key);
    u64 index = ModPow2(hash_idx, cap);
    u64 start_idx = index;
    do {
      if ((is_occupied[index] == MapSlot_Occupied) && (equal(keys[index], key))) {
        return &data[index];
      } 
      else if (is_occupied[index] == MapSlot_Empty) {
        return null;
      }
    } while (index != start_idx);
    return null;
  }
  void remove(Key key) {
    u64 hash_idx = hash(key);
    u64 index = ModPow2(hash_idx, cap);
    while (is_occupied[index] != MapSlot_Empty) {
      if ((is_occupied[index] == MapSlot_Occupied) && (keys[index] == key)) {
        is_occupied[index] = MapSlot_Deleted;
        --count;
        return;
      }
      index = ModPow2(index + 1, cap);
    }
  }
  void grow() {
    if (data) {
      T* old_data = data;
      Key* old_keys = keys;
      MapSlot* old_is_occupied = is_occupied;
      u32 old_cap = cap;
      cap *= DEFAULT_RESIZE_FACTOR;
      SoA_Field fields[] = {
        SoA_push_field(&data, T),
        SoA_push_field(&keys, Key),
        SoA_push_field(&is_occupied, MapSlot),
      };
      mem_alloc_soa(alloc, cap, fields, ArrayCount(fields));
      Loop (i, old_cap) {
        if (old_is_occupied[i] == MapSlot_Occupied) {
          add(old_keys[i], old_data[i]);
        }
      }
      mem_free(alloc, old_data);
    }
    else {
      cap = DEFAULT_CAPACITY;
      SoA_Field fields[] = {
        SoA_push_field(&data, T),
        SoA_push_field(&keys, Key),
        SoA_push_field(&is_occupied, MapSlot),
      };
      mem_alloc_soa(alloc, cap, fields, ArrayCount(fields));
    }
  }
};

template<typename Key, typename T>
struct MapAuto {
  static constexpr f32 LF = 0.8;
  u32 count = 0;
  u32 cap = 0;
  Allocator alloc = {};
  T* data = null;
  Key* keys = null;
  MapSlot* is_occupied = null;
  void insert(Key key, T val) {
    if (count >= cap*LF) { grow(); }
    u64 hash_idx = hash_memory(&key, sizeof(Key));
    u64 index = ModPow2(hash_idx, cap);
    while (is_occupied[index] == MapSlot_Occupied) {
      index = ModPow2(index + 1, cap);
    }
    keys[index] = key;
    data[index] = val;
    is_occupied[index] = MapSlot_Occupied;
    ++count;
  }
  T& get(Key key) {
    u64 hash_idx = hash_memory(&key, sizeof(Key));
    u64 index = ModPow2(hash_idx, cap);
    while (is_occupied[index] != MapSlot_Empty) {
      if ((is_occupied[index] == MapSlot_Occupied) && (MemMatchStruct(&keys[index], &key))) {
        goto found;
      }
      index = ModPow2(index + 1, cap);
    }
    Assert(false);
    found:
    return data[index];
  }
  void erase(Key key) {
    u64 hash_idx = hash_memory(&key, sizeof(Key));
    u64 index = ModPow2(hash_idx, cap);
    while (is_occupied[index] != MapSlot_Empty) {
      if ((is_occupied[index] == MapSlot_Occupied) && (keys[index] == key)) {
        is_occupied[index] = MapSlot_Deleted;
        --count;
        return;
      }
      index = ModPow2(index + 1, cap);
    }
  }
  void grow() {
    if (data) {
      T* old_data = data;
      Key* old_keys = keys;
      MapSlot* old_is_occupied = is_occupied;
      u32 old_cap = cap;
      cap *= DEFAULT_RESIZE_FACTOR;
      SoA_Field fields[] = {
        SoA_push_field(&data, T),
        SoA_push_field(&keys, Key),
        SoA_push_field(&is_occupied, MapSlot),
      };
      mem_alloc_soa(alloc, cap, fields, ArrayCount(fields));
      Loop (i, old_cap) {
        if (old_is_occupied[i] == MapSlot_Occupied) {
          insert(old_keys[i], old_data[i]);
        }
      }
      mem_free(alloc, old_data);
    }
    else {
      cap = DEFAULT_CAPACITY;
      SoA_Field fields[] = {
        SoA_push_field(&data, T),
        SoA_push_field(&keys, Key),
        SoA_push_field(&is_occupied, MapSlot),
      };
      mem_alloc_soa(alloc, cap, fields, ArrayCount(fields));
    }
  }
};
