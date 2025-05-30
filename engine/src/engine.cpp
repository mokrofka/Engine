#include "engine.h"
#include "render/r_frontend.h"


#include "sys/texture.h"
#include "sys/material_sys.h"
#include "sys/geometry.h"
#include "sys/res_sys.h"
#include "sys/shader_sys.h"

#include "asset_watch.h"
#include "event.h"
#include "input.h"
#include "ui.h"
#include "network.h"
#include "test.h"
#include "ecs.h"

struct EngineState {
  Arena* arena;
  App* app;
  b8 is_running;
  b8 is_suspended;

  Clock clock;
  f32 last_time;
};

global EngineState st;

internal void engine_on_window_closed();
internal void engine_on_window_resized(Window* window);

internal b32 app_on_event(u32 code, void* sender, void* listener_inst, EventContext context);
internal b32 app_on_key(u32 code, void* sender, void* listener_inst, EventContext context);
internal b32 app_on_resized(u32 code, void* sender, void* listener_inst, EventContext context);

internal void check_dll_changes(App* app);
internal void app_create(App* app);
internal void load_game_lib_init(App* app);

f32 delta_time;
void engine_create(App* app) {
  global_allocator_init();
  Assign(app->arena, mem_alloc(AppSize));
  app->arena->size = AppSize;
  tctx_init(app->arena);
  
  app_create(app);
  Scratch scratch;
  
  st.app = app;
  st.arena = arena_alloc(app->arena, EngineSize);
  
  {
    logging_init(st.arena);
  }

  {
    platform_init(st.arena);
  }
  
  {
    network_init(st.arena);
  }
  // test();
  component_register(Some);
  
  {
    ResSysConfig res_sys_cfg = {
      .asset_base_path = "../assets"_
    };
    res_sys_init(st.arena, res_sys_cfg);
  }

  {
    EventSysConfig config = {
      .mem_reserve = KB(1)
    };
    event_init(st.arena, config);
    
    os_register_process_key(input_process_key);
    os_register_process_mouse_move(input_process_mouse_move);
    os_register_window_closed_callback(engine_on_window_closed);
    os_register_window_resized_callback(engine_on_window_resized);
    
    event_register(EventCode_ApplicationQuit, 0, app_on_event);
    event_register(EventCode_KeyPressed, 0, app_on_key);
    event_register(EventCode_KeyReleased, 0, app_on_key);
    event_register(EventCode_Resized, 0, app_on_resized);
  }

  {
    input_init(st.arena);
  }
  
  {
    WindowConfig config = {
      .position_x = 100,
      .position_y = 100,
      .width = 800,
      .height = 600,
      .name = app->name};
    os_window_create(st.arena, config);
  }

  {
    asset_watch_init(st.arena);
  }
  
  {
    ShaderSysConfig config = {
      .mem_reserve = MB(1),
      .shader_count_max = 1024,
      .uniform_count_max = 128,
      .global_textures_max = 31,
      .instance_textures_max = 31
    };
    shader_sys_init(st.arena, config);
  }

  {
    R_Config config = {
      .mem_reserve = MB(10)
    };
    r_init(st.arena, config);
  }
  
  {
    ui_init();
  }

  {
    GeometrySysConfig geometry_sys_config = {
      .max_geometry_count = 4096,
    };
    geometry_sys_init(st.arena, geometry_sys_config);
  }

  app->init(app);
}

