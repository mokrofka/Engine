#include "os.h"

struct Application {
  struct Arena* arena;
  b8 (*init)(struct Application* app_inst);
  b8 (*update)(struct Application* app_inst);
  b8 (*render)(struct Application* app_inst);
  b8 (*on_resize)(struct Application* app_inst);
  
  String name;
  String full_name;
  void* state;
  void* engine_state;
  
  f64 delta_time;
  
  DynamicLibrary game_lib;
};
