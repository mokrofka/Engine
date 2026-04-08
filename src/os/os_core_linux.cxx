#include "lib.h"

#if OS_LINUX

#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <spawn.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

struct OS_LNX_FileIter {
  DIR* dir;
  struct dirent* dp;
  String path;
};

struct OS_State {
  Allocator arena;
  String binary_filepath;
  String binary_directory;
  String binary_name;
};

global OS_State os_st;

String os_get_current_filepath()     { return os_st.binary_filepath; }
String os_get_current_directory()    { return os_st.binary_directory; }
String os_get_current_binary_name()  { return os_st.binary_name; }

void os_init(String name) {
  os_st.arena = arena_init();
  os_st.binary_filepath = name;
  os_st.binary_directory = str_chop_last_slash(name);
  os_st.binary_name = str_skip_last_slash(name);
}

void os_exit(i32 exit_code) { _exit(exit_code); }

u64 os_timer_frequency() { return Million(1); }

u64 os_timer_now() {
  struct timeval time;
  gettimeofday(&time, null);
  u64 result = time.tv_usec + os_timer_frequency()*time.tv_sec;
  return result;
}

u64 os_now_ns() {
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec*Billion(1) + ts.tv_nsec;
}

void os_sleep_ms(u64 ms) { 
  timespec ts;
  ts.tv_sec = ms / Thousand(1);
  ts.tv_nsec = (ms % Thousand(1)) * Million(1);
  nanosleep(&ts, null);
}

void os_console_write(String message, u32 color) {
  String color_str;
  switch (color) {
    case 0: color_str = "\x1b[0m";  break; // Reset
    case 1: color_str = "\x1b[90m"; break; // Gray
    case 2: color_str = "\x1b[36m"; break; // Cyan
    case 3: color_str = "\x1b[32m"; break; // Green
    case 4: color_str = "\x1b[33m"; break; // Yellow
    case 5: color_str = "\x1b[31m"; break; // Red
  }
  iovec iov[] = {
    {.iov_base = color_str.str, .iov_len = color_str.size},
    {.iov_base = message.str, .iov_len = message.size}
  };
  writev(STDOUT_FILENO, iov, ArrayCount(iov));
}

String os_get_environment(String name) {
  Scratch scratch;
  String name_c = push_str_copy(scratch, name);
  String result = getenv((char*)name_c.str);
  return result;
}

//////////////////////////////////////////////////////////////////////////
// Memory

u8*  os_reserve(u64 size)                  { return (u8*)mmap(null, size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0); }
b32  os_commit(void* ptr, u64 size)        { return mprotect(ptr, size, PROT_READ | PROT_WRITE); }
void os_decommit(void* ptr, u64 size)      { }
void os_release(void* ptr, u64 size)       { munmap(ptr, size);}
u8*  os_reserve_large(u64 size)            { return 0; }
b32  os_commit_large(void* ptr, u64 size)  { return 0; }

//////////////////////////////////////////////////////////////////////////
// Files

FileProperties os_lnx_file_properties_from_stat(struct stat fd_stat) {
  FileProperties props = {
    .size = (u64)fd_stat.st_size,
    .modified = fd_stat.st_mtim.tv_sec*Billion(1) + fd_stat.st_mtim.tv_nsec,
    .created = fd_stat.st_ctim.tv_sec*Billion(1) + fd_stat.st_ctim.tv_nsec,
  };
  if(fd_stat.st_mode & S_IFDIR) {
    props.flags |= FilePropertyFlag_IsFolder;
  }
  return props;
}

OS_Handle os_file_open(String path, OS_AccessFlags flags) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
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
  OS_Handle handle = {};
  if (fd != -1) {
    handle.v = fd;
  }
  return handle;
}

void os_file_close(OS_Handle file) {
  if (file.v == 0) { return; }
  int fd = file.v;
  close(fd); 
}

u64 os_file_read(OS_Handle file, u64 size, void* out_data) {
  if (file.v == 0) { return 0; }
  int fd = file.v;
  u64 read_result = read(fd, out_data, size);
  return read_result;
}

u64 os_file_write(OS_Handle file, u64 size, void* data) {
  if (file.v == 0) { return 0; }
  int fd = file.v;
  u64 size_written = write(fd, data, size);
  return size_written;
}

u64 os_file_size(OS_Handle file) {
  FileProperties props = os_file_properties(file);
  return props.size;
}

FileProperties os_file_properties(OS_Handle file) {
  if (file.v == 0) { return {}; }
  Scratch scratch;
  struct stat fd_stat = {};
  int fd = file.v;
  int fstat_result = fstat(fd, &fd_stat);
  FileProperties props = {};
  if (fstat_result != -1) {
    props = os_lnx_file_properties_from_stat(fd_stat);
  }
  return props;
}

