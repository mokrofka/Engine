#pragma once
#include "base.h"
#include "logger.h"
#include "mem.h"
#include "maths.h"

const u32 INDEX_BITS = 22;
const u32 INDEX_MASK = (1u << INDEX_BITS) - 1;

inline u32 id_idx(u32 id) { return id & INDEX_MASK; }
inline u32 id_generation(u32 id) { return id >> INDEX_BITS; }
inline u32 id_make(u32 generation, u32 idx) { return (generation << INDEX_BITS) | idx; }
inline u32 generation_bits(u32 gen) { return gen & Bit(32 - INDEX_BITS) - 1; }

////////////////////////////////////////////////////////////////////////
// Array

template<typename T, i32 N>
struct Array {
  u32 count;
  static constexpr i32 cap = N;
  T data[N];
  T& operator[](u32 idx) {
    Assert(idx < cap);
    return data[idx];
  }
};

template<typename T, i32 N> Slice<T> slice(Array<T, N>& arr) {
  return {arr.data, arr.count};
}
template<typename T, i32 N> void array_add(Array<T, N>& arr, T a) {
  Assert(arr.count < arr.cap);
  arr.data[arr.count++] = a;
}
template<typename T, i32 N, typename ... Args> void array_add(Array<T, N>& arr, Args... args) {
  var list = {args...};
  for (T x : list) {
    array_add(arr, x);
  }
}
template<typename T, i32 N> void array_swap_remove(Array<T, N>& arr, u32 idx) {
  Assert(idx < arr.count);
  arr.data[idx] = arr.data[--arr.count];
}
template<typename T, i32 N> void array_clear(Array<T, N>& arr) {
  arr.count = 0;
}
template<typename T, i32 N> T array_pop(Array<T, N>& arr) {
  return arr.data[--arr.count];
}
template<typename T, i32 N> b32 array_exists(Array<T, N>& arr, T a, b32(*fn)(T a, T b) = equal) {
  Loop (i, arr.count) {
    T x = arr[i];
    if (fn(x, a)) return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////
// Darray

template <typename T>
struct Darray {
  u32 count;
  u32 cap;
  Allocator alloc;
  T* data;
  T& operator[](u32 idx) {
    Assert(idx < cap);
    return data[idx];
  }
};

template<typename T> Slice<T> slice(Darray<T>& arr) {
  return {arr.data, arr.count};
}
template<typename T> Darray<T> darray_make(Allocator alloc) {
  Darray<T> res = {
    .alloc = alloc,
  };
  return res;
}
template<typename T> void darray_free(Darray<T>& arr) {
  if (arr.data) { mem_free(arr.alloc, arr.data); } 
}
template<typename T> void darray_grow(Darray<T>& arr, u32 elem_count) {
  if (arr.data) {
    u32 old_cap = arr.cap;
    arr.cap = Max(arr.cap * DEFAULT_RESIZE_FACTOR, arr.count + elem_count);
    arr.data = mem_realloc_array(arr.alloc, arr.data, old_cap, arr.cap);
  } else {
    arr.cap = Max(DEFAULT_CAPACITY, elem_count);
    arr.data = push_array(arr.alloc, T, arr.cap);
  }
}
template<typename T> void darray_reserve(Darray<T>& arr, u32 min_cap) {
  if (arr.cap >= min_cap) return;
  u32 old_cap = arr.cap;
  u32 new_cap = Max(old_cap * DEFAULT_RESIZE_FACTOR, min_cap);
  if (arr.data) {
    arr.data = mem_realloc_array(arr.alloc, arr.data, old_cap, new_cap);
  } else {
    arr.data = push_array(arr.alloc, T, new_cap);
  }
  arr.cap = new_cap;
}
template<typename T> T darray_clone(Darray<T>& arr, Allocator alloc) {
  Darray<T> result = {
    .count = arr.count,
    .cap = arr.cap,
    .alloc = alloc,
    .data = push_array(alloc, T, arr.cap),
  };
  MemCopyArray(result.data, arr.data, arr.count);
  return result;
}
template<typename T> void darray_add(Darray<T>& arr) {
  if (arr.count >= arr.cap) {
    darray_grow(arr, 0);
  }
  ++arr.count;
}
template<typename T> void darray_add(Darray<T>& arr, T a) {
  if (arr.count >= arr.cap) {
    darray_grow(arr, 0);
  }
  arr.data[arr.count++] = a;
}
template<typename T, typename...Args> void darray_add(Darray<T>& arr, Args...args) {
  var list = { args... };
  for (T x : list) {
    darray_add(arr, x);
  }
}
template<typename T> void darray_add_elems(Darray<T>& arr, Slice<T> elems) {
  if (arr.count + elems.count >= arr.cap) {
    darray_grow(arr, elems.count);
  }
  MemCopyArray(arr.data + arr.count, elems.data, elems.count);
  arr.count += elems.count;
}
template<typename T> void darray_swap_remove(Darray<T>& arr, u32 idx) {
  Assert(idx < arr.count);
  arr.data[idx] = arr.data[--arr.count];
}
template<typename T> void darray_clear(Darray<T>& arr) {
  arr.count = 0; 
}
template<typename T> T darray_pop(Darray<T>& arr) {
  return arr.data[--arr.count];
}
template<typename T> T darray_back(Darray<T>& arr) {
  return arr.data[arr.count-1];
}
template<typename T> b32 darray_exists(Darray<T>& arr, T a, b32(*fn)(T a, T b) = equal) {
  Loop (i, arr.count) {
    T x = arr[i];
    if (fn(x, a)) return true;
  }
  return false;
}
template<typename T> b32 darray_exists_at(Darray<T>& arr, T a, u32* out_idx, b32(*fn)(T a, T b) = equal) {
  Loop (i, arr.count) {
    if (fn(arr.data[i], a)) {
      *out_idx = i;
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////
// ArrayHandler

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
};

template<typename T, i32 N> T& array_handler_get(ArrayHandler<T, N>& arr, u32 h) {
#if BUILD_DEBUG
    u32 idx = id_idx(h);
    Assert(idx < arr.count);
    Assert(generation_bits(arr.generations[idx]++) == id_generation(h));
    u32 index = arr.sparse[idx];
    return arr.data[index];
#else
    Assert(h < arr.count);
    u32 idx = arr.sparse[h];
    return arr.data[idx];
#endif
}
template<typename T, i32 N> u32 array_handler_add(ArrayHandler<T, N>& arr, T a) {
#if BUILD_DEBUG
  u32 idx = arr.count++;
  arr.sparse[idx] = idx;
  arr.dense[idx] = idx;
  arr.data[idx] = a;
  u32 h = id_make(arr.generations[idx], idx);
  return h;
#else
  u32 idx = arr.count++;
  arr.sparse[idx] = idx;
  arr.dense[idx] = idx;
  arr.data[idx] = a;
  u32 h = idx;
  return h;
#endif
}
template<typename T, i32 N> void array_handler_remove(ArrayHandler<T, N>& arr, u32 h) {
#if BUILD_DEBUG
    u32 idx = id_idx(h);
    Assert(idx < arr.count);
    Assert(generation_bits(arr.generations[idx]++) == id_generation(h));
    u32 idx_removed = arr.sparse[idx];
    u32 idx_last = arr.count - 1;
    arr.data[idx_removed] = arr.data[idx_last];
    u32 last_entity = arr.dense[idx_last];
    arr.sparse[last_entity] = idx_removed;
    arr.dense[idx_removed] = last_entity;
    --arr.count;
#else
    u32 idx = h;
    Assert(idx < arr.count);
    u32 idx_removed = arr.sparse[idx];
    u32 idx_last = arr.count - 1;
    arr.data[idx_removed] = arr.data[idx_last];
    u32 last_entity = arr.dense[idx_last];
    arr.sparse[last_entity] = idx_removed;
    arr.dense[idx_removed] = last_entity;
    --arr.count;
#endif
}
template<typename T, i32 N> void array_handler_clear(ArrayHandler<T, N>& arr) {
  arr.count = 0;
}

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
};

template<typename T> DarrayHandler<T> darray_handler_make(Allocator alloc) {
  DarrayHandler<T> res = {
    .alloc = alloc,
  };
  return res;
}
template<typename T> T& darray_handler_get(DarrayHandler<T>& arr, u32 h) {
#if BUILD_DEBUG
  u32 idx = id_idx(h);
  Assert(idx < arr.cap);
  Assert(generation_bits(arr.generations[idx]) == id_generation(h));
  u32 index = arr.sparse[idx];
  return arr.data[index];
#else
  Assert(h < arr.count);
  u32 idx = arr.sparse[h];
  return arr.data[idx];
#endif
}
template<typename T> void darray_handler_grow(DarrayHandler<T>& arr) {
#if BUILD_DEBUG
  if (arr.data) {
    u32 cap_old = arr.cap;
    arr.cap *= DEFAULT_RESIZE_FACTOR;
    SoA_Field fields[] = {
      SoA_push_field(&arr.sparse, u32),
      SoA_push_field(&arr.dense, u32),
      SoA_push_field(&arr.data, T),
      SoA_push_field(&arr.generations, u32),
    };
    mem_realloc_soa(arr.alloc, cap_old, arr.cap, ArraySlice(fields));
    MemZeroArray(arr.generations+cap_old, arr.cap-cap_old);
  } else {
    arr.cap = DEFAULT_CAPACITY;
    SoA_Field fields[] = {
      SoA_push_field(&arr.sparse, u32),
      SoA_push_field(&arr.dense, u32),
      SoA_push_field(&arr.data, T),
      SoA_push_field(&arr.generations, u32),
    };
    mem_alloc_soa(arr.alloc, arr.cap, ArraySlice(fields));
    MemZeroArray(arr.generations, arr.cap);
  }
#else
  if (arr.data) {
    u32 cap_old = arr.cap;
    arr.cap *= DEFAULT_RESIZE_FACTOR;
    SoA_Field fields[] = {
      SoA_push_field(&arr.sparse, u32),
      SoA_push_field(&arr.dense, u32),
      SoA_push_field(&arr.data, T),
    };
    mem_realloc_soa(arr.alloc, cap_old, arr.cap, ArraySlice(fields));
  } else {
    arr.cap = DEFAULT_CAPACITY;
    SoA_Field fields[] = {
      SoA_push_field(&arr.sparse, u32),
      SoA_push_field(&arr.dense, u32),
      SoA_push_field(&arr.data, T),
    };
    mem_alloc_soa(arr.alloc, arr.cap, ArraySlice(fields));
  }
#endif
}
template<typename T> u32 darray_handler_add(DarrayHandler<T>& arr, T a) {
#if BUILD_DEBUG
  if (arr.count >= arr.cap) {
    darray_handler_grow(arr);
  }
  u32 idx = arr.count++;
  arr.sparse[idx] = idx;
  arr.dense[idx] = idx;
  arr.data[idx] = a;
  u32 h = id_make(arr.generations[idx], idx);
  return h;
#else
  if (arr.count >= arr.cap) {
    darray_handler_grow(arr);
  }
  u32 idx = arr.count++;
  arr.sparse[idx] = idx;
  arr.dense[idx] = idx;
  arr.data[idx] = a;
  return idx;
#endif
}
template<typename T> void darray_handler_remove(DarrayHandler<T>& arr, u32 h) {
#if BUILD_DEBUG
  u32 idx = id_idx(h);
  Assert(generation_bits(arr.generations[idx]++) == id_generation(h));
  u32 idx_removed = arr.sparse[idx];
  u32 idx_last = arr.count - 1;
  arr.data[idx_removed] = arr.data[idx_last];
  u32 last_entity = arr.dense[idx_last];
  arr.sparse[last_entity] = idx_removed;
  arr.dense[idx_removed] = last_entity;
  --arr.count;
#else
  u32 idx = h;
  u32 idx_removed = arr.sparse[idx];
  u32 idx_last = arr.count - 1;
  arr.data[idx_removed] = arr.data[idx_last];
  u32 last_entity = arr.dense[idx_last];
  arr.sparse[last_entity] = idx_removed;
  arr.dense[idx_removed] = last_entity;
  --arr.count;
#endif
}
template<typename T> void darray_handler_clear(DarrayHandler<T>& arr) {
  arr.count = 0;
}

////////////////////////////////////////////////////////////////////////
// Pool

template<typename T>
struct ObjectPool {
  static_assert(sizeof(T) >= 4);
  u32 head;
  u32 cap;
  Allocator alloc;
  union {
    T v;
    u32 next_free;
  }*data;
#if BUILD_DEBUG
  u32* generations;
#endif
};

template<typename T> ObjectPool<T> object_pool_make(Allocator alloc) {
  ObjectPool<T> res = {
    .alloc = alloc,
  };
  return res;
}
template<typename T> T& object_pool_get(ObjectPool<T>& p, u32 h) {
#if BUILD_DEBUG
  u32 idx = id_idx(h);
  Assert(generation_bits(p.generations[idx]) == id_generation(h));
  return p.data[idx].v;
#else
  return p.data[h];
#endif
}
template<typename T> void object_pool_grow(ObjectPool<T>& p) {
#if BUILD_DEBUG
  if (p.data) {
    u32 cap_old = p.cap;
    p.cap *= DEFAULT_RESIZE_FACTOR;
    SoA_Field fields[] = {
      SoA_push_field(&p.generations, u32),
      SoA_push_field(&p.data, T),
    };
    mem_realloc_soa(p.alloc, cap_old, p.cap, ArraySlice(fields));
    p.head = cap_old;
    for (i32 i = cap_old; i < p.cap - 1; ++i) {
      p.data[i].next_free = i + 1;
    }
    *(u32*)&p.data[p.cap - 1] = U32_MAX;
    MemZeroArray(p.generations + cap_old, p.cap - cap_old);
  } else {
    p.cap = DEFAULT_CAPACITY;
    SoA_Field fields[] = {
      SoA_push_field(&p.generations, u32),
      SoA_push_field(&p.data, T),
    };
    mem_alloc_soa(p.alloc, p.cap, ArraySlice(fields));
    Loop(i, p.cap - 1) {
      p.data[i].next_free = i + 1;
    }
    p.data[p.cap - 1].next_free = U32_MAX;
    MemZeroArray(p.generations, p.cap);
  }
#else
  if (p.data) {
    u32 cap_old = p.cap;
    p.cap *= DEFAULT_RESIZE_FACTOR;
    p.data = mem_realloc_array(p.alloc, p.data, cap_old, p.cap);
    p.head = cap_old;
    for (i32 i = cap_old; i < p.cap - 1; ++i) {
      p.data[i].next_free = i + 1;
    }
    *(u32*)&p.data[p.cap - 1] = U32_MAX;
  } else {
    p.cap = DEFAULT_CAPACITY;
    p.data = push_array(p.alloc, T, p.cap);
    Loop(i, p.cap - 1) {
      p.data[i].next_free = i + 1;
    }
    p.data[p.cap - 1].next_free = U32_MAX;
  }
#endif
}
template<typename T> u32 object_pool_alloc(ObjectPool<T>& p) {
  u32 head_idx = id_idx(p.head);
  if (head_idx >= p.cap) {
    object_pool_grow(p);
  }
  u32 result = p.head;
  u32 idx = id_idx(result);
  p.head = p.data[idx].next_free;
  return result;
}
template<typename T> u32 object_pool_alloc(ObjectPool<T>& p, T a) {
  u32 h = object_pool_alloc(p);
  object_pool_get(p, h) = a;
  return h;
}
template<typename T> void object_pool_free(ObjectPool<T>& p, u32 h) {
#if BUILD_DEBUG
  u32 idx = id_idx(h);
  Assert(generation_bits(p.generations[idx]++) == id_generation(h));
  p.data[idx].next_free = p.head;
  p.head = id_make(p.generations[idx], idx);
#else
  u32 idx = h;
  p.data[idx].next_free = p.head;
  p.head = h;
#endif
}
template<typename T> u32 object_pool_clear(ObjectPool<T>& p) {
#if BUILD_DEBUG
  p.head = 0;
  Loop (i, p.cap - 1) {
    p.data[i].next_Free = i + 1;
  }
  p.data[p.cap - 1].next_free = U32_MAX;
  MemZeroArray(p.generations, p.cap);
#else
  p.head = 0;
  Loop (i, p.cap - 1) {
    p.data[i].next_free = i + 1;
  }
  p.data[p.cap - 1].next_free = U32_MAX;
#endif
}

template<typename T>
struct ObjectPoolLinklist {
  static_assert(sizeof(T) >= 4);
  u32 head;
  u32 cap;
  Allocator alloc;
  u32 first;
  u32 last;
  struct {
    union {
      T data;
      u32 next_free;
    };
    u32 next;
    u32 prev;
  }*data;
#if BUILD_DEBUG
  u32* generations;
#endif
};

// for (u32 node = pool.first; node != U32_MAX; node = pool.data[id_idx(node)].next) {

template<typename T> ObjectPoolLinklist<T> object_pool_linklist_make(Allocator alloc) {
  ObjectPoolLinklist<T> res = {
    .alloc = alloc,
  };
  return res;
}
template<typename T> T& object_pool_linklist_get(ObjectPoolLinklist<T>& p, u32 h) {
#if BUILD_DEBUG
  u32 idx = id_idx(h);
  Assert(generation_bits(p.generations[idx]) == id_generation(h));
  return p.data[idx].data;
#else
  return p.data[h];
#endif
}
template<typename T> void object_pool_linklist_grow(ObjectPoolLinklist<T>& p) {
#if BUILD_DEBUG
  if (p.data) {
    u32 cap_old = p.cap;
    p.cap *= DEFAULT_RESIZE_FACTOR;
    SoA_Field fields[] = {
      SoA_push_field(&p.generations, u32),
      SoA_push_field(&p.data, *p.data),
    };
    mem_realloc_soa(p.alloc, cap_old, p.cap, ArraySlice(fields));
    p.head = cap_old;
    for (i32 i = cap_old; i < p.cap - 1; ++i) {
      p.data[i].next_free = i + 1;
    }
    p.data[p.cap - 1].next_free = U32_MAX;
    MemZeroArray(p.generations + cap_old, p.cap - cap_old);
  } else {
    p.cap = DEFAULT_CAPACITY;
    SoA_Field fields[] = {
      SoA_push_field(&p.generations, u32),
      SoA_push_field(&p.data, *p.data),
    };
    mem_alloc_soa(p.alloc, p.cap, ArraySlice(fields));
    Loop(i, p.cap - 1) {
      p.data[i].next_free = i + 1;
    }
    p.data[p.cap - 1].next_free = U32_MAX;
    MemZeroArray(p.generations, p.cap);
    p.first = U32_MAX;
    p.last = U32_MAX;
  }
#else
  if (p.data) {
    u32 cap_old = p.cap;
    p.cap *= DEFAULT_RESIZE_FACTOR;
    p.data = mem_realloc_array(p.alloc, p.data, cap_old, p.cap);
    p.head = cap_old;
    for (i32 i = cap_old; i < p.cap - 1; ++i) {
      p.data[i].next_free = i + 1;
    }
    p.data[p.cap - 1].next_free = U32_MAX;
  } else {
    p.cap = DEFAULT_CAPACITY;
    p.data = push_array(p.alloc, T, p.cap);
    Loop(i, p.cap - 1) {
      p.data[i].next_free = i + 1;
    }
    p.data[p.cap - 1].next_free = U32_MAX;
  }
#endif
}
template<typename T> u32 object_pool_linklist_alloc(ObjectPoolLinklist<T>& p) {
  u32 head_idx = id_idx(p.head);
  if (head_idx >= p.cap) {
    object_pool_linklist_grow(p);
  }
  u32 result = p.head;
  u32 idx = id_idx(result);
  p.head = p.data[idx].next_free;

  // add to link list
  var& n = p.data[idx];
  n.prev = p.last;
  n.next = U32_MAX;
  if (p.last != U32_MAX) {
    p.data[id_idx(p.last)].next = result;
  } else {
    p.first = result;
  }
  p.last = result;

  return result;
}
template<typename T> u32 object_pool_linklist_alloc(ObjectPoolLinklist<T>& p, T a) {
  u32 h = object_pool_linklist_alloc(p);
  object_pool_linklist_get(p, h) = a;
  return h;
}
template<typename T> void object_pool_linklist_free(ObjectPoolLinklist<T>& p, u32 h) {
#if BUILD_DEBUG
  u32 idx = id_idx(h);
  Assert(generation_bits(p.generations[idx]++) == id_generation(h));
  p.data[idx].next_free = p.head;
  p.head = id_make(p.generations[idx], idx);

  // remove from link list
  var& n = p.data[idx];
  if(n.prev != U32_MAX) {
    p.data[id_idx(n.prev)].next = n.next;
  } else {
    p.first = n.next;
  }
  if(n.next != U32_MAX) {
    p.data[id_idx(n.next)].prev = n.prev;
  } else {
    p.last = n.prev;
  }

#else
  u32 idx = h;
  p.data[idx].next_free = p.head;
  p.head = h;

  // remove from link list
  if(p.prev != U32_MAX) {
    p.data[p.prev].next = p.next;
  } else {
    p.first = p.next;
  }
  if(p.next != U32_MAX) {
    p.data[p.next].prev = p.prev;
  } else {
    p.last = p.prev;
  }
#endif
}

////////////////////////////////////////////////////////////////////////
// SparseSet

// template <typename T>
// struct SparseSet {
//   u32 count;
//   u32 cap;
//   u32 sparse_count;
//   Allocator alloc;
//   u32* sparse;
//   u32* dense;
//   T* data;
//   SparseSet() = default;
//   SparseSet(Allocator alloc_) { *this = {}; alloc = alloc_; }
//   void init(Allocator alloc_) { *this = {}; alloc = alloc_; }
//   void deinit() { if (data) { mem_free(alloc, sparse); mem_free(alloc, dense); }; }
//   T* begin() { return data; }
//   T* end()   { return data + count; }
//   T& get(u32 handle) {
//     Assert(handle < cap);
//     DebugDo(Assert(sparse[handle] != INVALID_ID));
//     u32 idx = sparse[handle];
//     return data[idx];
//   }
//   void add(u32 handle) {
//     if (count >= cap) {
//       grow();
//     }
//     if (handle >= sparse_count) {
//       grow_max_index(handle);
//     }
//     sparse[handle] = count;
//     dense[count] = handle;
//     ++count;
//   }
//   void add(u32 handle, T element) {
//     if (count >= cap) {
//       grow();
//     }
//     if (handle >= sparse_count) {
//       grow_max_index(handle);
//     }
//     sparse[handle] = count;
//     dense[count] = handle;
//     data[count] = element;
//     ++count;
//   }
//   void remove(u32 handle) {
//     DebugDo(Assert(sparse[handle] != INVALID_ID));
//     u32 idx_removed = sparse[handle];
//     u32 idx_last = count - 1;
//     data[idx_removed] = data[idx_last];
//     u32 last_entity = dense[idx_last];
//     sparse[last_entity] = idx_removed;
//     dense[idx_removed] = last_entity;
//     --count;
//     DebugDo(sparse[handle] = INVALID_ID);
//   }
//   void grow() {
//     if (data) {
//       u32 cap_old = cap;
//       cap *= DEFAULT_RESIZE_FACTOR;
//       SoA_Field fields[] = {
//         SoA_push_field(&dense, u32),
//         SoA_push_field(&data, T),
//       };
//       mem_realloc_soa(alloc, cap_old, cap, ArraySlice(fields));
//     }
//     else {
//       cap = DEFAULT_CAPACITY;
//       SoA_Field fields[] = {
//         SoA_push_field(&dense, u32),
//         SoA_push_field(&data, T),
//       };
//       mem_alloc_soa(alloc, cap, ArraySlice(fields));
//       sparse = push_array(alloc, u32, cap);
//     }
//   }
//   void grow_max_index(u32 handle) {
//     u32 modifier = CeilIntDiv(handle+1, sparse_count); // +1 since hanlde is idx in array
//     u32 old_count = sparse_count;
//     sparse_count *= modifier;
//     sparse = mem_realloc_array(alloc, sparse, old_count, sparse_count);
//   }
// };

// // it returns stable indexes and iterate through them
// struct SparseSetIndex {
//   u32 count;
//   u32 cap;
//   u32 sparse_count;
//   Allocator alloc;
//   u32* sparse;
//   u32* dense;
//   SparseSetIndex() = default;
//   SparseSetIndex(Allocator alloc_);
//   void init(Allocator alloc_);
//   void deinit();
//   u32* begin();
//   u32* end();
//   void add(u32 id);
//   void remove(u32 id);
//   void grow();
//   void grow_max_index(u32 id);
// };

// struct DarrayIndexHandler {
//   u32 count;
//   u32 cap;
//   Allocator alloc;
//   u32* sparse;
//   u32* dense;
//   u32* begin();
//   u32* end();
//   DarrayIndexHandler() = default;
//   DarrayIndexHandler(Allocator alloc_);
//   void init(Allocator alloc_);
//   void deinit();
//   u32 add();
//   void remove(u32 id);
//   void grow();
// };

////////////////////////////////////////////////////////////////////////
// IdPool

struct IdPool {
  u32 count;
  u32 cap;
  u32* ids;
  Allocator alloc;
#if BUILD_DEBUG
  u32* generations;
#endif
};

IdPool id_pool_make(Allocator alloc);
u32 id_pool_alloc(IdPool& p);
void id_pool_free(IdPool& p, u32 h);
void id_pool_clear(IdPool& p);

struct StaticIdPool {
  u32 count;
  u32 cap;
  u32* ids;
#if BUILD_DEBUG
  u32* generations;
#endif
};

StaticIdPool static_id_pool_make(Allocator alloc, u32 cap);
u32 static_id_pool_alloc(StaticIdPool& p);
void static_id_pool_free(StaticIdPool& p, u32 h);
void static_id_pool_clear(StaticIdPool& p);

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
};

template<typename Key, typename T> Map<Key, T> map_make(Allocator alloc) {
  Map<Key, T> res = {
    .alloc = alloc,
  };
  return res;
}
template<typename Key, typename T> void map_grow(Map<Key, T>& m) {
  if (m.data) {
    T* old_data = m.data;
    Key* old_keys = m.keys;
    MapSlot* old_is_occupied = m.is_occupied;
    u32 old_cap = m.cap;
    m.cap *= DEFAULT_RESIZE_FACTOR;
    SoA_Field fields[] = {
      SoA_push_field(&m.data, T),
      SoA_push_field(&m.keys, Key),
      SoA_push_field(&m.is_occupied, MapSlot),
    };
    mem_alloc_soa(m.alloc, m.cap, ArraySlice(fields));
    Loop (i, old_cap) {
      if (old_is_occupied[i] == MapSlot_Occupied) {
        map_set(m, old_keys[i], old_data[i]);
      }
    }
    mem_free(m.alloc, old_data);
  }
  else {
    m.cap = DEFAULT_CAPACITY;
    SoA_Field fields[] = {
      SoA_push_field(&m.data, T),
      SoA_push_field(&m.keys, Key),
      SoA_push_field(&m.is_occupied, MapSlot),
    };
    mem_alloc_soa(m.alloc, m.cap, ArraySlice(fields));
  }
}
template<typename Key, typename T> T* map_set(Map<Key, T>& m, Key key, T val) {
  if (m.count >= m.cap*m.LF) { map_grow(m); }
  u64 hash_idx = hash(key);
  u64 idx = ModPow2(hash_idx, m.cap);
  while (m.is_occupied[idx] == MapSlot_Occupied) {
    if (equal(m.keys[idx], key)) break;
    idx = ModPow2(idx + 1, m.cap);
  }
  m.keys[idx] = key;
  m.data[idx] = val;
  m.is_occupied[idx] = MapSlot_Occupied;
  ++m.count;
  return &m.data[idx];
}
template<typename Key, typename T> Result<T> map_get(Map<Key, T>& m, Key key) {
  if (!m.data) return ResultErr();
  u64 hash_idx = hash(key);
  u64 idx = ModPow2(hash_idx, m.cap);
  Loop (i, m.cap) {
    if ((m.is_occupied[idx] == MapSlot_Occupied) && (equal(m.keys[idx], key))) {
      return ResultOk(m.data[idx]);
    } 
    else if (m.is_occupied[idx] == MapSlot_Empty) {
      break;
    }
    idx = ModPow2(idx + 1, m.cap);
  }
  return ResultErr();
}
template<typename Key, typename T> T* map_remove(Map<Key, T>& m, Key key) {
  u64 hash_idx = hash(key);
  u64 idx = ModPow2(hash_idx, m.cap);
  while (m.is_occupied[idx] != MapSlot_Empty) {
    if ((m.is_occupied[idx] == MapSlot_Occupied) && (equal(m.keys[idx] == key))) {
      m.is_occupied[idx] = MapSlot_Deleted;
      --m.count;
      return;
    }
    idx = ModPow2(idx + 1, m.cap);
  }
}
template<typename Key, typename T> T* map_clear(Map<Key, T>& m, Key key) {
  m.count = 0;
  MemZeroArray(m.data, m.cap);
  MemZeroArray(m.keys, m.cap);
  MemZeroArray(m.is_occupied, m.cap);
}

// template<typename T>
// struct HashedStrMap {
//   u32 cap;
//   T* data;
// #if BUILD_DEBUG
//   String* strs;
//   MapSlot* is_occupied;
// #endif
//   void init(Allocator alloc, u32 size) {
// #if BUILD_DEBUG
//     cap = size;
//     data = push_array(alloc, T, size);
//     strs = push_array(alloc, String, size);
//     is_occupied = push_array(alloc, MapSlot, size);
// #else
//     cap = size;
//     data = push_array(alloc, T, size);
// #endif
//   }
//   void add(u64 key, T val, String str = {}) {
// #if BUILD_DEBUG
//     u64 idx = ModPow2(key, cap);
//     Assert(is_occupied[idx] != MapSlot_Occupied);
//     strs[idx] = str;
//     data[idx] = val;
//     is_occupied[idx] = MapSlot_Occupied;
// #else
//     u64 index = ModPow2(key, cap);
//     data[index] = val;
// #endif
//   }
//   T* get(u64 key) {
// #if BUILD_DEBUG
//     u64 idx = ModPow2(key, cap);
//     Assert(is_occupied[idx] == MapSlot_Occupied);
//     return &data[idx];
// #else
//     u64 idx = ModPow2(key, cap);
//     return &data[idx];
// #endif
//   }
//   String get_str(u64 key) {
// #if BUILD_DEBUG
//     u64 idx = ModPow2(key, cap);
//     Assert(is_occupied[idx] == MapSlot_Occupied);
//     return strs[idx];
// #else
//     return {};
// #endif
//   }
// };

////////////////////////////////////////////////////////////////////////
// Sort

///////////////////////////////////
// Insert

template<typename T, typename Cmp> void sort_insert(Slice<T> slice, Cmp cmp) {
  for (i32 i = 1; i < slice.count; ++i) {
    T key = slice[i];
    i32 j = i - 1;
    while (j >= 0 && cmp(key, slice[j])) {
      slice[j + 1] = slice[j];
      j--;
    }
    slice[j + 1] = key;
  }
}
// #define sort_insert_l(data, ...) sort_insert(data, [](var a, var b) __VA_ARGS__)

///////////////////////////////////
// Quick

template<typename T, typename Cmp> i32 lomuto_partition(T* arr, i32 low, i32 high, Cmp cmp) {
  T pivot = arr[high];
  i32 i = low;
  for (i32 j = low; j < high; ++j) {
    if (cmp(arr[j], pivot)) {
      Swap(arr[i], arr[j]);
      i++;
    }
  }
  Swap(arr[i], arr[high]);
  return i;
}
template<typename T, typename Cmp> void _quick_sort(Slice<T> arr, i32 low, i32 high, Cmp cmp) {
  if (low < high) {
    i32 p = lomuto_partition(arr.data, low, high, cmp);
    _quick_sort(arr, low, p - 1, cmp);
    _quick_sort(arr, p + 1, high, cmp);
  }
}
template<typename T, typename Cmp> void sort_quick(Slice<T> arr, Cmp cmp) { _quick_sort(arr, 0, arr.count-1, cmp); }

///////////////////////////////////
// Merge

template<typename T, typename Cmp> void _merge(T* arr, T* tmp, u32 left, u32 mid, u32 right, Cmp cmp) {
  u32 i = left;
  u32 j = mid + 1;
  u32 k = left;
  while (i <= mid && j <= right) {
    if (arr[i] <= arr[j]) {
      tmp[k++] = arr[i++];
    } else {
      tmp[k++] = arr[j++];
    }
  }
  while (i <= mid) {
    tmp[k++] = arr[i++];
  }
  while (j <= right) {
    tmp[k++] = arr[j++];
  }
  for (u32 x = left; x <= right; ++x) {
    arr[x] = tmp[x];
  }
}
template<typename T, typename Cmp> void _merge_sort(T* arr, T* tmp, u32 left, u32 right, Cmp cmp) {
  if (left >= right) return;
  u32 mid = left + (right - left) / 2;
  _merge_sort(arr, tmp, left, mid, cmp);
  _merge_sort(arr, tmp, mid + 1, right, cmp);
  _merge(arr, tmp, left, mid, right, cmp);
}
template<typename T, typename Cmp> void sort_merge(Allocator alloc, Slice<T> arr, Cmp cmp) { Slice tmp = push_slice(alloc, T, arr.count); _merge_sort(arr.data, tmp.data, 0, arr.count-1, cmp); }

///////////////////////////////////
// Radix

u32 sort_i32_key_to_u32(i32 x);
u32 sort_f32_key_to_u32(f32 sort_key);

struct SortEntry {
  u32 sort_key;
  u32 idx;
};

void sort_radix(Allocator alloc, Slice<SortEntry> arr);

////////////////////////////////////////////////////////////////////////
// List sort

template<typename T, typename Cmp> Slice<T> sort_list_insert(Allocator arena, T first, Cmp cmp) {
  var sorted_arr = darray_make<T>(arena);
  for (T it = first; it != 0; it = it->next) {
    darray_add(sorted_arr, it);
  }
  sort_insert(slice(sorted_arr), cmp);
  return slice(sorted_arr);
}
