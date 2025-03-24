#include "game.h"

#include <application_type.h>
#include <engine.h>
#include <event.h>

#include <platform/platform.h>
#include <logger.h>
#include <memory.h>

struct MyData {
  i32 x,y,z; 
};

struct GameState {
  struct Arena* arena;

  MyData* data;
};

b8 application_initialize(Application* game_inst) {
  game_inst->state = push_struct(game_inst->arena, GameState);
  GameState* state = (GameState*)game_inst->state;
  state->arena = arena_alloc(game_inst->arena, MB(400));

  state->data = push_struct(state->arena, MyData);

  // event_register(EVENT_CODE_KEY_PRESSED, i, test_event);
  return true;
}
  
#include <math/maths.h>

b8 application_update(Application* game_inst) {

  GameState* game_state = (GameState*)game_inst->state;

  MyData* data = game_state->data;
  
  v2 vec(1,2);
  // vec = vec * 2;
  vec = 2*vec;

  // Info("%f %f", vec.x,vec.y);
  
  return true;
}
