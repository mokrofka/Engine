#include "application.h"

#include <platform/platform.h>
#include <core/memory.h>
#include <core/logger.h>

b8 application_create(Application* app) {
  initialize_memory(app);
  app->total_size = GB(1);
  app->config.start_pos_x = 100;
  app->config.start_pos_y = 100;
  app->config.start_width_x = 1280;
  app->config.start_height_y = 720;
  app->config.name = cstr8("Platform.exe");
  
  app->state.is_running = true;
  app->state.permanent_arena = arena_alloc(app->main_arena, KB(1));
  
  app->engine_memory = arena_alloc(app->main_arena, MB(300));
  app->game_memory = arena_alloc(app->main_arena, MB(300));
  
  app->engine_state = push_array(app->engine_memory, u8, KB(1));
  app->game_state = push_array(app->game_memory, u8, KB(1));
  
  Arena** engine_state = (Arena**)app->engine_state;
  *engine_state = app->engine_memory;
  
  Arena** game_state = (Arena**)app->game_state;
  *game_state = app->game_memory;
  u64* memory_points_to_game_state = (u64*)game_state; 
  *(++memory_points_to_game_state) = (u64)app->engine_state;

  {
    Temp scratch = GetScratch(0, 0);
    u8* buffer = push_array(scratch.arena, u8, 100);
    platform_get_EXE_filename(buffer);
    String directory_with_slash = str8_chop_last_segment(cstr8((char*)buffer));
    app->engine_lib.src_full_filename = push_str8_cat(app->state.permanent_arena,
                                                             directory_with_slash, cstr8("dll\\engine.dll"));
    app->engine_lib.temp_full_filename = push_str8_cat(app->state.permanent_arena,
                                                             directory_with_slash, cstr8("engine.dll"));
    app->game_lib.src_full_filename = push_str8_cat(app->state.permanent_arena,
                                                             directory_with_slash, cstr8("dll\\game.dll"));
    app->game_lib.temp_full_filename = push_str8_cat(app->state.permanent_arena,
                                                             directory_with_slash, cstr8("game_temp.dll"));
    ReleaseScratch(scratch);
  }
  
  return true;
}

void load_engine_lib(Application* app) {
  app->engine_lib.dll_last_time_write = platform_get_last_write_time(app->engine_lib.src_full_filename);
  platform_copy_file(app->engine_lib.src_full_filename, app->engine_lib.temp_full_filename);
  // platform_dynamic_library_load(app->engine_lib.src_full_filename, &app->engine_lib);
  platform_dynamic_library_load(app->engine_lib.temp_full_filename, &app->engine_lib);
  app->engine_create = (b8(*)(void*))platform_dynamic_library_load_function("engine_create", &app->engine_lib);
}

void unload_engine_lib(Application* app) {
  platform_dynamic_library_unload(&app->engine_lib);
  app->engine_create = 0;
}

void copy_engine(Application* app) {
  platform_copy_file(app->engine_lib.src_full_filename, app->engine_lib.temp_full_filename);
}

void load_game_lib(Application* app) {
  app->game_lib.dll_last_time_write = platform_get_last_write_time(app->game_lib.src_full_filename);
  app->engine_lib.dll_last_time_write = platform_get_last_write_time(app->engine_lib.src_full_filename);
  platform_copy_file(app->game_lib.src_full_filename, app->game_lib.temp_full_filename);
  platform_dynamic_library_load(app->game_lib.temp_full_filename, &app->game_lib);
  app->game_update = (void(*)(void*))platform_dynamic_library_load_function("game_update", &app->game_lib);
  app->game_initialize = (void(*)(void*))platform_dynamic_library_load_function("game_initialize", &app->game_lib);
}

void unload_game_lib(Application* app) {
  platform_dynamic_library_unload(&app->game_lib);
  app->game_update = 0;
}

b8 engine_create(Application* app) {
  load_engine_lib(app);
  app->engine_create(app->engine_state);
  // unload_engine_lib(app);

  return true;
}

void check_dll_changes(Application* app) {
  u64 new_engine_dll_write_time = platform_get_last_write_time(app->engine_lib.src_full_filename);
  u64 new_game_dll_write_time = platform_get_last_write_time(app->game_lib.src_full_filename);
  b8 game_write = platform_compare_file_time(new_game_dll_write_time, app->game_lib.dll_last_time_write);
  b8 engine_write = platform_compare_file_time(new_engine_dll_write_time, app->engine_lib.dll_last_time_write);
  
  if (game_write) {
    unload_game_lib(app);
    load_game_lib(app);
  }
  if (engine_write) {
    // unload_engine_lib(app);
    unload_game_lib(app);
    copy_engine(app);
    load_game_lib(app);
    // load_engine_lib(app);
  }
}

b8 engine_run(Application* app) {
  
  load_game_lib(app);
  app->game_initialize(app->game_state);
  
  while (app->state.is_running) {
    if (!platform_pump_messages(&app->state.platform.platform_state)) {
      app->state.is_running = false;
    }
    
    check_dll_changes(app);

    app->game_update(app->game_state);
  }
  
  
  return true;
}

