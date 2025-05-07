#include "binary_loader.h"
#include "loader_utils.h"

b32 binary_loader_load(Arena* arena, ResLoader* self, String name, Res* out_res) {
  Scratch scratch(&arena, 1);
  Assert(self && name && out_res);

  String file_path = push_strf(scratch, "%s/%s/%s", res_sys_base_path(), (String)self->type_path64, name);

  // TODO: Should be using an allocator here.
  str_copy(out_res->file_path64, file_path);

  OS_Handle f = os_file_open(file_path, OS_AccessFlag_Read);
  if (!f) {
    Error("binary_loader_load - unable to open file for binary reading: '%s'", file_path);
    return false;
  }

  u64 file_size = os_file_size(f);
  if (!file_size) {
    Error("Unable to binary read file: %s", file_path);
    os_file_close(f);
    return false;
  }

  u8* buffer;
  if (arena) {
    buffer = push_buffer(arena, u8, file_size);
  } else {
    buffer = mem_alloc(file_size);
  }
  u64 read_size = os_file_read(f, file_size, buffer);
  if (read_size == 0) {
    Error("Unable to binary read file: %s", file_path);
    os_file_close(f);
    return false;
  }

  os_file_close(f);

  out_res->data = buffer;
  out_res->data_size = read_size;
  str_copy(out_res->name64, name);

  return true;
}

void binary_loader_unload(ResLoader* self, Res* res) {
  res_unload(self, res);
}

ResLoader binary_res_loader_create() {
  ResLoader loader;
  loader.type = ResType_Binary;
  loader.custom_type64.size = 0;
  loader.load = binary_loader_load;
  loader.unload = binary_loader_unload;
  loader.type_path64.size = 0;

  return loader;
}

Binary res_load_binary(Arena* arena, String filepath) {
  Scratch scratch(&arena, 1);
  Binary binary = {};

  String file_path = push_strf(scratch, "%s/%s", res_sys_base_path(), filepath);

  OS_Handle f = os_file_open(file_path, OS_AccessFlag_Read);
  if (!f) {
    Error("binary_loader_load - unable to open file for binary reading: '%s'", file_path);
    goto error;
  }

  u64 file_size = os_file_size(f);
  if (!file_size) {
    Error("Unable to binary read file: %s", file_path);
    os_file_close(f);
    goto error;
  }

  u8* buffer;
  if (arena) {
    buffer = push_buffer(arena, u8, file_size);
  } else {
    buffer = mem_alloc(file_size);
  }
  u64 read_size = os_file_read(f, file_size, buffer);
  if (read_size == 0) {
    Error("Unable to binary read file: %s", file_path);
    os_file_close(f);
    goto error;
  }

  os_file_close(f);

  str_copy(binary.file_path64, file_path);
  binary.data = buffer;
  binary.size = file_size;

  error:
  return binary;
}
