#include "input.h"
#include "event.h"

#include "core/memory.h"
#include "core/logger.h"

struct KeyboardState {
  b8 keys[256];
};

struct MouseState {
  i16 x;
  i16 y;
  b8 buttons[MOUSE_BUTTON_MAX];
};

struct InputState {
  KeyboardState keyboard_current;
  KeyboardState keyboard_previous;
  MouseState mouse_current;
  MouseState mouse_previous;
};

internal InputState* state;

b8 input_initialize(u64* memory_requirement, void* out_state) {
  *memory_requirement = sizeof(InputState);
  if (out_state == 0) {
    return true;
  }
  state = (InputState*)out_state;
  Info("Input subsystem initialized.");
  return true;
}

void input_shutdown() {
  
}

void input_update(f64 delta_time) {
  
  // Copy current states to previous states.
  MemCopyStruct(&state->keyboard_previous, &state->keyboard_current);
  MemCopyStruct(&state->mouse_previous, &state->mouse_previous);
}

void input_process_key(Keys key, b8 pressed) {
  // Only handle this if the state actually changed.
  if (state->keyboard_current.keys[key] != pressed) {
    // Update internal state.
    state->keyboard_current.keys[key] = pressed;
    
    // Fire off an event for immediate processing.
    EventContext context;
    context.data.u16[0] = key;
    event_fire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context);
  }
}

void input_process_button(MouseButtons button, b8 pressed) {
  // If the state changed, fire an event.
  if (state->mouse_current.buttons[button] != pressed) {
    state->mouse_current.buttons[button] = pressed;
    
    // Fire the event.
    EventContext context;
    context.data.u16[0] = button;
    event_fire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, context);
  }
}

void input_process_mouse_move(i16 x, i16 y) {
  // Only process if actually different.
  if (state->mouse_current.x != x || state->mouse_current.y != y) {
    // NOTE: Enable this if debugging.
    // KDEBUG("Mouse pos: %i, %i!", x, y);
    
    state->mouse_current.x = x;
    state->mouse_current.y = y;
    
    // Fire the event.
    EventContext context;
    context.data.u16[0] = x;
    context.data.u16[1] = y;
    event_fire(EVENT_CODE_MOUSE_MOVED, 0, context);
  }
}

void input_process_mouse_wheel(i8 z_delta) {
  // NOTE: no internal state to update.
  // Fire the event.
  EventContext context;
  context.data.u8[0] = z_delta;
  event_fire(EVENT_CODE_MOUSE_WHEEL, 0, context);
}

b8 input_is_key_down(Keys key) {
  if (!state) {
    return false;
  }
  return state->keyboard_current.keys[key] == true;
}

b8 input_is_key_up(Keys key) {
  if (!state) {
    return true;
  }
  return state->keyboard_current.keys[key] == false;
}

b8 input_was_key_down(Keys key) {
  if (!state) {
    return false;
  }
  return state->keyboard_previous.keys[key] == true;
}

b8 input_was_key_up(Keys key) {
  if (!state) {
    return true;
  }
  return state->keyboard_previous.keys[key] == false;
}

// mouse input
b8 input_is_button_down(MouseButtons button) {
  if (!state) {
    return false;
  }
  return state->mouse_current.buttons[button] == true;
}

b8 input_is_button_up(MouseButtons button) {
  if (!state) {
    return true;
  }
  return state->mouse_current.buttons[button] == false;
}

b8 input_was_button_down(MouseButtons button) {
  if (!state) {
    return false;
  }
  return state->mouse_previous.buttons[button] == true;
}

b8 input_was_button_up(MouseButtons button) {
  if (!state) {
    return true;
  }
  return state->mouse_previous.buttons[button] == false;
}

void input_get_mouse_position(i32* x, i32* y) {
  if (!state) {
    *x = 0;
    *y = 0;
    return;
  }
  *x = state->mouse_current.x;
  *y = state->mouse_current.y;
}

void input_get_previous_mouse_position(i32* x, i32* y) {
  if (!state) {
    *x = 0;
    *y = 0;
    return;
  }
  *x = state->mouse_previous.x;
  *y = state->mouse_previous.y;
}
