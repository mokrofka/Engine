#include "hashtable.h"

#include "memory.h"
#include "logger.h"

u64 hash_name(const char* name, u32 element_count) {
  // A multipler to use when generating a hash. Prime to hopefully avoid collisions
  local const u64 multiplier = 97;
  
  const u8* us;
  u64 hash = 0;
  
  for (us = (const u8*)name; *us; ++us) {
    hash = hash * multiplier + *us;
  }
  
  // Mod it against the size of the table
  hash %= element_count;
  
  return hash;
}

void hashtable_create(u64 element_size, u32 element_count, void* memory, b8 is_pointer_type, Hashtable* hashtable) {
  AssertMsg(memory && hashtable, "hashtable_create failed! Pointer to memory and out_hashtable are required");
  AssertMsg(element_count && element_size, "element_size and element_count must be a positive non-zero value");
  
  hashtable->memory = (u8*)memory;
  hashtable->element_size = element_size;
  hashtable->element_count = element_count;
  hashtable->is_pointer_type = is_pointer_type;
  MemZero(hashtable->memory, element_size * element_count);
}

void hashtable_destroy(Hashtable* table) {
  Assert(table);
  MemZeroStruct(table);
}

void hashtable_set(Hashtable* table, const char* name, void* value) {
  AssertMsg(table && name && value, "hashtable_set requires table, name and value to exist.")
  AssertMsg(!table->is_pointer_type, "hashtable_set should not be used with tables that have pointer to types. Use hashtable_ptr instead.");
  
  u64 hash = hash_name(name, table->element_count);
  MemCopy(table->memory + (table->element_size*hash), value, table->element_size);
}

void hashtable_set_ptr(Hashtable* table, const char* name, void** value) {
  AssertMsg(table && name, "hashtable_set requires table and name to exist.");
  AssertMsg(!table->is_pointer_type, "hashtable_set should not be used with tables that have pointer to types. Use hashtable_ptr instead.");
  
  u64 hash = hash_name(name, table->element_count);
  ((void**)table->memory)[hash] = value ? *value : 0;
}

void hashtable_get(Hashtable* table, const char* name, void* out_value) {
  AssertMsg(table && name && out_value, "hashtable_set requires table, name and out_value to exist.");
  AssertMsg(!table->is_pointer_type, "hashtable_set should not be used with tables that have pointer to types. Use hashtable_ptr instead.");
  
  u64 hash = hash_name(name, table->element_count);
  MemCopy(out_value, table->memory + (table->element_size * hash), table->element_size);
}

void hashtable_get_ptr(Hashtable* table, const char* name, void** out_value) {
  AssertMsg(table && name && out_value, "hashtable_set requires table, name and out_value to exist.");
  AssertMsg(!table->is_pointer_type, "hashtable_set should not be used with tables that have pointer to types. Use hashtable_ptr instead.");
  
  u64 hash = hash_name(name, table->element_count);
  *out_value = ((void**)table->memory)[hash];
}

void hashtable_fill(Hashtable* table, void* value) {
  AssertMsg(table || value, "hashtable_set requires table and name to exist.");
  AssertMsg(!table->is_pointer_type, "hashtable_set should not be used with tables that have pointer to types. Use hashtable_ptr instead.");

  for (u32 i = 0; i < table->element_count; ++i) {
    MemCopy(table->memory + (table->element_size * i), value, table->element_size);
  }
}
