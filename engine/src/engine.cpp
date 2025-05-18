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

void test();

struct EngineState {
  Arena* arena;
  App* app;
  b8 is_running;
  b8 is_suspended;

  Clock clock;
  f32 last_time;
  
  // TODO temp
  Geometry* test_geometry;
  Geometry* test_ui_geometry;
  // end
};

global EngineState* state;

internal void engine_on_window_closed();
internal void engine_on_window_resized(Window* window);

internal b32 app_on_event(u32 code, void* sender, void* listener_inst, EventContext context);
internal b32 app_on_key(u32 code, void* sender, void* listener_inst, EventContext context);
internal b32 app_on_resized(u32 code, void* sender, void* listener_inst, EventContext context);

internal void check_dll_changes(App* app);
internal void app_create(App* app);
internal void load_game_lib_init(App* app);

// TODO temp
b32 event_on_debug_event(u32 code, void* sender, void* listener_inst, EventContext data) {
  Scratch scratch;
  String names[] = {
    "cobblestone"_,
    "paving"_,
    "paving2"_,
  };
  local i8 choice = 2;
  
  // Save off the old name
  String old_name = names[choice];
  
  ++choice;
  choice %= 3;
  
  // Load up the new texture
  // state->test_geometry->material->diffuse_map.texture = texture_system_acquire(names[choice], true);
  // if (!state->test_geometry->material->diffuse_map.texture) {
  //   Warn("event_on_debug_event no texture! using default"_);
  //   state->test_geometry->material->diffuse_map.texture = texture_system_get_default_texture();
  // }
  
  // Release the old texture
  texture_system_release(old_name);
  return true;
}
// TODO end temp
void engine_create(App* app) {
  global_allocator_init();
  app_create(app);
  wchar_t whello[] = L"Hello";
  Scratch scratch;
  String s = push_str_wchar(scratch, whello, 5);
  Info("%s", s);
  
  app->engine_state = push_struct(app->arena, EngineState);
  
  Assign(state, app->engine_state);
  state->app = app;
  state->arena = arena_alloc(app->arena, EngineSize);
  
  {
    logging_init(state->arena);
  }

  {
    platform_init(state->arena);
  }
  
  {
    network_init(state->arena);
  }
  // test();
  // foo("D:\\VS_Code\\Engine\\assets\\shaders"_);
  
  {
    ResSysConfig res_sys_cfg = {
      .asset_base_path = "../assets"_
    };
    res_sys_init(state->arena, res_sys_cfg);
  }

  {
    EventSysConfig config = {
      .mem_reserve = KB(1)
    };
    event_init(state->arena, config);
    
    os_register_process_key(input_process_key);
    os_register_process_mouse_move(input_process_mouse_move);
    os_register_window_closed_callback(engine_on_window_closed);
    os_register_window_resized_callback(engine_on_window_resized);
    
    event_register(EventCode_ApplicationQuit, 0, app_on_event);
    event_register(EventCode_KeyPressed, 0, app_on_key);
    event_register(EventCode_KeyReleased, 0, app_on_key);
    event_register(EventCode_Resized, 0, app_on_resized);
    // TODO temp
    event_register(EventCode_Debug0, 0, event_on_debug_event);
    // end
  }

  {
    input_init(state->arena);
  }
  
  {
    WindowConfig config = {
      .position_x = 100,
      .position_y = 100,
      .width = 680,
      .height = 480,
      .name = app->name};
    os_window_create(state->arena, config);
  }
  
  {
    ShaderSysConfig config = {
      .mem_reserve = MB(1),
      .shader_count_max = 1024,
      .uniform_count_max = 128,
      .global_textures_max = 31,
      .instance_textures_max = 31
    };
    shader_sys_init(state->arena, config);
  }

  {
    R_Config config = {
      .mem_reserve = MB(10)
    };
    r_init(state->arena, config);
  }
  
  {
    ui_init();
  }

  {
    TextureSystemConfig texture_sys_config = {
      .max_texture_count = 65536,
    };
    texture_system_init(state->arena, texture_sys_config);
  }

  {
    MaterialSystemConfig material_sys_config = {
        .max_material_count = 4096,
    };
    material_system_init(state->arena, material_sys_config);
  }

  {
    GeometrySysConfig geometry_sys_config = {
      .max_geometry_count = 4096,
    };
    geometry_sys_init(state->arena, geometry_sys_config);
  }
  
  {
    asset_watch_init(state->arena);
  }

  {
    // GeometryConfig config = geometry_sys_generate_plane_config(10.0f, 5.0f, 5, 5, 5.0f, 2.0f, "test geometry"_, "test_material"_);
    // state->test_geometry = geometry_sys_acquire_from_config(config, true);
    
    // // Load up some test UI geometry.
    // GeometryConfig ui_config;
    // ui_config.vertex_size = sizeof(Vertex2D);
    // ui_config.vertex_count = 4;
    // ui_config.index_size = sizeof(u32);
    // ui_config.index_count = 6;
    // str_copy(ui_config.material_name64, "test_ui_material"_);
    // str_copy(ui_config.name64, "test_ui_geometry"_);

    // const f32 f = 512.0f;
    // Vertex2D uiverts [4];
    // uiverts[0].position.x = 0.0f;  // 0    3
    // uiverts[0].position.y = 0.0f;  //
    // uiverts[0].texcoord.x = 0.0f;  //
    // uiverts[0].texcoord.y = 0.0f;  // 2    1

    // uiverts[1].position.y = f;
    // uiverts[1].position.x = f;
    // uiverts[1].texcoord.x = 1.0f;
    // uiverts[1].texcoord.y = 1.0f;

    // uiverts[2].position.x = 0.0f;
    // uiverts[2].position.y = f;
    // uiverts[2].texcoord.x = 0.0f;
    // uiverts[2].texcoord.y = 1.0f;

    // uiverts[3].position.x = f;
    // uiverts[3].position.y = 0.0;
    // uiverts[3].texcoord.x = 1.0f;
    // uiverts[3].texcoord.y = 0.0f;
    // ui_config.vertices = uiverts;

    // // Indices - counter-clockwise
    // u32 indices[6] = {2, 1, 0, 3, 0, 1};
    // ui_config.indices = indices;

    // // Get UI geometry from config.
    // state->test_ui_geometry = geometry_sys_acquire_from_config(ui_config, true);
  }

  // TODO temp
  // state->test_geometry = geometry_sys_get_default();
  // TODO end

  app->init(app);
}

