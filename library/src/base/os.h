#pragma once
#include "defines.h"
#include "str.h"
#include "mem.h"
#include "maths.h"

#include "base/os/input.h"
#include "base/os/event.h"

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

  u32 width;
  u32 height;

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

typedef PtrInt OS_Handle;

struct OS_ProcessInfo {
  String binary_path;
};

enum OS_AccessFlags {
  OS_AccessFlag_Read = Bit(0),
  OS_AccessFlag_Write = Bit(1)
};

struct Clock {
  f64 start_time;
  f64 elapsed;
};

struct Buffer {
  u8* data;
  u64 size;
};

KAPI extern f32 delta_time;

using WindowClosedCallback = void (*)();
using WindowResizedCallback = void (*)(Window* window);
using ProcessKeyCallback = void (*)(Key key, b32 pressed);
using ProcessMouseMoveCallback = void (*)(u32 x, u32 y);

KAPI void os_init();
KAPI void os_pump_messages();

KAPI void os_window_create(WindowConfig config);
KAPI void os_entry_point(void (*main)());

KAPI void os_console_write(String message, u32 color);
KAPI void os_console_write_error(String message, u32 color);

KAPI f32 os_now_seconds();
KAPI void os_sleep(u64 ms);

KAPI void os_register_process_key(ProcessKeyCallback  callback);
KAPI void os_register_process_mouse_move(ProcessMouseMoveCallback callback);
KAPI void os_register_window_closed_callback(WindowClosedCallback callback);
KAPI void os_register_window_resized_callback(WindowResizedCallback callback);

KAPI void os_window_destroy();
KAPI void* os_get_handle_info();
KAPI void* os_get_window_handle();
KAPI v2u os_get_framebuffer_size();
KAPI void os_mouse_enable();
KAPI void os_mouse_disable();
KAPI v2 os_get_mouse_delta();

//////////////////////////////////////////////////////////////////////////
// Memory
KAPI u8* os_reserve(u64 size);
KAPI void os_commit(void* ptr, u64 size);
KAPI void os_decommit(void* ptr, u64 size);
KAPI void os_release(void* ptr, u64 size);

KAPI void* os_reserve_large(u64 size);
KAPI b32 os_commit_large(void* ptr, u64 size);

//////////////////////////////////////////////////////////////////////////
// Files
KAPI OS_Handle      os_file_open(String path, OS_AccessFlags flags);
KAPI OS_Handle      os_directory_open(String path);
KAPI void           os_directory_watch(OS_Handle dir_handle, u32 id);
KAPI String         os_directory_watch_pop_name(Arena* arena, OS_Handle dir, u32 id);
KAPI b32            os_directory_check_change(OS_Handle dir_handle, u32 id);
KAPI void           os_file_close(OS_Handle file);
KAPI u64            os_file_read(OS_Handle file, u64 size, u8* out_data);
KAPI u64            os_file_write(OS_Handle file, u64 size, u8* data);
KAPI u64            os_file_size(OS_Handle file);
KAPI FileProperties os_properties_from_file(OS_Handle file);
KAPI FileProperties os_properties_from_file_path(String path);
KAPI b32            os_copy_file_path(String dst, String src);
KAPI b32            os_file_path_exists(String path);
KAPI String         os_exe_filename(Arena* arena);
KAPI b32            os_file_compare_time(u64 new_write_time, u64 last_write_time);

KAPI String os_get_current_directory();
KAPI String os_get_current_binary_name();
KAPI String os_get_current_filepath();

KAPI OS_Handle os_lib_open(String path);
KAPI void      os_lib_close(OS_Handle lib);
KAPI VoidProc* os_lib_get_proc(OS_Handle lib, String name);

KAPI void os_process_create(String cmd);
KAPI void os_is_process_alive();

KAPI void clock_update(Clock* clock);
KAPI void clock_start(Clock* clock);
KAPI void clock_stop(Clock* clock);

KAPI void os_show_window();
