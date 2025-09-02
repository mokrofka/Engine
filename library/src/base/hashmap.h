#pragma once
#include "defines.h"
#include "str.h"
#include "logger.h"
#include "maths.h"

enum SlotState {
  MapSlot_Empty,
  MapSlot_Occupied,
  MapSlot_Deleted
};

template<typename Key, typename T>
struct Map {
  static constexpr f32 LF = 0.7;

  Arena* arena;
  u32 count;
  u32 cap;
  T* data;
  Key* keys;
  u8* is_ocuppied;

  Map(Arena* arena_, u32 cap_ = 8) {
    arena = arena_;
    count = 0;
    cap = cap_;
    data = push_array(arena, T, cap);
    keys = push_array(arena, Key, cap);
    is_ocuppied = push_array(arena, u8, cap);
    MemSet(is_ocuppied, MapSlot_Empty, cap);
  }

  void insert(Key key, T val) {
    if (count > cap*LF) { grow(); }
    u64 hash_idx = hash(key);
    u64 index = TrunctPow2(hash_idx, cap);
    while (is_ocuppied[index] == MapSlot_Occupied) {
      index = TrunctPow2(index + 1, cap);
    }
    keys[index] = key;
    data[index] = val;
    is_ocuppied[index] = MapSlot_Occupied;
    ++count;
  }

  T* find(Key key) {
    u64 hash_idx = hash(key);
    u64 index = TrunctPow2(hash_idx, cap);
    while (is_ocuppied[index] != MapSlot_Empty) {
      if (is_ocuppied[index] == MapSlot_Occupied && keys[index] == key)
        return &data[index];
      index = TrunctPow2(index + 1, cap);
    }

    return null;
  }

  void erase(Key key) {
    u64 hash_idx = hash(key);
    u64 index = TrunctPow2(hash_idx, cap);
    while (is_ocuppied[index] != MapSlot_Empty) {
      if (is_ocuppied[index] == MapSlot_Occupied && keys[index] == key) {
        is_ocuppied[index] = MapSlot_Deleted;
        --count;
        return;
      }
      index = TrunctPow2(index + 1, cap);
    }
  }

  void grow() {
    u64 old_cap = cap;
    T* old_data = data;
    Key* old_keys = keys;
    u8* old_occupied = is_ocuppied;
    count = 0;
    cap *= 2;
    data = push_array(arena, T, cap);
    keys = push_array(arena, Key, cap);
    is_ocuppied = push_array(arena, u8, cap);
    MemSet(is_ocuppied, MapSlot_Empty, cap);
    Loop (i, old_cap) {
      if (old_occupied[i] == MapSlot_Occupied) {
        insert(old_keys[i], old_data[i]);
      }
    }
  }

  T& operator[](Key key) {
    T* val = find(key);
    if (val) {
      return *val;
    }

    T default_val = {};
    insert(key, default_val);
    return *find(key);
  }

};
