#include <entry.h>

b8 load_game_lib(Application* app) {
  
  app->game_lib.dll_last_time_write = platform_get_last_write_time(app->game_lib.src_full_filename);
  platform_copy_file(app->game_lib.src_full_filename, app->game_lib.temp_full_filename);
  if (!platform_dynamic_library_load(str_lit("game_temp.dll"), &app->game_lib)) {
    return false;
  }
  
  app->update = (b8(*)(Application*))platform_dynamic_library_load_function("application_update", &app->game_lib);
  if (!app->update) {
    return false;
  }
  app->initialize = (b8(*)(Application*))platform_dynamic_library_load_function("application_initialize", &app->game_lib);
  if (!app->initialize) {
    return false;
  }

  return true;
}
  
b8 application_create(Application* out_app) {
  out_app->arena = arena_alloc(GB(1));
  tctx_initialize(out_app->arena);
  
  u8 buffer[100] = {};
  u32 file_size = platform_get_EXE_filename(buffer);
  out_app->full_name = push_str_copy(out_app->arena, str(buffer, file_size));
  out_app->name = str_skip_last_slash(out_app->full_name);
  
  String full_name_without_slash = str_chop_last_segment(out_app->full_name);
  out_app->game_lib.src_full_filename =
      push_str_cat(out_app->arena, full_name_without_slash, cstr("game.dll"));
  out_app->game_lib.temp_full_filename =
      push_str_cat(out_app->arena, full_name_without_slash, cstr("game_temp.dll"));

  if (!load_game_lib(out_app)) {
    Error("Initial game lib load failed!");
    return false;
  }

  return true;
}

b8 application_initialize(Application* app) {

  return true;
}
