#pragma once

#include "defines.h"

struct Hashtable {
  u64 element_size;
  u32 element_count;
  b8 is_pointer_type;
  u8* memory;
};

KAPI void hashtable_create(u64 element_size, u32 element_count, void* memory, b8 is_pointer_type, Hashtable* hashtable);

KAPI void hashtable_destroy(Hashtable* table);

KAPI void hashtable_set(Hashtable* table, const char* name, void* value);

KAPI void hashtable_set_ptr(Hashtable* table, const char* name, void** value);

KAPI void hashtable_get(Hashtable* table, const char* name, void* out_value);

KAPI void hashtable_get_ptr(Hashtable* table, const char* name, void** out_value);

KAPI void hashtable_fill(Hashtable* table, void* value);

