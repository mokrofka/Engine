#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include "lib.h"

struct Win32HandleInfo {
  HINSTANCE h_instance;
};

struct WindowPlatformState {
  HWND hwnd;
};

struct OS_State {
  Arena* arena;
  b8 is_mouse;
  f32 pos_x;
  f32 pos_y;
  f32 delta_x;
  f32 delta_y;
  f32 old_x;
  f32 old_y;
  HINSTANCE instance;
  Window window;
  
  WindowClosedCallback window_closed_callback;
  WindowResizedCallback window_resized_callback;
  ProcessKeyCallback process_key;
  ProcessMouseMoveCallback process_mouse_move;

  String binary_name;
  String binary_filepath;
  String binary_directory;
};
f32 delta_time; // maybe put somewhere else

global OS_State st;

// Clock
global f64 clock_frequency;
global UINT min_period;
global LARGE_INTEGER start_time;

void entry_point();
internal LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);
void w32_entry_point_caller(void (*main)()) {
  global_allocator_init();
  tctx_init();
  
  st.instance = GetModuleHandleA(null);

  // Clock
  {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);
  }
  
  WNDCLASSA wc = {
    .lpfnWndProc = win32_process_message,
    .hInstance = st.instance,
    .lpszClassName = "class",
  };
  RegisterClassA(&wc);

  Scratch scratch;
  u8* buff = push_buffer(scratch, 512);
  u32 size = GetModuleFileNameA(0, (char*)buff, 512);

  st.binary_filepath = String(buff, size);
  st.binary_directory = str_chop_last_slash(String(buff, size));
  st.binary_name = str_skip_last_slash(st.binary_filepath);

#ifdef MONOLITHIC_BUILD
  entry_point();
#else
  main();
#endif
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  w32_entry_point_caller(null);
}

KAPI void os_entry_point(void (*main)()) {
  w32_entry_point_caller(main);
}

void os_window_create(WindowConfig config) {
  st.window.name = config.name;

  st.window.width = config.width;
  st.window.height = config.height;
  
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
  window_style |= WS_OVERLAPPEDWINDOW;
  
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
    window_ex_style, "class", (char*)config.name.str,
    window_style, window_x, window_y, window_width, window_height,
    0, 0, st.instance, 0);

  if (handle == 0) {
    MessageBoxA(NULL, "window creating failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
    
    Error("Window creating failed");
  } else {
    st.window.hwnd = handle;
  }
}

void os_pump_messages() {
  MSG message;
  while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&message);
    DispatchMessageA(&message);
  }
}

void os_console_write(String message, u32 color) {
  HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
  u32 levels[6] = {64, 4, 6, 2, 1, 8};
  SetConsoleTextAttribute(console_handle, levels[color]);
  OutputDebugStringA((char*)message.str);
  LPDWORD number_written = 0;
  WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message.str, message.size, number_written, 0);
}

void os_console_write_error(String message, u32 color) {
  HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
  u32 levels[6] = {64, 4, 6, 2, 1, 8};
  SetConsoleTextAttribute(console_handle, levels[color]);
  OutputDebugStringA((char*)message.str);
  LPDWORD number_written = 0;
  WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message.str, message.size, number_written, 0);
}

void os_message_box(String message) {
  MessageBox(null, (char*)message.str, "My App", MB_OK);
}

f32 os_now_seconds() {
  LARGE_INTEGER now_time;
  QueryPerformanceCounter(&now_time);
  return (f32)now_time.QuadPart * clock_frequency;
}

void os_sleep(u64 ms) {
  Sleep(ms);
}

void os_register_process_key(ProcessKeyCallback  callback) {
  st.process_key = callback;
}

void os_register_process_mouse_move(ProcessMouseMoveCallback callback) {
  st.process_mouse_move = callback;
}

void os_register_window_closed_callback(WindowClosedCallback callback) {
  st.window_closed_callback = callback;
}

void os_register_window_resized_callback(WindowResizedCallback callback) {
  st.window_resized_callback = callback;
}

