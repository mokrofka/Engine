#pragma once
#include "base.h"
#include "logger.h"
#include "mem.h"

const u32 INDEX_BITS = 22;
const u32 INDEX_MASK = (1u << INDEX_BITS) - 1;

inline u32 id_idx(u32 h) { return h & INDEX_MASK; }
inline u32 id_generation(u32 h) { return h >> INDEX_BITS; }
inline u32 id_make(u32 generation, u32 idx) { return (generation << INDEX_BITS) | idx; }
inline u32 generation_bitmask(u32 gen) { return gen & Bit(32 - INDEX_BITS) - 1; }

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
template<typename T, i32 N> u32 array_push_empty(Array<T, N>& arr) {
  Assert(arr.count < arr.cap);
  return arr.count++;
}
template<typename T, i32 N> u32 array_push(Array<T, N>& arr, T a) {
  Assert(arr.count < arr.cap);
  arr.data[arr.count] = a;
  return arr.count++;
}
template<typename T, i32 N, typename ... Args> void array_push(Array<T, N>& arr, Args... args) {
  var list = {args...};
  for (T x : list) {
    array_push(arr, x);
  }
}
template<typename T, i32 N> void array_push_elems(Array<T, N>& arr, Slice<T> elems) {
  Assert(arr.count + elems.count <= arr.cap);
  MemCopyArray(arr.data + arr.count, elems.data, elems.count);
  arr.count += elems.count;
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

///////////////////////////////////
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
#define array_make(T, alloc) _array_make<T>(alloc)
template<typename T> Darray<T> _array_make(Allocator alloc) {
  Darray<T> res = {
    .alloc = alloc,
  };
  return res;
}
template<typename T> void array_destroy(Darray<T>& arr) {
  if (arr.data) { mem_free(arr.alloc, arr.data); } 
}
template<typename T> void array_grow(Darray<T>& arr, u32 elem_count) {
  if (arr.data) {
    u32 old_cap = arr.cap;
    arr.cap = Max(arr.cap * DEFAULT_RESIZE_FACTOR, arr.count + elem_count);
    arr.data = mem_realloc_array(arr.alloc, arr.data, old_cap, arr.cap);
  } else {
    arr.cap = Max(DEFAULT_CAPACITY, elem_count);
    arr.data = push_array(arr.alloc, T, arr.cap);
  }
}
template<typename T> void array_reserve(Darray<T>& arr, u32 min_cap) {
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
template<typename T> T array_clone(Darray<T>& arr, Allocator alloc) {
  Darray<T> result = {
    .count = arr.count,
    .cap = arr.cap,
    .alloc = alloc,
    .data = push_array(alloc, T, arr.cap),
  };
  MemCopyArray(result.data, arr.data, arr.count);
  return result;
}
template<typename T> u32 array_push_empty(Darray<T>& arr) {
  if (arr.count >= arr.cap) {
    array_grow(arr, 0);
  }
  return arr.count++;
}
template<typename T> u32 array_push(Darray<T>& arr, T a) {
  if (arr.count >= arr.cap) {
    array_grow(arr, 0);
  }
  arr.data[arr.count] = a;
  return arr.count++;
}
template<typename T, typename...Args> void array_push(Darray<T>& arr, Args...args) {
  var list = { args... };
  for (T x : list) {
    array_push(arr, x);
  }
}
template<typename T> void array_push_elems(Darray<T>& arr, Slice<T> elems) {
  if (arr.count + elems.count >= arr.cap) {
    array_grow(arr, elems.count);
  }
  MemCopyArray(arr.data + arr.count, elems.data, elems.count);
  arr.count += elems.count;
}
template<typename T> void array_swap_remove(Darray<T>& arr, u32 idx) {
  Assert(idx < arr.count);
  arr.data[idx] = arr.data[--arr.count];
}
template<typename T> void array_clear(Darray<T>& arr) {
  arr.count = 0; 
}
template<typename T> T array_pop(Darray<T>& arr) {
  return arr.data[--arr.count];
}
template<typename T> T array_back(Darray<T>& arr) {
  return arr.data[arr.count-1];
}
template<typename T> b32 array_exists(Darray<T>& arr, T a, b32(*fn)(T a, T b) = equal) {
  Loop (i, arr.count) {
    T x = arr[i];
    if (fn(x, a)) return true;
  }
  return false;
}
template<typename T> b32 array_exists_at(Darray<T>& arr, T a, u32* out_idx, b32(*fn)(T a, T b) = equal) {
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

template<typename T, i32 N, typename Handle>
struct ArrayHandler {
  static constexpr i32 cap = N;
  u32 count;
  u32 sparse[N];
  u32 dense[N];
  T data[N];
  u32 generations[N];
};

template<typename T, i32 N, typename Handle> T& array_handler_get(ArrayHandler<T, N, Handle>& a, Handle h) {
  Assert(h.idx < a.count);
  Assert(a.generations[h.idx]== id_generation(h.gen));
  u32 idx = a.sparse[h.idx];
  return a.data[idx];
}
template<typename T, i32 N, typename Handle> Handle array_handler_push(ArrayHandler<T, N, Handle>& a, T elem) {
  Assert(a.count < a.cap);
  u32 idx = a.count++;
  a.sparse[idx] = idx;
  a.dense[idx] = idx;
  a.data[idx] = elem;
  Handle res = {idx, a.generations[idx]};
  return res;
}
template<typename T, i32 N, typename Handle> void array_handler_remove(ArrayHandler<T, N, Handle>& a, Handle h) {
  Assert(h.idx < a.count);
  Assert(a.generations[h.idx]++ == h.gen);
  u32 idx_removed = a.sparse[h.idx];
  u32 idx_last = a.count - 1;
  a.data[idx_removed] = a.data[idx_last];
  u32 last_entity = a.dense[idx_last];
  a.sparse[last_entity] = idx_removed;
  a.dense[idx_removed] = last_entity;
  --a.count;
}
template<typename T, i32 N, typename Handle> void array_handler_clear(ArrayHandler<T, N, Handle>& arr) {
  arr.count = 0;
}

///////////////////////////////////
// DarrayHandler

template <typename T, typename Handle>
struct DarrayHandler {
  u32 count;
  u32 cap;
  Allocator alloc;
  u32* sparse;
  u32* dense;
  T* data;
  u32* generations;
};

#define array_handler_make(T, H, alloc) _array_handler_make<T, H>(alloc)
template<typename T, typename Handle> DarrayHandler<T, Handle> _array_handler_make(Allocator alloc) {
  DarrayHandler<T, Handle> res = {
    .alloc = alloc,
  };
  return res;
}
template<typename T, typename Handle> T& array_handler_get(DarrayHandler<T, Handle>& a, Handle h) {
  Assert(h.idx < a.cap);
  Assert(a.generations[h.idx] == h.gen);
  u32 idx = a.sparse[h.idx];
  return a.data[idx];
}
template<typename T, typename Handle> void array_handler_grow(DarrayHandler<T, Handle>& a) {
  if (a.data) {
    u32 cap_old = a.cap;
    a.cap *= DEFAULT_RESIZE_FACTOR;
    SoA_Field fields[] = {
      SoA_push_field(a.sparse),
      SoA_push_field(a.dense),
      SoA_push_field(a.data),
      SoA_push_field(a.generations),
    };
    mem_realloc_soa(a.alloc, cap_old, a.cap, slice(fields));
    MemZeroArray(a.generations+cap_old, a.cap-cap_old);
  } else {
    a.cap = DEFAULT_CAPACITY;
    SoA_Field fields[] = {
      SoA_push_field(a.sparse),
      SoA_push_field(a.dense),
      SoA_push_field(a.data),
      SoA_push_field(a.generations),
    };
    mem_alloc_soa(a.alloc, a.cap, slice(fields));
    MemZeroArray(a.generations, a.cap);
  }
}
template<typename T, typename Handle> Handle array_handler_push(DarrayHandler<T, Handle>& a, T elem) {
  if (a.count >= a.cap) {
    array_handler_grow(a);
  }
  u32 idx = a.count++;
  a.sparse[idx] = idx;
  a.dense[idx] = idx;
  a.data[idx] = elem;
  Handle res = {idx, a.generations[idx]};
  return res;
}
template<typename T, typename Handle> void array_handler_remove(DarrayHandler<T, Handle>& a, Handle h) {
  Assert(a.generations[h.idx]++ == h.gen);
  u32 idx_removed = a.sparse[h.idx];
  u32 idx_last = a.count - 1;
  a.data[idx_removed] = a.data[idx_last];
  u32 last_entity = a.dense[idx_last];
  a.sparse[last_entity] = idx_removed;
  a.dense[idx_removed] = last_entity;
  --a.count;
}
template<typename T, typename Handle> void array_handler_clear(DarrayHandler<T, Handle>& arr) {
  arr.count = 0;
}

////////////////////////////////////////////////////////////////////////
// Pool

template<typename T, i32 N, typename Handle>
struct Pool {
  static_assert(sizeof(T) >= 4);
  u32 head;
  static constexpr i32 cap = N;
  u32 max_idx;
  struct {
    union {
      T elem;
      u32 next_free;
    };
    u32 gen;
  } data[N];
};

template<typename T, i32 N, typename Handle> T& pool_get(Pool<T, N, Handle>& p, Handle h) {
  Assert(pool_is_valid_handle(p, h));
  return p.data[h.idx].elem;
}
template<typename T, i32 N, typename Handle> Handle pool_push_empty(Pool<T, N, Handle>& p) {
  u32 idx = p.head;
  if (idx > 0) {
    p.head = p.data[idx].next_free;
  } else {
    idx = ++p.max_idx;
    Assert(idx < p.cap);
  }
  Handle res = {idx, p.data[idx].gen};
  return res;
}
template<typename T, i32 N, typename Handle> Handle pool_push(Pool<T, N, Handle>& p, T a) {
  Handle h = pool_push_empty(p);
  p.data[h.idx].elem = a;
  return h;
}
template<typename T, i32 N, typename Handle> void pool_remove(Pool<T, N, Handle>& p, Handle h) {
  Assert(pool_is_valid_handle(p, h));
  ++p.data[h.idx].gen;
  p.data[h.idx].next_free = p.head;
  p.head = h.idx;
}
template<typename T, i32 N, typename Handle> u32 pool_clear(Pool<T, N, Handle>& p) {
  p.head = 0;
  p.max_idx = 0;
  ArrayZero(p.data);
}
template<typename T, i32 N, typename Handle> b32 pool_is_valid_handle(Pool<T, N, Handle>& p, Handle h) {
  if (h.idx <= 0 || h.idx > p.max_idx) {
    return false;
  }
  if (p.data[h.idx].gen != h.gen) {
    return false;
  }
  return true;
}

///////////////////////////////////
// Dpool

template<typename T, typename Handle>
struct Dpool {
  static_assert(sizeof(T) >= 4);
  u32 head;
  u32 cap;
  u32 max_idx;
  Allocator alloc;
  struct  {
    union {
      T elem;
      u32 next_free;
    };
    u32 gen;
  } *data;
};

#define pool_make(T, H, alloc) _pool_make<T, H>(alloc)
template<typename T, typename Handle> Dpool<T, Handle> _pool_make(Allocator alloc) {
  Dpool<T, Handle> res = {
    .alloc = alloc,
  };
  return res;
}
template<typename T, typename Handle> T& pool_get(Dpool<T, Handle>& p, Handle h) {
  Assert(pool_is_valid_handle(p, h));
  return p.data[h.idx].elem;
}
template<typename T, typename Handle> void pool_grow(Dpool<T, Handle>& p) {
  if (p.data) {
    u32 cap_old = p.cap;
    p.cap *= DEFAULT_RESIZE_FACTOR;
    SoA_Field fields[] = {
      SoA_push_field(p.data),
    };
    mem_realloc_soa_zero(p.alloc, cap_old, p.cap, slice(fields));
  } else {
    p.cap = DEFAULT_CAPACITY;
    SoA_Field fields[] = {
      SoA_push_field(p.data),
    };
    mem_alloc_soa_zero(p.alloc, p.cap, slice(fields));
  }
}
template<typename T, typename Handle> Handle pool_push_empty(Dpool<T, Handle>& p) {
  u32 idx = p.head;
  if (idx > 0) {
    p.head = p.data[idx].next_free;
  } else {
    idx = ++p.max_idx;
    if (idx >= p.cap) {
      pool_grow(p);
    }
  }
  Handle res = {idx, p.data[idx].gen};
  return res;
}
template<typename T, typename Handle> Handle pool_push(Dpool<T, Handle>& p, T a) {
  Handle h = pool_push_empty(p);
  p.data[h.idx].elem = a;
  return h;
}
template<typename T, typename Handle> void pool_remove(Dpool<T, Handle>& p, Handle h) {
  pool_is_valid_handle(p, h);
  ++p.data[h.idx].gen;
  p.data[h.idx].next_free = p.head;
  p.head = h.idx;
}
template<typename T, typename Handle> u32 pool_clear(Dpool<T, Handle>& p) {
  p.head = 0;
  p.max_idx = 0;
  MemZeroArray(p.data, p.max_idx);
}
template<typename T, typename Handle> b32 pool_is_valid_handle(Dpool<T, Handle>& p, Handle h) {
  if (h.idx <= 0 || h.idx > p.max_idx) {
    return false;
  }
  if (p.data[h.idx].gen != h.gen) {
    return false;
  }
  return true;
}

///////////////////////////////////
// PoolLinkList

template<typename T, i32 N, typename Handle>
struct PoolLinkList {
  static_assert(sizeof(T) >= 4);
  u32 head;
  static constexpr i32 cap = N;
  u32 first;
  u32 last;
  u32 max_idx;
  struct {
    union {
      T elem;
      u32 next_free;
    };
    u32 next;
    u32 prev;
    u32 gen;
  } data[N];
};

template<typename T, i32 N, typename Handle> T& pool_get(PoolLinkList<T, N, Handle>& p, Handle h) {
  Assert(pool_is_valid_handle(p, h));
  return p.data[h.idx].elem;
}
template<typename T, i32 N, typename Handle> Handle pool_push(PoolLinkList<T, N, Handle>& p) {
  u32 idx = p.head;
  if (idx > 0) {
    p.head = p.data[idx].next_free;
  } else {
    idx = ++p.max_idx;
    Assert(idx < p.cap);
  }
  idll_list_push_back(p.data, p, idx);
  Handle res = {idx, p.data[idx].gen};
  return res;
}
template<typename T, i32 N, typename Handle> Handle pool_push(PoolLinkList<T, N, Handle>& p, T a) {
  Handle h = pool_push(p);
  p.data[h.idx].elem = a;
  return h;
}
template<typename T, i32 N, typename Handle> void pool_remove(PoolLinkList<T, N, Handle>& p, Handle h) {
  Assert(pool_is_valid_handle(p, h));
  ++p.data[h.idx].gen;
  p.data[h.idx].next_free = p.head;
  p.head = h.idx;
  idll_list_remove(p.data, p, h.idx);
}
template<typename T, i32 N, typename Handle> Handle pool_get_handle(PoolLinkList<T, N, Handle>& p, u32 idx) {
  Handle res = {idx, p.data[idx].gen};
  return res;
}
template<typename T, i32 N, typename Handle> void pool_clear(PoolLinkList<T, N, Handle>& p) {
  p.head = 0;
  p.max_idx = 0;
  MemZeroArray(p.data, p.max_idx);
}
template<typename T, i32 N, typename Handle> b32 pool_is_valid_handle(PoolLinkList<T, N, Handle>& p, Handle h) {
  if (h.idx <= 0 || h.idx > p.max_idx) {
    return false;
  }
  if (p.data[h.idx].gen != h.gen) {
    return false;
  }
  return true;
}

///////////////////////////////////
// DpoolLinkList

template<typename T, typename Handle>
struct DpoolLinkList {
  static_assert(sizeof(T) >= 4);
  u32 head;
  u32 cap;
  Allocator alloc;
  u32 first;
  u32 last;
  u32 max_idx;
  struct {
    union {
      T elem;
      u32 next_free;
    };
    u32 next;
    u32 prev;
    u32 gen;
  } *data;
};

#define pool_linklist_make(T, H, alloc) _pool_linklist_make<T, H>(alloc)
template<typename T, typename Handle> DpoolLinkList<T, Handle> _pool_linklist_make(Allocator alloc) {
  DpoolLinkList<T, Handle> res = {
    .alloc = alloc,
  };
  return res;
}
template<typename T, typename Handle> T& pool_get(DpoolLinkList<T, Handle>& p, Handle h) {
  Assert(pool_is_valid_handle(p, h));
  return p.data[h.idx].elem;
}
template<typename T, typename Handle> void pool_grow(DpoolLinkList<T, Handle>& p) {
  if (p.data) {
    u32 cap_old = p.cap;
    p.cap *= DEFAULT_RESIZE_FACTOR;
    SoA_Field fields[] = {
      SoA_push_field(p.data),
    };
    mem_realloc_soa_zero(p.alloc, cap_old, p.cap, slice(fields));
  } else {
    p.cap = DEFAULT_CAPACITY;
    SoA_Field fields[] = {
      SoA_push_field(p.data),
    };
    mem_alloc_soa_zero(p.alloc, p.cap, slice(fields));
  }
}
template<typename T, typename Handle> Handle pool_push(DpoolLinkList<T, Handle>& p) {
  u32 idx = p.head;
  if (idx > 0) {
    p.head = p.data[idx].next_free;
  } else {
    idx = ++p.max_idx;
    if (idx >= p.cap) {
      pool_grow(p);
    }
  }
  idll_list_push_back(p.data, p, idx);
  Handle res = {idx, p.data[idx].gen};
  return res;
}
template<typename T, typename Handle> Handle pool_push(DpoolLinkList<T, Handle>& p, T a) {
  Handle h = pool_push(p);
  p.data[h.idx].elem = a;
  return h;
}
template<typename T, typename Handle> void pool_remove(DpoolLinkList<T, Handle>& p, Handle h) {
  pool_is_valid_handle(p, h);
  ++p.data[h.idx].gen;
  p.data[h.idx].next_free = p.head;
  p.head = h.idx;
  idll_list_remove(p.data, p, h.idx);
}
template<typename T, typename Handle> Handle pool_get_handler(DpoolLinkList<T, Handle>& p, u32 idx) {
  Handle res = {idx, p.data[idx].gen};
  return res;
}
template<typename T, typename Handle> void pool_clear(DpoolLinkList<T, Handle>& p) {
  p.head = 0;
  p.max_idx = 0;
  MemZeroArray(p.data, p.max_idx);
}
template<typename T, typename Handle> b32 pool_is_valid_handle(DpoolLinkList<T, Handle>& p, Handle h) {
  if (h.idx <= 0 || h.idx > p.max_idx) {
    return false;
  }
  if (p.data[h.idx].gen != h.gen) {
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////
// Queue

template<typename T, i32 N>
struct Queue {
  static constexpr u32 cap = N;
  u32 count;
  u32 first;
  T data[N];
};

template<typename T, i32 N> void queue_push(Queue<T, N>& q, T elem) {
  Assert(q.count < q.cap);
  u32 idx = (q.first + q.count++) % q.cap;
  q.data[idx] = elem;
}

template<typename T, i32 N> void queue_push_front(Queue<T, N>& q, T elem) {
  Assert(q.count < q.cap);
  q.first = (q.first + q.cap - 1) % q.cap;
  q.data[q.first] = elem;
  ++q.count;
}

template<typename T, i32 N> T queue_pop(Queue<T, N>& q) {
  Assert(q.count > 0);
  T res = q.data[q.first];
  q.first = (q.first + 1) % q.cap;
  --q.count;
  return res;
}

template<typename T, i32 N> T queue_pop_back(Queue<T, N>& q) {
  Assert(q.count > 0);
  --q.count;
  u32 idx = (q.first + q.count) % q.cap;
  T res = q.data[idx];
  return res;
}

template<typename T, i32 N> T queue_back(Queue<T, N>& q) {
  Assert(q.count);
  u32 idx = (q.first + q.count - 1) % q.cap;
  return q.data[idx];
}

template<typename T, i32 N> T queue_front(Queue<T, N>& q) {
  Assert(q.count);
  return q.data[q.first];
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
//       mem_realloc_soa(alloc, cap_old, cap, slice(fields));
//     }
//     else {
//       cap = DEFAULT_CAPACITY;
//       SoA_Field fields[] = {
//         SoA_push_field(&dense, u32),
//         SoA_push_field(&data, T),
//       };
//       mem_alloc_soa(alloc, cap, slice(fields));
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
void id_pool_destroy(IdPool& p);
void id_pool_clear(IdPool& p);

template<i32 N>
struct StaticIdPool {
  u32 count;
  static constexpr i32 cap = N;
  u32 ids[N];
#if BUILD_DEBUG
  u32 generations[N];
#endif
};

template<i32 N> void id_pool_init(StaticIdPool<N>& p) {
#if BUILD_DEBUG
  p.generations[0] = 1;
#endif
  Loop (i, p.cap) {
    p.ids[i] = i;
  }
}
template<i32 N> u32 id_pool_push(StaticIdPool<N>& p) {
  Assert(p.count + 1 <= p.cap);
#if BUILD_DEBUG
  u32 res = id_make(p.generations[p.count], p.count++);
  return res;
#else
  return p.ids[p.count++];
#endif
}
template<i32 N> void id_pool_remove(StaticIdPool<N>& p, u32 h) {
  u32 idx = id_idx(h);
  Assert(generation_bitmask(p.generations[idx]++) == id_generation(h));
  p.ids[--p.count] = idx;
}
template<i32 N> void id_pool_clear(StaticIdPool<N>& p) {
  p.count = 0;
}

////////////////////////////////////////////////////////////////////////
// Map

enum MapSlot : u8 {
  MapSlot_Empty,
  MapSlot_Occupied,
  MapSlot_Deleted
};

template<typename Key, typename T, i32 N>
struct Map {
  u32 count;
  static constexpr u32 cap = N;
  T data[N];
  Key keys[N];
  MapSlot is_occupied[N];
};

template<typename Key, typename T, i32 N> T* map_set(Map<Key, T, N>& m, Key key, T val) {
  Assert(m.count < m.cap);
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
template<typename Key, typename T, i32 N> Result<T> map_get(Map<Key, T, N>& m, Key key) {
  u64 hash_idx = hash(key);
  u64 idx = ModPow2(hash_idx, m.cap);
  Loop (i, m.cap) {
    if ((m.is_occupied[idx] == MapSlot_Occupied) && (equal(m.keys[idx], key))) {
      return m.data[idx];
    } 
    else if (m.is_occupied[idx] == MapSlot_Empty) {
      break;
    }
    idx = ModPow2(idx + 1, m.cap);
  }
  return Err();
}
template<typename Key, typename T, i32 N> void map_remove(Map<Key, T, N>& m, Key key) {
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
template<typename Key, typename T, i32 N> void map_clear(Map<Key, T, N>& m, Key key) {
  m.count = 0;
  u8* start = (u8*)m.data;
  u8* end = Offset(m.is_occupied, m.cap * sizeof(MapSlot));
  MemZero(start, u64(end - start));
}

///////////////////////////////////
// Dmap

template<typename Key, typename T>
struct Dmap {
  static constexpr f32 LF = 0.8;
  u32 count;
  u32 cap;
  Allocator alloc;
  T* data;
  Key* keys;
  MapSlot* is_occupied;
};

#define map_make(K, V, alloc) _map_make<K, V>(alloc)
template<typename Key, typename T> Dmap<Key, T> _map_make(Allocator alloc) {
  Dmap<Key, T> res = {
    .alloc = alloc,
  };
  return res;
}
template<typename Key, typename T> void map_grow(Dmap<Key, T>& m) {
  if (m.data) {
    T* old_data = m.data;
    Key* old_keys = m.keys;
    MapSlot* old_is_occupied = m.is_occupied;
    u32 old_cap = m.cap;
    m.cap *= DEFAULT_RESIZE_FACTOR;
    SoA_Field fields[] = {
      SoA_push_field(m.data),
      SoA_push_field(m.keys),
      SoA_push_field(m.is_occupied),
    };
    mem_alloc_soa(m.alloc, m.cap, slice(fields));
    Loop (i, old_cap) {
      if (old_is_occupied[i] == MapSlot_Occupied) {
        map_set(m, old_keys[i], old_data[i]);
      }
    }
    mem_free(m.alloc, old_data, mem_soa_size(old_cap, slice(fields)));
  }
  else {
    m.cap = DEFAULT_CAPACITY;
    SoA_Field fields[] = {
      SoA_push_field(m.data),
      SoA_push_field(m.keys),
      SoA_push_field(m.is_occupied),
    };
    mem_alloc_soa(m.alloc, m.cap, slice(fields));
  }
}
template<typename Key, typename T> T* map_set(Dmap<Key, T>& m, Key key, T val) {
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
template<typename Key, typename T> Result<T> map_get(Dmap<Key, T>& m, Key key) {
  if (!m.data) return Err();
  u64 hash_idx = hash(key);
  u64 idx = ModPow2(hash_idx, m.cap);
  Loop (i, m.cap) {
    if ((m.is_occupied[idx] == MapSlot_Occupied) && (equal(m.keys[idx], key))) {
      return m.data[idx];
    } 
    else if (m.is_occupied[idx] == MapSlot_Empty) {
      break;
    }
    idx = ModPow2(idx + 1, m.cap);
  }
  return Err();
}
template<typename Key, typename T> void map_remove(Dmap<Key, T>& m, Key key) {
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
template<typename Key, typename T> void map_clear(Dmap<Key, T>& m, Key key) {
  m.count = 0;
  u8* start = (u8*)m.data;
  u8* end = Offset(m.is_occupied, m.cap * sizeof(MapSlot));
  MemZero(start, u64(end - start));
}

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

///////////////////////////////////
// Quick

template<typename T, typename Cmp> i32 _lomuto_partition(T* arr, i32 low, i32 high, Cmp cmp) {
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
    i32 p = _lomuto_partition(arr.data, low, high, cmp);
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
  var sorted_arr = array_make(T, arena);
  for (T it = first; it != 0; it = it->next) {
    array_push(sorted_arr, it);
  }
  sort_insert(slice(sorted_arr), cmp);
  return slice(sorted_arr);
}
