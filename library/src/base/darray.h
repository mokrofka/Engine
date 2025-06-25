#pragma once

KAPI void* _darray_create(u32 length, u32 stride);

KAPI void darray_destroy(void* array);

KAPI void* _darray_resize(void* array);

KAPI void _darray_push(void* array, void* value_ptr);

#define darray_create(T) \
  (T*)_darray_create(8, sizeof(T))

#define darray_reserve(type, capacity) \
  (T*)_darray_create(capacity, sizeof(T))

#define darray_push(array, value) \
  {                               \
    auto temp = value;            \
    _darray_push(&array, &temp);  \
  }
