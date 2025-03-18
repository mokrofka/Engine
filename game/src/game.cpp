#include "game.h"

#include <platform/platform.h>
#include <core/logger.h>
#include <core/memory.h>

#include <engine.h>
#include <event.h>

void game_initialize(void* memory) {
  GameState* game_state = (GameState*)memory;
  
  if (!game_state->is_initialized) {
    game_state->permanent_arena = arena_alloc(game_state->main_arena, MB(40));
    game_state->transient_arena = arena_alloc(game_state->main_arena, MB(200));
    game_state->data = push_array(game_state->permanent_arena, MyData, 1);
    game_state->is_initialized = true;
  }
}

b8 test_event(u16 code, void* sender, void* listener_inst, EventContext context) {
  Info("I'm an event");
  // Info("I'm an event");
  // Info("I'm an event");
  // Info("I'm an event");
  // Info("I'm an event");
  return true;
}

i32* i = (i32*)1; 
void print() {
  Info("it's function");
}
  
void game_update(void* memory) {

  GameState* game_state = (GameState*)memory;
  
  if (!game_state->is_stated) {
    
    engine_restore_state(game_state->engine_memory);
    game_state->is_stated = true;
    event_register(EVENT_CODE_KEY_PRESSED, i, test_event);
  }
  test_game_function(print);
  EventContext context = {};
  event_fire(EVENT_CODE_KEY_PRESSED, 0, context);

  MyData* data = game_state->data;
  // Trace("my values are %i %i %i", data->x-=1,data->y+=1000,data->z++);
  
  platform_sleep(100);
}