void engine_run(App* app) {
  os_show_window();
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
      state->app->delta_time = delta;
      f64 frame_start_time = os_now_seconds();

      check_dll_changes(app);

      // // TODO refactor packet creation
      R_Packet packet = {};
      // packet.delta_time = delta;
      
      // // TODO temp
      // GeometryRenderData test_render;
      // test_render.geometry = state->test_geometry;
      // test_render.model = mat4_identity();
      
      // packet.geometry_count = 1;
      // packet.geometries = &test_render;

      // GeometryRenderData test_ui_render;
      // test_ui_render.geometry = state->test_ui_geometry;
      // test_ui_render.model = mat4_translation(v3(0, 0, 0));
      // packet.ui_geometry_count = 1;
      // packet.ui_geometries = &test_ui_render;
      // // TODO end

      // r_draw_frame(&packet);
      r_begin_draw_frame(&packet);

      state->app->update(state->app);
      
      r_end_draw_frame(&packet);
      
      f64 frame_end_time = os_now_seconds();
      f64 frame_elapsed_time = frame_end_time - frame_start_time;
      os_set_delta_time_second(frame_elapsed_time);
      running_time += frame_elapsed_time;
      f64 remaining_seconds = target_frame_seconds - frame_elapsed_time;

      if (remaining_seconds > 0) {
        u32 remaining_ms = (remaining_seconds * 1000);

        // If there is time left, give it back to the OS.
        b32 limit_frames = true;
        if (remaining_ms > 0 && limit_frames) {
          os_sleep(remaining_ms - 1);
        }

        ++frame_count;
      }

      input_update();
      asset_watch_update();

      state->last_time = current_time;
    }
  }
  
  state->is_running = false;

  event_unregister(EventCode_ApplicationQuit, 0, app_on_event);
  event_unregister(EventCode_KeyPressed, 0, app_on_key);
  event_unregister(EventCode_KeyReleased, 0, app_on_key);
  event_unregister(EventCode_Resized, 0, app_on_resized);
  // TODO temp
  event_unregister(EventCode_Debug0, 0, event_on_debug_event);
  // TODO end

  // geometry_sys_shutdown();
  material_system_shutdown(); texture_system_shutdown();
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
    event_fire(EventCode_Resized, window, context);
  }
}

internal b32 app_on_event(u32 code, void* sender, void* listener_inst, EventContext context) {
  switch (code) {
    case EventCode_ApplicationQuit: {
      Info("EVENT_CORE_APPLICATION_QUIT received, shutting down.\n");
      state->is_running = false;
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
  GetProcAddr(app->update, app->lib, "application_update"_);
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
  
  app->lib = os_lib_open(("game_temp.dll"_));
  GetProcAddr(app->update, app->lib, ("application_update"_));
  GetProcAddr(app->init, app->lib, ("application_init"_));
  
  Assert(app->lib && app->init && app->update);
}

internal void app_create(App* app) {
  MemAlloc(app->arena, AppSize);
  app->arena->res = AppSize;
  tctx_init(app->arena);
  Scratch scratch;
  
  String filepath = os_exe_filename(scratch);
  app->file_path = push_str_copy(app->arena, filepath);
  app->name = str_skip_last_slash(app->file_path);
  
  String file_directory = str_chop_after_last_slash(app->file_path);
  app->lib_file_path = push_str_cat(app->arena, file_directory, "game.dll"_);
  app->lib_temp_file_path = push_str_cat(app->arena, file_directory, "game_temp.dll"_);

  load_game_lib_init(app);
}
