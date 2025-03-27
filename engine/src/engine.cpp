#include "engine.h"

#include "application_type.h"
#include "renderer/renderer_frontend.h"

#include <logger.h>
#include <memory.h>
#include <platform/platform.h>
#include <clock.h>
#include "event.h"
#include "input.h"

struct EngineSystemStates {
  u64 event_system_memory_requirement;
  struct EventState* event_system;
  
  u64 input_system_memory_requirement;
  struct InputState* input_system;
  
  u64 platform_system_memory_requirement;
  struct PlatformState* platform_system;
  
  u64 renderer_system_memory_requirement;
  struct RendererState* renderer_system;
  
  u64 logging_system_memory_requirement;
  struct LoggingState* logging_system;
};

struct EngineState {
  Application* game_inst;
  Arena* arena;
  b8 is_running;
  b8 is_suspended;
  EngineSystemStates systems;
  Window* window;

  Clock clock;
  f64 last_time;
};

internal EngineState* engine_state;

b8 test_event(u16 code, void* sender, void* listener_inst, EventContext context) {
  Info("event from engine");
  Info("event from engine");
  Info("event from engine");
  
  return true;
}

internal void engine_on_process_key(Keys key, b8 pressed);
internal void engine_on_window_closed();
internal void engine_on_window_resized(Window* window);

b8 application_on_event(u16 code, void* sender, void* listener_inst, EventContext context);
b8 application_on_key(u16 code, void* sender, void* listener_inst, EventContext context);
b8 application_on_resized(u16 code, void* sender, void* listener_inst, EventContext context);

internal void check_dll_changes(Application* app);

