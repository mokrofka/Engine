#pragma once
#include "defines.h"
#include "logger.h"
#include "mem.h"
#include <initializer_list>

template<typename T, i32 N>
struct Array
{
  static constexpr i32 cap = N;
  i32 count = 0;
  T data[N];
  T* begin() { return data; }
  T* end()   { return data + N; }
  Array() = default;
  Array(std::initializer_list<T> init) {
    Assert(init.size() <= N);
    count = init.size();
    int i = 0;
    for (auto& e : init) data[i++] = e;
  }

  T& operator[](i32 idx) {
    Assert(idx >= 0 && "idx negative!");
    Assert(idx < count && "Idx out of bounds!");
    return data[idx];
  }
};

#define DEFAULT_ARRAY_CAPACITY      8
#define DEFAULT_ARRAY_RESIZE_FACTOR 2
template <typename T>
struct Darray {
  i32 count = 0;
  i32 cap = 0;
  T* data = null;
  T* begin() { return data; }
  T* end()   { return data + count; }
  Darray() {
    cap = DEFAULT_ARRAY_CAPACITY;
    Assign(data, mem_alloc(sizeof(T)*cap));
  }
  Darray(i32 size) {
    cap = size;
    Assign(data, mem_alloc(sizeof(T)*cap));
  }
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

template <typename T>
struct DarrayArena {
  i32 count = 0;
  i32 cap = 0;
  T* data = null;
  Arena* arena;
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

template<typename T, i32 N>
void append(Array<T, N>& a, T b) { 
  Assert(len(a) < a.cap && "Array Full!");
  a.data[a.count++] = b;
}
template<typename T, i32 N, typename... Args>
void append(Array<T, N>& a, T first, Args... rest) {
  append(a, first);
  (append(a, rest), ...);
}
template<typename T>
void append(Darray<T> a, T b) { 
  if (len(a) >= a.cap) {
    a.cap = a.cap ? a.cap*DEFAULT_ARRAY_RESIZE_FACTOR : DEFAULT_ARRAY_CAPACITY;
    if (a.data) {
      Assign(a.data, mem_realloc(a.data, sizeof(T)*a.cap));
    } else {
      Assign(a.data, mem_alloc(sizeof(T)*a.cap));
    }
  }
  a.data[a.count++] = b;
}
template<typename T, typename... Args>
void append(Darray<T>& a, T first, Args... rest) {
  append(a, first);
  (append(a, rest), ...);
}
template<typename T>
void append(DarrayArena<T>& a, T b) { 
  if (len(a) >= a.cap) {
    a.cap = a.cap ? a.cap*DEFAULT_ARRAY_RESIZE_FACTOR : DEFAULT_ARRAY_CAPACITY;
    if (a.data) {
      T* new_data = push_array(a.arena, T, a.cap);
      MemCopyTyped(new_data, a.data, a.count);
    } else {
      a.data = push_array(a.arena, T, a.cap);
    }
  }
  a.data[a.count++] = b;
}

template<typename T, i32 N>
void remove(Array<T, N>& a, i32 idx) { 
  Assert(idx >= 0 && "idx negative!");
  Assert(idx < len(a) && "idx out of bounds!");
  a.data[idx] = a.data[--a.count];
}
template<typename T>
void remove(Darray<T>& a, i32 idx) { 
  Assert(idx >= 0 && "idx negative!");
  Assert(idx < len(a) && "idx out of bounds!");
  a.data[idx] = a.data[--a.count];
}
template<typename T>
void remove(DarrayArena<T>& a, i32 idx) { 
  Assert(idx >= 0 && "idx negative!");
  Assert(idx < len(a) && "idx out of bounds!");
  a.data[idx] = a.data[--a.count];
}

template<typename T>
void reserve(Darray<T>& a, u64 size) { 
  a.cap += size;
  Assign(a.data, mem_realloc(a.data, sizeof(T)*a.cap));
}

template<typename T>
void free(Darray<T>& a) {
  mem_free(a.data);
  a.count = 0;
  a.cap = 0;
  a.data = null;
}

template<typename T, i32 N> 
INLINE i32 len(Array<T, N> a) { 
  return a.count;
}
template<typename T>
INLINE i32 len(Darray<T> a) { 
  return a.count;
}
template<typename T>
INLINE i32 len(DarrayArena<T> a) { 
  return a.count;
}
INLINE i32 len(const char* a) { 
  return String(a).size;
}