void os_window_destroy() {
  Trace("Destroying window...");
  DestroyWindow((HWND)st.window.hwnd);
}

void* os_get_handle_info() {
  return st.instance;
}

void* os_get_window_handle() {
  return st.window.hwnd;
}

v2i os_get_framebuffer_size() {
  return v2i(st.window.width, st.window.height);
}

void os_mouse_enable() {
  st.is_mouse = true; 
  ClipCursor(null);
  ShowCursor(true);
}

void os_mouse_disable() {
  st.is_mouse = false;
  POINT ul = {0, 0};
  POINT lr = {st.window.width, st.window.height};

  // Convert client coords to screen coords
  ClientToScreen((HWND)st.window.hwnd, &ul);
  ClientToScreen((HWND)st.window.hwnd, &lr);

  RECT clipRect = {ul.x, ul.y, lr.x, lr.y};
  ClipCursor(&clipRect);
  ShowCursor(false);
}

v2 os_get_mouse_delta() {
  return v2(st.delta_x, st.delta_y);
}

//////////////////////////////////////////////////////////////////////////
// Memory
#define PAGE_SIZE 4096

u8* os_reserve(u64 size) {
  u8* result = (u8*)VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
  return result;
}

void os_commit(void* ptr, u64 size) {
  VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
}

void os_decommit(void* ptr, u64 size) {
  VirtualFree(ptr, size, MEM_DECOMMIT);
}

void os_release(void* ptr, u64 size) {
  VirtualFree(ptr, 0, MEM_RELEASE);
}

void* os_reserve_large(u64 size) {
  // we commit on reserve because windows
  void* result = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);
  return result;
}

b32 os_commit_large(void* ptr, u64 size) {
  return 1;
}

//////////////////////////////////////////////////////////////////////////
// File

OS_Handle os_file_open(String path, OS_AccessFlags mode) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  OS_Handle result = {};
  DWORD handle_permission = 0;
  DWORD handle_creation = 0;

  if ((mode & OS_AccessFlag_Read) != 0 && (mode & OS_AccessFlag_Write) != 0) {
    handle_permission |= GENERIC_READ | GENERIC_WRITE;
  } else if ((mode & OS_AccessFlag_Read) != 0 && (mode & OS_AccessFlag_Write) == 0) {
    handle_permission |= GENERIC_READ;
    handle_creation |= OPEN_EXISTING;
  } else if ((mode & OS_AccessFlag_Read) == 0 && (mode & OS_AccessFlag_Write) != 0) {
    handle_permission |= GENERIC_WRITE;
    handle_creation |= CREATE_ALWAYS;
  } else {
    Error("Invalid mode passed while trying to open file: '%s'", path);
    return result;
  }
  
  HANDLE win32_handle = CreateFileA((char*)path_c.str, handle_permission,
                                     FILE_SHARE_READ, 0, handle_creation, 0, null);
  if (win32_handle == INVALID_HANDLE_VALUE) {
    return result;
  }
  result = (PtrInt)win32_handle;

  return result;
}

OVERLAPPED overlapped[2];
DWORD bytesReturned;
u8 buffer[KB(1) * 2];

OS_Handle os_directory_open(String path) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  HANDLE handle = CreateFileA(
      (char*)path_c.str,
      FILE_LIST_DIRECTORY,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
      NULL);

  if (handle == INVALID_HANDLE_VALUE) {
    Error("Failed to open directory handle. Error: %u\n", GetLastError());
    return 0;
  }
  return (OS_Handle)handle;
}

void os_directory_watch(OS_Handle dir_handle, u32 id) {
  BOOL success = ReadDirectoryChangesW(
      (HANDLE)dir_handle,
      &buffer[id * KB(1)],
      KB(1),
      false, // monitor subdirectories too
      FILE_NOTIFY_CHANGE_LAST_WRITE,
      &bytesReturned,
      &overlapped[id],
      NULL);

  if (!success) {
    Error("ReadDirectoryChangesW failed. Error: %u", GetLastError());
    return;
  }
}

