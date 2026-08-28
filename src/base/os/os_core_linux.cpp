#include "../base_impl.h"

#if OS_LINUX

#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
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

_LockScope::_LockScope(Mutex mutex_) {
  mutex = mutex_;
  os_mutex_lock(mutex);
}
_LockScope::~_LockScope() {
  os_mutex_unlock(mutex);
}

global u64 _cpu_frequency;
u64 cpu_timer_now() { return __rdtsc(); }
u64 cpu_frequency() { return _cpu_frequency; }
void cpu_find_frequency() {
  u64 os_freq = os_timer_frequency();
  u64 cpu_start = cpu_timer_now();
  u64 os_start = os_timer_now();
  u64 milliseconds = 1;
  u64 os_end = 0;
  u64 os_elapsed = 0;
  u64 os_wait_time = os_freq * milliseconds / 1000;
  while (os_elapsed < os_wait_time) {
    os_end = os_timer_now();
    os_elapsed = os_end - os_start;
  }
  u64 cpu_end = cpu_timer_now();
  u64 cpu_elapsed = cpu_end - cpu_start;
  u64 cpu_freq = 0;
  if (cpu_elapsed) {
    cpu_freq = os_freq * cpu_elapsed / os_elapsed;
  }
  _cpu_frequency = cpu_freq;
}

struct OS_LNX_FileIter {
  DIR* dir;
  struct dirent* dp;
  String path;
};

enum OS_LNX_EntityType {
  LNX_EntityType_Thread,
  LNX_EntityType_Mutex,
  LNX_EntityType_MutexRW,
  LNX_EntityType_Semaphore,
  LNX_EntityType_Barrier,
};

struct OS_LNX_Entity {
  OS_LNX_Entity* next;
  OS_LNX_EntityType type;
  union {
    struct {
      pthread_t handle;
      ThreadEntryPointFn* func;
      void* ptr;
    } thread;
    pthread_mutex_t mutex;
    pthread_rwlock_t rwmutex;
    pthread_cond_t cv;
    sem_t semaphore;
    pthread_barrier_t barrier;
  };
};

struct OS_State {
  Arena arena;
  OS_LNX_Entity* entity_free;
  Array<OS_LNX_Entity, 128> entities;
  String binary_filepath;
  String binary_directory;
  String binary_name;
  u64 mem_commited;
  u64 mem_address_space_reserve;
};

global OS_State os_st;

OS_LNX_Entity* os_lnx_entity_alloc(OS_LNX_EntityType type) {
  OS_LNX_Entity* entity = 0;
  entity = os_st.entity_free;
  if (entity) {
    sll_stack_pop(os_st.entity_free);
  } else {
    entity = &os_st.entities[os_st.entities.count];
    array_push(os_st.entities, {});
  }
  entity->type = type;
  return entity;
}

void os_lnx_entity_release(OS_LNX_Entity* entity) {
  sll_stack_push(os_st.entity_free, entity);
}

String os_cur_filepath()     { return os_st.binary_filepath; }
String os_cur_directory()    { return os_st.binary_directory; }
String os_cur_binary_name()  { return os_st.binary_name; }
u64 os_commited_size()       { return os_st.mem_commited; }
u64 os_reserved_size()       { return os_st.mem_address_space_reserve; }

