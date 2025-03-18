#include "engine.h"

#include <core/logger.h>
#include <core/memory.h>
#include <platform/platform.h>

#include "event.h"
#include "input.h"

struct EngineState {
  struct Arena* main_arena;
  struct Arena* permanent_arena;
  struct Arena* transient_arena;
  b8 is_initialized;
  b8 is_used;
  EngineSystemStates systems;
  Window* window;
};

b8 test_event(u16 code, void* sender, void* listener_inst, EventContext context) {
  Info("event from engine");
  Info("event from engine");
  Info("event from engine");
  
  return true;
}

void test_game_function(void(*function)()) {
  Info("hello");
  Info("hello");
  function();
}

internal void engine_on_process_key(Keys key, b8 pressed);
  
b8 application_on_key(u16 code, void* sender, void* listener_inst, EventContext context);

b8 engine_create(void* memory) {
  EngineState* engine_state = (EngineState*)memory;
  
  engine_state->permanent_arena = arena_alloc(engine_state->main_arena, MB(40));
  engine_state->transient_arena = arena_alloc(engine_state->main_arena, MB(250));

  EngineSystemStates* systems = &engine_state->systems;
  
  // Platform system
  {
    u64* mem_required = &systems->platform_system_memory_requirement;
    platform_system_startup(mem_required, 0);
    systems->platform_system = push_buffer(engine_state->permanent_arena, PlatformState, *mem_required);
    if (!platform_system_startup(mem_required, systems->platform_system)) {
      Error("Failed to initialize platform layer");
      return false;
    }
  }
  
  // Event system
  {
    u64* mem_required = &systems->event_system_memory_requirement;
    event_initialize(mem_required, 0);
    systems->event_system = push_buffer(engine_state->permanent_arena, EventState, *mem_required);
    if (!event_initialize(mem_required, systems->event_system)) {
      Error("Failed to initialize event system");
      return false;
    }
    platform_register_process_key(engine_on_process_key);
    event_register(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
  }
  
  // Input system
  {
    u64* mem_required = &systems->input_system_memory_requirement;
    input_initialize(mem_required, 0);
    systems->input_system = push_buffer(engine_state->permanent_arena, InputState, *mem_required);
    if (!input_initialize(mem_required, systems->input_system)) {
      Error("Failed to initialize input system");
      return false;
    }
  }
  
  // Window creation
  {
    WindowConfig config = {
      .name = cstr8("Engine.exe"),
      .position_x = 100,
      .position_y = 100,
      .width = 1280,
      .height = 720
    };
    window_create(engine_state->window, config);
  }
  
  engine_state->is_initialized = true;
  Trace("Engine creation");
  return true;
}

b8 engine_restore_state(void* memory) {
  EngineState* engine_state = (EngineState*)memory;
  
  EngineSystemStates systems = engine_state->systems;

  event_restore_state(systems.event_system);

  return true;
}

internal void engine_on_process_key(Keys key, b8 pressed) {
  input_process_key(key, pressed);
}

b8 application_on_key(u16 code, void* sender, void* listener_inst, EventContext context) {
  if (code == EVENT_CODE_KEY_PRESSED) {
    u16 key_code = context.data.u16[0];
    if (key_code == KEY_ESCAPE) {
      // NOTE: Technically firing an event to itself, but there may be other listeners.
      EventContext data = {};
      event_fire(EVENT_CODE_APPLICATION_QUIT, 0, data);

      // Block anything else from processing this.
      return true;
    } else if (key_code == KEY_A) {
      // Example on checking for a key
      Debug("Explicit - A key pressed!");
    } else {
      Debug("'%c' key pressed in window.", key_code);
    }
  } else if (code == EVENT_CODE_KEY_RELEASED) {
    u16 key_code = context.data.u16[0];
    if (key_code == KEY_B) {
      // Example on checking for a key
      Debug("Explicit - B key release!");
    } else {
      Debug("'%c' key released in window.", key_code);
    }
  }
  return false;
}
