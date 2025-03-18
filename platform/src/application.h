#include "defines.h"
#include "platform/platform.h"

struct ApplicationConfig {
  i16 start_pos_x;
  
  i16 start_pos_y;
  
  i16 start_width_x;
  
  i16 start_height_y;
  
  String name;
};

struct ApplicationState {
  // Game* game_inst;
  Arena* permanent_arena;
  b8 is_running;
  b8 is_suspended;
  // PlatformState platform;
  Window platform;
  i16 width;
  i16 height;
  // Clock clock;
  f64 last_time;
};

// struct GameState {
//   Arena* permanent_arena;
//   Arena* transient_arena;
// };

// struct EngineState {
//   Arena* permanent_arena;
//   Arena* transient_arena;
// };

struct Application {
  Arena* main_arena;
  // Arena* permanent_storage;
  u64 total_size;
  ApplicationConfig config;
  ApplicationState state;
  DynamicLibrary engine_lib;
  DynamicLibrary game_lib;
  b8 (*engine_create)(void*);
  void (*game_update)(void*);
  void (*game_initialize)(void*);
  Arena* game_memory;
  Arena* engine_memory;
  
  void* game_state;
  void* engine_state;
};

b8 application_create(Application* state);

b8 engine_create(Application* out_application);

b8 engine_run(Application* app_inst);