b8 engine_create(Application* game_inst) {
  game_inst->engine_state = push_struct(game_inst->arena, EngineState);
  engine_state = (EngineState*)game_inst->engine_state;
  engine_state->game_inst = game_inst;
  
  engine_state->arena = arena_alloc(game_inst->arena, MB(400));

  EngineSystemStates* systems = &engine_state->systems;
  
  // Platform system
  {
    u64* mem_required = &systems->platform_system_memory_requirement;
    platform_system_startup(mem_required, 0);
    systems->platform_system = push_buffer(engine_state->arena, PlatformState, *mem_required);
    if (!platform_system_startup(mem_required, systems->platform_system)) {
      Error("Failed to initialize platform layer");
      return false;
    }
  }
  
  // Logging system
  {
    u64* mem_required = &systems->logging_system_memory_requirement;
    logging_initialize(mem_required, 0);
    systems->logging_system = push_buffer(engine_state->arena, LoggingState, *mem_required);
    if (!logging_initialize(mem_required, systems->logging_system)) {
      Error("Failed to initialize platform layer");
      return false;
    }
  }
  
  // Event system
  {
    u64* mem_required = &systems->event_system_memory_requirement;
    event_initialize(mem_required, 0);
    systems->event_system = push_buffer(engine_state->arena, EventState, *mem_required);
    if (!event_initialize(mem_required, systems->event_system)) {
      Error("Failed to initialize event system");
      return false;
    }
    platform_register_process_key(engine_on_process_key);
    platform_register_window_closed_callback(engine_on_window_closed);
    platform_register_window_resized_callback(engine_on_window_resized);
    
    event_register(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
    event_register(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
    event_register(EVENT_CODE_KEY_RELEASED, 0, application_on_key);
    event_register(EVENT_CODE_RESIZED, 0, application_on_resized);
  }
  
  // Input system
  {
    u64* mem_required = &systems->input_system_memory_requirement;
    input_initialize(mem_required, 0);
    systems->input_system = push_buffer(engine_state->arena, InputState, *mem_required);
    if (!input_initialize(mem_required, systems->input_system)) {
      Error("Failed to initialize input system");
      return false;
    }
  }
  
  // Window creation
  {
    WindowConfig config = {
      .position_x = 100,
      .position_y = 100,
      .width = 1280,
      .height = 720,
      .name = game_inst->name
    };
    window_create(engine_state->window, config);
  }
  
  // Renderer startup
  {
    u64* mem_required = &systems->renderer_system_memory_requirement;
    renderer_initialize(mem_required, 0);
    systems->renderer_system = push_buffer(engine_state->arena, RendererState, *mem_required);
    if (!renderer_initialize(mem_required, systems->renderer_system)) {
      Error("Failed to initialize renderer. Aborting application.");
      return false;
    }
  }
  

  if (!engine_state->game_inst->initialize(engine_state->game_inst)) {
    Fatal("Game failed to initialize.");
    return false;
  }

  return true;
}

b8 engine_run(Application* game_inst) {
  engine_state->is_running = true;
  
  clock_start(&engine_state->clock);
  clock_update(&engine_state->clock);
  engine_state->last_time = engine_state->clock.elapsed;
  f64 running_time = 0;
  u8 frame_count = 0;
  f64 target_frame_seconds = 1.0f / 60;
  
  while (engine_state->is_running) {
    
    if (!platform_pump_messages()) {
      engine_state->is_running = false;
    }

    if (!engine_state->is_suspended) {
      clock_update(&engine_state->clock);
      f64 current_time = engine_state->clock.elapsed;
      f64 delta = (current_time - engine_state->last_time);
      engine_state->game_inst->delta_time = delta;
      f64 frame_start_time = platform_get_absolute_time();

      check_dll_changes(game_inst);
      if (!engine_state->game_inst->update(engine_state->game_inst)) {
        Fatal("Game update failed, shutting down.");
        engine_state->is_running = false;
        break;
      }

      RenderPacket packet;
      packet.delta_time = delta;
      renderer_draw_frame(&packet);

      f64 frame_end_time = platform_get_absolute_time();
      f64 frame_elapsed_time = frame_end_time - frame_start_time;
      running_time += frame_elapsed_time;
      f64 remaining_seconds = target_frame_seconds - frame_elapsed_time;

      if (remaining_seconds > 0) {
        u64 remaining_ms = (remaining_seconds * 1000);

        // If there is time left, give it back to the OS.
        b8 limit_frames = true;
        if (remaining_ms > 0 && limit_frames) {
          platform_sleep(remaining_ms - 1);
        }

        ++frame_count;
      }

      input_update();

      engine_state->last_time = current_time;
    }
  }
  
  engine_state->is_running = false;

  renderer_shutdown();
  platform_window_destroy(engine_state->window);

  event_unregister(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
  event_unregister(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
  event_unregister(EVENT_CODE_KEY_RELEASED, 0, application_on_key);
  event_unregister(EVENT_CODE_RESIZED, 0, application_on_resized);
  return true;
}

internal void engine_on_process_key(Keys key, b8 pressed) {
  input_process_key(key, pressed);
}

internal void engine_on_window_closed() {
  event_fire(EVENT_CODE_APPLICATION_QUIT, 0, (EventContext){});
}

internal void engine_on_window_resized(Window* window) {
  // Handle minimization
  if (window->width == 0 || window->height == 0) {
    Info("Window minimized, suspending application.");
    engine_state->is_suspended = true;
  } else {
    if (engine_state->is_suspended) {
      Info("Window restored, resuming application.");
      engine_state->is_suspended = false;
    }

    // Fire an event for anything listening for window resizes.
    EventContext context = {0};
    context.data.u16[0] = window->width;
    context.data.u16[1] = window->height;
    event_fire(EVENT_CODE_RESIZED, (Window*)window, context);
  }
}

b8 application_on_event(u16 code, void* sender, void* listener_inst, EventContext context) {
  switch (code) {
    case EVENT_CODE_APPLICATION_QUIT: {
      Info("EVENT_CORE_APPLICATION_QUIT received, shutting down.\n");
      engine_state->is_running = false;
      return true;
    }
  }
  
  return false;
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
    } else {
      Debug("'%c' key pressed in window.", key_code);
    }
  } else if (code == EVENT_CODE_KEY_RELEASED) {
    u16 key_code = context.data.u16[0];
    Debug("'%c' key released in window.", key_code);
  }
  return false;
}

b8 application_on_resized(u16 code, void* sender, void* listener_inst, EventContext context) {
  u16 width = context.data.u16[0];
  u16 height = context.data.u16[1];
  Debug("Window resize: %i, %i", width, height);

  renderer_on_resized(width, height);
  return true;
}

void load_game_lib(Application* app) {
  app->game_lib.dll_last_time_write = platform_get_last_write_time(app->game_lib.src_full_filename);
  platform_copy_file(app->game_lib.src_full_filename, app->game_lib.temp_full_filename);
  platform_dynamic_library_load(app->game_lib.temp_full_filename, &app->game_lib);
  app->update = (b8(*)(Application*))platform_dynamic_library_load_function("application_update", &app->game_lib);
}

void unload_game_lib(Application* app) {
  platform_dynamic_library_unload(&app->game_lib);
  app->update = 0;
}

internal void check_dll_changes(Application* app) {
  u64 new_game_dll_write_time = platform_get_last_write_time(app->game_lib.src_full_filename);
  b8 game_write = platform_compare_file_time(new_game_dll_write_time, app->game_lib.dll_last_time_write);

  if (game_write) {
    unload_game_lib(app);
    load_game_lib(app);
  }
}

