#include "defines.h"

struct MyData {
  i32 x,y,z; 
};

struct GameState {
  struct Arena* main_arena;
  
  void* engine_memory;
  b8 is_initialized;
  b8 is_stated;
  
  struct Arena* permanent_arena;
  struct Arena* transient_arena;

  MyData* data;
};

C_LINKAGE_BEGIN

__declspec(dllexport) void game_update(void* memory);
__declspec(dllexport) void game_initialize(void* memory);

C_LINKAGE_END
