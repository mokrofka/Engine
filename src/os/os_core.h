#pragma once
#include "base/base.h"
#include "base/str.h"
#include "base/mem.h"

struct Thread {
  u64 v;
};

typedef void ThreadEntryPointFunctionType(void* p);

struct Task {
  void (*func)(void* arg);
  void* arg;
};

struct Mutex {
  u64 v;
};

struct OS_Handle {
  u64 v;
};

typedef u32 FilePropertyFlags;
enum {
  FilePropertyFlag_IsFolder = Bit(0),
};

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

typedef u32 OS_AccessFlags;
enum {
  OS_AccessFlag_Read       = Bit(0),
  OS_AccessFlag_Write      = Bit(1),
  OS_AccessFlag_Execute    = Bit(2),
  OS_AccessFlag_Append     = Bit(3),
  OS_AccessFlag_ShareRead  = Bit(4),
  OS_AccessFlag_ShareWrite = Bit(5),
};

typedef u32 OS_WatchFlags;
enum {
  OS_WatchFlag_Create = Bit(0),
  OS_WatchFlag_Delete = Bit(1),
  OS_WatchFlag_Modify = Bit(2),
};

struct OS_Watch{
  OS_Handle handle;
  OS_WatchFlags flags;
};

String os_get_current_filepath();
String os_get_current_directory();
String os_get_current_binary_name();

void os_init(String name);
void os_exit(i32 exit_code);

u64 os_timer_frequency();
u64 os_timer_now();
u64 os_now_ns();
void os_sleep_ms(u64 ms);
void os_console_write(String message, u32 color);
String os_get_environment(String name);

//////////////////////////////////////////////////////////////////////////
// Memory

u8*  os_reserve(u64 size);
b32  os_commit(void* ptr, u64 size);
void os_decommit(void* ptr, u64 size);
void os_release(void* ptr, u64 size);

//////////////////////////////////////////////////////////////////////////
// Files

OS_Handle      os_file_open(String path, OS_AccessFlags flags);
void           os_file_close(OS_Handle file);
u64            os_file_read(OS_Handle file, u64 size, void* out_data);
u64            os_file_write(OS_Handle file, u64 size, void* data);
u64            os_file_size(OS_Handle file);
FileProperties os_file_properties(OS_Handle file);
Buffer         os_file_path_read_all(Allocator arena, String path);
b32            os_file_path_exists(String path);
b32            os_file_path_copy(String dst, String src);
void           os_file_path_copy_mtime(String src, String dst);
FileProperties os_file_path_properties(String path);
b32            os_file_path_equal_mtime(String a, String b);

// Directory
OS_Handle os_directory_open(String path);
OS_Handle os_directory_create(String path);
OS_Handle os_directory_create_p(String path);
b32       os_directory_path_exist(String path);

// Watch
OS_Watch   os_watch_open(OS_WatchFlags flags);
void       os_watch_close(OS_Watch watch);
OS_Handle  os_watch_attach(OS_Watch watch, String name);
void       os_watch_deattach(OS_Watch watch, OS_Handle attached);
StringList os_watch_check(Allocator arena, OS_Watch watch);

// Directory iteration
OS_FileIter* os_file_iter_begin(Allocator arena, String path, OS_FileIterFlags flags);
b32          os_file_iter_next(Allocator arena, OS_FileIter *iter, OS_FileInfo *info_out);
void         os_file_iter_end(OS_FileIter* iter);

////////////////////////////////////////////////////////////////////////
// Processes

OS_Handle os_process_launch(StringList list);
i32       os_process_join(OS_Handle handle);

////////////////////////////////////////////////////////////////////////
// Threads
void thread_pool_init(u32 num_threads);
Thread os_thread_launch(ThreadEntryPointFunctionType *func, void *ptr);
b32 os_thread_join(Thread handle, u64 endt_us);
void os_thread_detach(Thread handle);

////////////////////////////////////////////////////////////////////////
// Lib

OS_Handle os_lib_open(String path);
void      os_lib_close(OS_Handle lib);
void*     os_lib_get_proc(OS_Handle lib, String name);

