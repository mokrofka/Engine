#include "lib.h"

#if OS_LINUX

#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <dirent.h>

struct OS_LNX_FileIter {
  DIR *dir;
  struct dirent *dp;
  String path;
};

struct OS_State {
  Arena* arena;

  String binary_name;
  String binary_filepath;
  String binary_directory;
};

f32 delta_time;

global OS_State st;

void os_exit(i32 exit_code) {
  _exit(exit_code);
}

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
  return (f64)ts.tv_sec + NsToSec(ts.tv_nsec);
}

void os_sleep(u64 ms) { 
  timespec ts = {
    .tv_sec = (i32)ms / Thousand(1),
    .tv_nsec = ((i32)ms % Thousand(1)) * Million(1),
  };
  nanosleep(&ts, null);
}

String os_get_current_directory()    { return st.binary_directory; }
String os_get_current_binary_name()  { return st.binary_name; }
String os_get_current_filepath()     { return st.binary_filepath; }

FileProperties os_lnx_file_properties_from_stat(struct stat s) {
  FileProperties props = {
    .size = (u64)s.st_size,
    .modified = s.st_mtim.tv_sec*1000000000ull + s.st_mtim.tv_nsec,
    .created = s.st_ctim.tv_sec*1000000000ull + s.st_ctim.tv_nsec,
  };
  return props;
}

//////////////////////////////////////////////////////////////////////////
// Memory

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
u8*   os_reserve(u64 size)                  { return (u8*)mmap(null, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); }
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
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  DIR *dir = opendir((char*)path_c.str);
  return (OS_Handle)dir;
}

void os_directory_create(String path) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  mkdir((char*)path_c.str, S_IRWXU);
}

OS_Handle os_directory_create_p(String path) {
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

  return result;
}

b32 os_directory_path_exist(String path) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  b32 result = 0;
  struct stat st;
  b32 success = stat((char*)path_c.str, &st);

  if (success == 0 && S_ISDIR(st.st_mode)) {
    result = true;
  }

  return result;
}

void os_directory_watch(OS_Handle dir_handle, u32 id) {
}

String os_directory_watch_pop_name(Arena* arena, OS_Handle dir, u32 id) {
  return "hello";
}

b32 os_directory_check_change(OS_Handle dir_handle, u32 id) {
  return 0;
}

OS_FileIter* os_file_iter_begin(Arena* arena, String path, OS_FileIterFlags flags) {
  OS_FileIter* base_iter = push_struct(arena, OS_FileIter);
  base_iter->flags = flags;
  OS_LNX_FileIter *iter; Assign(iter, base_iter->memory);
  String path_c = push_str_copy(arena, path);
  iter->dir = opendir((char *)path_c.str);
  iter->path = path_c;
  return base_iter;
}

b32 os_file_iter_next(Arena* arena, OS_FileIter* iter, OS_FileInfo* info_out) {
  Scratch scratch(&arena);
  OS_LNX_FileIter* lnx_iter = (OS_LNX_FileIter*)iter->memory;

  b32 good = 0;
  while (true) {
    lnx_iter->dp = readdir(lnx_iter->dir);
    good = (lnx_iter->dp != 0);

    b32 filtered = 0;
    struct stat st;
    if (good) {
      String full_path = push_strf(scratch, "%s/%s", lnx_iter->path, String(lnx_iter->dp->d_name));
      stat((char*)full_path.str, &st);

      filtered = ((FlagExists(st.st_mode, S_IFDIR) && iter->flags & OS_FileIterFlag_SkipFolders) ||
                  (FlagExists(st.st_mode, S_IFREG) && iter->flags & OS_FileIterFlag_SkipFiles) ||
                  (lnx_iter->dp->d_name[0] == '.' && lnx_iter->dp->d_name[1] == 0) ||
                  (lnx_iter->dp->d_name[0] == '.' && lnx_iter->dp->d_name[1] == '.' && lnx_iter->dp->d_name[2] == 0));
    }

    if (good && !filtered) {
      info_out->name = push_str_copy(arena, String(lnx_iter->dp->d_name));
      info_out->props = os_lnx_file_properties_from_stat(st);
      break;
    }

    if (!good) {
      break;
    }
  }
  
  return good;
}

void os_file_iter_end(OS_FileIter *iter) {
  OS_LNX_FileIter *lnx_iter; Assign(lnx_iter, iter->memory);
  closedir(lnx_iter->dir);
}

FileProperties os_properties_from_file(OS_Handle file) {
  Scratch scratch;
  
  struct stat st;
  fstat(file, &st);
  FileProperties props = os_lnx_file_properties_from_stat(st);
  return props;
}

FileProperties os_properties_from_file_path(String path) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  
  struct stat st;
  stat((char*)path_c.str, &st);
  FileProperties props = os_lnx_file_properties_from_stat(st);
  return props;
}

b32 os_copy_file_path(String dst, String src) {
  OS_Handle src_h = os_file_open(src, OS_AccessFlag_Read);
  OS_Handle dst_h = os_file_open(dst, OS_AccessFlag_Write);

  FileProperties props = os_properties_from_file(src_h);
  int src_fd = src_h;
  int dst_fd = dst_h;
  sendfile(dst_fd, src_fd, null, props.size);

  os_file_close(src_h);
  os_file_close(dst_h);
  return true;
}

b32 os_file_path_exists(String path) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  b32 result = 0;
  struct stat st;
  stat((char*)path_c.str, &st);

  if (S_ISREG(st.st_mode)) {
    result = true;
  }

  return result;
}

b32 os_file_compare_time(u64 new_write_time, u64 last_write_time) {
  if (new_write_time > last_write_time) {
    return true;
  }
  return false;
}

b32 os_file_path_compare_time(String a, String b) {
  Scratch scratch;
  String a_c = push_str_copy(scratch, a);
  String b_c = push_str_copy(scratch, b);
  struct stat st_a, st_b;
  stat((char*)a_c.str, &st_a);
  stat((char*)b_c.str, &st_b);
  FileProperties props_a = os_lnx_file_properties_from_stat(st_a);
  FileProperties props_b = os_lnx_file_properties_from_stat(st_b);
  
  b32 result = 0;
  if (props_a.modified != props_b.modified) {
    result = true;
  }
  return result;
}

void os_file_copy_mtime(String src, String dst) {
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
void os_lib_close(OS_Handle lib)   { dlclose((void*)lib); }

void* os_lib_get_proc(OS_Handle lib, String name) {
  Scratch scratch;
  String name_c = push_str_copy(scratch, name);
  void* result = dlsym((void*)lib, (char*)name_c.str);
  AssertMsg(result, "dlopen failed: %s", String(dlerror()));
  return result;
}


#endif
