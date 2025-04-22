#include "game.h"

#include <engine.h>

// HACK This should not be available outside the engine
#include <render/r_frontend.h>

#include <event.h>
#include <input.h>

struct GameState {
  Arena* arena;
  
  mat4 view;
  v3 camera_position;
  v3 camera_euler;
  b8 camera_view_dirty;
};

GameState* state;

internal void recalculate_view_matrix() {
  if (state->camera_view_dirty) {
    mat4 rotaion = mat4_euler_xyz(state->camera_euler.x, state->camera_euler.y, state->camera_euler.z);
    mat4 translation = mat4_translation(state->camera_position);
    
    state->view = translation * rotaion;
    state->view = mat4_inverse(state->view);
    state->camera_view_dirty = false;
  }
}

internal void camera_yaw(f32 amount) {
  state->camera_euler.y += amount;
  state->camera_view_dirty = true;
}

internal void camera_pitch(f32 amount) {
  state->camera_euler.x += amount;
  
  // Clamp to avoid Gimball lock
  f32 limit = deg_to_rad(89.0f);
  state->camera_euler.x = Clamp(-limit, state->camera_euler.x, limit);
  
  state->camera_view_dirty = true;
}

#include "test.h"
void application_init(App* app) {
  test();
  Scratch scratch;
  Arena* arena = arena_alloc(scratch, 16);
  push_buffer(arena, u8, 128);
  
  app->state = push_struct(app->arena, GameState);
  Assign(state, app->state);
  state->arena = arena_alloc(app->arena, GameSize);
  
  state->camera_position = v3(0,0,30.0f);
  state->camera_euler = v3_zero();
  
  state->camera_view_dirty = true;
  
  u8* buff = push_buffer(scratch, u8, KB(1));
  FreeList fl = free_list_create(scratch, KB(1));
  
  void* data1 = free_list_alloc(fl, 1, 8);
  // void* data2 = free_list_alloc(fl, 1, 8);
  // void* data3 = free_list_alloc(fl, 1, 8);
  
  // free_list_free(&fl, data1);
  // free_list_free(&fl, data2);
  // free_list_free(&fl, data3);
  Pool p = pool_create(scratch, 100, 8);
  u8* data = pool_alloc(p);
  u8* new_data = free_list_alloc(fl, 100);
  
  
  
}

void application_update(App* app) {
  Assign(state, app->state);
  f32 delta_time = app->delta_time;
  // TODO temp
  if (input_is_key_up(KEY_T) && input_was_key_down(KEY_T)) {
    Debug("Swapping texture!");
    EventContext context = {};
    event_fire(EVENT_CODE_DEBUG0, app, context);
  }
  
  f32 rotation_speed = 2.0f;

  // HACK temp back to move camera around
  if (input_is_key_down(KEY_A) || input_is_key_down(KEY_LEFT)) {
    camera_yaw(rotation_speed * app->delta_time);
  }
  if (input_is_key_down(KEY_D) || input_is_key_down(KEY_RIGHT)) {
    camera_yaw(-rotation_speed * app->delta_time);
  }
  if (input_is_key_down(KEY_UP)) {
    camera_pitch(rotation_speed * app->delta_time);
  }
  if (input_is_key_down(KEY_DOWN)) {
    camera_pitch(-rotation_speed * app->delta_time);
  }
  
  f32 temp_move_speed = 50.0f;
  v3 velocity = v3_zero();
  
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
    state->camera_position += velocity * temp_move_speed * delta_time;
    state->camera_view_dirty = true;
  }
  
  recalculate_view_matrix();

  // HACK This should not be available outside the engine
  r_set_view(state->view);
}

void application_render(struct App* app) {
}

void application_on_resize(struct App* app) {
}
