#include "lib.h"
#include "event.h"

struct RegisteredEvent {
  void* listener;
  PFN_On_Event callback;
};

struct EventSystemState {
  Arena* arena;
  DarrayArena<RegisteredEvent> registered[EventCode_COUNT];
};

global EventSystemState st;

void event_init() {
  st.arena = arena_alloc();
  Loop (i, EventCode_COUNT) {
    st.registered[i].arena = st.arena;
  }
}

void event_register(u32 code, void* listener, PFN_On_Event on_event) {
  Loop (i, st.registered[code].count) {
    if(st.registered[code][i].listener == listener) {
      Warn("You're registering the same event!");
      return;
    }
  }
  append(st.registered[code], {listener, on_event});
}

void event_unregister(u32 code, void* listener, PFN_On_Event on_event) {
  if (st.registered[code].count == 0) {
    Assert(!"you're trying to unregister nothing!");
  }
  
  Loop (i, st.registered[code].count) {
    if (st.registered[code][i].listener == listener && st.registered[code][i].callback == on_event) {
      remove(st.registered[code], i);
    }
  }
}

void event_fire(u32 code, void* sender, EventContext context) {
  Loop (i, st.registered[code].count) {
    RegisteredEvent e = st.registered[code][i];
    if (e.callback(code, sender, e.listener, context)) {
      return;
    }
  }
}
