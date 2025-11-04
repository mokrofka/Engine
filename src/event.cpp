#include "lib.h"
#include "event.h"

struct RegisteredEvent {
  void* listener;
  PFN_On_Event callback;
};

struct EventSystemState {
  Arena* arena;
  Darray<RegisteredEvent> registered[EventCode_COUNT];
};

global EventSystemState st;

void event_register(u32 code, void* listener, PFN_On_Event on_event) {
  for (RegisteredEvent e : st.registered[code]) {
    if(e.listener == listener) {
      Warn("You're registering the same event!");
      return;
    }
  }
  append(st.registered[code], {listener, on_event});
}

void event_unregister(u32 code, void* listener, PFN_On_Event on_event) {
  if (len(st.registered[code]) == 0) {
    Assert(!"you're trying to unregister nothing!");
  }
  u32 index = 0;
  for (RegisteredEvent e : st.registered[code]) {
    if (e.listener == listener && e.callback == on_event) {
      remove(st.registered[code], index);
    }
    ++index;
  }
}

void event_fire(u32 code, void* sender, EventContext context) {
  for (RegisteredEvent e : st.registered[code]) {
    if (e.callback(code, sender, e.listener, context)) {
      return;
    }
  }
}

