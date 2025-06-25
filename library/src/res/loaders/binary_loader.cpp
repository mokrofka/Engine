#include "sys/res.h"

Buffer res_binary_load(Arena* arena, String filename) {
  Scratch scratch(&arena);
  String filepath = push_strf(scratch, "%s/%s", res_sys_base_path(), filename);

  OS_Handle f = os_file_open(filepath, OS_AccessFlag_Read);
  if (!f) {
    Error("res_binary_load - unable to open file: '%s'", filepath);
    os_file_close(f);
    return {};
  }

  u64 file_size = os_file_size(f);
  u8* buffer = push_buffer(arena, file_size);
  u64 read_size = os_file_read(f, file_size, buffer);
  if (read_size == 0) {
    Error("Unable to binary read file: %s", filepath);
    os_file_close(f);
    return {};
  }

  Buffer result = {
    .data = buffer,
    .size = file_size,
  };

  os_file_close(f);
  return result;
}
