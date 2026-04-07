#pragma once
#include "base.h"
#include "logger.h"
#include "mem.h"
#include "maths.h"

const u32 DEFAULT_CAPACITY = 8;
const u32 DEFAULT_RESIZE_FACTOR = 2;

const u32 INDEX_BITS = 22;
const u32 INDEX_MASK = (1u << INDEX_BITS) - 1;

template<typename T>
struct Handle {
  u32 handle;
#if BUILD_DEBUG
  u32 idx() { return handle & INDEX_MASK; }
  u32 generation() { return handle >> INDEX_BITS; }
#else
  u32 idx(Handle<T> handle) { return handle.handle; }
#endif
};

////////////////////////////////////////////////////////////////////////
// C++ crazyness

template <typename T>
struct ContainerEqual {
  NO_DEBUG b32 operator()(T a, T b) { return equal(a,b); }
};
template <typename T>
struct ContainerMemEqual {
  NO_DEBUG b32 operator()(T a, T b) { return MemMatchStruct(&a, &b); }
};

enum EqualMode {
  EqualMode_Mem
};

////////////////////////////////////////////////////////////////////////
// Pool

template<typename T>
struct ObjectPool {
  u32 head;
  u32 cap;
  Allocator alloc;
  T* data;
#if BUILD_DEBUG
  u32* generations;
#endif
  ObjectPool() = default;
  ObjectPool(Allocator alloc_) { *this = {}; alloc = alloc_; }
  void init(Allocator alloc_) { *this = {}; alloc = alloc_; }
  void deinit() { if (data) mem_free(alloc, data); }
  T& get(Handle<T> handle) {
#if BUILD_DEBUG
    u32 idx = handle.handle & INDEX_MASK;
    u32 generation = handle.handle >> INDEX_BITS;
    Assert(idx < cap);
    Assert(generations[idx] == generation);
    return data[idx];
#else
    return data[handle.handle];
#endif
  }
  Handle<T> add() {
#if BUILD_DEBUG
    u32 head_idx = head & INDEX_MASK;
    if (head_idx >= cap) {
      grow();
    }
    u32 result = head;
    u32 idx = result & INDEX_MASK;
    Assert((idx & INDEX_BITS) < cap);
    head = *(u32*)&data[idx];
    Handle<T> handle = {result};
    return handle;
#else
    if (head >= cap) {
      grow();
    }
    u32 result = head;
    head = *(u32*)&data[head];
    Handle<T> handle = {result};
    return handle;
  #endif
  }
  Handle<T> add(T e) {
    Handle<T> handle = add();
    get(handle) = e;
    return handle;
  }
  void remove(Handle<T> handle) {
#if BUILD_DEBUG
    u32 idx = handle.handle & INDEX_MASK;
    u32 generation = handle.handle >> INDEX_BITS;
    Assert(generations[idx] == generation);
    ++generation;
    generations[idx] = generation;
    handle.handle = (generation << INDEX_BITS) | idx;
    *(u32*)&data[idx] = head;
    head = handle.handle;
#else
    *(u32*)&data[idx] = head;
    head = handle.handle;
#endif
  }
  void grow() {
#if BUILD_DEBUG
    if (data) {
      u32 cap_old = cap;
      cap *= DEFAULT_RESIZE_FACTOR;
      SoA_Field fields[] = {
        SoA_push_field(&generations, u32),
        SoA_push_field(&data, T),
      };
      mem_realloc_soa(alloc, generations, cap_old, cap, fields, ArrayCount(fields));
      head = cap_old;
      for (i32 i = cap_old; i < cap-1; ++i) {
        *(u32*)&data[i] = i+1;
      }
      *(u32*)&data[cap-1] = U32_MAX;
      MemZeroArray(generations+cap_old, cap-cap_old);
    } else {
      cap = DEFAULT_CAPACITY;
      SoA_Field fields[] = {
        SoA_push_field(&generations, u32),
        SoA_push_field(&data, T),
      };
      mem_alloc_soa(alloc, cap, fields, ArrayCount(fields));
      Loop (i, cap-1) {
        *(u32*)&data[i] = i+1;
      }
      *(u32*)&data[cap-1] = U32_MAX;
      MemZeroArray(generations, cap);
    }
#else
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
#endif
  }
  void clear() {
#if BUILD_DEBUG
    head = 0;
    Loop (i, cap-1) {
      *(u32*)&data[i] = i+1;
    }
    *(u32*)&data[cap-1] = U32_MAX;
    MemZeroArray(generations, cap);
#else
    head = 0;
    Loop (i, cap-1) {
      *(u32*)&data[i] = i+1;
    }
    *(u32*)&data[cap-1] = U32_MAX;
#endif
  }
};

