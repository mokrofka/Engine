#include "lib.h"

#if OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

struct OS_State {
  Arena* arena;
  // HINSTANCE instance;
  // HANDLE console_hanlde;
  Window window;

  f64 clock_frequency;
  LARGE_INTEGER start_time;

  String binary_name;
  String binary_filepath;
  String binary_directory;

  u32 screen_width;
  u32 screen_height;
  
  WindowClosedCallback window_closed_callback;
  WindowResizedCallback window_resized_callback;
  ProcessKeyCallback process_key;
  ProcessMouseMoveCallback process_mouse_move;

  i32 mouse_dx;
  i32 mouse_dy;
};

f32 delta_time;

global OS_State st;

void entry_point();
intern LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);

KAPI void os_init() {
  global_allocator_init();
  tctx_init();
  Scratch scratch;

  HINSTANCE hinstance = GetModuleHandleA(null);
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);

  WNDCLASSA wc = {
    .lpfnWndProc = win32_process_message,
    .hInstance = hinstance,
    .lpszClassName = "class",
  };
  RegisterClassA(&wc);

  Arena* arena = mem_arena_alloc(KB(1));
  u8* buff = push_buffer(scratch, 512);
  u32 size = GetModuleFileNameA(0, (char*)buff, 512);
  String name = push_str_copy(arena, String(buff, size));

  st = {
    .arena = arena,
    .instance = hinstance,
    .console_hanlde = GetStdHandle(STD_OUTPUT_HANDLE),
    .clock_frequency = 1.0 / frequency.QuadPart,
    .binary_name = str_skip_last_slash(name),
    .binary_filepath = name,
    .binary_directory = str_chop_last_slash(name),
    .screen_width = cast(u32)GetSystemMetrics(SM_CXSCREEN),
    .screen_height = cast(u32)GetSystemMetrics(SM_CYSCREEN),
  };
  QueryPerformanceCounter(&st.start_time);
}

void os_toggle_fullscreen() {
  local WINDOWPLACEMENT window_position;
  local i32 fullscreen_switch;
  fullscreen_switch = (fullscreen_switch+1) % 2;
  HWND hwnd = (HWND)st.window.hwnd;
  if (fullscreen_switch) {
    GetWindowPlacement(hwnd, &window_position);
    SetWindowPos(hwnd, null, 0, 0, st.screen_width,st.screen_height, NoFlags);
  } else {
    RECT rect = window_position.rcNormalPosition;
    SetWindowPos(hwnd, null, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, NoFlags);
  }
}

void base_main_thread_entry(void (*entry)());
#ifdef MONOLITHIC_BUILD
int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprev_instance, LPSTR lp_cmd_line, i32 nshow_cmd) { base_main_thread_entry(entry_point); }
#endif
void os_entry_point(void (*entry)()) { base_main_thread_entry(entry); }

void os_window_create(WindowConfig config) {
  Scratch scratch;

  u32 window_x = st.screen_width / 2;
  u32 window_y = 0;
  u32 window_width = st.screen_width / 2;
  u32 window_height = st.screen_height / 2;

  String window_name_c = push_str_copy(scratch, os_get_current_binary_name());
  HWND hwnd = CreateWindowA(
    "class",
    cast(char*)window_name_c.str,
    WS_POPUP,
    window_x, window_y, 
    window_width, window_height,
    null, null, st.instance, null);

  Assert(hwnd);

  st.window = {
    .name = config.name,
    .width = config.width,
    .height = config.height,
    .hwnd = cast(OS_Handle)hwnd
  };
}

void os_pump_messages() {
  MSG message;
  while (PeekMessageA(&message, null, 0, 0, PM_REMOVE)) {
    TranslateMessage(&message);
    DispatchMessageA(&message);
  }
}

