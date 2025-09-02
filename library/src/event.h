#pragma once
#include "base/defines.h"

union EventContext {
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
};

// Should return false if you want other listeners to listen
using PFN_On_Event = b32 (*)(u32 code, void* sender, void* listener_inst, EventContext data);

KAPI void event_init();

KAPI void event_register(u32 code, void* listener, PFN_On_Event on_event);
KAPI void event_unregister(u32 code, void* listener, PFN_On_Event on_event);
KAPI void event_fire(u32 code, void* sender, EventContext on_event);

enum SystemEventCode {
  EventCode_ApplicationQuit,
  EventCode_KeyPressed,
  EventCode_KeyReleased,
  EventCode_ButtonPressed,
  EventCode_ButtonReleased,
  EventCode_MouseMoved,
  EventCode_MouseWheel,
  EventCode_Resized,
  EventCode_ViewportResized,

  EventCode_COUNT
};
