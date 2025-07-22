#include "lib.h"

u64 hash_name(String name, u32 element_count) {
  // A multipler to use when generating a hash. Prime to hopefully avoid collisions
  #define multiplier 97
  
  u8* us = name.str;
  u64 hash = 0;
  
  Loop (i, name.size) {
    hash = hash * multiplier + *us;
    ++us;
  }
  
  // Mod it against the size of the table
  hash %= element_count;
  
  return hash;
}

HashMap hashmap_create(Arena* arena, u32 element_size, u32 element_count, b32 is_pointer_type) {
  Assert(element_count && element_size);
  HashMap hashtable = {
    .data = push_buffer(arena, element_size*element_count),
    .element_size = element_size,
    .element_count = element_count,
    .is_pointer_type = (b8)is_pointer_type,
  };
  return hashtable;
}

void hashmap_destroy(HashMap& table) {
  Assert(&table);
  MemZeroStruct(&table);
}

void hashmap_set(HashMap& table, String name, void* value) {
  Assert(name.str && value);
  Assert(!table.is_pointer_type);

  u64 hash = hash_name(name, table.element_count);
  MemCopy(table.data + (table.element_size*hash), value, table.element_size);
}

void hashmap_set_ptr(HashMap& table, String name, void** value) {
  Assert(name.str);
  Assert(!table.is_pointer_type);
  
  u64 hash = hash_name(name, table.element_count);
  ((void**)table.data)[hash] = value ? *value : 0;
}

void* hashmap_get(HashMap& table, String name) {
  Assert(name.str);
  Assert(!table.is_pointer_type);
  
  u64 hash = hash_name(name, table.element_count);
  void* result = Offset(table.data, table.element_size*hash);
  return result;
}

void hashmap_get_ptr(HashMap& table, String name, void** out_value) {
  Assert(name.str && out_value);
  Assert(!table.is_pointer_type);
  
  u64 hash = hash_name(name, table.element_count);
  *out_value = ((void**)table.data)[hash];
}

void hashmap_fill(HashMap& table, void* value) {
  Assert(value);
  Assert(!table.is_pointer_type);

  Loop (i, table.element_count) {
    MemCopy(table.data + (table.element_size * i), value, table.element_size);
  }
}
