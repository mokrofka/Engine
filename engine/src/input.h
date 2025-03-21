#pragma once

#include "defines.h"

#include <input_types.h>

#define DEFINE_KEY(name, code) KEY_##name = code

b8 input_initialize(u64* memory_requirement, void* out_state);
void input_shutdown();
void input_update();

// keyboard input
KAPI b8 input_is_key_down(Keys key);
KAPI b8 input_is_key_up(Keys key);
KAPI b8 input_was_key_down(Keys key);
KAPI b8 input_was_key_up(Keys key);

void input_process_key(Keys key, b8 pressed);

// mouse input
KAPI b8 input_is_button_down(MouseButtons button);
KAPI b8 input_is_button_up(MouseButtons button);
KAPI b8 input_was_button_down(MouseButtons button);
KAPI b8 input_was_button_up(MouseButtons button);
KAPI void input_get_mouse_position(i32* x, i32* y);
KAPI void input_get_previous_mouse_position(i32* x, i32* y);

void input_process_button(MouseButtons button, b8 pressed);
void input_process_mouse_move(i16 x, i16 y);
void input_process_mouse_wheel(i8 z_delta);
