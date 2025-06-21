#pragma once
#include "lib.h"

#include <input_types.h>

void input_init(Arena* arena);
void input_shutdown();
void input_update();

// keyboard
KAPI b32 input_is_key_down(Keys key);
KAPI b32 input_is_key_up(Keys key);
KAPI b32 input_was_key_down(Keys key);
KAPI b32 input_was_key_up(Keys key);
KAPI b32 input_was_key_pressed(Keys key);
KAPI b32 input_was_key_released(Keys key);

void input_process_key(Keys key, b32 pressed);

// mouse
KAPI b32 input_is_button_down(MouseButtons button);
KAPI b32 input_is_button_up(MouseButtons button);
KAPI b32 input_was_button_down(MouseButtons button);
KAPI b32 input_was_button_up(MouseButtons button);
KAPI void input_get_mouse_position(i32* x, i32* y);
KAPI void input_get_previous_mouse_position(u32* x, u32* y);

void input_process_button(MouseButtons button, b32 pressed);
void input_process_mouse_move(u32 x, u32 y);
void input_process_mouse_wheel(i32 z_delta);

