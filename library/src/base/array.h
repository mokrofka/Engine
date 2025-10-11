#pragma once
#include "defines.h"
#include "logger.h"
#include "mem.h"
#include <initializer_list>

#define DEFAULT_CAPACITY      8
#define DEFAULT_RESIZE_FACTOR 2

////////////////////////////////////////////////////////////////////////
// Array
template<typename T, i32 N>
struct Array {
  i32 count;
  static constexpr i32 cap = N;
  T data[N];
  T* begin() { return data; }
  T* end()   { return data + N; }
  Array() = default;
  Array(std::initializer_list<T> init) {
    Assert(init.size() <= N);
    count = 0;
    for (auto& e : init) data[count++] = e;
  }

  T& operator[](i32 idx) {
    Assert(idx >= 0 && "idx negative!");
    Assert(idx < count && "Idx out of bounds!");
    return data[idx];
  }
};
template<typename T, i32 N> void append(Array<T, N>& a, T b) { 
  Assert(len(a) < a.cap && "Array Full!");
  a.data[a.count++] = b;
}
template<typename T, i32 N, typename... Args> void append(Array<T, N>& a, T first, Args... rest) {
  append(a, first); (append(a, rest), ...);
}
template<typename T, i32 N> void remove(Array<T, N>& a, i32 idx) { 
  Assert(idx >= 0 && "idx negative!");
  Assert(idx < len(a) && "idx out of bounds!");
  a.data[idx] = a.data[--a.count];
}

////////////////////////////////////////////////////////////////////////
// Darray
template <typename T>
struct Darray {
  i32 count;
  i32 cap;
  T* data;
  T* begin() { return data; }
  T* end()   { return data + count; }
  Darray() = default;
  // Darray() {
  //   cap = DEFAULT_CAPACITY;
  //   Assign(data, mem_alloc(sizeof(T)*cap));
  // }
  // Darray(i32 size) {
  //   cap = size;
  //   Assign(data, mem_alloc(sizeof(T)*cap));
  // }
  Darray(std::initializer_list<T> init) {
    count = init.size();
    Assign(data, mem_alloc(sizeof(T)*count));
    int i = 0;
    for (auto& e : init) data[i++] = e;
  }
  T& operator[](i32 idx) {
    Assert(idx >= 0 && "idx negative!");
    Assert(idx < count && "idx out of bounds!");
    return data[idx];
  }
};

template<typename T> void append(Darray<T>& a, T b) { 
  if (len(a) >= a.cap) {
    a.cap = a.cap ? a.cap*DEFAULT_RESIZE_FACTOR : DEFAULT_CAPACITY;
    if (a.data) {
      Assign(a.data, mem_realloc(a.data, sizeof(T)*a.cap));
    } else {
      Assign(a.data, mem_alloc(sizeof(T)*a.cap));
    }
  }
  a.data[a.count++] = b;
}
template<typename T, typename... Args> void append(Darray<T>& a, T first, Args... rest) {
  append(a, first); (append(a, rest), ...);
}
template<typename T> void remove(Darray<T>& a, i32 idx) { 
  Assert(idx >= 0 && "idx negative!");
  Assert(idx < len(a) && "idx out of bounds!");
  a.data[idx] = a.data[--a.count];
}
template<typename T> void reserve(Darray<T>& a, u64 size) { 
  a.cap += size;
  Assign(a.data, mem_realloc(a.data, sizeof(T)*a.cap));
}
template<typename T> void free(Darray<T>& a) {
  mem_free(a.data);
  a.count = 0;
  a.cap = 0;
  a.data = null;
}

////////////////////////////////////////////////////////////////////////
// DarrayArena
template <typename T>
struct DarrayArena {
  i32 count = 0;
  i32 cap = 0;
  T* data = null;
  Arena* arena;
  T* begin() { return data; }
  T* end()   { return data + count; }
  DarrayArena() = default;
  DarrayArena(Arena* arena_) {
    arena = arena_;
  }
  T& operator[](i32 idx) {
    Assert(idx >= 0 && "idx negative!");
    Assert(idx < count && "idx out of bounds!");
    return data[idx];
  }
};

template<typename T> void append(DarrayArena<T>& a, T b) { 
  if (len(a) >= a.cap) {
    a.cap = a.cap ? a.cap*DEFAULT_RESIZE_FACTOR : DEFAULT_CAPACITY;
    if (a.data) {
      T* new_data = push_array(a.arena, T, a.cap);
      MemCopyTyped(new_data, a.data, a.count);
    } else {
      a.data = push_array(a.arena, T, a.cap);
    }
  }
  a.data[a.count++] = b;
}
template<typename T> void remove(DarrayArena<T>& a, i32 idx) { 
  Assert(idx >= 0 && "idx negative!");
  Assert(idx < len(a) && "idx out of bounds!");
  a.data[idx] = a.data[--a.count];
}

////////////////////////////////////////////////////////////////////////
// SparseSet
template <typename T>
struct SparseSet {
  i32 count;
  i32 cap;
  i32* entity_to_index;
  i32* entities;
  T* data;
  void add(u32 id) {
    Assert(count < cap);
    entity_to_index[id] = count;
    entities[count] = id;
    ++count;
  }
  void add(u32 id, T element) {
    Assert(count < cap);
    entity_to_index[id] = count;
    entities[count] = id;
    data[count] = element;
    ++count;
  }
  void remove(u32 id) {
    u32 idx_of_removed_entity = entity_to_index[id];
    u32 idx_of_last_element = count - 1;
    data[idx_of_removed_entity] = data[idx_of_last_element];

    u32 last_entity = entities[idx_of_last_element];

    entity_to_index[last_entity] = idx_of_removed_entity;
    entities[idx_of_removed_entity] = last_entity;

    --count;
  }
};

////////////////////////////////////////////////////////////////////////
// HandlerPool
struct HandlerPool {
  u32 count;
  u32 cap;
  u32* handlers;
  DebugDo(b8* used);
};

static HandlerPool make_handle_pool() {
  HandlerPool result = {
    .cap = DEFAULT_CAPACITY,
    .handlers = mem_alloc_typed(u32, DEFAULT_CAPACITY),
  };
  Loop (i, DEFAULT_CAPACITY) {
    result.handlers[i] = i;
  }
  DebugDo(
    result.used = mem_alloc_typed(b8, DEFAULT_CAPACITY);
    MemZero(result.used, DEFAULT_CAPACITY);
  );
  return result;
}
static u32 append(HandlerPool& h) {
  u32 cap_old = h.cap;
  if (h.count >= h.cap) {
    h.cap *= DEFAULT_RESIZE_FACTOR;
    h.handlers = mem_realloc_typed(h.handlers, u32, h.cap);
    for (i32 i = cap_old; i < h.cap; ++i) {
      h.handlers[i] = i;
    }
  }
  DebugDo(
    if (h.count >= cap_old) {
      h.used = mem_realloc_typed_zero(h.used, b8, h.cap);
    }
    h.used[h.count] = true
  );
  u32 id = h.handlers[h.count];
  ++h.count;
  return id;
}
static void remove(HandlerPool& h, u32 id) {
  Assert(IsBetween(0, id, h.cap-1));
  DebugDo(
    Assert(h.used[id])
    h.used[id] = false;
  );
  h.handlers[--h.count] = id;
}

template<typename T, i32 N> INLINE i32 len(Array<T, N> a)    { return a.count; }
template<typename T>        INLINE i32 len(Darray<T> a)      { return a.count; }
template<typename T>        INLINE i32 len(DarrayArena<T> a) { return a.count; }
                            INLINE i32 len(const char* a)    { return String(a).size; }


