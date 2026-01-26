#pragma once
#include "base/base.h"
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

enum OS_AccessFlags {
  OS_AccessFlag_Read       = Bit(0),
  OS_AccessFlag_Write      = Bit(1),
  OS_AccessFlag_Execute    = Bit(2),
  OS_AccessFlag_Append     = Bit(3),
  OS_AccessFlag_ShareRead  = Bit(4),
  OS_AccessFlag_ShareWrite = Bit(5),
};

typedef u32 OS_WatchFlags;
enum {
  OS_WatchFlag_Create     = Bit(0),
  OS_WatchFlag_Delete     = Bit(1),
  OS_WatchFlag_Modify     = Bit(2),
  OS_WatchFlag_CloseWrite = Bit(3),
  OS_WatchFlag_MovedTo    = Bit(4),
};

struct OS_Watch{
  OS_Handle handle;
  OS_WatchFlags flags;
};

KAPI String os_get_current_filepath();
KAPI String os_get_current_directory();
KAPI String os_get_current_binary_name();

KAPI void os_init(String name);
KAPI void os_exit(i32 exit_code);

KAPI u64 os_timer_frequency();
KAPI u64 os_timer_now();
KAPI u64 os_now_ns();
KAPI void os_console_write(String message, u32 color);
KAPI void os_sleep_ms(u64 ms);

//////////////////////////////////////////////////////////////////////////
// Memory

KAPI u8*  os_reserve(u64 size);
KAPI b32  os_commit(void* ptr, u64 size);
KAPI void os_decommit(void* ptr, u64 size);
KAPI void os_release(void* ptr, u64 size);
KAPI u8*  os_reserve_large(u64 size);
KAPI b32  os_commit_large(void* ptr, u64 size);

//////////////////////////////////////////////////////////////////////////
// Files

KAPI OS_Handle      os_file_open(String path, OS_AccessFlags flags);
KAPI void           os_file_close(OS_Handle file);
KAPI u64            os_file_read(OS_Handle file, u64 size, u8* out_data);
KAPI u64            os_file_write(OS_Handle file, u64 size, u8* data);
KAPI u64            os_file_size(OS_Handle file);
KAPI FileProperties os_file_properties(OS_Handle file);
KAPI Buffer         os_file_read_all(Allocator arena, String path);
KAPI b32            os_file_path_exists(String path);
KAPI b32            os_file_path_copy(String dst, String src);
KAPI void           os_file_path_time_copy(String src, String dst);
KAPI FileProperties os_file_path_properties(String path);

// Directory
KAPI OS_Handle os_directory_open(String path);
KAPI void      os_directory_create(String path);
KAPI void      os_directory_create_recursively(String path);

// Watch
OS_Watch   os_watch_open(OS_WatchFlags flags);
void       os_watch_close(OS_Watch watch);
OS_Handle  os_watch_attach(OS_Watch watch, String name);
void       os_watch_deattach(OS_Watch watch, OS_Handle attached);
StringList os_watch_check(Allocator arena, OS_Watch watch);

// Directory iteration
KAPI OS_FileIter* os_file_iter_begin(Allocator arena, String path, OS_FileIterFlags flags);
KAPI b32          os_file_iter_next(Allocator arena, OS_FileIter *iter, OS_FileInfo *info_out);
KAPI void         os_file_iter_end(OS_FileIter* iter);

////////////////////////////////////////////////////////////////////////
// Processes

KAPI OS_Handle os_process_launch(String cmd);
KAPI void      os_process_join(OS_Handle handle);

////////////////////////////////////////////////////////////////////////
// Lib

KAPI OS_Handle os_lib_open(String path);
KAPI void      os_lib_close(OS_Handle lib);
KAPI void*     os_lib_get_proc(OS_Handle lib, String name);

