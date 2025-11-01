#pragma once
#include "base/defines.h"
#include "base/str.h"
#include "base/mem.h"
#include "base/maths.h"

typedef u64 OS_Handle;
typedef u32 FilePropertyFlags;

struct FileProperties {
  u64 size;
  DenseTime modified;
  DenseTime created;
  FilePropertyFlags flags;
};

typedef u32 OS_FileIterFlags;
enum {
  OS_FileIterFlag_SkipFolders     = Bit(0),
  OS_FileIterFlag_SkipFiles       = Bit(1),
  OS_FileIterFlag_SkipHiddenFiles = Bit(2),
  OS_FileIterFlag_Done            = Bit(31),
};

struct OS_FileIter {
  OS_FileIterFlags flags;
  u8 memory[800];
};

struct OS_FileInfo {
  String name;
  FileProperties props;
};

struct OS_ProcessInfo {
  String binary_path;
};

enum OS_AccessFlags {
  OS_AccessFlag_Read       = Bit(0),
  OS_AccessFlag_Write      = Bit(1),
  OS_AccessFlag_Execute    = Bit(2),
  OS_AccessFlag_Append     = Bit(3),
  OS_AccessFlag_ShareRead  = Bit(4),
  OS_AccessFlag_ShareWrite = Bit(5),
};

struct Buffer {
  u8* data;
  u64 size;
};

KAPI extern f32 delta_time;

KAPI void os_exit(i32 exit_code);

KAPI void os_init();
KAPI void os_pump_messages();

KAPI void os_entry_point(void (*main)());

KAPI void os_console_write(String message, u32 color);
KAPI void os_console_write_error(String message, u32 color);

KAPI f64  os_now_seconds();
KAPI void os_sleep(u64 ms);

KAPI String    os_get_current_directory();
KAPI String    os_get_current_binary_name();
KAPI String    os_get_current_filepath();

//////////////////////////////////////////////////////////////////////////
// Memory
KAPI u8*   os_reserve(u64 size);
KAPI u8*   os_shm_reserve(u64 size, OS_Handle* handler);
KAPI b32   os_commit(void* ptr, u64 size);
KAPI void  os_decommit(void* ptr, u64 size);
KAPI void  os_release(void* ptr, u64 size);
KAPI void* os_reserve_large(u64 size);
KAPI b32   os_commit_large(void* ptr, u64 size);

//////////////////////////////////////////////////////////////////////////
// Files
KAPI OS_Handle      os_file_open(String path, OS_AccessFlags flags);
KAPI void           os_file_close(OS_Handle file);
KAPI u64            os_file_read(OS_Handle file, u64 size, u8* out_data);
KAPI u64            os_file_write(OS_Handle file, u64 size, u8* data);
KAPI u64            os_file_size(OS_Handle file);
KAPI FileProperties os_properties_from_file(OS_Handle file);
KAPI FileProperties os_properties_from_file_path(String path);
KAPI b32            os_copy_file_path(String dst, String src);
KAPI b32            os_file_path_exists(String path);
KAPI b32            os_file_compare_time(u64 new_write_time, u64 last_write_time);
KAPI b32            os_file_path_compare_time(String a, String b);
KAPI void           os_file_copy_mtime(String src, String dst);

////////////////////////////////////////////////////////////////////////
// Directory
KAPI OS_Handle os_directory_open(String path);
KAPI void      os_directory_create(String path);
KAPI OS_Handle os_directory_create_p(String path);
KAPI b32       os_directory_path_exist(String path);;
KAPI void      os_directory_watch(OS_Handle dir_handle, u32 id);
KAPI String    os_directory_watch_pop_name(Arena* arena, OS_Handle dir, u32 id);
KAPI b32       os_directory_check_change(OS_Handle dir_handle, u32 id);

// Directory iteration
KAPI OS_FileIter* os_file_iter_begin(Arena *arena, String path, OS_FileIterFlags flags);
KAPI b32          os_file_iter_next(Arena *arena, OS_FileIter *iter, OS_FileInfo *info_out);
KAPI void         os_file_iter_end(OS_FileIter *iter);

////////////////////////////////////////////////////////////////////////
// Processes
KAPI OS_Handle os_process_launch(String cmd);
KAPI void      os_process_join(OS_Handle handle);

////////////////////////////////////////////////////////////////////////
// Lib
KAPI OS_Handle os_lib_open(String path);
KAPI void      os_lib_close(OS_Handle lib);
KAPI void*     os_lib_get_proc(OS_Handle lib, String name);

