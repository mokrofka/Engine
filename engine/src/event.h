#pragma once
#include "lib.h"

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

// Should return true if handled
using PFN_On_Event = b32 (*)(u32 code, void* sender, void* listener_inst, EventContext data);

void event_init(Arena* arena);

KAPI b32 event_register(u32 code, void* listener, PFN_On_Event on_event);
KAPI b32 event_unregister(u32 code, void* listener, PFN_On_Event on_event);
KAPI b32 event_fire(u32 code, void* sender, EventContext on_event);

// System the application down on the next frame 255
enum SystemEventCode {
  // Shuts the application down on the next frame.
  EventCode_ApplicationQuit = 0x01,

  // Keyboard key pressed.
  /* context usage:
   * u16 key_code = data.data.u16[0];
   */
  EventCode_KeyPressed = 0x02,

  // Keyboard key released.
  /* context usage:
   * u16 key_code = data.data.u16[0];
   */
  EventCode_KeyReleased = 0x03,

  // Mouse button pressed.
  /* context usage:
   * u16 key_code = data.data.u16[0];
   */
  EventCode_ButtonPressed  = 0x04,

  // Mouse button released.
  /* context usage:
   * u16 key_code = data.data.u16[0];
   */
  EventCode_ButtonReleased = 0x05,

  // Mouse moved.
  /* context usage:
   * u16 x = data.data.u16[0];
   * u16 y = data.data.u16[1];
   */
  EventCode_MouseMoved = 0x06,

  /* context usage:
   * u8 z_delta = data.data.u8[0];
   */
  EventCode_MouseWheel = 0x07,
  
  // Resized/resolution changed from the OS.
  /* context usage:
   * u16 x = data.data.u16[0];
   * u16 y = data.data.u16[1];
   */
  EventCode_Resized = 0x08,
  
  EventCode_Debug0 = 0x10,
  EventCode_Debug1 = 0x11,
  EventCode_Debug2 = 0x12,
  EventCode_Debug3 = 0x13,
  EventCode_Debug4 = 0x14,

  EventCode_COUNT = 0xFF
};
