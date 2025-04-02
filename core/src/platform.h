#pragma once

#include "input_types.h"
#include "defines.h"

struct DynamicLibraryFunction {
  const char* name;
  void* pfn;
};

struct DynamicLibrary {
  void* handle;
  String src_full_filename;
  String temp_full_filename;
  DynamicLibraryFunction* functions;
  u64 dll_last_time_write;
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

  struct WindowPlatformState* platform_state;
  struct WindowRendererState* renderer_state;
};

typedef void (*PlatformWindowClosedCallback)();
typedef void (*PlatformProcessKey)(Keys key, b8 pressed);
typedef void (*PlatformWindowResizedCallback)(struct Window* window);
  
KAPI b8 platform_system_startup(u64* memory_requirement, struct PlatformState* state_out);

KAPI b8 window_create(Window* window, WindowConfig config);
  
void platform_shutdown(struct PlatformState* plat_state);

KAPI b8 platform_pump_messages();

void* platform_allocate(u64 size, b8 at_base);
void* platform_allocate(u64 size);
void platform_free(void* block, b8 aligned);
void* platform_zero_memory_(void* block, u64 size);
void* platform_copy_memory_(void* dest, const void* source, u64 size);
void* platform_set_memory_(void* dest, i32 value, u64 size);
b8 platform_compare_memory_(void* a, void* b, u64 size);

void platform_console_write(const char* message, u8 colour);
void platform_console_write_error(const char* message, u8 colour);

KAPI f64 platform_get_absolute_time();
// Sleep on the thread for the provided ms. This blocks the main thread.
// Should only be used for giving time back to the OS for unused update power.
// Therefore it is not exported.
KAPI void platform_sleep(u64 ms);

KAPI b8 platform_dynamic_library_load(String name, DynamicLibrary* out_library);
KAPI b8 platform_dynamic_library_unload(DynamicLibrary* library);
KAPI void* platform_dynamic_library_load_function(const char* name, DynamicLibrary* library);

KAPI u64 platform_get_last_write_time(String filename);
KAPI b8 platform_compare_file_time(u64 new_dll_write_time, u64 dll_last_write_time);
KAPI void platform_copy_file(String file, String new_file);
KAPI u32 platform_get_EXE_filename(u8* buffer);

KAPI void platform_register_process_key(PlatformProcessKey callback);
KAPI void platform_register_window_closed_callback(PlatformWindowClosedCallback callback);
KAPI void platform_register_window_resized_callback(PlatformWindowResizedCallback callback);

KAPI void platform_window_destroy(Window* window);
KAPI struct Win32HandleInfo platform_get_handle_info();
KAPI struct WindowPlatformState* platform_get_window_handle();
KAPI void platform_get_framebuffer_size(u32* width, u32* height);

struct FileHandle {
  void* handle;
  b8 is_valid;
};

enum FileModes {
  FILE_MODE_READ = 0x1,
  FILE_MODE_WRITE = 0x2,
};

KAPI b8 filesystem_file_exists(String path);
KAPI u64 filesystem_file_size(FileHandle handle);
KAPI b8 filesystem_open(const char* path, FileModes mode, FileHandle* handle);
KAPI void filesystem_close(FileHandle* handle);
KAPI b8 filesystem_read(FileHandle handle, u64 size, void* dest);
KAPI b8 filesystem_read_file(FileHandle* handle, void* dest);
KAPI b8 filesystem_read_file(struct Arena* arena, FileHandle* handle, void** dest);
KAPI b8 filesystem_write(FileHandle* handle, u64 size, const void* source);

struct Clock {
  f64 start_time;
  f64 elapsed;
};

// Updates the provided clock. Should be called just before checking elapsed time.
// Has no effect on non-started clocks.
KAPI void clock_update(Clock* clock);

// Starts the provided clock. Resets elapsed.
KAPI void clock_start(Clock* clock);

// Stops the provided clock. Does not reset elapsed time.
KAPI void clock_stop(Clock* clock);

