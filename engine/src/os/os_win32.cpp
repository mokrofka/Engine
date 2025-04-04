#include "os.h"

#include "memory.h"
#include "logger.h"

#include <windows.h>

struct Win32HandleInfo {
  HINSTANCE h_instance;
};

struct WindowPlatformState {
  HWND hwnd;
};

struct PlatformState {
  Win32HandleInfo handle;
  Window* window;
  // PlatformWindowClosedCallback window_closed_callback;
  // PlatformWindowResizedCallback window_resized_callback;
  // PlatformProcessKey process_key;
  WindowClosedCallback window_closed_callback;
  WindowResizedCallback window_resized_callback;
  ProcessKeyCallback  process_key;
};

global PlatformState* state;

// Clock
global f64 clock_frequency;
global UINT min_period;
global LARGE_INTEGER start_time;

internal LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);
internal void clock_setup() {
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);
  clock_frequency = 1.0 / (f64)frequency.QuadPart;
  QueryPerformanceCounter(&start_time);
}

b8 platform_init(Arena* arena) {
  u64 memory_requirement = sizeof(PlatformState)+sizeof(Window)+sizeof(WindowPlatformState);
  state = push_buffer(arena, PlatformState, memory_requirement);
  
  // TODO
  SetCurrentDirectoryA("D:\\VS_Code\\Engine\\bin");
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

b8 os_window_create(Window* window, WindowConfig config) {
  window = (Window*)((u8*)state + sizeof(PlatformState));
  window->platform_state = (WindowPlatformState*)((u8*)window + sizeof(Window));
  state->window = window;
  state->window->width = config.width;
  state->window->height = config.height;
  
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

void os_platform_shutdown() {
  if (state->window->platform_state->hwnd) {
    DestroyWindow(state->window->platform_state->hwnd);
    state->window->platform_state->hwnd = 0;
  }
}

b8 os_pump_messages() {
  MSG message;
  while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&message);
    DispatchMessageA(&message);
  }
  
  return true;
}

#define BaseAddress (void*)TB(2)
void* os_allocate(u64 size, b8 at_base) {
  return VirtualAlloc(BaseAddress, size, MEM_RESERVE | MEM_COMMIT,
                      PAGE_READWRITE);
}

void* allocate(u64 size) {
  return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT,
                      PAGE_READWRITE);
}

void os_free(void* block, b8 aligned) {
  VirtualFree(block, 0, 0);
}

void* _platform_memory_zero(void* block, u64 size) {
  return memset(block, 0, size);
}

void* _platform_memory_copy(void* dest, const void* source, u64 size) {
  return memcpy(dest, source, size);
}

void* _platform_memory_set(void* dest, i32 value, u64 size) {
  return memset(dest, value, size);
}

b8 _platform_memory_compare(void* a, void* b, u64 size) {
  return memcmp(a, b, size) ? 0 : 1;
}

void os_console_write(const char* message, u8 color) {
  HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
  const u8 levels[6] = {64, 4, 6, 2, 1, 8};
  SetConsoleTextAttribute(console_handle, levels[color]);
  OutputDebugStringA(message);
  u64 length = strlen(message);
  LPDWORD number_written = 0;
  WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, 0);
}

void os_console_write_error(const char* message, u8 color) {
  HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
  const u8 levels[6] = {64, 4, 6, 2, 1, 8};
  SetConsoleTextAttribute(console_handle, levels[color]);
  OutputDebugStringA(message);
  u64 length = strlen(message);
  LPDWORD number_written = 0;
  WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, number_written, 0);
}

f64 os_now_seconds() {
  LARGE_INTEGER now_time;
  QueryPerformanceCounter(&now_time);
  return (f64)now_time.QuadPart * clock_frequency;
}

void os_sleep(u64 ms) {
  Sleep(ms);
}

void* os_library_load(String name) {
  HMODULE library = LoadLibraryA((char*)name.str);
  if (!library) {
    AssertMsg(false, "Didn't load library");
  }

  return (void*)library;
}

b8 os_library_unload(DynamicLibrary library) {
  if (!library.handle) {
    return false;
  }
  BOOL result = FreeLibrary((HMODULE)library.handle);
  if (result == 0) {
    return false;
  }
  return true;
}

void* os_library_load_function(String name, DynamicLibrary library) {
  if (!name.str || !library.handle) {
    return 0;
  }

  FARPROC f_addr = GetProcAddress((HMODULE)library.handle, (char*)name.str);
  if (!f_addr) {
    AssertMsg(false, "Didn't load function");
  }

  return (void*)f_addr;
}

void os_register_process_key(ProcessKeyCallback  callback) {
  state->process_key = callback;
}

void os_register_window_closed_callback(WindowClosedCallback callback) {
  state->window_closed_callback = callback;
}

void os_register_window_resized_callback(WindowResizedCallback callback) {
  state->window_resized_callback = callback;
}

void os_window_destroy(Window* window) {
  Trace("Destroying window...");
  DestroyWindow(state->window->platform_state->hwnd);
  state->window->platform_state->hwnd = 0;
  state->window = 0;
}

void* os_get_handle_info() {
  return state->handle.h_instance;
}

