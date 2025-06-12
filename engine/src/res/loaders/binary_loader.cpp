#include "res/res_types.h"

#include "sys/res_sys.h"

Binary res_binary_load(Arena* arena, String filepath) {
  Scratch scratch(&arena, 1);
  Binary binary = {};
  
  String file_path = push_strf(scratch, "%s/%s", res_sys_base_path(), filepath);

  OS_Handle f = os_file_open(file_path, OS_AccessFlag_Read);
  if (!f) {
    Error("binary_loader_load - unable to open file for binary reading: '%s'", file_path);
    os_file_close(f);
    return binary;
  }

  u64 file_size = os_file_size(f);
  if (!file_size) {
    Error("Unable to binary read file: %s", file_path);
    os_file_close(f);
    return binary;
  }

  u8* buffer;
  if (arena) {
    buffer = push_buffer(arena, file_size);
  } else {
    buffer = mem_alloc(file_size);
  }
  u64 read_size = os_file_read(f, file_size, buffer);
  if (read_size == 0) {
    Error("Unable to binary read file: %s", file_path);
    os_file_close(f);
    return binary;
  }

  str_copy(binary.file_path64, file_path);
  binary.data = buffer;
  binary.size = file_size;

  os_file_close(f);
  return binary;
}
