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

void hashtable_create(u64 element_size, u32 element_count, void* memory, b8 is_pointer_type, HashTable* hashtable) {
  if (!memory || !hashtable) {
    Error("hashtable_create failed! Pointer to memory and out_hashtable are required.");
    return;
  }
  if (!element_count || !element_size) {
    Error("element_size and element_count must be a positive non-zero value.");
    return;
  }
  
  // TODO Might want to require an allocator and allocate this memory instead
  hashtable->memory = (u8*)memory;
  hashtable->element_count = element_count;
  hashtable->element_size = element_size;
  hashtable->is_pointer_type = is_pointer_type;
  MemZero(hashtable->memory, element_size * element_count);
}

void HashTable_destroy(HashTable* table) {
  if (table) {
    // TODO if using allocator above, free memory here
    MemZeroStruct(table);
  }
}

b8 hashtable_set(HashTable* table, const char* name, void* value) {
  if (!table || !name || !value) {
    Error("hashtable_set requires table, name and value to exist.");
    return false;
  }
  if (table->is_pointer_type) {
    Error("hashtable_set should not be used with tables that have pointer to types. Use hashtable_ptr instead.");
    return false;
  }
  
  u64 hash = hash_name(name, table->element_count);
  MemCopy(table->memory + (table->element_size*hash), value, table->element_size);
  return true;
}

b8 hashtable_set_ptr(HashTable* table, const char* name, void** value) {
  if (!table || !name) {
    Error("hashtable_set requires table and name to exist.");
    return false;
  }
  if (table->is_pointer_type) {
    Error("hashtable_set should not be used with tables that have pointer to types. Use hashtable_ptr instead.");
    return false;
  }
  
  u64 hash = hash_name(name, table->element_count);
  ((void**)table->memory)[hash] = value ? *value : 0;
  return true;
}

b8 hashtable_get(HashTable* table, const char* name, void* out_value) {
  if (!table || !name || !out_value) {
    Error("hashtable_set requires table, name and out_value to exist.");
    return false;
  }
  if (table->is_pointer_type) {
    Error("hashtable_set should not be used with tables that have pointer to types. Use hashtable_ptr instead.");
    return false;
  }
  
  u64 hash = hash_name(name, table->element_count);
  MemCopy(out_value, table->memory + (table->element_size * hash), table->element_size);
  return true;
}

b8 hashtable_get_ptr(HashTable* table, const char* name, void** out_value) {
  if (!table || !name || !out_value) {
    Error("hashtable_set requires table, name and out_value to exist.");
    return false;
  }
  if (table->is_pointer_type) {
    Error("hashtable_set should not be used with tables that have pointer to types. Use hashtable_ptr instead.");
    return false;
  }
  
  u64 hash = hash_name(name, table->element_count);
  *out_value = ((void**)table->memory)[hash];
  return *out_value != 0;
}

b8 hashtable_fill(HashTable* table, void* value) {
  if (!table || !value) {
    Error("hashtable_set requires table and name to exist.");
    return false;
  }
  if (table->is_pointer_type) {
    Error("hashtable_set should not be used with tables that have pointer to types. Use hashtable_ptr instead.");
    return false;
  }

  for (u32 i = 0; i < table->element_count; ++i) {
    MemCopy(table->memory + (table->element_size * i), value, table->element_size);
  }

  return true;
}