void engine_run(App* app) {
  os_show_window();
  st.is_running = true;
  
  clock_start(&st.clock);
  clock_update(&st.clock);
  st.last_time = st.clock.elapsed;
  f64 running_time = 0;
  u8 frame_count = 0;
  f64 target_frame_seconds = 1.0f / 60;
  
  while (st.is_running) {
    
    os_pump_messages();

    if (!st.is_suspended) {
      clock_update(&st.clock);
      f64 current_time = st.clock.elapsed;
      delta_time = (current_time - st.last_time);
      f64 frame_start_time = os_now_seconds();

      check_dll_changes(app);

      r_begin_draw_frame();

      st.app->update(st.app);
      
      r_end_draw_frame();
      
      f64 frame_end_time = os_now_seconds();
      f64 frame_elapsed_time = frame_end_time - frame_start_time;
      running_time += frame_elapsed_time;
      f64 remaining_seconds = target_frame_seconds - frame_elapsed_time;

      // if (remaining_seconds > 0) {
      //   u32 remaining_ms = (remaining_seconds * 1000);

      //   // If there is time left, give it back to the OS.
      //   b32 limit_frames = true;
      //   if (remaining_ms > 0 && limit_frames) {
      //     os_sleep(remaining_ms - 1);
      //   }

      //   ++frame_count;
      // }

      input_update();
      asset_watch_update();

      st.last_time = current_time;
    }
  }
  
  st.is_running = false;

  event_unregister(EventCode_ApplicationQuit, 0, app_on_event);
  event_unregister(EventCode_KeyPressed, 0, app_on_key);
  event_unregister(EventCode_KeyReleased, 0, app_on_key);
  event_unregister(EventCode_Resized, 0, app_on_resized);

  // geometry_sys_shutdown();
  // material_system_shutdown(); texture_system_shutdown();
  ui_shutdown();
  r_shutdown();
  // res_sys_shutdown();
  os_window_destroy();
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
    EventContext context = {0};
    context.data.u16[0] = window->width;
    context.data.u16[1] = window->height;
    event_fire(EventCode_Resized, window, context);
  }
}

internal b32 app_on_event(u32 code, void* sender, void* listener_inst, EventContext context) {
  switch (code) {
    case EventCode_ApplicationQuit: {
      Info("EVENT_CORE_APPLICATION_QUIT received, shutting down.\n");
      st.is_running = false;
      return true;
    }
  }
  
  return false;
}

internal b32 app_on_key(u32 code, void* sender, void* listener_inst, EventContext context) {
  if (code == EventCode_KeyPressed) {
    u16 key_code = context.data.u16[0];
    if (key_code == KEY_ESCAPE) {
      // NOTE: Technically firing an event to itself, but there may be other listeners.
      EventContext data = {};
      event_fire(EventCode_ApplicationQuit, 0, data);

      // Block anything else from processing this.
      return true;
    } else {
      Debug("'%c' key pressed in window.", key_code);
    }
  } else if (code == EventCode_KeyReleased) {
    u16 key_code = context.data.u16[0];
    Debug("'%c' key released in window.", key_code);
  }
  return false;
}

internal b32 app_on_resized(u32 code, void* sender, void* listener_inst, EventContext context) {
  u32 width = context.data.u16[0];
  u32 height = context.data.u16[1];
  Debug("Window resize: %i, %i", width, height);

  r_on_resized(width, height);
  return true;
}

internal void load_game_lib(App* app) {
  FileProperties props = os_properties_from_file_path(app->lib_file_path);
  app->modified = props.modified;
  os_copy_file_path(app->lib_temp_file_path, app->lib_file_path);
  app->lib = os_lib_open(app->lib_temp_file_path);
  GetProcAddr(app->update, app->lib, "app_update"_);
  GetProcAddr(app->on_resize, app->lib, "app_on_resize"_);
}

internal void check_dll_changes(App* app) {
  FileProperties props = os_properties_from_file_path(app->lib_file_path);
  u64 new_write_time = props.modified;
  b32 game_modified = os_file_compare_time(new_write_time, app->modified);

  if (game_modified) {
    os_lib_close(app->lib);
    load_game_lib(app);
  }
}

internal void load_game_lib_init(App* app) {
  FileProperties file_props = os_properties_from_file_path(app->lib_file_path);

  app->modified = file_props.modified;
  os_copy_file_path(app->lib_temp_file_path, app->lib_file_path);
  
  app->lib = os_lib_open("game_temp.dll"_);
  GetProcAddr(app->update, app->lib, "app_update"_);
  GetProcAddr(app->init, app->lib, "app_init"_);
  GetProcAddr(app->on_resize, app->lib, "app_on_resize"_);
  
  Assert(app->lib && app->init && app->update && app->on_resize);
}

internal void app_create(App* app) {
  Scratch scratch;
  
  String filepath = os_exe_filename(scratch);
  app->file_path = push_str_copy(app->arena, filepath);
  app->name = str_skip_last_slash(app->file_path);
  
  String file_directory = str_chop_after_last_slash(app->file_path);
  app->lib_file_path = push_str_cat(app->arena, file_directory, "game.dll"_);
  app->lib_temp_file_path = push_str_cat(app->arena, file_directory, "game_temp.dll"_);

  load_game_lib_init(app);
}
