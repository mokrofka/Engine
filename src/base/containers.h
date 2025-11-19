#pragma once
#include "defines.h"
#include "logger.h"
#include "mem.h"
#include "maths.h"

#include <initializer_list>


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

  T& operator[](u32 idx) {
    Assert(IsInsideBound(idx, cap));
    return data[idx];
  }
};

template<typename T, i32 N> T& append(Array<T, N>& a, T b) { 
  Assert(len(a) < a.cap && "array full");
  T& e = a.data[a.count];
  a.data[a.count++] = b;
  return e;
}
template<typename T, i32 N, typename... Args> void append(Array<T, N>& a, T first, Args... rest) {
  append(a, first);
  (append(a, rest), ...);
}
template<typename T, i32 N> void remove(Array<T, N>& a, i32 idx) { 
  Assert(IsInsideBound(idx, len(a)));
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
    Assert(IsInsideBound(idx, cap));
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
  Assert(IsInsideBound(idx, a.cap));
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
    Assert(IsInsideBound(idx, count));
    return data[idx];
  }
};

template<typename T> void append(DarrayArena<T>& a, T b) { 
  if (len(a) >= a.cap) {
    if (a.data) {
      a.cap *= DEFAULT_RESIZE_FACTOR;
      T* new_data = push_array(a.arena, T, a.cap);
      MemCopyTyped(new_data, a.data, a.count);
      a.data = new_data;
    } else {
      a.cap = DEFAULT_CAPACITY;
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
template<typename T> void reserve(DarrayArena<T>& a, u32 size) { 
  a.cap += size;
  if (a.data) {
    T* new_data = push_array(a.arena, T, a.cap);
    MemCopyTyped(new_data, a.data, a.count);
  } else {
    a.data = push_array(a.arena, T, a.cap);
  }
}
template<typename T> b32 exists(DarrayArena<T>& a, T e) { 
  for (T x : a) {
    if (x == e) return true;
  }
  return false;
}
template<typename T> b32 exists_at(DarrayArena<T>& a, T e, u32* index) { 
  Loop (i, len(a)) {
    if (a[i] == e) {
      *index = i;
      return true;
    }
  }
  return false;
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
// IdPool

struct IdPool {
  u32 count;
  u32 cap;
  u32* handlers;
  DebugDo(b8* used);
};

static IdPool make_handle_pool() {
  IdPool result = {
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
static u32 append(IdPool& h) {
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
static void remove(IdPool& h, u32 id) {
  Assert(IsInsideBound(id, h.cap));
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



// b2IdPool b2CreateIdPool( void )
// {
// 	b2IdPool pool = { 0 };
// 	pool.freeArray = b2IntArray_Create( 32 );
// 	return pool;
// }

// void b2DestroyIdPool( b2IdPool* pool )
// {
// 	b2IntArray_Destroy( &pool->freeArray );
// 	*pool = ( b2IdPool ){ 0 };
// }

// int b2AllocId( b2IdPool* pool )
// {
// 	int count = pool->freeArray.count;
// 	if ( count > 0 )
// 	{
// 		int id = b2IntArray_Pop( &pool->freeArray );
// 		return id;
// 	}

// 	int id = pool->nextIndex;
// 	pool->nextIndex += 1;
// 	return id;
// }

// void b2FreeId( b2IdPool* pool, int id )
// {
// 	B2_ASSERT( pool->nextIndex > 0 );
// 	B2_ASSERT( 0 <= id && id < pool->nextIndex );
// 	b2IntArray_Push( &pool->freeArray, id );
// }

// #if B2_VALIDATE

// void b2ValidateFreeId( b2IdPool* pool, int id )
// {
// 	int freeCount = pool->freeArray.count;
// 	for ( int i = 0; i < freeCount; ++i )
// 	{
// 		if ( pool->freeArray.data[i] == id )
// 		{
// 			return;
// 		}
// 	}

// 	B2_ASSERT( 0 );
// }

// void b2ValidateUsedId( b2IdPool* pool, int id )
// {
// 	int freeCount = pool->freeArray.count;
// 	for ( int i = 0; i < freeCount; ++i )
// 	{
// 		if ( pool->freeArray.data[i] == id )
// 		{
// 			B2_ASSERT( 0 );
// 		}
// 	}
// }

// #else

// void b2ValidateFreeId( b2IdPool* pool, int id )
// {
// 	B2_UNUSED( pool, id );
// }

// void b2ValidateUsedId( b2IdPool* pool, int id )
// {
// 	B2_UNUSED( pool, id );
// }
// #endif

////////////////////////////////////////////////////////////////////////
// Hashmap

enum {
  MapSlot_Empty,
  MapSlot_Occupied,
  MapSlot_Deleted
};

template<typename Key, typename T>
struct Map {
  static constexpr f32 LF = 0.7;

  u32 count;
  u32 cap;
  T* data;
  Key* keys;
  u8* is_occupied;

  Map() {
    MemZeroStruct(this);
  }

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
      if ((is_occupied[index] == MapSlot_Occupied) && (keys[index] == key))
        return data[index];
      index = ModPow2(index + 1, cap);
    }
    Assert(true);
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
      u8* old_is_occupied = is_occupied;
      u32 old_cap = cap;

      cap *= DEFAULT_RESIZE_FACTOR;
      u64 size = (sizeof(T) + sizeof(Key) + sizeof(u8)) * cap;
      u8* buff = mem_realloc_zero(data, size);

      data = (T*)buff;
      keys = (Key*)Offset(data, sizeof(T) * cap);
      is_occupied = (u8*)(Offset(keys, sizeof(Key) * cap));

      Loop (i, old_cap) {
        if (old_is_occupied[i] == MapSlot_Occupied) {
          insert(old_keys[i], old_data[i]);
        }
      }

      mem_free(old_data);
    }
    else {
      cap = DEFAULT_CAPACITY;
      u64 size = (sizeof(T) + sizeof(Key) + sizeof(u8)) * cap;
      u8* buff = mem_alloc_zero(size);

      Assign(data, buff);
      Assign(keys, Offset(data, sizeof(T)*cap));
      Assign(is_occupied, (Offset(keys, sizeof(Key)*cap)));
    }
  }

};

