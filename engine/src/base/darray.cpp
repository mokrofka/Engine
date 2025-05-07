#include "darray.h"

struct DarrayHeader {
  u32 capacity;
  u32 size;
  u32 stride;
  u32 padding;
};
#define HeaderSize sizeof(DarrayHeader)

#define DefaultCapacity 8
#define ResizeFactor 2

void* _darray_create(u32 length, u32 stride) {
  u32 array_size = length * stride;
  void* new_array = 0;
  MemAlloc(new_array, HeaderSize + array_size);

  ClearMemory(new_array, HeaderSize + array_size);
  Assert(length && "length should be > 0");
  
  DarrayHeader* header; Assign(header, new_array);
  header->capacity = length;
  header->size = 0;
  header->stride = stride;

  return (void*)((u8*)new_array + HeaderSize);
}

void darray_destroy(void* array) {
  Assert(array);
  DarrayHeader* header; Assign(header, (u8*)array - HeaderSize);
  mem_free(header);
}

void* _darray_resize(void* array) {
  DarrayHeader* header; Assign(header, (u8*)array - HeaderSize);
  void* temp = _darray_create((ResizeFactor * header->capacity), header->stride);

  DarrayHeader* new_header; Assign(new_header, (u8*)temp - HeaderSize);
  new_header->size = header->size;

  MemCopy(temp, array, header->size * header->stride);

  darray_destroy(array);
  return temp;
}

void _darray_push(void* parray, void* value_ptr) {
  void* array = *(void**)parray;
  Assert(array && value_ptr);
  DarrayHeader* header; Assign(header, ((u8*)array - HeaderSize));
  if (header->size >= header->capacity) {
    *(void**)parray = _darray_resize(array);
    array = *(void**)parray;
    Assign(header, (u8*)array - HeaderSize);
  }

  PtrInt addr = (PtrInt)array;
  addr += (header->size * header->stride);
  MemCopy((void*)addr, value_ptr, header->stride);
  ++header->size;
}
