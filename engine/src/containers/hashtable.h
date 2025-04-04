#pragma once

#include "defines.h"

struct HashTable {
  u64 element_size;
  u32 element_count;
  b8 is_pointer_type;
  u8* memory;
};

KAPI void hashtable_create(u64 element_size, u32 element_count, void* memory, b8 is_pointer_type, HashTable* hashtable);

KAPI void HashTable_destroy(HashTable* table);

KAPI b8 hashtable_set(HashTable* table, const char* name, void* value);

KAPI b8 hashtable_set_ptr(HashTable* table, const char* name, void** value);

KAPI b8 hashtable_get(HashTable* table, const char* name, void* out_value);

KAPI b8 hashtable_get_ptr(HashTable* table, const char* name, void** out_value);

KAPI b8 hashtable_fill(HashTable* table, void* value);

