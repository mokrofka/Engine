#include "defines.h"

struct EngineSystemStates {
  u64 event_system_memory_requirement;
  struct EventState* event_system;
  
  u64 input_system_memory_requirement;
  struct InputState* input_system;
  
  u64 platform_system_memory_requirement;
  struct PlatformState* platform_system;
};

KAPI b8 engine_restore_state(void* memory);
KAPI void test_game_function(void(*function)());

C_LINKAGE_BEGIN

KAPI b8 engine_create(void* memory);

C_LINKAGE_END
