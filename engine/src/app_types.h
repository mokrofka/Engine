#include "os.h"

struct Application {
  struct Arena* arena;
  void (*init)(struct Application* app_inst);
  void (*update)(struct Application* app_inst);
  void (*render)(struct Application* app_inst);
  void (*on_resize)(struct Application* app_inst);
  
  String name;
  String full_name;
  void* state;
  void* engine_state;
  
  f64 delta_time;
  
  DynamicLibrary game_lib;
};