// not generic
template<typename T, i32 cap>
struct StaticObjectPool {
  u32 head;
  T* data;
#if BUILD_DEBUG
  u32* generations;
#endif
  void init(Allocator arena, u32* generations_) {
    data = push_array(arena, T, cap);
    generations = generations_;
    clear();
  }
  T& get(Handle<T> handle) {
#if BUILD_DEBUG
    u32 idx = handle.handle & INDEX_MASK;
    u32 generation = handle.handle >> INDEX_BITS;
    Assert(idx < cap);
    Assert(generations[idx] == generation);
    return data[idx];
#else
    return data[handle.handle];
#endif
  }
  Handle<T> add() {
#if BUILD_DEBUG
    u32 head_idx = head & INDEX_MASK;
    u32 result = head;
    u32 idx = result & INDEX_MASK;
    Assert((idx & INDEX_BITS) < cap);
    head = *(u32*)&data[idx];
    Handle<T> handle = {result};
    return handle;
#else
    if (head >= cap) {
      grow();
    }
    u32 result = head;
    head = *(u32*)&data[head];
    Handle<T> handle = {result};
    return handle;
  #endif
  }
  Handle<T> add(T e) {
    Handle<T> handle = add();
    get(handle) = e;
    return handle;
  }
  void remove(Handle<T> handle) {
#if BUILD_DEBUG
    u32 idx = handle.handle & INDEX_MASK;
    u32 generation = handle.handle >> INDEX_BITS;
    Assert(generations[idx] == generation);
    ++generation;
    generations[idx] = generation;
    handle.handle = (generation << INDEX_BITS) | idx;
    *(u32*)&data[idx] = head;
    head = handle.handle;
#else
    *(u32*)&data[idx] = head;
    head = handle.handle;
#endif
  }
  void clear() {
#if BUILD_DEBUG
    head = 0;
    Loop (i, cap) {
      *(u32*)&data[i] = i+1;
    }
    MemZeroArray(generations, cap);
#else
    head = 0;
    Loop (i, cap-1) {
      *(u32*)&data[i] = i+1;
    }
#endif
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
  T& operator[](u32 idx) {
    Assert(idx < cap);
    return data[idx];
  }
  void add(T a) { 
    Assert(count < cap);
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
  INLINE b32 exists(T a)                      { return exists_impl(a, ContainerEqual<T>{}); }
  INLINE b32 exists(T a, EqualMode mode)      { return exists_impl(a, ContainerMemEqual<T>{}); }
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
  b32 exists(T a)                      { return exists_impl(a, ContainerEqual<T>{}); }
  b32 exists(T a, EqualMode mode)      { return exists_impl(a, ContainerMemEqual<T>{}); }
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
  b32 exists_at(T a, u32* idx)                      { return exists_at_impl(a, idx, ContainerEqual<T>{}); }
  b32 exists_at(T a, u32* idx, EqualMode mode)      { return exists_at_impl(a, idx, ContainerMemEqual<T>{}); }
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
      u32 cap_old = cap;
      cap *= DEFAULT_RESIZE_FACTOR;
      SoA_Field fields[] = {
        SoA_push_field(&dense, u32),
        SoA_push_field(&data, T),
      };
      mem_realloc_soa(alloc, dense, cap_old, cap, fields, ArrayCount(fields));
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
  u32 sparse[N];
  u32 dense[N];
  T data[N];
#if BUILD_DEBUG
  u32 generations[N];
#endif
  T* begin() { return data; }
  T* end()   { return data + count; }
  T& get(Handle<T> handle) {
#if BUILD_DEBUG
    u32 idx = handle.handle & INDEX_MASK;
    u32 generation = handle.handle >> INDEX_BITS;
    u32 index = sparse[idx];
    return data[index];
#else
    u32 index = sparse[handle];
    return data[index];
#endif
  }
  Handle<T> add(T e) {
#if BUILD_DEBUG
    u32 idx = count++;
    sparse[idx] = idx;
    dense[idx] = idx;
    data[idx] = e;
    Handle<T> handle = (generations[idx] << INDEX_BITS) | idx;
    return handle;
#else
    u32 idx = count++;
    sparse[idx] = idx;
    dense[idx] = idx;
    data[idx] = e;
    Handle<T> handle = {idx};
    return handle;
#endif
  }
  void remove(Handle<T> handle) {
#if BUILD_DEBUG
    u32 idx = handle.handle & INDEX_MASK;
    u32 generation = handle.handle >> INDEX_BITS;
    Assert(generations[idx] == generation);
    ++generation;
    generations[idx] = generation;

    u32 idx_removed = sparse[handle];
    u32 idx_last = count - 1;
    data[idx_removed] = data[idx_last];
    u32 last_entity = dense[idx_last];
    sparse[last_entity] = idx_removed;
    dense[idx_removed] = last_entity;
    --count;
#else
    u32 idx_removed = sparse[handle];
    u32 idx_last = count - 1;
    data[idx_removed] = data[idx_last];
    u32 last_entity = dense[idx_last];
    sparse[last_entity] = idx_removed;
    dense[idx_removed] = last_entity;
    --count;
#endif
  }
};

template <typename T>
struct DarrayHandler {
  u32 count;
  u32 cap;
  Allocator alloc;
  u32* sparse;
  u32* dense;
  T* data;
#if BUILD_DEBUG
  u32* generations;
#endif
  DarrayHandler() = default;
  DarrayHandler(Allocator alloc_) { *this = {}; alloc = alloc_; }
  void init(Allocator alloc_) { *this = {}; alloc = alloc_; }
  void deinit() { if (sparse) mem_free(alloc, sparse); }
  T* begin() { return data; }
  T* end()   { return data + count; }
  T& get(Handle<T> handle) {
#if BUILD_DEBUG
    Assert(handle.idx() < cap);
    Assert(handle.generation() == generations[handle.idx()]);
    u32 index = sparse[handle.idx()];
    return data[index];
#else
    Assert(handle.handle < cap);
    DebugDo(Assert(sparse[handle.handle] != INVALID_ID));
    u32 index = sparse[handle.handle];
    return data[index];
#endif
  }
  Handle<T> add(T e) {
#if BUILD_DEBUG
    if (count >= cap) {
      grow();
    }
    u32 idx = count++;
    sparse[idx] = idx;
    dense[idx] = idx;
    data[idx] = e;
    Handle<T> handle = {(generations[idx] << INDEX_BITS) | idx};
    return handle;
#else
    if (count >= cap) {
      grow();
    }
    u32 idx = count++;
    sparse[idx] = idx;
    dense[idx] = idx;
    data[idx] = e;
    Handle<T> handle = {idx};
    return handle;
#endif
  }
  void remove(Handle<T> handle) {
#if BUILD_DEBUG
    u32 idx = handle.handle & INDEX_MASK;
    u32 generation = handle.handle >> INDEX_BITS;
    Assert(generations[idx] == generation);
    ++generation;
    generations[idx] = generation;

    u32 idx_removed = sparse[idx];
    u32 idx_last = count - 1;
    data[idx_removed] = data[idx_last];
    u32 last_entity = dense[idx_last];
    sparse[last_entity] = idx_removed;
    dense[idx_removed] = last_entity;
    --count;
#else
    u32 idx_removed = sparse[handle];
    u32 idx_last = count - 1;
    data[idx_removed] = data[idx_last];
    u32 last_entity = dense[idx_last];
    sparse[last_entity] = idx_removed;
    dense[idx_removed] = last_entity;
    --count;
#endif
  }
  void grow() {
#if BUILD_DEBUG
    if (data) {
      u32 cap_old = cap;
      cap *= DEFAULT_RESIZE_FACTOR;
      SoA_Field fields[] = {
        SoA_push_field(&sparse, u32),
        SoA_push_field(&dense, u32),
        SoA_push_field(&data, T),
        SoA_push_field(&generations, u32),
      };
      mem_realloc_soa(alloc, sparse, cap_old, cap, fields, ArrayCount(fields));
      MemZeroArray(generations+cap_old, cap-cap_old);
    }
    else {
      cap = DEFAULT_CAPACITY;
      SoA_Field fields[] = {
        SoA_push_field(&sparse, u32),
        SoA_push_field(&dense, u32),
        SoA_push_field(&data, T),
        SoA_push_field(&generations, u32),
      };
      mem_alloc_soa(alloc, cap, fields, ArrayCount(fields));
      MemZeroArray(generations, cap);
    }
#else
    if (data) {
      u32 cap_old = cap;
      cap *= DEFAULT_RESIZE_FACTOR;
      SoA_Field fields[] = {
        SoA_push_field(&sparse, u32),
        SoA_push_field(&dense, u32),
        SoA_push_field(&data, T),
      };
      mem_realloc_soa(alloc, sparse, cap_old, cap, fields, ArrayCount(fields));
    }
    else {
      cap = DEFAULT_CAPACITY;
      SoA_Field fields[] = {
        SoA_push_field(&sparse, u32),
        SoA_push_field(&dense, u32),
        SoA_push_field(&data, T),
      };
      mem_alloc_soa(alloc, cap, fields, ArrayCount(fields));
    }
    #endif
  }
  void clear() {
    count = 0;
  }
};

struct DarrayIndexHandler {
  u32 count;
  u32 cap;
  Allocator alloc;
  u32* sparse;
  u32* dense;
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
    u64 idx = ModPow2(hash_idx, cap);
    while (is_occupied[idx] == MapSlot_Occupied) {
      idx = ModPow2(idx + 1, cap);
    }
    keys[idx] = key;
    data[idx] = val;
    is_occupied[idx] = MapSlot_Occupied;
    ++count;
  }
  T* get(Key key) {
    u64 hash_idx = hash(key);
    u64 idx = ModPow2(hash_idx, cap);
    u64 start_idx = idx;
    Loop (i, cap) {
      if ((is_occupied[idx] == MapSlot_Occupied) && (equal(keys[idx], key))) {
        return &data[idx];
      } 
      else if (is_occupied[idx] == MapSlot_Empty) {
        break;
      }
      idx = ModPow2(idx + 1, cap);
    }
    return null;
  }
  T* get_or_add(Key key, T val) {
    u64 hash_idx = hash(key);
    u64 idx = ModPow2(hash_idx, cap);
    u64 start_idx = idx;
    Loop (i, cap) {
      if ((is_occupied[idx] == MapSlot_Occupied) && (equal(keys[idx], key))) {
        return &data[idx];
      } 
      else if (is_occupied[idx] == MapSlot_Empty) {
        break;
      }
      idx = ModPow2(idx + 1, cap);
    }
    if (count >= cap*LF) { grow(); }
    keys[idx] = key;
    data[idx] = val;
    is_occupied[idx] = MapSlot_Occupied;
    ++count;
    return &data[idx];
  }
  void remove(Key key) {
    u64 hash_idx = hash(key);
    u64 idx = ModPow2(hash_idx, cap);
    while (is_occupied[idx] != MapSlot_Empty) {
      if ((is_occupied[idx] == MapSlot_Occupied) && (equal(keys[idx] == key))) {
        is_occupied[idx] = MapSlot_Deleted;
        --count;
        return;
      }
      idx = ModPow2(idx + 1, cap);
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
  u32 count;
  u32 cap;
  Allocator alloc;
  T* data;
  Key* keys;
  MapSlot* is_occupied;
  MapAuto() = default;
  MapAuto(Allocator alloc_) { *this = {}; alloc = alloc_; }
  void init(Allocator alloc_) { *this = {}; alloc = alloc_; }
  void add(Key key, T val) {
    if (count >= cap*LF) { grow(); }
    u64 hash_idx = hash_memory(&key, sizeof(Key));
    u64 idx = ModPow2(hash_idx, cap);
    while (is_occupied[idx] == MapSlot_Occupied) {
      idx = ModPow2(idx + 1, cap);
    }
    keys[idx] = key;
    data[idx] = val;
    is_occupied[idx] = MapSlot_Occupied;
    ++count;
  }
  T* get(Key key) {
    if (!data) return null;
    u64 hash_idx = hash_memory(&key, sizeof(Key));
    u64 idx = ModPow2(hash_idx, cap);
    u64 start_idx = idx;
    Loop (i, cap) {
      if ((is_occupied[idx] == MapSlot_Occupied) && (MemMatchStruct(&keys[idx], &key))) {
        return &data[idx];
      } 
      else if (is_occupied[idx] == MapSlot_Empty) {
        break;
      }
      idx = ModPow2(idx + 1, cap);
    }
    return null;
  }
  T* get_or_add(Key key, T val) {
    u64 hash_idx = hash(key);
    u64 idx = ModPow2(hash_idx, cap);
    u64 start_idx = idx;
    Loop (i, cap) {
      if ((is_occupied[idx] == MapSlot_Occupied) && (MemMatchStruct(&keys[idx], &key))) {
        return &data[idx];
      } 
      else if (is_occupied[idx] == MapSlot_Empty) {
        break;
      }
      idx = ModPow2(idx + 1, cap);
    }
    if (count >= cap*LF) { grow(); }
    keys[idx] = key;
    data[idx] = val;
    is_occupied[idx] = MapSlot_Occupied;
    ++count;
    return &data[idx];
  }
  void remove(Key key) {
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

template<typename T>
struct HashedStrMap {
  u32 cap;
  T* data;
#if BUILD_DEBUG
  String* strs;
  MapSlot* is_occupied;
#endif
  void init(Allocator alloc, u32 size) {
#if BUILD_DEBUG
    cap = size;
    data = push_array(alloc, T, size);
    strs = push_array(alloc, String, size);
    is_occupied = push_array(alloc, MapSlot, size);
#else
    cap = size;
    data = push_array(alloc, T, size);
#endif
  }
  void add(u64 key, T val, String str = {}) {
#if BUILD_DEBUG
    u64 idx = ModPow2(key, cap);
    Assert(is_occupied[idx] != MapSlot_Occupied);
    strs[idx] = str;
    data[idx] = val;
    is_occupied[idx] = MapSlot_Occupied;
#else
    u64 index = ModPow2(key, cap);
    data[index] = val;
#endif
  }
  T* get(u64 key) {
#if BUILD_DEBUG
    u64 idx = ModPow2(key, cap);
    Assert(is_occupied[idx] == MapSlot_Occupied);
    return &data[idx];
#else
    u64 idx = ModPow2(key, cap);
    return &data[idx];
#endif
  }
  String get_str(u64 key) {
#if BUILD_DEBUG
    u64 idx = ModPow2(key, cap);
    Assert(is_occupied[idx] == MapSlot_Occupied);
    return strs[idx];
#else
    return {};
#endif
  }
};



