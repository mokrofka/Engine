#pragma once
#include "base/defines.h"
#include "base/str.h"
#include "base/mem.h"
#include "base/maths.h"



typedef PtrInt OS_Handle;
typedef u32 FilePropertyFlags;

struct FileProperties {
  u64 size;
  DenseTime modified;
  DenseTime created;
  FilePropertyFlags flags;
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

KAPI void os_toggle_fullscreen();

KAPI void os_init();
KAPI void os_pump_messages();

KAPI void os_entry_point(void (*main)());

KAPI void os_console_write(String message, u32 color);
KAPI void os_console_write_error(String message, u32 color);

KAPI f64  os_now_seconds();
KAPI void os_sleep(u64 ms);

KAPI void      os_show_window();
KAPI void      os_window_destroy();
KAPI OS_Handle os_get_handle_info();
KAPI OS_Handle os_get_window_handle();
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

////////////////////////////////////////////////////////////////////////
// Directory
KAPI OS_Handle os_directory_open(String path);
KAPI void      os_directory_watch(OS_Handle dir_handle, u32 id);
KAPI String    os_directory_watch_pop_name(Arena* arena, OS_Handle dir, u32 id);
KAPI b32       os_directory_check_change(OS_Handle dir_handle, u32 id);

////////////////////////////////////////////////////////////////////////
// Lib
KAPI OS_Handle os_lib_open(String path);
KAPI void      os_lib_close(OS_Handle lib);
KAPI VoidProc* os_lib_get_proc(OS_Handle lib, String name);

KAPI void os_process_create(String cmd);

