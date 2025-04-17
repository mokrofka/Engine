#pragma once
#include "lib.h"

struct App {
  Arena* arena;
  void (*init)(App* app_inst);
  void (*update)(App* app_inst);
  void (*render)(App* app_inst);
  void (*on_resize)(App* app_inst);
  
  String name;
  String file_path;
  void* state;
  void* engine_state;
  
  f64 delta_time;
  
  OS_Handle lib;
  String lib_file_path;
  String lib_temp_file_path;
  u64 modified;
};

KAPI void engine_create(struct App* game_inst);

KAPI void engine_run(struct App* game_inst);