void os_console_write(String message, u32 color) {
  HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

  u32 levels[] = {8, 1, 2, 6, 4};
  // Trace, Debug, Info, Warn, Error
  SetConsoleTextAttribute(console_handle, levels[color]);
  OutputDebugStringA((char*)message.str);
  LPDWORD number_written = 0;
  WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message.str, message.size, number_written, 0);
}

void os_message_box(String message) { 
  Scratch scratch;
  String message_c = push_str_copy(scratch, message);
  MessageBox(null, (char*)message_c.str, "My App", MB_OK); 
}

f64 os_now_seconds() {
  LARGE_INTEGER now_time;
  QueryPerformanceCounter(&now_time);
  return (f64)now_time.QuadPart * st.clock_frequency;
}

void os_sleep(u64 ms) { Sleep(ms); }

void os_hide_cursor() { }

void os_cursor_update() {
  local v2i center = {(i32)st.window.width / 2, (i32)st.window.height / 2};
  v2i current_pos;
  GetCursorPos((POINT*)&current_pos);
  
  i32 dx = current_pos.x - center.x;
  i32 dy = current_pos.y - center.y;

  SetCursorPos(center.x, center.y);
}

v2i  os_get_mouse_pos()         { v2i pos; GetCursorPos(cast(POINT*)&pos); return pos; }
void os_set_mouse_pos(v2i pos)  {          SetCursorPos(pos.x, pos.y); }

void os_register_process_key(ProcessKeyCallback  callback)                { st.process_key = callback; }
void os_register_process_mouse_move(ProcessMouseMoveCallback callback)    { st.process_mouse_move = callback; }
void os_register_window_closed_callback(WindowClosedCallback callback)    { st.window_closed_callback = callback; }
void os_register_window_resized_callback(WindowResizedCallback callback)  { st.window_resized_callback = callback; }

void      os_show_window()              { ShowWindow((HWND)st.window.hwnd, SW_SHOW); }
void      os_window_destroy()           { DestroyWindow((HWND)st.window.hwnd); }
OS_Handle os_get_handle_info()          { return (OS_Handle)st.instance; }
OS_Handle os_get_window_handle()        { return st.window.hwnd; }
v2u       os_get_window_size()          { return v2u{st.window.width, st.window.height}; }
String    os_get_current_directory()    { return st.binary_directory; }
String    os_get_current_binary_name()  { return st.binary_name; }
String    os_get_current_filepath()     { return st.binary_filepath; }

//////////////////////////////////////////////////////////////////////////
// Memory

#define PAGE_SIZE 4096
u8*   os_reserve(u64 size)                  { return (u8*)VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE); }
void  os_commit(void* ptr, u64 size)        { VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE); }
void  os_decommit(void* ptr, u64 size)      { VirtualFree(ptr, size, MEM_DECOMMIT); }
void  os_release(void* ptr, u64 size)       { VirtualFree(ptr, 0, MEM_RELEASE); }
void* os_reserve_large(u64 size)            { return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE); } // we commit on reserve because windows
b32   os_commit_large(void* ptr, u64 size)  { return 1; }

//////////////////////////////////////////////////////////////////////////
// File

OS_Handle os_file_open(String path, OS_AccessFlags flags) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);

  DWORD access_flags = 0;
  DWORD share_mode = 0;
  DWORD creation_disposition = OPEN_EXISTING;

  if(flags & OS_AccessFlag_Read)       {access_flags |= GENERIC_READ;}
  if(flags & OS_AccessFlag_Write)      {access_flags |= GENERIC_WRITE;}
  if(flags & OS_AccessFlag_Execute)    {access_flags |= GENERIC_EXECUTE;}
  if(flags & OS_AccessFlag_ShareRead)  {share_mode |= FILE_SHARE_READ;}
  if(flags & OS_AccessFlag_ShareWrite) {share_mode |= FILE_SHARE_WRITE|FILE_SHARE_DELETE;}
  if(flags & OS_AccessFlag_Write)      {creation_disposition = CREATE_ALWAYS;}
  if(flags & OS_AccessFlag_Append)     {creation_disposition = OPEN_ALWAYS; access_flags |= FILE_APPEND_DATA; }
  
  HANDLE file = CreateFileA((char*)path_c.str, access_flags, share_mode, null, creation_disposition, NoFlags, null);
  if (file == INVALID_HANDLE_VALUE) {
    return {};
  }

  return cast(OS_Handle)file;
}

