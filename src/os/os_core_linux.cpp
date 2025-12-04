#include "lib.h"
#include <cstdio>

#if OS_LINUX

#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <sys/inotify.h>
#include <sys/time.h>
#include <dirent.h>

struct OS_LNX_FileIter {
  DIR* dir;
  struct dirent* dp;
  String path;
};

struct OS_State {
  Arena* arena;

  String binary_name;
  String binary_filepath;
  String binary_directory;
};

global OS_State st;

String os_get_current_directory()    { return st.binary_directory; }
String os_get_current_binary_name()  { return st.binary_name; }
String os_get_current_filepath()     { return st.binary_filepath; }

void os_exit(i32 exit_code) { _exit(exit_code); }

void os_init() {
  Arena* arena = arena_alloc();
  const u32 max_path_length = 512;
  u8* buff[max_path_length];
  u32 size = readlink("/proc/self/exe", (char*)buff, max_path_length);
  String name = push_str_copy(arena, String((u8*)buff, size));
  st = {
    .arena = arena,
    .binary_name = str_skip_last_slash(name),
    .binary_filepath = name,
    .binary_directory = str_chop_last_slash(name),
  };
}

u64 os_timer_frequency() {
  return Million(1);
}

u64 os_timer_now() {
  struct timeval time;
  gettimeofday(&time, null);
  u64 result = os_timer_frequency()*time.tv_sec + time.tv_usec;
  return result;
}

f64 os_now_seconds() {
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (f64)ts.tv_sec + NsToSec((f64)ts.tv_nsec);
}

void os_sleep(u64 ms) { 
  timespec ts = {
    .tv_sec = (i32)ms / Thousand(1),
    .tv_nsec = ((i32)ms % Thousand(1)) * Million(1),
  };
  nanosleep(&ts, null);
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

FileProperties os_lnx_file_properties_from_stat(struct stat s) {
  FileProperties props = {
    .size = (u64)s.st_size,
    .modified = s.st_mtim.tv_sec*Billion(1) + s.st_mtim.tv_nsec,
    .created = s.st_ctim.tv_sec*Billion(1) + s.st_ctim.tv_nsec,
  };
  return props;
}

//////////////////////////////////////////////////////////////////////////
// Memory

u8*  os_reserve(u64 size)                  { return (u8*)mmap(null, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); }
b32  os_commit(void* ptr, u64 size)        { return mprotect(ptr, size, PROT_READ | PROT_WRITE); }
void os_decommit(void* ptr, u64 size)      { }
void os_release(void* ptr, u64 size)       { munmap(ptr, size);}
u8*  os_reserve_large(u64 size)            { return 0; }
b32  os_commit_large(void* ptr, u64 size)  { return 1; }

//////////////////////////////////////////////////////////////////////////
// Files

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
  FileProperties props = os_file_properties(file);
  return props.size;
}

FileProperties os_file_properties(OS_Handle file) {
  Scratch scratch;
  struct stat st;
  fstat(file, &st);
  FileProperties props = os_lnx_file_properties_from_stat(st);
  return props;
}

b32 os_file_path_exists(String path) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  int access_result = access((char*)path_c.str, F_OK);
  b32 result = 0;
  if (access_result == 0) {
    result = true;
  }
  return result;
}

b32 os_file_path_copy(String dst, String src) {
  OS_Handle src_h = os_file_open(src, OS_AccessFlag_Read);
  OS_Handle dst_h = os_file_open(dst, OS_AccessFlag_Write);
  FileProperties props = os_file_properties(src_h);
  int src_fd = src_h;
  int dst_fd = dst_h;
  sendfile(dst_fd, src_fd, null, props.size);
  props = os_file_properties(src_h);
  os_file_close(src_h);
  os_file_close(dst_h);
  return true;
}

void os_file_path_time_copy(String src, String dst) {
  Scratch scratch;
  String src_c = push_str_copy(scratch, src);
  String dst_c = push_str_copy(scratch, dst);
  struct stat st;
  stat((char*)src_c.str, &st);
  struct timespec times[2];
  times[0] = st.st_atim; // preserve access time
  times[1] = st.st_mtim; // copy modification time
  utimensat(AT_FDCWD, (char*)dst_c.str, times, 0);
}

FileProperties os_file_path_properties(String path) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  struct stat st;
  stat((char*)path_c.str, &st);
  FileProperties props = os_lnx_file_properties_from_stat(st);
  return props;
}

Buffer os_file_all_read(Arena* arena, String path) {
  Scratch scratch(&arena);
  OS_Handle f = os_file_open(path, OS_AccessFlag_Read);
  Assert(f);

  u64 file_size = os_file_size(f);
  u8* buffer = push_buffer(arena, file_size);
  u64 read_size = os_file_read(f, file_size, buffer);
  Assert(read_size);
  os_file_close(f);

  Buffer result = {
    .data = buffer,
    .size = file_size,
  };
  return result;
}

// Directory

OS_Handle os_directory_open(String path) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  DIR* dir = opendir((char*)path_c.str);
  OS_Handle result = (OS_Handle)dir;
  return result;
}

