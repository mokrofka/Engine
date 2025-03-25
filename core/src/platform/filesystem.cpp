#include "filesystem.h"

#include "logger.h"
#include "memory.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <windows.h>

// b8 filesystem_exists(String path) {
//   DWORD attributes = GetFileAttributesA((char*)path.str);
//   return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
// }

// b8 filesystem_open(String path, FileModes mode, b8 binary, FileHandle* out_handle) {
//   out_handle->is_valid = false;
//   out_handle->handle = 0;
//   const char* mode_str;

//   if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0) {
//     mode_str = binary ? "w+b" : "w+";
//   } else if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0) {
//     mode_str = binary ? "rb" : "r";
//   } else if ((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0) {
//     mode_str = binary ? "wb" : "w";
//   } else {
//     Error("Invalid mode passed while trying to open file: '%s'", path.str);
//     return false;
//   }

//   // Attempt to open the file.
//   FILE* file = fopen((char*)path.str, mode_str);
//   if (!file) {
//     Error("Error opening file: '%s'", path);
//     return false;
//   }

//   out_handle->handle = file;
//   out_handle->is_valid = true;

//   return true;
// }

// void filesystem_close(FileHandle* handle) {
//   if (handle->handle) {
//     fclose((FILE*)handle->handle);
//     handle->handle = 0;
//     handle->is_valid = false;
//   }
// }

// b8 filesystem_read_line(FileHandle* handle, char** line_buf) {
//   if (handle->handle) {
//     if (fgets(*line_buf, 32000, (FILE*)handle->handle) != 0) {
//       return true;
//     }
//   }
//   return false;
// }

// b8 filesystem_write_line(FileHandle* handle, const char* text) {
//   if (handle->handle) {
//     i32 result = fputs(text, (FILE*)handle->handle);
//     if (result != EOF) {
//       result = fputc('\n', (FILE*)handle->handle);
//     }
    
//     fflush((FILE*)handle->handle);
//     return result != EOF;
//   }

//   return false;
// }

// b8 fylesystem_read(FileHandle* handle, u64 data_size, void* out_data, u64* out_bytes_read) {

// }

// b8 filesystem_read_all_bytes(FileHandle* handle, u8** out_bytes, u64* out_bytes_read) {

// }

// b8 filesystem_write(FileHandle* handle, u64 data_size, const void* data, u64* out_bytes_read) {

// }
