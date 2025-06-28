#include "game.h"
#include "app.h"

#include "render/r_frontend.h"

#include "asset_watch.h"
#include "test.h"

App st;

internal void engine_on_window_closed();
internal void engine_on_window_resized(Window* window);
internal b32 app_on_event(u32 code, void* sender, void* listener_inst, EventContext context);
internal b32 app_on_key(u32 code, void* sender, void* listener_inst, EventContext context);
internal b32 app_on_resized(u32 code, void* sender, void* listener_inst, EventContext context);

Main entry_point() {
  os_init();
  st.is_running = true;

  WindowConfig config = {
    .position_x = 100,
    .position_y = 100,
    .width = 1000,
    .height = 600,
  };
  os_window_create(config);
  r_init();
  Scratch scratch;

#ifdef MONOLITHIC_BUILD
  st.init = app_init;
  st.update = app_update;
#else
  String current_dir = os_get_current_directory();
  st.lib_filepath = push_str_cat(scratch, current_dir, "/game.dll");
  st.lib_temp_filepath = push_str_cat(scratch, current_dir, "/game_temp.dll");
  
  os_copy_file_path(st.lib_temp_filepath, st.lib_filepath);
  st.lib = os_lib_open("game_temp.dll");
  Assign(st.init, os_lib_get_proc(st.lib, "app_init"));
  Assign(st.update, os_lib_get_proc(st.lib, "app_update"));
  
  Assert(st.lib && st.init && st.update);
  asset_watch_add(st.lib_filepath, []() {
    os_lib_close(st.lib);
    os_copy_file_path(st.lib_temp_filepath, st.lib_filepath);
    st.lib = os_lib_open(st.lib_temp_filepath);
    Assign(st.update, os_lib_get_proc(st.lib, "app_update"));
  });
#endif

  os_register_process_key(input_process_key);
  os_register_process_mouse_move(input_process_mouse_move);
  os_register_window_closed_callback(engine_on_window_closed);
  os_register_window_resized_callback(engine_on_window_resized);
  
  event_register(EventCode_ApplicationQuit, 0, app_on_event);
  event_register(EventCode_KeyPressed, 0, app_on_key);
  event_register(EventCode_KeyReleased, 0, app_on_key);
  event_register(EventCode_Resized, 0, app_on_resized);

  st.init(&st.state);

  os_show_window();
  
  f32 last_time = 0;
  while (st.is_running) {
    os_pump_messages();

    if (!st.is_suspended) {
      f32 current_time = os_now_seconds();
      delta_time = current_time - last_time;
      last_time = current_time;

      r_begin_draw_frame();
      st.update(st.state);
      r_end_draw_frame();

      input_update();
      asset_watch_update();
    }
  }
  
}

internal void engine_on_window_closed() {
  event_fire(EventCode_ApplicationQuit, 0, (EventContext){});
}

internal void engine_on_window_resized(Window* window) {
  // Handle minimization
  if (window->width == 0 || window->height == 0) {
    Info("Window minimized, suspending application.");
    st.is_suspended = true;
  } else {
    if (st.is_suspended) {
      Info("Window restored, resuming application.");
      st.is_suspended = false;
    }

    // Fire an event for anything listening for window resizes.
    EventContext context = {
      .u16 = {(u16)window->width, (u16)window->height}
    };
    event_fire(EventCode_Resized, window, context);
  }
}

internal b32 app_on_event(u32 code, void* sender, void* listener_inst, EventContext context) {
  Info("EVENT_CORE_APPLICATION_QUIT received, shutting down.\n");
  st.is_running = false;
  return true;
}

internal b32 app_on_key(u32 code, void* sender, void* listener_inst, EventContext context) {
  if (code == EventCode_KeyPressed) {
    u16 key_code = context.u16[0];
    if (key_code == Key_Escape) {
      // NOTE: Technically firing an event to itself, but there may be other listeners.
      EventContext data = {};
      event_fire(EventCode_ApplicationQuit, 0, data);

      // Block anything else from processing this.
      return true;
    } else {
      Debug("'%c' key pressed in window.", key_code);
    }
  } else if (code == EventCode_KeyReleased) {
    u16 key_code = context.u16[0];
    Debug("'%c' key released in window.", key_code);
  }
  return false;
}

internal b32 app_on_resized(u32 code, void* sender, void* listener_inst, EventContext context) {
  u32 width = context.u16[0];
  u32 height = context.u16[1];
  Debug("Window resize: %i, %i", width, height);

  r_on_resized(width, height);
  return true;
}
