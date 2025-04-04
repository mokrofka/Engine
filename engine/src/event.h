#pragma once

#include "defines.h"

struct EventContext {
  // 128 bytes
  union {
    i64 i64[2];
    u64 u64[2];
    f64 f64[2];
    
    i32 i32[4];
    u32 u32[4];
    f32 f32[4];
    
    i16 i16[8];
    u16 u16[8];
    
    i8 i8[16];
    u8 u8[16];
    
    char c[16];
  } data;
};

struct Arena;

// Should return true if handled
using PFN_On_Event  = b8(*)(u16 code, void* sender, void* listener_inst, EventContext data);

b8 event_init(Arena* arena);

KAPI b8 event_register(u16 code, void* listener, PFN_On_Event  on_event);
KAPI b8 event_unregister(u16 code, void* listener, PFN_On_Event  on_event);
KAPI b8 event_fire(u16 code, void* sender, EventContext on_event);

// System the application down on the next frame 255
enum SystemEventCode {
  // Shuts the application down on the next frame.
  EVENT_CODE_APPLICATION_QUIT = 0x01,
  Event_Code_AppLication_Quit = 0x01,

  // Keyboard key pressed.
  /* context usage:
   * u16 key_code = data.data.u16[0];
   */
  EVENT_CODE_KEY_PRESSED = 0x02,
  Event_Code_Key_Pressed = 0x02,

  // Keyboard key released.
  /* context usage:
   * u16 key_code = data.data.u16[0];
   */
  EVENT_CODE_KEY_RELEASED = 0x03,

  // Mouse button pressed.
  /* context usage:
   * u16 key_code = data.data.u16[0];
   */
  EVENT_CODE_BUTTON_PRESSED = 0x04,

  // Mouse button released.
  /* context usage:
   * u16 key_code = data.data.u16[0];
   */
  EVENT_CODE_BUTTON_RELEASED = 0x05,

  // Mouse moved.
  /* context usage:
   * u16 x = data.data.u16[0];
   * u16 y = data.data.u16[1];
   */
  EVENT_CODE_MOUSE_MOVED = 0x06,

  /* context usage:
   * u8 z_delta = data.data.u8[0];
   */
  EVENT_CODE_MOUSE_WHEEL = 0x07,
  
  // Resized/resolution changed from the OS.
  /* context usage:
   * u16 x = data.data.u16[0];
   * u16 y = data.data.u16[1];
   */
  EVENT_CODE_RESIZED = 0x08,
  
  EVENT_CODE_DEBUG0 = 0x10,
  EVENT_CODE_DEBUG1 = 0x11,
  EVENT_CODE_DEBUG2 = 0x12,
  EVENT_CODE_DEBUG3 = 0x13,
  EVENT_CODE_DEBUG4 = 0x14,

  MAX_EVENT_CODE = 0xFF
};
