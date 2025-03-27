#include "game.h"

#include <application_type.h>

// HACK This should not be available outside the engine
#include <renderer/renderer_frontend.h>

#include <logger.h>
#include <memory.h>
#include "input.h"
#include "math/maths.h"

struct GameState {
  struct Arena* arena;
  
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
    
    state->view = rotaion * translation;
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

b8 application_initialize(Application* game_inst) {
  game_inst->state = push_struct(game_inst->arena, GameState);
  state = (GameState*)game_inst->state;
  state->arena = arena_alloc(game_inst->arena, MB(400));
  
  state->camera_position = v3(0,0,-30.0f); state->camera_euler = v3_zero();
  
  // state->view = mat4_translation(v3(0, 0, 30.0f));
  state->camera_view_dirty = true;
  
  return true;
}

b8 application_update(Application* game_inst) {
  state = (GameState*)game_inst->state;
  
  // HACK temp back to move camera around
  if (input_is_key_down(KEY_A) || input_is_key_down(KEY_LEFT)) {
    camera_yaw(1.0f * game_inst->delta_time);
  }
  if (input_is_key_down(KEY_D) || input_is_key_down(KEY_RIGHT)) {
    camera_yaw(-1.0f * game_inst->delta_time);
  }
  
  recalculate_view_matrix();

  // HACK This should not be available outside the engine
  renderer_set_view(state->view);

  return true;
}

b8 application_render(struct Application* game_inst) {
  return true;
}

void application_on_resize(struct Application* game_inst) {

}
