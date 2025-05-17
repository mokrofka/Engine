#pragma once
#include "defines.h"

struct HashMap {
  u32 element_size;
  u32 element_count;
  b8 is_pointer_type;
  u8* data;
};

HashMap hashmap_create(Arena* arena, u32 element_size, u32 element_count, b32 is_pointer_type = false);

KAPI void hashmap_destroy(HashMap& table);

KAPI void hashmap_set(HashMap& table, String name, void* value);

KAPI void hashmap_set_ptr(HashMap& table, String name, void** value);

KAPI void hashmap_get(HashMap& table, String name, void* out_value);

KAPI void hashmap_get_ptr(HashMap& table, String name, void** out_value);

KAPI void hashmap_fill(HashMap& table, void* value);

