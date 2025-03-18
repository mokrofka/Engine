#include "core/memory.h"
#include "platform/platform.h"

#include "core/logger.h"
#include "core/asserts.h"

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>

struct Win32HandleInfo {
  HINSTANCE h_instance;
};

struct WindowPlatformState {
  HWND hwnd;
};

struct PlatformState {
  Win32HandleInfo handle;
  Window* window;

  PlatformProcessKey process_key;
};

internal PlatformState* state;

// Clock
internal f64 clock_frequency;
internal UINT min_period;
internal LARGE_INTEGER start_time;

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);

void clock_setup() {
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);
  clock_frequency = 1.0 / (f64)frequency.QuadPart;
  QueryPerformanceCounter(&start_time);

  // TIMECAPS tc;
  // timeGetDevCaps(&tc, sizeof(tc));
  // min_period = tc.wPeriodMin;
}

b8 platform_system_startup(u64* memory_requirement, struct PlatformState* out_state) {
  *memory_requirement = sizeof(PlatformState)+sizeof(Window)+sizeof(WindowPlatformState);
  if (out_state == 0) {
    return true;
  }
  state = out_state;
  state->handle.h_instance = GetModuleHandleA(0);

  clock_setup();
  
  HICON icon = LoadIcon(state->handle.h_instance, IDI_APPLICATION);
  WNDCLASSA wc = {};
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = win32_process_message;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = state->handle.h_instance;
  wc.hIcon = icon;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszClassName = "kohi_window_class";
  
  if (!RegisterClassA(&wc)) {
    MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
    return false;
  }
  
  return true;
}

b8 window_create(Window* window, WindowConfig config) {
  // window = (Window*)malloc(sizeof(Window));
  window = (Window*)((u8*)state + sizeof(PlatformState));
  window->platform_state = (WindowPlatformState*)((u8*)window + sizeof(Window));
  // window->platform_state = (WindowPlatformState*)malloc(sizeof(PlatformState));
  // PlatformState* state = (PlatformState*)window->platform_state;
  // state->window = window;
  
  
  u32 client_x = config.position_x;
  u32 client_y = config.position_y;
  u32 client_width = config.width;
  u32 client_height = config.height;
  
  u32 window_x = client_x;
  u32 window_y = client_y;
  u32 window_width = client_width;
  u32 window_height = client_height;
  
  u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
  u32 window_ex_style = WS_EX_APPWINDOW;
  
  window_style |= WS_MAXIMIZEBOX;
  window_style |= WS_MINIMIZEBOX;
  window_style |= WS_THICKFRAME;
  
  // Obtain the size of the border.
  RECT border_rect = {0, 0, 0, 0};
  AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);
  
  // In this cae, the border rectangle is negative.
  window_x += border_rect.left;
  window_y += border_rect.top;
  
  // Grow by the size of the OS border.
  window_width += border_rect.right - border_rect.left;
  window_height += border_rect.bottom - border_rect.top;

  HWND handle = CreateWindowExA(
      window_ex_style, "kohi_window_class", (char*)config.name.str,
      window_style, window_x, window_y, window_width, window_height,
      0, 0, state->handle.h_instance, 0);
  
  if (handle == 0) {
    MessageBoxA(NULL, "window creating failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
    
    Fatal("Window creating failed!");
    return false;
  } else {
    window->platform_state->hwnd = handle;
  }
  
  b32 should_activate = 1; // TODO: if the window should accept input, this should be false.
  i32 show_window_command_flags = should_activate ? SW_SHOW :SW_SHOWNOACTIVATE;
  // If initially minimmized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE;
  // If initially minimmized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE;
  ShowWindow(window->platform_state->hwnd, show_window_command_flags);
  
  return true;
}

void platform_shutdown(PlatformState* plat_state) {
  // Simply cold-cast to the known type.
  PlatformState* state = plat_state;
  
  if (state->window->platform_state->hwnd) {
    DestroyWindow(state->window->platform_state->hwnd);
    state->window->platform_state->hwnd = 0;
  }
}

// b8 platform_pump_messages(PlatformState* plat_state) {
b8 platform_pump_messages(void* plat_state) {
  MSG message;
  while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&message);
    DispatchMessageA(&message);
  }
  
  return true;
}

#define BaseAddress (void*)TB(2)
void* platform_allocate(u64 size) {
  return VirtualAlloc(BaseAddress, size, MEM_RESERVE | MEM_COMMIT,
                      PAGE_READWRITE);
}

Arena* platform_allocate_arena(u64 size) {
  void* buffer = VirtualAlloc(BaseAddress, size, MEM_RESERVE | MEM_COMMIT,
                              PAGE_READWRITE);
  Arena* arena = (Arena*)buffer;
  // arena->base_pos = ARENA_HEADER;
  arena->pos = 0;
  arena->res = size;
  return arena;
}