#define MaxDirectoryWatches 20
#define DirectoryWatchSize 128
OVERLAPPED overlapped[MaxDirectoryWatches];
u8 buffer[DirectoryWatchSize * MaxDirectoryWatches];

OS_Handle os_directory_open(String path) {
  Scratch scratch;
  String path_c = push_str_copy(scratch, path);
  HANDLE handle = CreateFileA(
      (char*)path_c.str,
      FILE_LIST_DIRECTORY,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      null,
      OPEN_EXISTING,
      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
      null);

  if (handle == INVALID_HANDLE_VALUE) {
    Error("Failed to open directory handle. Error: %u\n", GetLastError());
    return 0;
  }
  return (OS_Handle)handle;
}

void os_directory_watch(OS_Handle dir_handle, u32 id) {
  DWORD bytes_returned;
  // Start watching for directory changes
  BOOL success = ReadDirectoryChangesW(
      (HANDLE)dir_handle,
      &buffer[id * DirectoryWatchSize],
      DirectoryWatchSize,
      false, // monitor subdirectories too
      FILE_NOTIFY_CHANGE_LAST_WRITE,
      &bytes_returned,
      &overlapped[id],
      null);

  if (!success) {
    Error("ReadDirectoryChangesW failed. Error: %u", GetLastError());
    return;
  }
}

String os_directory_watch_pop_name(Arena* arena, OS_Handle dir, u32 id) {
  FILE_NOTIFY_INFORMATION* info = (FILE_NOTIFY_INFORMATION*)&buffer[id*DirectoryWatchSize];
  String filename;
  do {
    filename = push_str_wchar(arena, info->FileName, info->FileNameLength / 2);
    Debug("Shader changed: %s", filename);

    if (info->NextEntryOffset == 0)
      break;
    info = (FILE_NOTIFY_INFORMATION*)((BYTE*)info + info->NextEntryOffset);
  } while (true);

  // NOTE: first ReadDirectoryChangesW - rearm overlapped dinge
  DWORD bytes_returned;
  BOOL success = ReadDirectoryChangesW(
      (HANDLE)dir,
      &buffer[id * DirectoryWatchSize],
      DirectoryWatchSize,
      false, // monitor subdirectories too
      FILE_NOTIFY_CHANGE_LAST_WRITE,
      &bytes_returned,
      &overlapped[id],
      null);

  // NOTE second ReadDirectoryChangesW - makes sure it's really rearmed since overlapped dinge several times returns true when only once rearmed
  // success = ReadDirectoryChangesW(
  //     (HANDLE)dir,
  //     &buffer[id * DirectoryWatchSize],
  //     DirectoryWatchSize,
  //     false,
  //     FILE_NOTIFY_CHANGE_LAST_WRITE,
  //     &bytes_returned,
  //     &overlapped[id],
  //     null);

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
      null,             // No module name, use command line
      (char*)cmd_c.str, // Command line
      null,             // Process handle not inheritable
      null,             // Thread handle not inheritable
      FALSE,            // No handle inheritance
      0,                // No creation flags
      null,             // Use parent's environment block
      null,             // Use parent's starting directory
      &si,              // Pointer to STARTUPINFO
      &pi);             // Pointer to PROCESS_INFORMATION

  if (!success) {
    Error("CreateProcess failed (%u).\n", GetLastError());
  }
  is_process_alive = true;
}