void os_directory_create(String path) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  mkdir((char*)path_c.str, S_IRWXU);
}

void os_directory_create_recursively(String path) {
  Scratch scratch;
  OS_Handle result;
  Loop (i, path.size) {
    u8 ch = path.str[i];
    if (char_is_slash(ch)) {
      String path_c = push_str_copy(scratch, str_prefix(path, i));
      result = mkdir((char*)path_c.str, S_IRWXU);
    }
  }
  String path_c = push_str_copy(scratch, path);
  result = mkdir((char*)path_c.str, S_IRWXU);
}

// Watch

OS_Watch os_watch_open(OS_WatchFlags flags) {
  OS_Handle fd = inotify_init1(IN_NONBLOCK);
  OS_Watch result = {
    .handle = fd,
    .flags = flags,
  };
  return result;
}

void os_watch_close(OS_Watch watch) {
  close(watch.handle);
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
  if (watch.flags & OS_WatchFlag_CloseWrite) {
    lnx_flags |= IN_CLOSE_WRITE;
  }
  if (watch.flags & OS_WatchFlag_MovedTo) {
    lnx_flags |= IN_MOVED_TO;
  }
  OS_Handle wd = inotify_add_watch(watch.handle, (char*)name_c.str, lnx_flags);
  return wd;
}

void os_watch_deattach(OS_Watch watch, OS_Handle attached) {
  inotify_rm_watch(watch.handle, attached);
}

StringList os_watch_check(Arena* arena, OS_Watch watch) {
  u8* buff = push_buffer(arena, KB(1));
  u64 read_size = os_file_read(watch.handle, KB(1), buff);
  if (read_size == -1) {
    return {};
  }
  StringList result = {};
  int offset = 0;
  while (offset < read_size) {
    struct inotify_event* event = (struct inotify_event*)&buff[offset];
    if (event->len) {
      int lnx_flags = 0;
      str_list_push(arena, &result, event->name);
    }
    offset += sizeof(struct inotify_event) + event->len;
  }
  return result;
}

// Directory iteration

OS_FileIter* os_file_iter_begin(Arena* arena, String path, OS_FileIterFlags flags) {
  OS_FileIter* base_iter = push_struct(arena, OS_FileIter);
  base_iter->flags = flags;
  OS_LNX_FileIter* iter = (OS_LNX_FileIter*)base_iter->memory;
  String path_c = push_str_copy(arena, path);
  iter->dir = opendir((char*)path_c.str);
  iter->path = path_c;
  return base_iter;
}

b32 os_file_iter_next(Arena* arena, OS_FileIter* iter, OS_FileInfo* info_out) {
  Scratch scratch(&arena);
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
    struct stat st;
    if (good) {
      String full_path = push_strf(scratch, "%s/%s", lnx_iter->path, String(lnx_iter->dp->d_name));
      stat((char*)full_path.str, &st);

      filtered = ((st.st_mode == S_IFDIR && iter->flags & OS_FileIterFlag_SkipFolders) ||
                  (st.st_mode == S_IFREG && iter->flags & OS_FileIterFlag_SkipFiles) ||
                  (lnx_iter->dp->d_name[0] == '.' && lnx_iter->dp->d_name[1] == 0) ||
                  (lnx_iter->dp->d_name[0] == '.' && lnx_iter->dp->d_name[1] == '.' && lnx_iter->dp->d_name[2] == 0));
    }

    // write output
    if (good && !filtered) {
      info_out->name = push_str_copy(arena, String(lnx_iter->dp->d_name));
      info_out->props = os_lnx_file_properties_from_stat(st);
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

OS_Handle os_process_launch(String cmd) {
  pid_t pid = fork();
  if (pid == 0) {
    Scratch scratch;
    char** argv = push_array(scratch, char*, 10 + 2);
    u32 args_count = 0;
    u32 start = 0;
    String next_word;
    while ((next_word = str_next_word(cmd, start)).size) {
      String arg_c = push_str_copy(scratch, next_word);
      argv[args_count] = (char*)arg_c.str;
      ++args_count;
    }
    argv[args_count] = null;
    execvp(argv[0], argv);
    os_exit(0);
  }
  return pid;
}

void os_process_join(OS_Handle handle) {
  int pid = handle;
  int status;
  waitpid(pid, &status, 0);
}

////////////////////////////////////////////////////////////////////////
// Lib

OS_Handle os_lib_open(String path) { 
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  OS_Handle result = (OS_Handle)dlopen((char*)path_c.str, RTLD_NOW); 
  AssertMsg(result, "dlopen failed: %s", String(dlerror()));
  return result;
}

void os_lib_close(OS_Handle lib) { dlclose((void*)lib); }

void* os_lib_get_proc(OS_Handle lib, String name) {
  Scratch scratch;
  String name_c = push_str_copy(scratch, name);
  void* result = dlsym((void*)lib, (char*)name_c.str);
  AssertMsg(result, "dlsym failed: %s", String(dlerror()));
  return result;
}


#endif
