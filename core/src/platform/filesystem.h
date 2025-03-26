#pragma once

#include "defines.h"

#include "strings.h"

// struct FileHandle {
//   void* handle;
//   b8 is_valid;
// };

// enum FileModes {
//   FILE_MODE_READ = 0x1,
//   FILE_MODE_WRITE = 0x2,
// };

// KAPI b8 filesystem_file_size(String path);

// KAPI b8 filesystem_open(String path, FileModes mode, b8 binary, FileHandle* out_handle);

// KAPI void filesystem_close(FileHandle* handle);

// KAPI b8 filesystem_read_line(FileHandle* handle, char** line_buf);

// KAPI b8 filesystem_write_line(FileHandle* handle, const char* text);

// KAPI b8 fylesystem_read(FileHandle* handle, u64 data_size, void* out_data, u64* out_bytes_read);

// KAPI b8 filesystem_read_all_bytes(FileHandle* handle, u8** out_bytes, u64* out_bytes_read);

// KAPI b8 filesystem_write(FileHandle* handle, u64 data_size, const void* data, u64* out_bytes_read);
