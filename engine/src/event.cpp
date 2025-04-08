#include "event.h"

#include <logger.h>
#include <memory.h>

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

// This should be more than enough codes...
#define MAX_MESSAGE_CODES 255

// State structure
struct EventSystemState {
  Arena* arena;
  // Lookup table for event codes.
  b8 is_initialized;
  EventCodeEntry registered[MAX_MESSAGE_CODES];
  void* memory;
};

global EventSystemState* state;

b8 event_init(Arena* arena) {
  u64 memory_reserved = KB(10);
  u64 memory_requirement = sizeof(EventSystemState) + memory_reserved;
  
  state = push_buffer(arena, EventSystemState, memory_requirement);
  state->is_initialized = true;
  state->arena = (Arena*)&state->memory;
  state->arena->res = memory_reserved;
  
  return true;
}

// void event_shutdown() {
//   // Free the events arrays. And objects pointed to should be destroyed on their own.
//   for(u16 i = 0; i < MAX_MESSAGE_CODES; ++i){
//     if(state->registered[i].events != 0) {
//       // darray_destroy(state.registered[i].events);
//       state->registered[i].events = 0;
//     }
//   }
// }

b8 event_register(u16 code, void* listener, PFN_On_Event  on_event) {
  if (state->is_initialized == false) {
    return false;
  }
  
  if (state->registered[code].events == 0) {
    // state->registered[code].events = push_array(state->arena, RegisteredEvent, 4);
    state->registered[code].events = push_array(state->arena, RegisteredEvent, 4);
    state->registered[code].array = {.res = 4, .pos = 0};
  }
  
  u64 register_count = state->registered[code].array.pos;
  for (u64 i = 0; i < register_count; ++i) {
    if(state->registered[code].events[i].listener == listener) {
      Warn("You're registering the same event!");
      return false;
    }
  }
  
  if (register_count >= state->registered[code].array.res) {
    RegisteredEvent* events = push_array(state->arena, RegisteredEvent, state->registered[code].array.res*5);
    MemCopyTyped(events, state->registered[code].events, state->registered[code].array.res);
    state->registered[code].events = events;
  } 
  
  // If at this point, no duplicate was found. Proceed with registration.
  RegisteredEvent event;
  event.listener = listener;
  event.callback = on_event;
  state->registered[code].events[register_count] = event;
  ++state->registered[code].array.pos;

  return true;
}

b8 event_unregister(u16 code, void* listener, PFN_On_Event on_event) {
  // On nothing is registered for the code, boot out.
  if (state->registered[code].array.pos == 0) {
    Warn("you're trying to unregister nothing!");
    return false;
  }
  
  u64 register_count = state->registered[code].array.pos;
  for (u64 i = 0; i < register_count; ++i) {
    RegisteredEvent e = state->registered[code].events[i];
    if (e.listener == listener && e.callback == on_event) {
      state->registered[code].events[i] = state->registered[code].events[register_count-1];
      --state->registered[code].array.pos;
      return true;
    }
  }
  
  // Not found.
  return false;
}

b8 event_fire(u16 code, void* sender, EventContext context) {
  if (state->is_initialized == false) {
    return false;
  }
  
  // On nothing is registered for the code, boot out.
  if (state->registered[code].events == 0) {
    return false;
  }
  
  u64 register_count = state->registered[code].array.pos;
  for (u64 i = 0; i < register_count; ++i) {
    RegisteredEvent e = state->registered[code].events[i];
    if (e.callback(code, sender, e.listener, context)) {
      // Message has been handled, do not send to other listeners.
      return true;
    }
  }
  
  // Not found.
  return false;
}


