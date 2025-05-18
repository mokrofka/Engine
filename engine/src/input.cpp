#include "input.h"
#include "event.h"

struct KeyboardState {
  b8 keys[256];
};

struct MouseState {
  u16 x;
  u16 y;
  b8 buttons[MOUSE_BUTTON_MAX];
};

struct InputState {
  KeyboardState keyboard_current;
  KeyboardState keyboard_previous;
  MouseState mouse_current;
  MouseState mouse_previous;
};

global InputState* state;

void input_init(Arena* arena) {
  state = push_struct(arena, InputState);
  Info("Input subsystem initialized."_);
}

void input_shutdown() {
  
}

void input_update() {
  // Copy current states to previous states.
  MemCopyStruct(&state->keyboard_previous, &state->keyboard_current);
  MemCopyStruct(&state->mouse_previous, &state->mouse_previous);
}

void input_process_key(Keys key, b32 pressed) {
  // Only handle this if the state actually changed.
  
  if (state->keyboard_current.keys[key] != pressed) {
    // Update internal state.
    state->keyboard_current.keys[key] = pressed;

    if (key == KEY_LALT) {
      Info("Left alt %s.", pressed ? "pressed"_ : "released"_);
    } else if (key == KEY_RALT) {
      Info("Right alt %s.", pressed ? "pressed"_ : "released"_);
    }

    if (key == KEY_LCONTROL) {
      Info("Left ctrl %s.", pressed ? "pressed"_ : "released"_);
    } else if (key == KEY_RCONTROL) {
      Info("Right ctrl %s.", pressed ? "pressed"_ : "released"_);
    }

    if (key == KEY_LSHIFT) {
      Info("Left shift %s.", pressed ? "pressed"_ : "released"_);
    } else if (key == KEY_RSHIFT) {
      Info("Right shift %s.", pressed ? "pressed"_ : "released"_);
    }

    // Fire off an event for immediate processing.
    EventContext context;
    context.data.u16[0] = key;
    event_fire(pressed ? EventCode_KeyPressed : EventCode_KeyReleased, 0, context);
  }
}

void input_process_button(MouseButtons button, b32 pressed) {
  // If the state changed, fire an event.
  if (state->mouse_current.buttons[button] != pressed) {
    state->mouse_current.buttons[button] = pressed;
    
    // Fire the event.
    EventContext context;
    context.data.u16[0] = button;
    event_fire(pressed ? EventCode_ButtonPressed  : EventCode_ButtonReleased, 0, context);
  }
}

void input_process_mouse_move(u32 x, u32 y) {
  // Only process if actually different.
  if (state->mouse_current.x != x || state->mouse_current.y != y) {
    // NOTE: Enable this if debugging.
    // Debug("Mouse pos: %i, %i!", x, y);
    
    state->mouse_current.x = x;
    state->mouse_current.y = y;
    
    // Fire the event.
    EventContext context;
    context.data.u16[0] = x;
    context.data.u16[1] = y;
    event_fire(EventCode_MouseMoved, 0, context);
  }
}

void input_process_mouse_wheel(i32 z_delta) {
  // NOTE: no internal state to update.
  // Fire the event.
  EventContext context;
  context.data.u8[0] = z_delta;
  event_fire(EventCode_MouseWheel, 0, context);
}

// keyboard input
b32 input_is_key_down(Keys key) {
  return state->keyboard_current.keys[key] == true;
}

b32 input_is_key_up(Keys key) {
  return state->keyboard_current.keys[key] == false;
}

b32 input_was_key_down(Keys key) {
  return state->keyboard_previous.keys[key] == true;
}

b32 input_was_key_up(Keys key) {
  return state->keyboard_previous.keys[key] == false;
}

b32 input_was_key_pressed(Keys key) {
  return input_is_key_down(key) && input_was_key_up(key);
}

b32 input_was_key_released(Keys key) {
  return input_is_key_up(key) && input_was_key_down(key);
}

// mouse input
b32 input_is_button_down(MouseButtons button) {
  return state->mouse_current.buttons[button] == true;
}

b32 input_is_button_up(MouseButtons button) {
  return state->mouse_current.buttons[button] == false;
}

b32 input_was_button_down(MouseButtons button) {
  return state->mouse_previous.buttons[button] == true;
}

b32 input_was_button_up(MouseButtons button) {
  return state->mouse_previous.buttons[button] == false;
}

void input_get_mouse_position(i32* x, i32* y) {
  *x = state->mouse_current.x;
  *y = state->mouse_current.y;
}

void input_get_previous_mouse_position(i32* x, i32* y) {
  *x = state->mouse_previous.x;
  *y = state->mouse_previous.y;
}