String os_directory_name_change(Arena* arena, u32 id) {
  FILE_NOTIFY_INFORMATION* info = (FILE_NOTIFY_INFORMATION*)&buffer[id*KB(1)];
  String filename;
  do {
    filename = push_str_wchar(arena, info->FileName, info->FileNameLength / 2);
    Debug("Shader changed: %s", filename);

    if (info->NextEntryOffset == 0)
      break;
    info = (FILE_NOTIFY_INFORMATION*)((BYTE*)info + info->NextEntryOffset);
  } while (true);
  return filename;
}

b32 os_directory_check_change(OS_Handle dir_handle, u32 id) {
  DWORD byte_transferred;
  b32 ready = GetOverlappedResult((HANDLE)dir_handle, &overlapped[id], &byte_transferred, FALSE);
  return ready;
}

void os_file_close(OS_Handle file) {
  CloseHandle((HANDLE)file);
}

u64 os_file_read(OS_Handle file, u64 size, u8* out_data) {
  HANDLE win32_handle = (HANDLE)file;
  DWORD bytes_read;
  if (file == 0) { return 0; }
  ReadFile(win32_handle, out_data, size, &bytes_read, null);
  if (bytes_read != size) { return 0; }
  return bytes_read;
}

u64 os_file_write(OS_Handle file, u64 size, void* data) {
  DWORD bytes_wrote;
  if (file == 0) { return 0; };
  WriteFile((HANDLE)file, data, size, &bytes_wrote, 0);
  return bytes_wrote;
}

u64 os_file_size(OS_Handle file) {
  u32 hsize;
  u32 lsize = GetFileSize((HANDLE)file, (LPDWORD)&hsize);
  u64 result = Compose64Bit(hsize, lsize);
  return result;
}

FileProperties os_properties_from_file(OS_Handle file) {
  BY_HANDLE_FILE_INFORMATION info;
  GetFileInformationByHandle((HANDLE)file, &info);
  FileProperties props = {
    .size = Compose64Bit(info.nFileSizeHigh, info.nFileSizeLow),
    .modified = Compose64Bit(info.ftLastWriteTime.dwHighDateTime, info.ftLastWriteTime.dwLowDateTime),
    .created = Compose64Bit(info.ftCreationTime.dwHighDateTime, info.ftCreationTime.dwLowDateTime),
    .flags = info.dwFileAttributes,
  };
  return props;
}

FileProperties os_properties_from_file_path(String path) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  
  WIN32_FILE_ATTRIBUTE_DATA info;
  GetFileAttributesExA((char*)path_c.str, GetFileExInfoStandard, &info);
  FileProperties props = {
    .size = Compose64Bit(info.nFileSizeHigh, info.nFileSizeLow),
    .modified = Compose64Bit(info.ftLastWriteTime.dwHighDateTime, info.ftLastWriteTime.dwLowDateTime),
    .created = Compose64Bit(info.ftCreationTime.dwHighDateTime, info.ftCreationTime.dwLowDateTime),
    .flags = info.dwFileAttributes,
  };
  return props;
}

b32 os_copy_file_path(String dst, String src) {
  b32 result = CopyFile((char*)src.str, (char*)dst.str, false);
  return result;
}

