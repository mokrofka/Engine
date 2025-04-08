#include "engine.h"

#include "app_types.h"
#include "render/r_frontend.h"

#include "systems/texture_system.h"

#include <logger.h>
#include <memory.h>
#include <os.h>
#include <event.h>
#include <input.h>

struct EngineState {
  Arena* arena;
  Application* game_inst;
  b8 is_running;
  b8 is_suspended;

  Clock clock;
  f64 last_time;
};

global EngineState* state;

internal void engine_on_process_key(Keys key, b8 pressed);
internal void engine_on_window_closed();
internal void engine_on_window_resized(Window* window);

internal b8 app_on_event(u16 code, void* sender, void* listener_inst, EventContext context);
internal b8 app_on_key(u16 code, void* sender, void* listener_inst, EventContext context);
internal b8 app_on_resized(u16 code, void* sender, void* listener_inst, EventContext context);

internal void check_dll_changes(Application* app);

b8 engine_create(Application* game_inst) {
  game_inst->engine_state = push_struct(game_inst->arena, EngineState);
  state = (EngineState*)game_inst->engine_state;
  state->game_inst = game_inst;
  
  state->arena = arena_alloc(game_inst->arena, MB(400));
  
  platform_init(state->arena);

  logging_init(state->arena);

  event_init(state->arena);
  os_register_process_key(input_process_key);
  
  os_register_window_closed_callback(engine_on_window_closed);
  os_register_window_resized_callback(engine_on_window_resized);

  event_register(EVENT_CODE_APPLICATION_QUIT, 0, app_on_event);
  event_register(EVENT_CODE_KEY_PRESSED, 0, app_on_key);
  event_register(EVENT_CODE_KEY_RELEASED, 0, app_on_key);
  event_register(EVENT_CODE_RESIZED, 0, app_on_resized);

  input_init(state->arena);

  {
    WindowConfig config = {
        .position_x = 100,
        .position_y = 100,
        .width = 680,
        .height = 480,
        .name = game_inst->name};
    os_window_create(config);
  }

  r_init(state->arena);

  TextureSystemConfig texture_sys_config = {
      .max_texture_count = 65536,
  };
  texture_system_init(state->arena, texture_sys_config);

  game_inst->init(game_inst);

  return true;
}

b8 engine_run(Application* game_inst) {
  state->is_running = true;
  
  clock_start(&state->clock);
  clock_update(&state->clock);
  state->last_time = state->clock.elapsed;
  f64 running_time = 0;
  u8 frame_count = 0;
  f64 target_frame_seconds = 1.0f / 60;
  
  while (state->is_running) {
    
    os_pump_messages();

    if (!state->is_suspended) {
      clock_update(&state->clock);
      f64 current_time = state->clock.elapsed;
      f64 delta = (current_time - state->last_time);
      state->game_inst->delta_time = delta;
      f64 frame_start_time = os_now_seconds();

      check_dll_changes(game_inst);
      state->game_inst->update(state->game_inst);

      R_Packet packet;
      packet.delta_time = delta;

      f64 frame_end_time = os_now_seconds();
      f64 frame_elapsed_time = frame_end_time - frame_start_time;
      r_draw_frame(&packet);
      running_time += frame_elapsed_time;
      f64 remaining_seconds = target_frame_seconds - frame_elapsed_time;

      if (remaining_seconds > 0) {
        u64 remaining_ms = (remaining_seconds * 1000);

        // If there is time left, give it back to the OS.
        b8 limit_frames = true;
        if (remaining_ms > 0 && limit_frames) {
          os_sleep(remaining_ms - 1);
        }

        ++frame_count;
      }

      input_update();

      state->last_time = current_time;
    }
  }
  
  state->is_running = false;

  texture_system_shutdown();
  r_shutdown();
  os_window_destroy();

  event_unregister(EVENT_CODE_APPLICATION_QUIT, 0, app_on_event);
  event_unregister(EVENT_CODE_KEY_PRESSED, 0, app_on_key);
  event_unregister(EVENT_CODE_KEY_RELEASED, 0, app_on_key);
  event_unregister(EVENT_CODE_RESIZED, 0, app_on_resized);
  return true;
}

internal void engine_on_window_closed() {
  event_fire(EVENT_CODE_APPLICATION_QUIT, 0, (EventContext){});
}

internal void engine_on_window_resized(Window* window) {
  // Handle minimization
  if (window->width == 0 || window->height == 0) {
    Info("Window minimized, suspending application.");
    state->is_suspended = true;
  } else {
    if (state->is_suspended) {
      Info("Window restored, resuming application.");
      state->is_suspended = false;
    }

    // Fire an event for anything listening for window resizes.
    EventContext context = {0};
    context.data.u16[0] = window->width;
    context.data.u16[1] = window->height;
    event_fire(EVENT_CODE_RESIZED, (Window*)window, context);
  }
}

internal b8 app_on_event(u16 code, void* sender, void* listener_inst, EventContext context) {
  switch (code) {
    case EVENT_CODE_APPLICATION_QUIT: {
      Info("EVENT_CORE_APPLICATION_QUIT received, shutting down.\n");
      state->is_running = false;
      return true;
    }
  }
  
  return false;
}

internal b8 app_on_key(u16 code, void* sender, void* listener_inst, EventContext context) {
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

internal b8 app_on_resized(u16 code, void* sender, void* listener_inst, EventContext context) {
  u16 width = context.data.u16[0];
  u16 height = context.data.u16[1];
  Debug("Window resize: %i, %i", width, height);

  r_on_resized(width, height);
  return true;
}

internal void load_game_lib(Application* app) {
  app->game_lib.last_time_write = os_file_last_write_time(app->game_lib.src_full_filename);
  os_file_copy(app->game_lib.src_full_filename, app->game_lib.temp_full_filename);
  // os_library_load(app->game_lib.temp_full_filename, &app->game_lib);
  app->game_lib.handle = os_library_load(app->game_lib.temp_full_filename);
  app->update = (void(*)(Application*))os_library_load_function(str_lit("application_update"), app->game_lib);
}

internal void unload_game_lib(Application* app) {
  os_library_unload(app->game_lib);
  app->update = 0;
}

internal void check_dll_changes(Application* app) {
  u64 new_game_dll_write_time = os_file_last_write_time(app->game_lib.src_full_filename);
  b8 game_write = os_file_compare_time(new_game_dll_write_time, app->game_lib.last_time_write);

  if (game_write) {
    unload_game_lib(app);
    load_game_lib(app);
  }
}