b32 os_file_path_exists(String path) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  struct stat fd_stat = {};
  int stat_result = stat((char*)path_c.str, &fd_stat);
  if (stat_result != -1) {
    return true;
  }
  return false;
}

b32 os_file_path_copy(String dst, String src) {
  b32 result = 0;
  OS_Handle src_h = os_file_open(src, OS_AccessFlag_Read);
  OS_Handle dst_h = os_file_open(dst, OS_AccessFlag_Write);
  if (src_h.v != 0 && dst_h.v != 0) {
    FileProperties props = os_file_properties(src_h);
    int src_fd = src_h.v;
    int dst_fd = dst_h.v;
    sendfile(dst_fd, src_fd, null, props.size);
    result = true;
  }
  os_file_close(src_h);
  os_file_close(dst_h);
  return result;
}

void os_file_path_copy_mtime(String src, String dst) {
  Scratch scratch;
  String src_c = push_str_copy(scratch, src);
  String dst_c = push_str_copy(scratch, dst);
  struct stat fd_stat;
  stat((char*)src_c.str, &fd_stat);
  struct timespec times[2];
  times[0] = fd_stat.st_atim;
  times[1] = fd_stat.st_mtim;
  utimensat(AT_FDCWD, (char*)dst_c.str, times, 0);
}

FileProperties os_file_path_properties(String path) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  struct stat fd_stat = {};
  stat((char*)path_c.str, &fd_stat);
  FileProperties props = os_lnx_file_properties_from_stat(fd_stat);
  return props;
}

Buffer os_file_path_read_all(Allocator arena, String path) {
  Scratch scratch(arena);
  OS_Handle f = os_file_open(path, OS_AccessFlag_Read);
  u64 file_size = os_file_size(f);
  u8* buffer = push_buffer(arena, file_size);
  u64 read_size = os_file_read(f, file_size, buffer);
  os_file_close(f);
  Buffer result = {
    .data = buffer,
    .size = file_size,
  };
  return result;
}

b32 os_file_path_equal_mtime(String a, String b) {
  FileProperties props_a = os_file_path_properties(a);
  FileProperties props_b = os_file_path_properties(b);
  if (props_a.modified == props_b.modified) {
    return true;
  }
  return false;
}

// Directory

OS_Handle os_directory_open(String path) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  DIR* dir = opendir((char*)path_c.str);
  OS_Handle result = {};
  if (dir != null) {
    result.v = (u64)dir;
  }
  return result;
}

OS_Handle os_directory_create(String path) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  int fd = mkdir((char*)path_c.str, S_IRWXU);
  OS_Handle result = {};
  if (fd != -1) {
    result.v = fd;
  }
  return result;
}

OS_Handle os_directory_create_p(String path) {
  Scratch scratch;
  Loop (i, path.size) {
    if (char_is_slash(path.str[i])) {
      String parent_dir = push_str_copy(scratch, str_prefix(path, i));
      if (!os_directory_path_exist(parent_dir)) {
        os_directory_create(parent_dir);
      }
    }
  }
  OS_Handle result = os_directory_create(path);
  return result;
}

b32 os_directory_path_exist(String path) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  os_file_path_properties(path);
  struct stat st;
  b32 success = stat((char*)path_c.str, &st);
  if (success == 0 && S_ISDIR(st.st_mode)) {
    return true;
  }
  return false;
}

// Watch

OS_Watch os_watch_open(OS_WatchFlags flags) {
  int fd = inotify_init1(IN_NONBLOCK);
  OS_Watch result = {
    .handle = {(u64)fd},
    .flags = flags,
  };
  return result;
}

void os_watch_close(OS_Watch watch) {
  int fd = watch.handle.v;
  close(fd);
}

OS_Handle os_watch_attach(OS_Watch watch, String name) {
  Scratch scratch;
  String name_c = push_str_copy(scratch, name);
  int lnx_flags = 0;
  if (watch.flags & OS_WatchFlag_Create) {
    lnx_flags |= IN_CREATE;
  }
  if (watch.flags & OS_WatchFlag_Delete) {
    lnx_flags |= IN_DELETE;
  }
  if (watch.flags & OS_WatchFlag_Modify) {
    lnx_flags |= IN_MODIFY;
  }
  int watch_fd = watch.handle.v;
  int fd = inotify_add_watch(watch_fd, (char*)name_c.str, lnx_flags);
  OS_Handle result = {};
  if (fd != -1) {
    result.v = fd;
  }
  return result;
}