b32 os_file_path_exists(String path) {
  DWORD attributes = GetFileAttributesA((char*)path.str);
  return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

String os_exe_filename(Arena* arena) {
  u8* buff = push_buffer(arena, 512);
  DWORD size = GetModuleFileNameA(0, (char*)buff, 512);
  return String(buff, size);
}

b32 os_file_compare_time(u64 new_write_time, u64 last_write_time) {
  if (CompareFileTime((FILETIME*)&new_write_time, (FILETIME*)&last_write_time) != 0) {
    return true; // is changed
  }
  return false;
}

String os_get_current_directory() {
  return st.binary_directory;
}

String os_get_current_binary_name() {
  return st.binary_name;
}

String os_get_current_filepath() {
  return st.binary_filepath;
}

OS_Handle os_lib_open(String path) {
  HMODULE mod = LoadLibraryA((char*)path.str);
  OS_Handle result = { (u64)mod };
  return result;
}

void os_lib_close(OS_Handle lib) {
  HMODULE mod = (HMODULE)lib;
  FreeLibrary(mod);
}

VoidProc* os_lib_get_proc(OS_Handle lib, String name) {
  Scratch scratch;
  String name_c = push_str_copy(scratch, name);
  VoidProc* result; Assign(result, GetProcAddress((HMODULE)lib, (char*)name_c.str));
  return result;
}

PROCESS_INFORMATION pi;
b32 is_process_alive;
void os_process_create(String cmd) {
  if (is_process_alive) {
    DWORD result = WaitForSingleObject(pi.hProcess, 0); // check immediately
    if (result == WAIT_OBJECT_0) {
      // Process finished
      is_process_alive = false;
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
    }
  }
  
  Scratch scratch;
  String cmd_c = push_str_copy(scratch, cmd);
  STARTUPINFO si = {};
  si.cb = sizeof(si);

  BOOL success = CreateProcess(
      NULL,             // No module name, use command line
      (char*)cmd_c.str, // Command line
      NULL,             // Process handle not inheritable
      NULL,             // Thread handle not inheritable
      FALSE,            // No handle inheritance
      0,                // No creation flags
      NULL,             // Use parent's environment block
      NULL,             // Use parent's starting directory
      &si,              // Pointer to STARTUPINFO
      &pi);             // Pointer to PROCESS_INFORMATION

  if (!success) {
    Error("CreateProcess failed (%u).\n", GetLastError());
  }
  is_process_alive = true;
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

///////////////////////////////////////////////////////////////////////////
// WinProc
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
internal LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param) {
  ImGui_ImplWin32_WndProcHandler(hwnd, msg, w_param, l_param);
  
  switch (msg) {
    case WM_ERASEBKGND: {
      // Notfy the OS that erasing will be handled by the application to prevent flicker.
      return 1;
    }
    case WM_PAINT: {
    } break;
    case WM_CLOSE: {
      st.window_closed_callback();
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
      
      Window* w = &st.window;

      if (width != w->width || height != w->height) {
        w->resizing = true;
        w->width = width;
        w->height = height;
        st.window_resized_callback(&st.window);
      }
    } break;
    
    case WM_MOUSEMOVE: // within client area
    case WM_NCMOUSEMOVE: { // within non-client area
      POINT center = {st.window.width / 2, st.window.height / 2};
      ClientToScreen((HWND)st.window.hwnd, &center);
      
      u32 x = GET_X_LPARAM(l_param);
      u32 y = GET_Y_LPARAM(l_param);
      // if (!st.is_mouse) {
      //   SetCursorPos(center.x, center.y);
      // }
      
      st.process_mouse_move(x, y);
    } break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP: {
      // Key pressed/released
      b32 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
      Key key = (Key)(u16)w_param;
      
      // shift alt ctrl
      b32 is_extended = (HIWORD(l_param) & KF_EXTENDED) == KF_EXTENDED;
      // Keypress only determines if _any_ alt/ctrl/shift key is pressed. Determine which one if so.
      if (w_param == VK_MENU) {
        key = is_extended ? Key_RAlt : Key_LAlt;
      } else if (w_param == VK_SHIFT) {
        // Annoyingly, KF_EXTENDED is not set for shift keys.
        u32 left_shift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
        u32 scancode = ((l_param & (0xFF << 16)) >> 16);
        key = scancode == left_shift ? Key_LShift : Key_RShift;
      } else if (w_param == VK_CONTROL) {
        key = is_extended ? Key_RControl : Key_LControl;
      }
      
      // Pass to the input subsytem for processing.
      st.process_key(key, pressed);
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

void os_show_window() {
  ShowWindow((HWND)st.window.hwnd, SW_SHOW);
}

#include "network_cpp.h"
