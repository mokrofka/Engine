#include "platform.h"

struct Application {
  b8 (*initialize)(struct Application* app_inst);
  b8 (*update)(struct Application* app_inst);
  b8 (*render)(struct Application* app_inst);
  b8 (*on_resize)(struct Application* app_inst);
  
  String name;
  String full_name;
  Arena* arena;
  void* state;
  void* engine_state;
  
  f64 delta_time;
  
  DynamicLibrary game_lib;
};
