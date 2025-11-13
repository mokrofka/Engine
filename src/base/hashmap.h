#pragma once
#include "defines.h"
#include "maths.h"
#include "logger.h"
#include "mem.h"

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
    u64 index = TrunctPow2(hash_idx, cap);
    while (is_occupied[index] == MapSlot_Occupied) {
      index = TrunctPow2(index + 1, cap);
    }
    keys[index] = key;
    data[index] = val;
    is_occupied[index] = MapSlot_Occupied;
    ++count;
  }

  T& get(Key key) {
    u64 hash_idx = hash(key);
    u64 index = TrunctPow2(hash_idx, cap);
    while (is_occupied[index] != MapSlot_Empty) {
      if ((is_occupied[index] == MapSlot_Occupied) && (keys[index] == key))
        return data[index];
      index = TrunctPow2(index + 1, cap);
    }
    Assert(true);
    return data[index];
  }

  void erase(Key key) {
    u64 hash_idx = hash(key);
    u64 index = TrunctPow2(hash_idx, cap);
    while (is_occupied[index] != MapSlot_Empty) {
      if ((is_occupied[index] == MapSlot_Occupied) && (keys[index] == key)) {
        is_occupied[index] = MapSlot_Deleted;
        --count;
        return;
      }
      index = TrunctPow2(index + 1, cap);
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


