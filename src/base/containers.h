
#pragma once
#include "defines.h"
#include "logger.h"
#include "mem.h"
#include "maths.h"

#define DEFAULT_CAPACITY      8
#define DEFAULT_RESIZE_FACTOR 2

template <typename T>
struct InterfaceEqual {
  NO_DEBUG b32 operator()(T a, T b) { return equal(a,b); }
};
template <typename T>
struct InterfaceMemEqual {
  NO_DEBUG b32 operator()(T a, T b) { return MemMatchStruct(&a, &b); }
};
template <typename T>
struct InterfaceLessthan {
  NO_DEBUG b32 operator()(T a, T b) { return lessthan(a,b); }
};

enum EqualMode {
  EqualDefault,
  EqualMem
};

// TODO: implement object pool indexes through u32 freelist with handler generation

////////////////////////////////////////////////////////////////////////
// Array

template<typename T, i32 N>
struct Array {
  static constexpr i32 cap = N;
  u32 count = 0;
  T data[N] = {};

  T* begin() { return data; }
  T* end()   { return data + count; }

  Array() = default;

  Array(std::initializer_list<T> init) {
    Assert(init.size() <= cap);
    count = init.size();
    u32 i = 0;
    for (T x : init) {
      data[i++] = x;
    }
  }
  INLINE T& operator[](u32 idx) {
    Assert(idx < cap);
    return data[idx];
  }
  T& append(T a) { 
    Assert(count < cap);
    T& e = data[count];
    data[count++] = a;
    return e;
  }
  void append(std::initializer_list<T> slice) {
    for (T x : slice) {
      append(x);
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
  u32 count = 0;
  u32 cap = 0;
  Allocator alloc = {};
  T* data = null;

  T* begin() { return data; }
  T* end()   { return data + count; }

  Darray() = default;
  Darray(Allocator alloc_) { alloc = alloc_; }

  void init(Allocator alloc_) { alloc = alloc_; }
  void deinit() { mem_free(alloc, data); }

  INLINE T& operator[](u32 idx) {
    Assert(idx < cap);
    return data[idx];
  }

  T& append(T b) { 
    if (count >= cap) {
      if (data) {
        u32 odl_cap = cap;
        cap *= DEFAULT_RESIZE_FACTOR;
        data = mem_realloc_array(alloc, data, odl_cap, T, cap);
      } else {
        cap = DEFAULT_CAPACITY;
        data = mem_alloc_array(alloc, T, cap);
      }
    }
    T& e = data[count];
    data[count++] = b;
    return e;
  }
  void append(std::initializer_list<T> slice) {
    for (T x : slice) {
      append(x);
    }
  }
  void reserve(u32 min_cap) { 
    if (cap >= min_cap) return;
    u32 old_cap = cap;
    u32 new_cap = Max(cap*DEFAULT_RESIZE_FACTOR, min_cap);
    if (data) {
      data = mem_realloc_array(alloc, data, old_cap, T, new_cap);
    } else {
      data = mem_alloc_array(alloc, T, new_cap);
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
    for (T x : *this) {
      if (eq(x, a)) return true;
    }
    return false;
  }
  INLINE b32 exists(T a, bool(*fn)(T a, T b)) { return exists_impl(a, fn); }
  INLINE b32 exists(T a)                      { return exists_impl(a, InterfaceEqual<T>{}); }
  INLINE b32 exists(T a, EqualMode mode)      { return exists_impl(a, InterfaceMemEqual<T>{}); }
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
  INLINE b32 exists_at(T a, u32* idx, bool(*fn)(T a, T b)) { return exists_at_impl(a, idx, fn); }
  INLINE b32 exists_at(T a, u32* idx)                      { return exists_at_impl(a, idx, InterfaceEqual<T>{}); }
  INLINE b32 exists_at(T a, u32* idx, EqualMode mode)      { return exists_at_impl(a, idx, InterfaceMemEqual<T>{}); }
};

////////////////////////////////////////////////////////////////////////
// SparseSet

template <typename T>
struct SparseSet {
  u32 count = 0;
  u32 cap = 0;
  u32 sparse_count = 0;
  Allocator alloc = {};
  u32* sparse = null;
  u32* dense = null;
  T* data = null;

  T* begin() { return data; }
  T* end()   { return data + count; }

  SparseSet() = default;
  SparseSet(Allocator alloc_) { alloc = alloc_; }

  void init(Allocator alloc_) { alloc = alloc_; }
  void deinit() { 
    mem_free(alloc, sparse); 
    mem_free(alloc, dense); 
  }

  T& operator[](u32 id) {
    Assert(id < cap);
    DebugDo(Assert(sparse[id] != INVALID_ID));
    u32 idx = sparse[id];
    return data[idx];
  }
  void append(u32 id) {
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
  void append(u32 id, T element) {
    if (count >= cap) {
      grow();
    }
    if (id >= sparse_count) {
      grow_max_index(id);
    }
    sparse[id] = count;
    dense[count] = id;
    data[count] = element;
    ++count;
  }
  void remove(u32 id) {
    DebugDo(Assert(sparse[id] != INVALID_ID));
    u32 idx_of_removed_entity = sparse[id];
    u32 idx_of_last_element = count - 1;
    data[idx_of_removed_entity] = data[idx_of_last_element];

    u32 last_entity = dense[idx_of_last_element];

    sparse[last_entity] = idx_of_removed_entity;
    dense[idx_of_removed_entity] = last_entity;

    --count;

    DebugDo(sparse[id] = INVALID_ID);
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
      sparse = mem_alloc_array(alloc, u32, cap);
    }
  }
  void grow_max_index(u32 id) {
    u32 modifier = CeilIntDiv(id+1, sparse_count);
    u32 old_count = sparse_count;
    sparse_count *= modifier;
    sparse = mem_realloc_array(alloc, sparse, old_count, u32, sparse_count);
  }
};

// Just stores ids
struct SparseSetIndex {
  u32 count = 0;
  u32 cap = 0;
  u32 sparse_count = 0;
  Allocator alloc;
  u32* sparse = null;
  u32* dense = null;

  u32* begin() { return dense; }
  u32* end()   { return dense + count; }

  void append(u32 id) {
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
  void remove(u32 id) {
    u32 idx_of_removed_entity = sparse[id];
    u32 idx_of_last_element = count - 1;

    u32 last_entity = dense[idx_of_last_element];

    sparse[last_entity] = idx_of_removed_entity;
    dense[idx_of_removed_entity] = last_entity;

    --count;
  }
  void grow() {
    if (dense) {
      u32 old_cap = cap;
      cap *= DEFAULT_RESIZE_FACTOR;
      dense = mem_realloc_array(alloc, dense, old_cap, u32, cap);
    }
    else {
      cap = DEFAULT_CAPACITY;
      dense = mem_alloc_array(alloc, u32, cap);
      sparse_count = cap;
      sparse = mem_alloc_array(alloc, u32, sparse_count);
    }
  }
  void grow_max_index(u32 id) {
    u32 modifier = CeilIntDiv(id+1, sparse_count);
    u32 old_sparse_count = sparse_count;
    sparse_count *= modifier;
    sparse = mem_realloc_array(alloc, sparse, old_sparse_count, u32, sparse_count);
  }
};

// template<typename T, i32 N>
// struct SparseArray {
//   static constexpr i32 cap = N;
//   u32 count = 0;
//   u32 entity_to_index[N] = {};
//   u32 entities[N] = {};
//   T data[N] = {};

//   T* begin() { return data; }
//   T* end()   { return data + count; }

//   SparseArray() = default;

//   T& operator[](u32 idx) {
//     Assert(idx < cap);
//     DebugDo(Assert(entity_to_index[idx] != INVALID_ID));
//     u32 index = entity_to_index[idx];
//     return data[index];
//   }
//   u32 append(T e) {
//     u32 id = count++;
//     entity_to_index[id] = id;
//     entities[id] = id;
//     data[id] = e;
//     return id;
//   }
//   void remove(u32 id) {
//     DebugDo(Assert(entity_to_index[id] != INVALID_ID));
//     u32 idx_of_removed_entity = entity_to_index[id];
//     u32 idx_of_last_element = count - 1;
//     data[idx_of_removed_entity] = data[idx_of_last_element];

//     u32 last_entity = entities[idx_of_last_element];

//     entity_to_index[last_entity] = idx_of_removed_entity;
//     entities[idx_of_removed_entity] = last_entity;

//     --count;

//     DebugDo(entity_to_index[id] = INVALID_ID);
//   }
// };

// template <typename T>
// struct SparseDarray {
//   u32 count = 0;
//   u32 cap = 0;
//   u32* entity_to_index = null;
//   u32* entities = null;
//   T* data = null;

//   T* begin() { return data; }
//   T* end()   { return data + count; }

//   T& operator[](u32 idx) {
//     Assert(idx < cap);
//     DebugDo(Assert(entity_to_index[idx] != INVALID_ID));
//     u32 index = entity_to_index[idx];
//     return data[index];
//   }
//   u32 append(T e) {
//     if (count >= cap) {
//       grow();
//     }
//     u32 id = count++;
//     entity_to_index[id] = id;
//     entities[id] = id;
//     data[id] = e;
//     return id;
//   }
//   void remove(u32 id) {
//     DebugDo(Assert(entity_to_index[id] != INVALID_ID));
//     u32 idx_of_removed_entity = entity_to_index[id];
//     u32 idx_of_last_element = count - 1;
//     data[idx_of_removed_entity] = data[idx_of_last_element];

//     u32 last_entity = entities[idx_of_last_element];

//     entity_to_index[last_entity] = idx_of_removed_entity;
//     entities[idx_of_removed_entity] = last_entity;

//     --count;

//     DebugDo(entity_to_index[id] = INVALID_ID);
//   }
//   void grow() {
//     if (data) {
//       cap *= DEFAULT_RESIZE_FACTOR;
//       // entities = global_realloc_array(entities, u32, cap);
//       // entity_to_index = global_realloc_array(entity_to_index, u32, cap);
//       // data = global_realloc_array(entities, T, cap);
//     }
//     else {
//       cap = DEFAULT_CAPACITY;
//       entities = global_alloc_array(u32, cap);
//       data = global_alloc_array(T, cap);
//       entity_to_index = global_alloc_array(u32, cap);
//     }
//   }
// };

////////////////////////////////////////////////////////////////////////
// IdPool

struct IdPool {
  u32 next_idx;
  Darray<u32> array;
};

inline u32 id_pool_alloc(IdPool& p) {
  if (p.array.count > 0) {
    return p.array.pop();
  }
  u32 id = p.next_idx++;
  return id;
}

inline void id_pool_free(IdPool& p, u32 id) {
  Assert(p.next_idx > 0);
  Assert(IsInsideBound(id, p.next_idx));
  Assert(!p.array.exists(id));
  p.array.append(id);
}

////////////////////////////////////////////////////////////////////////
// Hashmap

enum MapSlot : u8 {
  MapSlot_Empty,
  MapSlot_Occupied,
  MapSlot_Deleted
};

template<typename Key, typename T>
struct Map {
  static constexpr f32 LF = 0.7;

  u32 count = 0;
  u32 cap = 0;
  Allocator alloc = {};
  T* data = null;
  Key* keys = null;
  MapSlot* is_occupied = null;

  void insert(Key key, T val) {
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

  T& get(Key key) {
    u64 hash_idx = hash(key);
    u64 index = ModPow2(hash_idx, cap);
    while (is_occupied[index] != MapSlot_Empty) {
      if ((is_occupied[index] == MapSlot_Occupied) && (equal(keys[index], key))) {
        goto found;
      }
      index = ModPow2(index + 1, cap);
    }
    Assert(false);

    found:
    return data[index];
  }

  void erase(Key key) {
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