void os_init(String name) {
  os_st.arena = arena_make("os arena");
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

void os_sleep_us(u64 us) {
  timespec ts;
  ts.tv_sec = us / Million(1);
  ts.tv_nsec = (us % Million(1)) * Thousand(1);
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

u8*  os_reserve(u64 size)                  { atomic_add(&os_st.mem_address_space_reserve, size); return (u8*)mmap(null, size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0); }
b32  os_commit(void* ptr, u64 size)        { atomic_add(&os_st.mem_commited, size); return mprotect(ptr, size, PROT_READ | PROT_WRITE);}
void os_decommit(void* ptr, u64 size)      { mprotect(ptr, size, PROT_NONE);}
void os_release(void* ptr, u64 size)       { munmap(ptr, size);}

//////////////////////////////////////////////////////////////////////////
// Files

FileProperties os_lnx_file_properties_from_stat(struct stat fd_stat) {
  FileProperties props = {
    .size = (u64)fd_stat.st_size,
    .modified = (u64)fd_stat.st_mtim.tv_sec*Billion(1) + fd_stat.st_mtim.tv_nsec,
    .created = (u64)fd_stat.st_ctim.tv_sec*Billion(1) + fd_stat.st_ctim.tv_nsec,
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
  if (flags & OS_AccessFlag_Trunc) {
    lnx_flags |= O_TRUNC;
  }
  if(flags & OS_AccessFlag_Append) {
    lnx_flags |= O_APPEND;
  }
  if(flags & (OS_AccessFlag_Write|OS_AccessFlag_Append)) {
    lnx_flags |= O_CREAT;
  }
  int fd = open((char*)path_c.str, lnx_flags, 0755);
  // if (fd == -1) {
  //   String str = strerror(errno);
  //   Info("%s", str);
  // }
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

u64 os_file_read(OS_Handle file, Slice<u8> out_data) {
  if (file.v == 0) { return 0; }
  int fd = file.v;
  u64 read_result = read(fd, out_data.data, out_data.size);
  return read_result;
}

u64 os_file_write(OS_Handle file, Slice<u8> data) {
  if (file.v == 0) { return 0; }
  int fd = file.v;
  u64 size_written = write(fd, data.data, data.size);
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

b32 os_file_path_copy(String src, String dst) {
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

Slice<u8> os_file_path_read_all(Allocator arena, String path) {
  Scratch scratch(arena);
  OS_Handle f = os_file_open(path, OS_AccessFlag_Read);
  u64 file_size = os_file_size(f);
  Slice buffer = push_buffer_slice(arena, file_size);
  u64 read_size = os_file_read(f, buffer);
  os_file_close(f);
  return {buffer.data, read_size};
}

String os_file_path_read_all_str(Allocator arena, String path) { return str_make(os_file_path_read_all(arena, path)); }

u64 os_file_path_write_all(String path, Slice<u8> data) {
  OS_Handle file = os_file_open(path, OS_AccessFlag_Write);
  u64 write_size = os_file_write(file, data);
  os_file_close(file);
  return write_size;
}

DenseTime os_file_path_mtime(String path) {
  return os_file_path_properties(path).modified;
}

void os_file_path_rename(String path, String new_name) {

}

void os_file_path_remove(String path) {

}

void os_file_path_move(String src, String dst) {

}

///////////////////////////////////
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

OS_Handle os_directory_make(String path) {
  Scratch scratch;
  Loop (i, path.size) {
    if (char_is_slash(path.str[i])) {
      String parent_dir = push_str_copy(scratch, str_prefix(path, i));
      if (!os_directory_path_exist(parent_dir)) {
        mkdir((char*)parent_dir.str, S_IRWXU);
      }
    }
  }
  OS_Handle result = {};
  String path_c = push_str_copy(scratch, path);
  int fd = mkdir((char*)path_c.str, S_IRWXU);
  if (fd != -1) {
    result.v = fd;
  }
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

///////////////////////////////////
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

Slice<String> os_watch_check(Allocator arena, OS_Watch watch) {
  Slice buf = push_buffer_slice(arena, KB(1));
  u64 read_size = os_file_read(watch.handle, buf);
  if (read_size == -1) {
    return {};
  }
  var strs = array_make(String, arena);
  int offset = 0;
  while (offset < read_size) {
    struct inotify_event* event = (struct inotify_event*)&buf[offset];
    if (event->len) {
      array_push(strs, event->name);
    }
    offset += sizeof(struct inotify_event) + event->len;
  }
  return slice(strs);
}

///////////////////////////////////
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

Slice<OS_FileInfo> os_file_iter_directory(Allocator arena, String path, OS_FileIterFlags flags) {
  var file_pathes = array_make(OS_FileInfo, arena);
  OS_FileIter* it = os_file_iter_begin(arena, path, flags);
  for (OS_FileInfo info = {}; os_file_iter_next(arena, it, &info);) {
    array_push(file_pathes, info);
  }
  os_file_iter_end(it);
  return slice(file_pathes);
}

////////////////////////////////////////////////////////////////////////
// Processes

OS_Handle os_process_make(Slice<String> arr) {
  Scratch scratch;
  OS_Handle handle = {};
  char** argv = push_array(scratch, char*, arr.count + 1);
  argv[arr.count] = null;
  Loop (i, arr.count) {
    argv[i] = (char*)arr[i].str;
  }
  pid_t pid = 0;
  int spawn_code = posix_spawnp(&pid, argv[0], null, null, argv, null);
  if (spawn_code == 0) {
    handle.v = spawn_code;
  }
  return handle;
}

OS_Handle os_process_make(StringList list) {
  Scratch scratch;
  var arr = push_slice(scratch, String, list.node_count);
  u32 i = 0;
  LoopNode (it, list.first) {
    arr[i++] = it->string;
  }
  return os_process_make(arr);
}

i32 os_process_join(OS_Handle handle) {
  int pid = handle.v;
  int status;
  waitpid(pid, &status, 0);
  return status;
}

////////////////////////////////////////////////////////////////////////
// Threads

void* os_thread_entry(void* ctx) {
  OS_LNX_Entity* entity = (OS_LNX_Entity*)ctx;
  entity->thread.func(entity->thread.ptr);
  return null;
}

Thread os_thread_make(ThreadEntryPointFn* func, void* ptr) {
  OS_LNX_Entity* entity = os_lnx_entity_alloc(LNX_EntityType_Thread);
  entity->thread.func = func;
  entity->thread.ptr = ptr;
  pthread_create(&entity->thread.handle, null, os_thread_entry, entity);
  Thread handle = {(u64)entity};
  return handle;
}

b32 os_thread_join(Thread handle) {
  int join_result = pthread_join(handle.v, 0);
  b32 result = (join_result == 0);
  return result;
}

void os_thread_detach(Thread handle) {
  pthread_detach(handle.v);
}

///////////////////////////////////
// Sync primitives

Mutex os_mutex_make() {
  OS_LNX_Entity* entity = os_lnx_entity_alloc(LNX_EntityType_Mutex);
  pthread_mutex_init(&entity->mutex, null);
  Mutex handle = {(u64)entity};
  return handle;
}

void os_mutex_destroy(Mutex mutex) {
  OS_LNX_Entity* entity = (OS_LNX_Entity*)mutex.v;
  pthread_mutex_destroy(&entity->mutex);
  os_lnx_entity_release(entity);
}

void os_mutex_lock(Mutex mutex) {
  OS_LNX_Entity* entity = (OS_LNX_Entity*)mutex.v;
  pthread_mutex_lock(&entity->mutex);
}

void os_mutex_unlock(Mutex mutex) {
  OS_LNX_Entity* entity = (OS_LNX_Entity*)mutex.v;
  pthread_mutex_unlock(&entity->mutex);
}

b32 os_mutex_try_lock(Mutex mutex) {
  OS_LNX_Entity* entity = (OS_LNX_Entity*)mutex.v;
  int res = pthread_mutex_trylock(&entity->mutex);
  return res == 0;
}

RWMutex os_rw_mutex_make() {
  OS_LNX_Entity* entity = os_lnx_entity_alloc(LNX_EntityType_Mutex);
  pthread_rwlock_init(&entity->rwmutex, null);
  RWMutex handle = {(u64)entity};
  return handle;
}

void os_rw_mutex_destroy(RWMutex mutex) {
  OS_LNX_Entity* entity = (OS_LNX_Entity*)mutex.v;
  pthread_rwlock_destroy(&entity->rwmutex);
}

void os_rw_mutex_read_lock(RWMutex mutex) {
  OS_LNX_Entity* entity = (OS_LNX_Entity*)mutex.v;
  pthread_rwlock_rdlock(&entity->rwmutex);
}

void os_rw_mutex_write_lock(RWMutex mutex) {
  OS_LNX_Entity* entity = (OS_LNX_Entity*)mutex.v;
  pthread_rwlock_wrlock(&entity->rwmutex);
}

void os_rw_mutex_unlock(RWMutex mutex) {
  OS_LNX_Entity* entity = (OS_LNX_Entity*)mutex.v;
  pthread_rwlock_unlock(&entity->rwmutex);
}

CondVar os_cond_var_make() {
  OS_LNX_Entity* entity = os_lnx_entity_alloc(LNX_EntityType_Mutex);
  pthread_cond_init(&entity->cv, null);
  CondVar handle = {(u64)entity};
  return handle;
}

void os_cond_var_destroy(CondVar cv) {
  OS_LNX_Entity *entity = (OS_LNX_Entity*)cv.v;
  pthread_cond_destroy(&entity->cv);
}

void os_cond_var_wait(CondVar cv, Mutex mutex) {
  OS_LNX_Entity* cv_entity = (OS_LNX_Entity*)cv.v;
  OS_LNX_Entity* mutex_entity = (OS_LNX_Entity*)mutex.v;
  pthread_cond_wait(&cv_entity->cv, &mutex_entity->mutex);
}

void os_cond_var_wake_one(CondVar cv) {
  OS_LNX_Entity* cv_entity = (OS_LNX_Entity*)cv.v;
  pthread_cond_signal(&cv_entity->cv);
}

void os_cond_var_wake_all(CondVar cv) {
  OS_LNX_Entity* cv_entity = (OS_LNX_Entity*)cv.v;
  pthread_cond_broadcast(&cv_entity->cv);
}

Semaphore os_semaphore_make(u32 count) {
  OS_LNX_Entity* s_entity = os_lnx_entity_alloc(LNX_EntityType_Semaphore);
  sem_init(&s_entity->semaphore, 0, count);
  Semaphore handle = {(u64)s_entity};
  return handle;
}

void os_semaphore_destroy(Semaphore semaphore) {
  OS_LNX_Entity* entity = (OS_LNX_Entity*)semaphore.v;
  sem_destroy(&entity->semaphore);
}

void os_semaphore_take(Semaphore semaphore) {
  OS_LNX_Entity* entity = (OS_LNX_Entity*)semaphore.v;
  sem_wait(&entity->semaphore);
}

b32 os_semaphore_try_take(Semaphore semaphore) {
  OS_LNX_Entity* entity = (OS_LNX_Entity*)semaphore.v;
  int res = sem_trywait(&entity->semaphore);
  return res == 0;
}

void os_semaphore_drop(Semaphore semaphore) {
  OS_LNX_Entity* s_entity = (OS_LNX_Entity*)semaphore.v;
  sem_post(&s_entity->semaphore);
}

Barrier os_barrier_make(u64 count) {
  OS_LNX_Entity* entity = os_lnx_entity_alloc(LNX_EntityType_Barrier);
  pthread_barrier_init(&entity->barrier, null, count);
  Barrier handle = {(u64)entity};
  return handle;
}

void os_barrier_destroy(Barrier barrier) {
  OS_LNX_Entity* entity = (OS_LNX_Entity*)barrier.v;
  pthread_barrier_destroy(&entity->barrier);
}

void os_barrier_wait(Barrier barrier) {
  OS_LNX_Entity* entity = (OS_LNX_Entity*)barrier.v;
  pthread_barrier_wait(&entity->barrier);
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

typedef enum VkDebugUtilsMessageSeverityFlagBitsEXT {
  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT = 0x00000001,
  VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT = 0x00000010,
  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT = 0x00000100,
  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT = 0x00001000,
  VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} VkDebugUtilsMessageSeverityFlagBitsEXT;
typedef u32 VkDebugUtilsMessageTypeFlagsEXT;
typedef struct VkDebugUtilsMessengerCallbackDataEXT {
  u8 padd[40];
  const char* pMessage;
} VkDebugUtilsMessengerCallbackDataEXT;
typedef b32 VkBool32;
VkBool32 vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_types, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
  switch (message_severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
      Trace(String(callback_data->pMessage));
    } break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
      Info(String(callback_data->pMessage));
    } break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
      String skip_warnings[] = {
        "vkCreateGraphicsPipelines(): pCreateInfos[0].pVertexInputState Vertex attribute at location",
        "vkCreateGraphicsPipelines(): pCreateInfos[0] (SPIR-V Interface) VK_SHADER_STAGE_VERTEX_BIT has an Output value declared at Location",
        "(Warning - This VUID has now been reported 10 times, which is the duplicate_message_limit value, this will be the last time reporting it).",
        "vkCreateGraphicsPipelines(): pCreateInfos[0] (SPIR-V Interface) [EntryPoint \"vs_main\", VK_SHADER_STAGE_VERTEX_BIT] has an Output value declared at Location",
      };
      for (var skip : skip_warnings) {
        String warn = callback_data->pMessage;
        if (warn.size >= skip.size) {
          if (str_match(str_prefix(warn, skip.size), skip)) {
            return false;
          }
        }
      }
      Warn(String(callback_data->pMessage));
    } break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
      Error(String(callback_data->pMessage));
    } break;
    default:
  }
  return false;
}

#endif
