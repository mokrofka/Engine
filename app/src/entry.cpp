#include <entry.h>

internal b8 load_game_lib(Application* app) {
  app->game_lib.last_time_write = os_file_last_write_time(app->game_lib.src_full_filename);
  os_file_copy(app->game_lib.src_full_filename, app->game_lib.temp_full_filename);
  
  app->game_lib.handle = os_library_load(str_lit("game_temp.dll"));
  if (!app->game_lib.handle) {
    return false;
  }
  
  app->update = (void(*)(Application*))os_library_load_function(str_lit("application_update"), app->game_lib);
  if (!app->update) {
    return false;
  }
  app->init = (void(*)(Application*))os_library_load_function(str_lit("application_init"), app->game_lib);
  if (!app->init) {
    return false;
  }

  return true;
}
  
void application_create(Application* app) {
  app->arena = arena_alloc(GB(1));
  tctx_init(app->arena);
  
  u8 buffer[100] = {};
  u32 file_size = os_exe_filename(buffer);
  app->full_name = push_str_copy(app->arena, str(buffer, file_size));
  app->name = str_skip_last_slash(app->full_name);
  
  String full_name_without_slash = str_chop_last_segment(app->full_name);
  app->game_lib.src_full_filename =
      push_str_cat(app->arena, full_name_without_slash, cstr("game.dll"));
  app->game_lib.temp_full_filename =
      push_str_cat(app->arena, full_name_without_slash, cstr("game_temp.dll"));

  if (!load_game_lib(app)) {
    Error("Game lib load failed!");
  }
}
