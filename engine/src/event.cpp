#include "event.h"

struct Array {
  u32 res;
  u32 pos;
};

struct RegisteredEvent {
  void* listener;
  PFN_On_Event  callback;
};

struct EventCodeEntry {
  Array array;
  RegisteredEvent* events;
};

#define MAX_MESSAGE_CODES 255

struct EventSystemState {
  Arena* arena;
  EventCodeEntry registered[MAX_MESSAGE_CODES];
};

global EventSystemState st;

void event_init(Arena* arena) {
  st.arena = arena_alloc(arena, KB(1));
}

void event_register(u32 code, void* listener, PFN_On_Event on_event) {
  if (st.registered[code].events == null) {
    st.registered[code].events = push_array(st.arena, RegisteredEvent, 5);
    st.registered[code].array = {.res = 5, .pos = 0};
  }
  
  u32 register_count = st.registered[code].array.pos;
  Loop (i, register_count) {
    if(st.registered[code].events[i].listener == listener) {
      Warn("You're registering the same event!");
      return;
    }
  }
  
  if (register_count >= st.registered[code].array.res) {
    RegisteredEvent* events = push_array(st.arena, RegisteredEvent, st.registered[code].array.res*5);
    MemCopyTyped(events, st.registered[code].events, st.registered[code].array.res);
    st.registered[code].events = events;
  } 
  
  RegisteredEvent event = {
    .listener = listener,
    .callback = on_event,
  };
  st.registered[code].events[register_count] = event;
  ++st.registered[code].array.pos;

  return;
}

void event_unregister(u32 code, void* listener, PFN_On_Event on_event) {
  if (st.registered[code].array.pos == 0) {
    Warn("you're trying to unregister nothing!");
    return;
  }
  
  u32 register_count = st.registered[code].array.pos;
  Loop (i, register_count) {
    RegisteredEvent e = st.registered[code].events[i];
    if (e.listener == listener && e.callback == on_event) {
      st.registered[code].events[i] = st.registered[code].events[register_count-1];
      --st.registered[code].array.pos;
      return;
    }
  }
  
  return;
}

void event_fire(u32 code, void* sender, EventContext context) {
  if (st.registered[code].events == 0) {
    return;
  }
  
  u32 register_count = st.registered[code].array.pos;
  Loop (i, register_count) {
    RegisteredEvent e = st.registered[code].events[i];
    if (e.callback(code, sender, e.listener, context)) {
      return;
    }
  }
}


