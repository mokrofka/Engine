#pragma once
#include "lib.h"

struct Application {
  Arena* arena;
  void (*init)(struct Application* app_inst);
  void (*update)(struct Application* app_inst);
  void (*render)(struct Application* app_inst);
  void (*on_resize)(struct Application* app_inst);
  
  String name;
  String file_path;
  void* state;
  void* engine_state;
  
  f64 delta_time;
  
  OS_Handle lib;
  String lib_file_path;
  String lib_temp_filepath;
  u64 modified;
};
