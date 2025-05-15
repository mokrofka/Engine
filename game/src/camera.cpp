#include "camera.h"
#include "game.h"
#include "input.h"

void view_matrix_update() {
  if (state->camera_view_dirty) {
    mat4 translation = mat4_translation(state->camera_position);
    mat4 rotaion = mat4_euler_xyz(state->camera_direction.x, state->camera_direction.y, state->camera_direction.z);
    
    state->view = translation * rotaion;
    state->view = mat4_inverse(state->view);
    state->camera_view_dirty = false;
  }
}

internal void camera_yaw(f32 amount) {
  state->camera_direction.y += amount;
  state->camera_view_dirty = true;
}

internal void camera_pitch(f32 amount) {
  state->camera_direction.x += amount;
  
  // Clamp to avoid Gimball lock
  f32 limit = deg_to_rad(89.0f);
  state->camera_direction.x = Clamp(-limit, state->camera_direction.x, limit);
  
  state->camera_view_dirty = true;
}

void camera_manage() {
  f32 rotation_speed = 3.0f;

  // HACK temp back to move camera around
  if (input_is_key_down(KEY_A) || input_is_key_down(KEY_LEFT)) {
    camera_yaw(rotation_speed * state->delta);
  }
  if (input_is_key_down(KEY_D) || input_is_key_down(KEY_RIGHT)) {
    camera_yaw(-rotation_speed * state->delta);
  }
  if (input_is_key_down(KEY_UP)) {
    camera_pitch(rotation_speed * state->delta);
  }
  if (input_is_key_down(KEY_DOWN)) {
    camera_pitch(-rotation_speed * state->delta);
  }
  
  f32 temp_move_speed = 20.0f;
  v3 velocity = {};
  
  if (input_is_key_down(KEY_W)) {
    v3 forward = mat4_forward(state->view);
    velocity += forward;
  }
  if (input_is_key_down(KEY_S)) {
    v3 backward = mat4_backward(state->view);
    velocity += backward;
  }
  
  if (input_is_key_down(KEY_Q)) {
    v3 left = mat4_left(state->view);
    velocity += left;
  }
  if (input_is_key_down(KEY_E)) {
    v3 right = mat4_right(state->view);
    velocity += right;
  }
  
  if (input_is_key_down(KEY_SPACE)) {
    velocity.y += 1.0f;
  }
  if (input_is_key_down(KEY_X)) {
    velocity.y -= 1.0f;
  }
  
  v3 z = v3_zero();
  if (z != velocity) {
    // Be sure to normalize the velocity before applying speed 
    v3_normalize(&velocity);
    state->camera_position += velocity * temp_move_speed * state->delta;
    state->camera_view_dirty = true;
  }
  
  view_matrix_update();
}
