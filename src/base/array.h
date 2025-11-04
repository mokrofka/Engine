#pragma once
#include "defines.h"
#include "logger.h"
#include "mem.h"
#include "maths.h"

#include <initializer_list>

#define IsInsideArray(x, upper) (((0) <= (x)) && ((x) < (upper)))

////////////////////////////////////////////////////////////////////////
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

  T& operator[](u32 idx) {
    Assert(IsInsideArray(idx, cap));
    return data[idx];
  }
};

template<typename T, i32 N> void append(Array<T, N>& a, T b) { 
  Assert(len(a) < a.cap && "array full");
  a.data[a.count++] = b;
}
template<typename T, i32 N, typename... Args> void append(Array<T, N>& a, T first, Args... rest) {
  append(a, first);
  (append(a, rest), ...);
}
template<typename T, i32 N> void remove(Array<T, N>& a, i32 idx) { 
  Assert(IsInsideArray(idx, len(a)));
  a.data[idx] = a.data[--a.count];
}
template<typename T, i32 N> b32 exists(Array<T, N>& a, T e) { 
  for (T x : a) {
    if (x == e) return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////
// Darray
template <typename T>
struct Darray {
  u32 count = 0;
  u32 cap = 0;
  T* data = null;

  T* begin() { return data; }
  T* end()   { return data + count; }

  Darray() = default;

  Darray(std::initializer_list<T> init) {
    count = init.size();
    cap = next_pow2(count);
    Assign(data, mem_alloc_typed(T, cap));
    u32 i = 0;
    for (T x : init) {
      data[i++] = x;
    }
  }
  
  T& operator[](u32 idx) {
    Assert(IsInsideArray(idx, cap));
    return data[idx];
  }
};

template<typename T> void append(Darray<T>& a, T b) { 
  if (len(a) >= a.cap) {
    if (a.data) {
      a.cap *= DEFAULT_RESIZE_FACTOR;
      Assign(a.data, mem_realloc(a.data, sizeof(T)*a.cap));
    } else {
      a.cap = DEFAULT_CAPACITY;
      Assign(a.data, mem_alloc(sizeof(T)*a.cap));
    }
  }
  a.data[a.count++] = b;
}
template<typename T, typename... Args> void append(Darray<T>& a, T first, Args... rest) {
  append(a, first);
  (append(a, rest), ...);
}
template<typename T> void remove(Darray<T>& a, u32 idx) { 
  Assert(IsInsideArray(idx, a.cap));
  a.data[idx] = a.data[--a.count];
}
template<typename T> void reserve(Darray<T>& a, u32 size) { 
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
  u32 count = 0;
  u32 cap = 0;
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

// TODO: make array index

////////////////////////////////////////////////////////////////////////
// SparseSet
template <typename T>
struct SparseSet {
  u32 count = 0;
  u32 cap = 0;
  u32* entity_to_index = null;
  u32* entities = null;
  T* data;

  T* begin() { return data; }
  T* end()   { return data + count; }

  void add(u32 id) {
    if (count >= cap) {
      grow();
    }
    Assert(count < cap);
    entity_to_index[id] = count;
    entities[count] = id;
    ++count;
  }
  void add(u32 id, T element) {
    if (count >= cap) {
      grow();
    }
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
  T* get(u32 id) {
    u32 index = entity_to_index[id];
    return &data[index];
  }
  void grow() {
    if (data) {
      cap *= DEFAULT_RESIZE_FACTOR;
      u64 size = (sizeof(i32) + sizeof(i32) + sizeof(T)) * cap;
      u8* buff = mem_realloc(entity_to_index, size);

      entity_to_index = (u32*)buff;
      entities = (u32*)Offset(entity_to_index, sizeof(u32) * cap);
      data = (T*)Offset(entities, sizeof(u32) * cap);
    }
    else {
      cap = DEFAULT_CAPACITY;
      u64 size = (sizeof(i32) + sizeof(i32) + sizeof(T)) * cap;
      u8* buff = mem_alloc(size);

      entity_to_index = (u32*)buff;
      entities = (u32*)Offset(entity_to_index, sizeof(u32) * cap);
      data = (T*)Offset(entities, sizeof(u32) * cap);
    }
  }
};

// TODO: make id check to grow entity_to_index array 
struct SparseSetIndex {
  u32 count = 0;
  u32 cap = 0;
  u32* entity_to_index = null;
  u32* entities = null;

  u32* begin() { return entities; }
  u32* end()   { return entities + count; }

  void add(u32 id) {
    if (count >= cap) {
      grow();
    }
    Assert(count < cap);
    entity_to_index[id] = count;
    entities[count] = id;
    ++count;
  }
  void remove(u32 id) {
    u32 idx_of_removed_entity = entity_to_index[id];
    u32 idx_of_last_element = count - 1;

    u32 last_entity = entities[idx_of_last_element];

    entity_to_index[last_entity] = idx_of_removed_entity;
    entities[idx_of_removed_entity] = last_entity;

    --count;
  }
  void grow() {
    if (entity_to_index) {
      cap *= DEFAULT_RESIZE_FACTOR;
      u64 size = (sizeof(u32) + sizeof(u32)) * cap;
      u8* buff = mem_realloc(entity_to_index, size);

      entity_to_index = (u32*)buff;
      entities = (u32*)Offset(entity_to_index, sizeof(u32) * cap);
    }
    else {
      cap = DEFAULT_CAPACITY;
      u64 size = (sizeof(u32) + sizeof(u32)) * cap;
      u8* buff = mem_alloc(size);

      entity_to_index = (u32*)buff;
      entities = (u32*)Offset(entity_to_index, sizeof(u32) * cap);
    }
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

template<typename T, i32 N> INLINE u32 len(Array<T, N> a)    { return a.count; }
template<typename T>        INLINE u32 len(Darray<T> a)      { return a.count; }
template<typename T>        INLINE u32 len(DarrayArena<T> a) { return a.count; }
                            INLINE u32 len(const char* a)    { return String(a).size; }