void os_watch_deattach(OS_Watch watch, OS_Handle attached) {
  int watch_fd = watch.handle.v;
  int fd = attached.v;
  inotify_rm_watch(watch_fd, fd);
}

StringList os_watch_check(Allocator arena, OS_Watch watch) {
  u8* buf = push_buffer(arena, KB(1));
  u64 read_size = os_file_read(watch.handle, KB(1), buf);
  if (read_size == -1) {
    return {};
  }
  StringList result = {};
  int offset = 0;
  while (offset < read_size) {
    struct inotify_event* event = (struct inotify_event*)&buf[offset];
    if (event->len) {
      str_list_push(arena, &result, event->name);
    }
    offset += sizeof(struct inotify_event) + event->len;
  }
  return result;
}

// Directory iteration

OS_FileIter* os_file_iter_begin(Allocator arena, String path, OS_FileIterFlags flags) {
  OS_FileIter* base_iter = push_struct(arena, OS_FileIter);
  base_iter->flags = flags;
  OS_LNX_FileIter* iter = (OS_LNX_FileIter*)base_iter->memory;
  String path_c = push_str_copy(arena, path);
  iter->dir = opendir((char*)path_c.str);
  iter->path = path_c;
  return base_iter;
}

b32 os_file_iter_next(Allocator arena, OS_FileIter* iter, OS_FileInfo* info_out) {
  Scratch scratch(arena);
  b32 good = 0;
  OS_LNX_FileIter* lnx_iter = (OS_LNX_FileIter*)iter->memory;
  while (true) {
    // get next entry
    lnx_iter->dp = readdir(lnx_iter->dir);
    good = (lnx_iter->dp != 0);
    if (!good) {
      return false;
    }

    // filter
    b32 filtered = 0;
    struct stat fd_stat;
    if (good) {
      String full_path = push_strf(scratch, "%s/%s", lnx_iter->path, String(lnx_iter->dp->d_name));
      stat((char*)full_path.str, &fd_stat);
      filtered = ((S_ISDIR(fd_stat.st_mode) && iter->flags & OS_FileIterFlag_SkipFolders) ||
                  (S_ISREG(fd_stat.st_mode) && iter->flags & OS_FileIterFlag_SkipFiles) ||
                  (lnx_iter->dp->d_name[0] == '.' && lnx_iter->dp->d_name[1] == 0) ||
                  (lnx_iter->dp->d_name[0] == '.' && lnx_iter->dp->d_name[1] == '.' && lnx_iter->dp->d_name[2] == 0));
    }

    // write output
    if (good && !filtered) {
      info_out->name = push_str_copy(arena, String(lnx_iter->dp->d_name));
      info_out->props = os_lnx_file_properties_from_stat(fd_stat);
      break;
    }
  }
  return good;
}

void os_file_iter_end(OS_FileIter *iter) {
  OS_LNX_FileIter *lnx_iter = (OS_LNX_FileIter*)iter->memory;
  closedir(lnx_iter->dir);
}

////////////////////////////////////////////////////////////////////////
// Processes

OS_Handle os_process_launch(StringList list) {
  Scratch scratch;
  OS_Handle handle = {};
  char** argv = push_array(scratch, char*, list.node_count + 1);
  argv[list.node_count] = null;
  u32 i = 0;
  for EachNode(n, StringNode, list.first) {
    argv[i++] = (char*)n->string.str;
  }
  pid_t pid = 0;
  int spawn_code = posix_spawnp(&pid, argv[0], null, null, argv, null);
  if (spawn_code == 0) {
    handle.v = spawn_code;
  }
  return handle;
}

i32 os_process_join(OS_Handle handle) {
  int pid = handle.v;
  int status;
  waitpid(pid, &status, 0);
  return status;
}

////////////////////////////////////////////////////////////////////////
// Lib

OS_Handle os_lib_open(String path) { 
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  void* so = dlopen((char*)path_c.str, RTLD_NOW); 
  OS_Handle result = {(u64)so};
  AssertMsg(result.v, "dlopen failed: %s", String(dlerror()));
  return result;
}

void os_lib_close(OS_Handle lib) {
  void* so = (void*)lib.v;
  dlclose(so);
}

void* os_lib_get_proc(OS_Handle lib, String name) {
  Scratch scratch;
  String name_c = push_str_copy(scratch, name);
  void* so = (void*)lib.v;
  void* result = dlsym(so, (char*)name_c.str);
  AssertMsg(result, "dlsym failed: %s", String(dlerror()));
  return result;
}

#endif
