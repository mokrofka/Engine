#include <entry.h>

internal void load_game_lib(Application* app) {
  FileProperties file_props = os_properties_from_file_path(app->lib_file_path);

  app->modified = file_props.modified;
  os_copy_file_path(app->lib_temp_filepath, app->lib_file_path);
  
  // GET_PROC_ADDR(app->update, app->lib, str_lit("application_update"));
  
  app->lib = os_lib_open(str_lit("game_temp.dll"));
  // GET_PROC_ADDR(v, m, s)
  GetProcAddr(app->update, app->lib, str_lit("application_update"));
  GetProcAddr(app->init, app->lib, str_lit("application_init"));
  
  Assert(app->lib.u64 && app->init && app->update);
}
  
internal void application_create(Application* app) {
  app->arena = arena_alloc(GB(1));
  tctx_init(app->arena);
  Scratch scratch;
  
  String filepath = os_exe_filename(scratch);
  app->file_path = push_str_copy(app->arena, filepath);
  app->name = str_skip_last_slash(app->file_path);
  
  String file_directory = str_chop_after_last_slash(app->file_path);
  app->lib_file_path = push_str_cat(app->arena, file_directory, str_cstr("game.dll"));
  app->lib_temp_filepath = push_str_cat(app->arena, file_directory, str_cstr("game_temp.dll"));

  load_game_lib(app);
}
