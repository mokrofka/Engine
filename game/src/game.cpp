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

i32* i = (i32*)1; 

void print() {
  Info("it's function");
}

b8 test_event(u16 code, void* sender, void* listener_inst, EventContext context) {
  Info("I'm an event");
  return true;
}

b8 application_initialize(Application* game_inst) {
  game_inst->state = push_struct(game_inst->arena, GameState);
  GameState* state = (GameState*)game_inst->state;
  state->arena = arena_alloc(game_inst->arena, MB(400));

  state->data = push_struct(state->arena, MyData);

  // event_register(EVENT_CODE_KEY_PRESSED, i, test_event);
  return true;
}
  
b8 application_update(Application* game_inst) {

  GameState* game_state = (GameState*)game_inst->state;
  
  // test_game_function(print);
  // EventContext context = {};
  // event_fire(EVENT_CODE_KEY_PRESSED, 0, context);

  MyData* data = game_state->data;
  // Trace("my values are %i %i %i", data->x-=1,data->y+=1000,data->z++);
  // Info("hello");
  
  return true;
}
