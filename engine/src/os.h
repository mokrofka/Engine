#pragma once
#include "defines.h"
#include "input_types.h"
#include "math/math_types.h"

struct DynamicLibrary {
  void* handle;
  String src_full_filename;
  String temp_full_filename;
  u64 last_time_write;
};

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

struct OS_File {
  u64 u64;
};

enum FileModes {
  FILE_MODE_READ = 0x1,
  FILE_MODE_WRITE = 0x2,
};

struct Clock {
  f64 start_time;
  f64 elapsed;
};

struct Arena;

using WindowClosedCallback = void (*)();
using ProcessKeyCallback = void (*)(Keys key, b8 pressed);
using WindowResizedCallback = void (*)(Window* window);

void os_pump_messages();

void platform_init(Arena* arena);
void os_window_create(WindowConfig config);
void os_platform_shutdown();
void* vk_os_create_surface();

void* os_allocate(u64 size, b8 at_base);
void* os_allocate(u64 size);
void os_free(void* block, b8 aligned);
void* _platform_memory_zero(void* block, u64 size);
void* _platform_memory_copy(void* dest, const void* source, u64 size);
void* _platform_memory_set(void* dest, i32 value, u64 size);
b8 _platform_memory_compare(void* a, void* b, u64 size);

void os_console_write(const char* message, u8 color);
void os_console_write_error(const char* message, u8 color);

f64 os_now_seconds();
// Sleep on the thread for the provided ms. This blocks the main thread.
// Should only be used for giving time back to the OS for unused update power.
// Therefore it is not exported.
void os_sleep(u64 ms);

KAPI void* os_library_load(String name);
KAPI b8 os_library_unload(DynamicLibrary library);
KAPI void* os_library_load_function(String name, DynamicLibrary library);

void os_register_process_key(ProcessKeyCallback  callback);
void os_register_window_closed_callback(WindowClosedCallback callback);
void os_register_window_resized_callback(WindowResizedCallback callback);

void os_window_destroy();
void* os_get_handle_info();
void* os_get_window_handle();
v2i os_get_framebuffer_size();

// files
KAPI b8 os_file_path_exists(String path);
KAPI u64 os_file_size(OS_File handle);
KAPI OS_File os_file_open(String path, FileModes mode);
KAPI void os_file_close(OS_File handle);
KAPI u64 os_file_read(OS_File handle, u64 size, void* dest);
KAPI u64 os_file_write(OS_File handle, u64 size, const void* source);
KAPI void os_file_copy(String file, String new_file);
KAPI u64 os_file_last_write_time(String filename);
KAPI b8 os_file_compare_time(u64 new_dll_write_time, u64 dll_last_write_time);
KAPI u64 os_EXE_filename(void* buffer);


// Updates the provided clock. Should be called just before checking elapsed time.
// Has no effect on non-started clocks.
KAPI void clock_update(Clock* clock);

// Starts the provided clock. Resets elapsed.
KAPI void clock_start(Clock* clock);

// Stops the provided clock. Does not reset elapsed time.
KAPI void clock_stop(Clock* clock);
