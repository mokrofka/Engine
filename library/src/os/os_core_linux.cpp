#include "lib.h"

#if OS_LINUX

#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

struct OS_State {
  Arena* arena;

  String binary_name;
  String binary_filepath;
  String binary_directory;
};

f32 delta_time;

global OS_State st;

void entry_point();

KAPI void os_init() {
  Scratch scratch;
  Arena* arena = arena_alloc();

  u32 max_path_length = 512;
  u8* buff = push_buffer(scratch, max_path_length);
  u32 size = readlink("/proc/self/exe", (char*)buff, max_path_length);
  String name = push_str_copy(arena, String(buff, size));

  st = {
    .arena = arena,
    .binary_name = str_skip_last_slash(name),
    .binary_filepath = name,
    .binary_directory = str_chop_last_slash(name),
  };

}

void os_console_write(String message, u32 color) {

  String color_str;
  switch (color) {
    case 0: color_str = "\x1b[90m"; break; // Gray
    case 1: color_str = "\x1b[36m"; break; // Cyan
    case 2: color_str = "\x1b[32m"; break; // Green
    case 3: color_str = "\x1b[33m"; break; // Yellow
    case 4: color_str = "\x1b[31m"; break; // Red
    default: color_str = "\x1b[0m"; break; // Reset
  }
  write(STDOUT_FILENO, color_str.str, color_str.size);
  write(STDOUT_FILENO, message.str, message.size);
}

void os_message_box(String message) {
}

f64 os_now_seconds() {
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (f64)ts.tv_sec + (f64)ts.tv_nsec / 1e9;
}

void os_sleep(u64 ms) { 
  timespec ts = {
    .tv_sec = (i32)ms / 1000,
    .tv_nsec = ((i32)ms % 1000) * 1000000,
  };
  nanosleep(&ts, null);
}

void      os_show_window()              { }
void      os_window_destroy()           { }
OS_Handle os_get_handle_info()          { return 0; }
OS_Handle os_get_window_handle()        { return {}; }
String    os_get_current_directory()    { return st.binary_directory; }
String    os_get_current_binary_name()  { return st.binary_name; }
String    os_get_current_filepath()     { return st.binary_filepath; }

//////////////////////////////////////////////////////////////////////////
// Memory

#define PAGE_SIZE 4096
u8*   os_reserve(u64 size)                  { return (u8*)mmap(null, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); }
u8*   os_shm_reserve(u64 size, OS_Handle* handler) { 
  Scratch scratch;
  String name = String(push_buffer(scratch, 7), 7);
  name.str[0] = '/';
  name.str[7] = 0;
  for (i32 i = 1; i < 6; ++i) {
    name.str[i] = rand_u32();
  }
  String name_c = push_str_copy(scratch, name);
  int fd = shm_open((char*)name_c.str, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  shm_unlink((char*)name_c.str);
  if (fd < 0) Assert(0);

  if (ftruncate(fd, size) < 0) Assert(0);

  As(int)handler = fd;

  void* ptr = mmap(NULL, size, PROT_NONE, MAP_SHARED, fd, 0);
  return (u8*)ptr;
}
b32   os_commit(void* ptr, u64 size)        { return mprotect(ptr, size, PROT_READ | PROT_WRITE); }
void  os_decommit(void* ptr, u64 size)      { }
void  os_release(void* ptr, u64 size)       { munmap(ptr, size);}
void* os_reserve_large(u64 size)            { return 0; }
b32   os_commit_large(void* ptr, u64 size)  { return 1; }

//////////////////////////////////////////////////////////////////////////
// File

OS_Handle os_file_open(String path, OS_AccessFlags flags) {
  Scratch scratch;
  String path_c = push_str_copy(scratch.arena, path);
  int lnx_flags = 0;
  if(flags & OS_AccessFlag_Read && flags & OS_AccessFlag_Write) {
    lnx_flags = O_RDWR;
  }
  else if(flags & OS_AccessFlag_Write) {
    lnx_flags = O_WRONLY;
  }
  else if(flags & OS_AccessFlag_Read) {
    lnx_flags = O_RDONLY;
  }
  if(flags & OS_AccessFlag_Append) {
    lnx_flags |= O_APPEND;
  }
  if(flags & (OS_AccessFlag_Write|OS_AccessFlag_Append)) {
    lnx_flags |= O_CREAT;
  }
  int fd = open((char*)path_c.str, lnx_flags, 0755);
  OS_Handle handle = fd;
  return handle;
}

void os_file_close(OS_Handle file) {
  close(file);
}

u64 os_file_read(OS_Handle file, u64 size, u8* out_data) {
  u64 size_read = read(file, out_data, size);
  return size_read;
}

u64 os_file_write(OS_Handle file, u64 size, u8* data) {
  u64 size_written = write(file, data, size);
  return size_written;
}

u64 os_file_size(OS_Handle file) {
  FileProperties props = os_properties_from_file(file);
  return props.size;
}

////////////////////////////////////////////////////////////////////////
// Directory
OS_Handle os_directory_open(String path) {
  return {};
}

void os_directory_watch(OS_Handle dir_handle, u32 id) {
}

String os_directory_watch_pop_name(Arena* arena, OS_Handle dir, u32 id) {
  return "hello";
}

b32 os_directory_check_change(OS_Handle dir_handle, u32 id) {
  return 0;
}

FileProperties os_properties_from_file(OS_Handle file) {
  Scratch scratch;
  
  struct stat file_stat;
  fstat(file, &file_stat);
  FileProperties props = {
    .size = (u64)file_stat.st_size,
    .modified = file_stat.st_mtim.tv_sec*1000000000ULL + file_stat.st_mtim.tv_nsec,
  };
  return props;
}

FileProperties os_properties_from_file_path(String path) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  
  struct stat file_stat;
  stat((char*)path_c.str, &file_stat);
  FileProperties props = {
    .size = (u64)file_stat.st_size,
    .modified = file_stat.st_mtim.tv_sec*1000000000ull + file_stat.st_mtim.tv_nsec,
  };
  return props;
}

b32 os_copy_file_path(String dst, String src) {
  OS_Handle fd_src = os_file_open(src, OS_AccessFlag_Read);
  OS_Handle fd_dst = os_file_open(dst, OS_AccessFlag_Write);

  FileProperties props = os_properties_from_file(fd_src);
  u64 size = props.size;
  sendfile(fd_dst, fd_src, null, size);

  os_file_close(fd_src);
  os_file_close(fd_dst);
  return true;
}

b32 os_file_path_exists(String path) {
  return 0;
}

b32 os_file_compare_time(u64 new_write_time, u64 last_write_time) {
  if (new_write_time > last_write_time) {
    return true;
  }
  return false;
}

OS_Handle os_lib_open(String path) { 
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  OS_Handle result = (OS_Handle)dlopen((char*)path_c.str, RTLD_NOW); 
  AssertMsg(result, "dlopen failed: %s", String(dlerror()));
  return result;
 }
void os_lib_close(OS_Handle lib)   { dlclose((void*)lib); }

VoidProc* os_lib_get_proc(OS_Handle lib, String name) {
  Scratch scratch;
  String name_c = push_str_copy(scratch, name);
  void* result = dlsym((void*)lib, (char*)name_c.str);
  AssertMsg(result, "dlopen failed: %s", String(dlerror()));
  return (VoidProc*)result;
}

void os_process_create(String cmd) {
}

#endif