///////////////////////////////////////////////////////////////////////////
// WinProc
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
intern LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param) {
  ImGui_ImplWin32_WndProcHandler(hwnd, msg, w_param, l_param);
  
  switch (msg) {
    case WM_ERASEBKGND: {
      // Notfy the OS that erasing will be handled by the application to prevent flicker.
      return 1;
    }
    // case WM_PAINT: {
    //    PAINTSTRUCT ps;
    //    HDC hdc = BeginPaint(hwnd, &ps);

    //    // Clear the screen by filling it with white (or any color)
    //    HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    //    FillRect(hdc, &ps.rcPaint, blackBrush);
    //    DeleteObject(blackBrush);

    //    EndPaint(hwnd, &ps);

    // } break;
    case WM_CLOSE: {
      st.window_closed_callback();
      return true;
    }
    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }
    case WM_WINDOWPOSCHANGING: {

    } break;

    // case WM_WINDOWPOSCHANGING: {
    //   if (GetKeyState(VK_SHIFT) & 0x8000) {
    //     WINDOWPOS* NewPos = (WINDOWPOS*)l_param;

    //     RECT WindowRect;
    //     RECT ClientRect;
    //     GetWindowRect(hwnd, &WindowRect);
    //     GetClientRect(hwnd, &ClientRect);

    //     i32 ClientWidth = (ClientRect.right - ClientRect.left);
    //     i32 ClientHeight = (ClientRect.bottom - ClientRect.top);
    //     i32 WidthAdd = ((WindowRect.right - WindowRect.left) - ClientWidth);
    //     i32 HeightAdd = ((WindowRect.bottom - WindowRect.top) - ClientHeight);

    //     i32 RenderWidth = st.screen_width;
    //     i32 RenderHeight = st.screen_height;

    //     i32 SugX = NewPos->cx;
    //     i32 SugY = NewPos->cy;

    //     i32 NewCx = (RenderWidth * (NewPos->cy - HeightAdd)) / RenderHeight;
    //     i32 NewCy = (RenderHeight * (NewPos->cx - WidthAdd)) / RenderWidth;

    //     if (Abs(cast(f32)NewPos->cx - NewCx) < Abs(cast(f32)NewPos->cy - NewCy)) {
    //       NewPos->cx = NewCx + WidthAdd;
    //     } else {
    //       NewPos->cy = NewCy + HeightAdd;
    //     }
    //   }
    // } break;

    // case WM_WINDOWPOSCHANGED: {
    //   // TODO(casey): For now, we are setting the window styles in here
    //   // because sometimes Windows can reposition our window out of fullscreen
    //   // without going through our ToggleFullscreen(), and we want to put our
    //   // title bar and border back when it does!

    //   WINDOWPOS* NewPos = (WINDOWPOS*)l_param;

    //   b32 BecomingFullscreen = false;
    //   MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
    //   if (GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo)) {
    //     i32 MonWidth = (MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left);
    //     i32 MonHeight = (MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top);
    //     BecomingFullscreen = ((MonitorInfo.rcMonitor.left == NewPos->x) &&
    //                           (MonitorInfo.rcMonitor.top == NewPos->y) &&
    //                           (MonWidth == NewPos->cx) &&
    //                           (MonHeight == NewPos->cy));
    //   }

    //   DWORD OldStyle = GetWindowLong(hwnd, GWL_STYLE);
    //   DWORD FullscreenStyle = OldStyle & ~WS_OVERLAPPEDWINDOW;
    //   DWORD WindowedStyle = OldStyle | WS_OVERLAPPEDWINDOW;
    //   DWORD NewStyle = (BecomingFullscreen) ? FullscreenStyle : WindowedStyle;

    //   if (NewStyle != OldStyle) {
    //     SetWindowLong(hwnd, GWL_STYLE, NewStyle);
    //   }
    // } break;

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
      u32 x = GET_X_LPARAM(l_param);
      u32 y = GET_Y_LPARAM(l_param);
      
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

#endif
