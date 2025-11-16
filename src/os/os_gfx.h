#pragma once
#include "lib.h"

KAPI void os_gfx_init();
KAPI void os_gfx_shutdown();

KAPI void  os_pump_messages();
KAPI b32   os_window_should_close();
KAPI u8*   os_window_get_buffer();
KAPI v2i   os_get_window_size();
KAPI void* os_get_gfx_api_thing();
KAPI void  os_close_window();

enum MouseButtons {
  MouseButton_Left,
  MouseButton_Right,
  MouseButton_Middle,
  
  MouseButton_COUNT
};

enum Key {
  // Control keys
  Key_Backspace = 0X08,
  Key_Enter = 0X0d,
  Key_Tab = 0X09,
  Key_Shift = 0X10,
  Key_LShift = 0xA0,
  Key_RShift = 0xA1,
  Key_Control = 0X11,
  Key_LControl = 0xA2,
  Key_RControl = 0xA3,
  Key_LAlt = 0xA4,
  Key_RAlt = 0xA5,
  Key_Escape = 0x1B,
  Key_Capslock = 0x14,

  // Navigation
  Key_Space = 0x20,
  Key_Pageup = 0x21,
  Key_Pagedown = 0x22,
  Key_End = 0x23,
  Key_Home = 0x24,
  Key_Left = 0x25,
  Key_Up = 0x26,
  Key_Right = 0x27,
  Key_Down = 0x28,

  // Special keys
  Key_Pause = 0x13,
  Key_Print = 0x2A,
  Key_Printscreen = 0x2C,
  Key_Delete = 0x2E,
  Key_Lsuper = 0x5B,
  Key_Rsuper = 0x5C,
  Key_Apps = 0x5D,
  Key_Numlock = 0x90,

  // Numbers
  Key_0 = 0x30,
  Key_1 = 0x31,
  Key_2 = 0x32,
  Key_3 = 0x33,
  Key_4 = 0x34,
  Key_5 = 0x35,
  Key_6 = 0x36,
  Key_7 = 0x37,
  Key_8 = 0x38,
  Key_9 = 0x39,

  // Letters
  Key_A = 0x41,
  Key_B = 0x42,
  Key_C = 0x43,
  Key_D = 0x44,
  Key_E = 0x45,
  Key_F = 0x46,
  Key_G = 0x47,
  Key_H = 0x48,
  Key_I = 0x49,
  Key_J = 0x4A,
  Key_K = 0x4B,
  Key_L = 0x4C,
  Key_M = 0x4D,
  Key_N = 0x4E,
  Key_O = 0x4F,
  Key_P = 0x50,
  Key_Q = 0x51,
  Key_R = 0x52,
  Key_S = 0x53,
  Key_T = 0x54,
  Key_U = 0x55,
  Key_V = 0x56,
  Key_W = 0x57,
  Key_X = 0x58,
  Key_Y = 0x59,
  Key_Z = 0x5A,

  // Function keys
  Key_F1 = 0x70,
  Key_F2 = 0x71,
  Key_F3 = 0x72,
  Key_F4 = 0x73,
  Key_F5 = 0x74,
  Key_F6 = 0x75,
  Key_F7 = 0x76,
  Key_F8 = 0x77,
  Key_F9 = 0x78,
  Key_F10 = 0x79,
  Key_F11 = 0x7A,
  Key_F12 = 0x7B,

  // Symbols
  Key_Semicolon = 0x3B,
  Key_Apostrophe = 0xDE,
  Key_Quote = Key_Apostrophe,
  Key_Equal = 0xBB,
  Key_Comma = 0xBC,
  Key_Minus = 0xBD,
  Key_Dot = 0xBE,
  Key_Slash = 0xBF,
  Key_Grave = 0xC0,
  Key_LBracket = 0xDB,
  Key_RBracket = 0xDD,
  Key_Backslash = 0xDC,

  Key_COUNT
};

void os_input_update();

////////////////////////////////////////////////////////////////////////
// keyboard

KAPI b32 os_is_key_down(Key key);
KAPI b32 os_is_key_up(Key key);
KAPI b32 os_was_key_down(Key key);
KAPI b32 os_was_key_up(Key key);
KAPI b32 os_is_key_pressed(Key key);
KAPI b32 os_is_key_released(Key key);

KAPI void os_process_key(Key key, b32 pressed);

////////////////////////////////////////////////////////////////////////
// mouse

KAPI b32 os_is_button_down(MouseButtons button);
KAPI b32 os_is_button_up(MouseButtons button);
KAPI b32 os_was_button_down(MouseButtons button);
KAPI b32 os_was_button_up(MouseButtons button);
KAPI b32 os_is_button_pressed(MouseButtons button);
KAPI b32 os_is_button_released(MouseButtons button);

KAPI v2i  os_get_mouse_pos();