void* os_get_window_handle() {
  return state->window->platform_state->hwnd;
}

void os_get_framebuffer_size(u32* width, u32* height) {
  *width = state->window->width;
  *height = state->window->height;
}



//////////////////////////////////////////////////////////////////////////
// file
  
b8 os_file_path_exists(String path) {
  DWORD attributes = GetFileAttributesA((char*)path.str);
  return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

u64 os_file_size(OS_File handle) {
  LARGE_INTEGER size;
  GetFileSizeEx((HANDLE)handle.u64, &size);
  return size.QuadPart;
}

OS_File os_file_open(String path, FileModes mode) {
  OS_File result = {};
  DWORD handle_permission = 0;
  DWORD handle_creation = 0;

  if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0) {
    handle_permission |= GENERIC_READ | GENERIC_WRITE;
  } else if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0) {
    handle_permission |= GENERIC_READ;
    handle_creation |= OPEN_EXISTING;

  } else if ((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0) {
    handle_permission |= GENERIC_WRITE;
    handle_creation |= CREATE_ALWAYS;
  } else {
    Error("Invalid mode passed while trying to open file: '%s'", path);
    return result;
  }
  
  HANDLE win32_handle = CreateFileA((char*)path.str, handle_permission,
                                     FILE_SHARE_READ, 0, handle_creation, 0, null);
  if (win32_handle == INVALID_HANDLE_VALUE) {
    Error("Error opening file: '%s", path);
    return result;
  }
  
  result.u64 = (u64)win32_handle;

  return result;
}

void os_file_close(OS_File handle) {
  if (handle.u64) {
    CloseHandle((HANDLE)handle.u64);
  }
}

u64 os_file_read(OS_File handle, u64 size, void* dest) {
  HANDLE win32_handle = (HANDLE)handle.u64;
  DWORD bytes_read;

  if (handle.u64 && dest) {
    ReadFile(win32_handle, dest, size, &bytes_read, null);
    if (bytes_read != size) {
      return 0;
    }
    return bytes_read;
  }
  return 0;
}

u64 os_file_write(OS_File handle, u64 size, const void* source) {
  DWORD bytes_wrote;
  if (handle.u64) {
    WriteFile((HANDLE)handle.u64, source, size, &bytes_wrote, 0);
    if (size != bytes_wrote) {
      return 0;
    }
    return bytes_wrote;
  }
  return 0;
}

void os_file_copy(String file, String new_file) {
  CopyFile((char*)file.str, (char*)new_file.str, false);
}

u64 os_file_last_write_time(String filename) {
  FILETIME last_write_time = {};

  WIN32_FILE_ATTRIBUTE_DATA Data;
  if (GetFileAttributesEx((char*)filename.str, GetFileExInfoStandard, &Data)) {
    last_write_time = Data.ftLastWriteTime;
  }

  u64 result;
  MemCopyStruct(&result, &last_write_time);

  return result;
}

b8 os_file_compare_time(u64 new_dll_write_time, u64 dll_last_write_time) {
  if (CompareFileTime((FILETIME*)&new_dll_write_time, (FILETIME*)&dll_last_write_time) != 0) {
    return true; // is changed
  }
  return false;
}

u64 os_EXE_filename(void* buffer) {
  DWORD size = GetModuleFileNameA(0, (char*)buffer, 100);
  return size;
}


//////////////////////////////////////////////////////////////////////////
// Clock
void clock_update(Clock *clock) {
  if (clock->start_time != 0) {
    clock->elapsed = os_now_seconds() - clock->start_time;
  }
}

void clock_start(Clock *clock) {
  clock->start_time = os_now_seconds();
  clock->elapsed = 0;
}

void clock_stop(Clock *clock) {
  clock->start_time = 0;
}

internal LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param) {
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
      state->window_closed_callback();
      return true;
    }
    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }
    case WM_SIZE: {
      // Get the updated size.
      RECT r;
      GetClientRect(hwnd, &r);
      u32 width = r.right - r.left;
      u32 height = r.bottom - r.top;
      
      Window* w = state->window;
      
      if (width != w->width || height != w->height) {
        w->resizing = true;
        w->width = width;
        w->height = height;
        state->window_resized_callback(state->window);
      }
    } break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP: {
      // Key pressed/released
      b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
      Keys key = (Keys)(u16)w_param;
      
      // shift alt ctrl
      b8 is_extended = (HIWORD(l_param) & KF_EXTENDED) == KF_EXTENDED;
      // Keypress only determines if _any_ alt/ctrl/shift key is pressed. Determine which one if so.
      if (w_param == VK_MENU) {
        key = is_extended ? KEY_RALT : KEY_LALT;
      } else if (w_param == VK_SHIFT) {
        // Annoyingly, KF_EXTENDED is not set for shift keys.
        u32 left_shift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
        u32 scancode = ((l_param & (0xFF << 16)) >> 16);
        key = scancode == left_shift ? KEY_LSHIFT : KEY_RSHIFT;
      } else if (w_param == VK_CONTROL) {
        key = is_extended ? KEY_RCONTROL : KEY_LCONTROL;
      }
      
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