void platform_free(void* block, b8 aligned) {
  free(block);
}

void* platform_zero_memory(void* block, u64 size) {
  return memset(block, 0, size);
}

void* platform_copy_memory(void* dest, const void* source, u64 size) {
  return memcpy(dest, source, size);
}

void* platform_set_memory(void* dest, i32 value, u64 size) {
  return memset(dest, value, size);
}

void platform_console_write(const char* message, u8 colour) {
  HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
  static u8 levels[6] = {64, 4, 6, 2, 1, 8};
  SetConsoleTextAttribute(console_handle, levels[colour]);
  OutputDebugStringA(message);
  u64 length = strlen(message);
  LPDWORD number_written = 0;
  WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, 0);
}

void platform_console_write_error(const char* message, u8 colour) {
  HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
  static u8 levels[6] = {64, 4, 6, 2, 1, 8};
  SetConsoleTextAttribute(console_handle, levels[colour]);
  OutputDebugStringA(message);
  u64 length = strlen(message);
  LPDWORD number_written = 0;
  WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, number_written, 0);
}

f64 platform_get_absolute_time() {
  LARGE_INTEGER now_time;
  QueryPerformanceCounter(&now_time);
  return (f64)now_time.QuadPart * clock_frequency;
}

void platform_sleep(u64 ms) {
  Sleep(ms);
}

// b8 platform_dynamic_library_load(const char* name, DynamicLibrary* out_library) {
b8 platform_dynamic_library_load(String name, DynamicLibrary* out_library) {
  if (!out_library) {
    return false;
  }
  if (!name.str) {
  }

  out_library->filename = name;

  u32 max_retries = 10;
  HMODULE library = LoadLibraryA((char*)name.str);
  if (!library) {
    Assert(false);
  }

  // for (u32 i = 0; i < max_retries; ++i) {
  //   library = LoadLibraryA((char*)name.str);;
  //   if (library) goto end;

  //   // Wait and retry
  //   Sleep(10);
  // }
  // if (!library) {
  //   Assert(false);
  // }

  // end:
  out_library->handle = library;

  return true;
}

b8 platform_dynamic_library_unload(DynamicLibrary* library) {
  if (!library) {
    return false;
  }
  if (!library->handle) {
    return false;
  }
  BOOL result = FreeLibrary((HMODULE)library->handle);
  if (result == 0) {
    return false;
  }
  library->handle = 0;
  return true;
}

void* platform_dynamic_library_load_function(const char* name, DynamicLibrary* library) {
  if (!name || !library) {
    return 0;
  }

  if (!library->handle) {
    return 0;
  }

  FARPROC f_addr = GetProcAddress((HMODULE)library->handle, name);
  if (!f_addr) {
    Assert(f_addr);
  }

  return (void*)f_addr;
}

u64 platform_get_last_write_time(String filename) {
  FILETIME last_write_time = {};

  WIN32_FILE_ATTRIBUTE_DATA Data;
  if (GetFileAttributesEx((char*)filename.str, GetFileExInfoStandard, &Data)) {
    last_write_time = Data.ftLastWriteTime;
  }

  u64 result;
  MemCopy(&result, &last_write_time, sizeof(FILETIME));

  return result;
}

b8 platform_compare_file_time(u64 new_dll_write_time, u64 dll_last_write_time) {
  if (CompareFileTime((FILETIME*)&new_dll_write_time, (FILETIME*)&dll_last_write_time) != 0) {
    return true;
  }
  return false;
}

void platform_copy_file(String file, String new_file) {
  CopyFile((char*)file.str, (char*)new_file.str, false);
}

void platform_get_EXE_filename(u8* buffer) {
  DWORD SizeOfFilename = GetModuleFileNameA(0, (char*)buffer, 100);
}

void platform_register_process_key(PlatformProcessKey callback) {
  state->process_key = callback;
}

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param) {
  switch (msg) {
    case WM_ERASEBKGND: {
      // Notfy the OS that erasing will be handled by the application to prevent flicker.
      return 1;
    }
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      RECT rect;
      GetClientRect(hwnd, &rect);
      FillRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH)); // Fill window with black
      EndPaint(hwnd, &ps);
      return 0;
    } break;
    case WM_CLOSE: {
      return true;
    }
    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }
    case WM_SIZE: {
      // Get the updated size.
      // RECT r;
      // GetClientRect(hwnd, &r);
      // u32 width = r.right - r.left;
      // u32 height = r.bottom - r.top;

      // TODO: Fire an event for window resize.
    } break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP: {
      // Key pressed/released
      b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
      Keys key = (Keys)(u16)w_param;
      
      // Pass to the input subsytem for processing.
      state->process_key(key, pressed);
    } break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP: {
    } break;
  }
  
  return DefWindowProcA(hwnd, msg, w_param, l_param);
}
