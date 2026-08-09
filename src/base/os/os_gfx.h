#pragma once
#include "os_core.h"
#include "base/maths.h"

enum Key {
  // Control keys
  Key_Null,
  Key_Backspace = 1,
  Key_Enter,
  Key_Tab,
  Key_Delete,
  Key_LShift,
  Key_Shift = Key_LShift,
  Key_RShift,
  Key_LControl,
  Key_Ctrl = Key_LControl,
  Key_RControl,
  Key_LAlt,
  Key_Alt = Key_LAlt,
  Key_RAlt,
  Key_Escape,
  Key_Capslock,
  Key_Super,

  // Navigation
  Key_Space,
  Key_Pageup,
  Key_Pagedown,
  Key_End,
  Key_Home,
  Key_Left,
  Key_Up,
  Key_Right,
  Key_Down,

  // Special keys
  Key_Pause,
  Key_Print,
  Key_Printscreen,
  Key_Lsuper,
  Key_Rsuper,
  Key_Apps,
  Key_Numlock,

  // Numbers
  Key_0,
  Key_1,
  Key_2,
  Key_3,
  Key_4,
  Key_5,
  Key_6,
  Key_7,
  Key_8,
  Key_9,

  // Letters
  Key_A,
  Key_B,
  Key_C,
  Key_D,
  Key_E,
  Key_F,
  Key_G,
  Key_H,
  Key_I,
  Key_J,
  Key_K,
  Key_L,
  Key_M,
  Key_N,
  Key_O,
  Key_P,
  Key_Q,
  Key_R,
  Key_S,
  Key_T,
  Key_U,
  Key_V,
  Key_W,
  Key_X,
  Key_Y,
  Key_Z,

  // Function keys
  Key_F1,
  Key_F2,
  Key_F3,
  Key_F4,
  Key_F5,
  Key_F6,
  Key_F7,
  Key_F8,
  Key_F9,
  Key_F10,
  Key_F11,
  Key_F12,

  // Symbols
  Key_Semicolon,
  Key_Apostrophe,
  Key_Quote = Key_Apostrophe,
  Key_Comma,
  Key_Dot,
  Key_Equal,
  Key_Minus,
  Key_Grave,
  Key_LBracket,
  Key_RBracket,
  Key_Slash,
  Key_Backslash,

  Key_COUNT
};

enum MouseButton {
  MouseButton_Left,
  MouseButton_Right,
  MouseButton_Middle,
  MouseButton_COUNT,
};

typedef u32 OS_Modifiers;
enum {
  OS_Modifier_Ctrl  = Bit(0),
  OS_Modifier_Shift = Bit(1),
  OS_Modifier_Alt   = Bit(2),
};

enum OS_EventKind {
  OS_EventType_Key,
  OS_EventType_MouseButton,
  OS_EventType_MouseMove,
  OS_EventType_Scroll,
  OS_EventType_Modifier,
};

struct OS_InputEvent {
  OS_EventKind type;
  Key key;
  MouseButton mouse_button;
  u32 character;
  b32 is_pressed;
  OS_Modifiers modifier;
  f32 x, y;
  f32 scroll;
};

u32 os_key_to_character(Key key, OS_Modifiers modifiers);

void os_gfx_init();
void os_gfx_shutdown();

////////////////////////////////////////////////////////////////////////
// Window related
void   os_pump_messages();
b32    os_window_should_close();
void   os_window_close();
v2u    os_window_size();
u32    os_window_width();
u32    os_window_height();
v2u    os_screen_size();
u32    os_screen_width();
u32    os_screen_height();
void   os_get_gfx_handlers(void* out);
void   os_clipboard_text_set(String str);
String os_clipboard_text_get();

////////////////////////////////////////////////////////////////////////
// Cursor
void os_cursor_show();
void os_cursor_hide();
b32  os_cursor_is_hiden();
void os_cursor_confine_window();
void os_cursor_release_window();
void os_cursor_lock();
void os_cursor_unlock();

////////////////////////////////////////////////////////////////////////
// Input
void os_input_update();
Slice<OS_InputEvent> os_get_input_events();

b32 os_key_is_down(Key key);
b32 os_key_is_up(Key key);
b32 os_key_was_down(Key key);
b32 os_key_was_up(Key key);
b32 os_key_is_pressed(Key key);
b32 os_key_is_released(Key key);

OS_Modifiers os_key_modifiers();

b32 os_mouse_is_button_down(MouseButton button);
b32 os_mouse_is_button_up(MouseButton button);
b32 os_mouse_was_button_down(MouseButton button);
b32 os_mouse_was_button_up(MouseButton button);
b32 os_mouse_is_button_pressed(MouseButton button);
b32 os_mouse_is_button_released(MouseButton button);
v2  os_mouse_pos();
f32 os_mouse_wheel();
f32 os_mouse_wheel_horizontal();
v2  os_mouse_dt();
void os_mouse_set_pos(v2i pos); // works only when cursor is hidden

const char* imgui_platform_get_clipboard_text(struct ImGuiContext* ctx);
void imgui_platform_set_clipboard_text(struct ImGuiContext* ctx, const char* text);
