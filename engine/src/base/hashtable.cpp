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

Hashtable hashtable_create(Arena* arena, u64 element_size, u32 element_count, b32 is_pointer_type) {
  Assert(element_count && element_size && "element_size and element_count must be a positive non-zero value");
  Hashtable hashtable;
  
  hashtable.data = push_buffer(arena, u8, element_size * element_count);
  hashtable.element_size = element_size;
  hashtable.element_count = element_count;
  hashtable.is_pointer_type = is_pointer_type;
  MemZero(hashtable.data, element_size * element_count);
  return hashtable;
}

void hashtable_destroy(Hashtable* table) {
  Assert(table);
  MemZeroStruct(table);
}

void hashtable_set(Hashtable* table, String name, void* value) {
  Assert(table && name.str && value);
  Assert(!table->is_pointer_type);

  u64 hash = hash_name(name, table->element_count);
  MemCopy(table->data + (table->element_size*hash), value, table->element_size);
}

void hashtable_set_ptr(Hashtable* table, String name, void** value) {
  Assert(table && name.str);
  Assert(!table->is_pointer_type);
  
  u64 hash = hash_name(name, table->element_count);
  ((void**)table->data)[hash] = value ? *value : 0;
}

void hashtable_get(Hashtable* table, String name, void* out_value) {
  Assert(table && name.str && out_value);
  Assert(!table->is_pointer_type);
  
  u64 hash = hash_name(name, table->element_count);
  MemCopy(out_value, table->data + (table->element_size * hash), table->element_size);
}

void hashtable_get_ptr(Hashtable* table, String name, void** out_value) {
  Assert(table && name.str && out_value);
  Assert(!table->is_pointer_type);
  
  u64 hash = hash_name(name, table->element_count);
  *out_value = ((void**)table->data)[hash];
}

void hashtable_fill(Hashtable* table, void* value) {
  Assert(table || value);
  Assert(!table->is_pointer_type);

  Loop (i, table->element_count) {
    MemCopy(table->data + (table->element_size * i), value, table->element_size);
  }
}
