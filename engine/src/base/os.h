#pragma once
#include "defines.h"
#include "input_types.h"
#include "math_types.h"

struct WindowConfig {
  i32 position_x;
  i32 position_y;
  u32 width;
  u32 height;
  String title;
  String name;
};

struct Window {
  String name;
  String title;

  u16 width;
  u16 height;

  f32 device_pixel_ratio;

  b8 resizing;
  u16 frames_since_resize;

  void* hwnd;
};

typedef u64 DenseTime;
typedef u32 FilePropertyFlags;
struct FileProperties {
  u64 size;
  DenseTime modified;
  DenseTime created;
  FilePropertyFlags flags;
};

struct OS_FileInfo
{
  String name;
  FileProperties props;
};

struct OS_Handle {
  u64 u64;
};

enum OS_AccessFlags {
  OS_AccessFlag_Read = Bit(0),
  OS_AccessFlag_Write = Bit(1),
};

struct Clock {
  f64 start_time;
  f64 elapsed;
};

using WindowClosedCallback = void (*)();
using ProcessKeyCallback = void (*)(Keys key, b32 pressed);
using WindowResizedCallback = void (*)(Window* window);

void os_pump_messages();

void platform_init(Arena* arena);
void os_window_create(Arena* arena, WindowConfig config);
void os_platform_shutdown();
void* vk_os_create_surface();

Arena* os_main_arena_allocate(u64 size);
void os_free(void* block, b32 aligned);

void os_console_write(String message, u32 color);
void os_console_write_error(String message, u32 color);

f32 os_now_seconds();
void os_sleep(u64 ms);

void os_register_process_key(ProcessKeyCallback  callback);
void os_register_window_closed_callback(WindowClosedCallback callback);
void os_register_window_resized_callback(WindowResizedCallback callback);

void os_window_destroy();
void* os_get_handle_info();
void* os_get_window_handle();
v2i os_get_framebuffer_size();

//////////////////////////////////////////////////////////////////////////
// Memory

void* os_reserve(u64 size);
void os_commit(void* ptr, u64 size);
void os_decommit(void* ptr, u64 size);
void os_release(void* ptr, u64 size);

void* os_reserve_large(u64 size);
b32 os_commit_large(void* ptr, u64 size);

// files
OS_Handle      os_file_open(String path, OS_AccessFlags flags);
void           os_file_close(OS_Handle file);
u64            os_file_read(OS_Handle file, u64 size, void *out_data);
u64            os_file_write(OS_Handle file, u64 size, void *data);
u64            os_file_size(OS_Handle file);
FileProperties os_properties_from_file(OS_Handle file);
FileProperties os_properties_from_file_path(String path);
b32            os_copy_file_path(String dst, String src);
b32            os_file_path_exists(String path);
String         os_exe_filename(Arena* arena);
b32            os_file_compare_time(u64 new_write_time, u64 last_write_time);

using PROC = i64(*)();
OS_Handle os_lib_open(String path);
void      os_lib_close(OS_Handle lib);
PROC      os_lib_get_proc(OS_Handle lib, String name);

void clock_update(Clock* clock);
void clock_start(Clock* clock);
void clock_stop(Clock* clock);

void os_show_window();
