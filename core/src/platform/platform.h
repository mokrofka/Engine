#pragma once

#include "core/strings.h"
#include "input_types.h"
#include "defines.h"

struct DynamicLibraryFunction {
  const char* name;
  void* pfn;
};

struct DynamicLibrary {
  void* handle;
  String filename;
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

typedef void (*PlatformProcessKey)(Keys key, b8 pressed);

KAPI b8 platform_system_startup(u64* memory_requirement, struct PlatformState* state_out);

KAPI b8 window_create(Window* window, WindowConfig config);
  
void platform_shutdown(struct PlatformState* plat_state);

// KAPI b8 platform_pump_messages(struct PlatformState* plat_state);
KAPI b8 platform_pump_messages(void* plat_state);

void* platform_allocate(u64 size);
struct Arena* platform_allocate_arena(u64 size);
void platform_free(void* block, b8 aligned);
void* platform_zero_memory(void* block, u64 size);
void* platform_copy_memory(void* dest, const void* source, u64 size);
void* platform_set_memory(void* dest, i32 value, u64 size);

void platform_console_write(const char* message, u8 colour);
void platform_console_write_error(const char* message, u8 colour);


f64 platform_get_absolute_time();

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
KAPI void platform_get_EXE_filename(u8* buffer);

KAPI void platform_register_process_key(PlatformProcessKey callback);
