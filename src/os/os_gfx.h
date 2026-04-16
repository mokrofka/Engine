#pragma once
#include "os/os_core.h"
#include "base/maths.h"

KAPI void os_gfx_init();
KAPI void os_gfx_shutdown();

KAPI void os_pump_messages();
KAPI b32  os_window_should_close();
KAPI v2u  os_get_window_size();
KAPI v2   os_get_mouse_pos();
KAPI void os_get_gfx_api_handlers(void* out);
KAPI void os_close_window();

enum Key {
  // Control keys
  Key_Backspace,
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

  // Mouse
  MouseKey_Left,
  MouseKey_Right,
  MouseKey_Middle,

  Key_COUNT
};

typedef u32 OS_Modifiers;
enum {
  OS_Modifier_Ctrl  = Bit(0),
  OS_Modifier_Shift = Bit(1),
  OS_Modifier_Alt   = Bit(2),
};

enum OS_EventKind {
  OS_EventKind_Key,
  OS_EventKind_MouseButton,
  OS_EventKind_MouseMove,
  OS_EventKind_Scroll,
  OS_EventKind_Modifier,
};

struct OS_InputEvent {
  OS_EventKind type;
  Key key;
  u32 character;
  b32 is_pressed;
  OS_Modifiers modifier;
  f32 x, y;
  f32 scroll_x, scroll_y;
};

u32 os_key_to_str(Key key, OS_Modifiers modifiers);
Slice<OS_InputEvent> os_get_events();

void os_input_update();

KAPI b32 os_is_key_down(Key key);
KAPI b32 os_is_key_up(Key key);
KAPI b32 os_was_key_down(Key key);
KAPI b32 os_was_key_up(Key key);
KAPI b32 os_is_key_pressed(Key key);
KAPI b32 os_is_key_released(Key key);

void os_clipboard_write(String str);
String os_clipboard_read();

