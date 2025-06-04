#include "lib.h"
#include "ecs.h"

struct Object {
  u32 id;
};

struct Camera {
  mat4 view;
  mat4 projection;
  b8 view_dirty;
  v3 position;
  v3 direction;
  f32 yaw;
  f32 pitch;
  f32 fov;
};

struct GameState {
  Arena* arena;
  
  Camera camera;
  
  u32 entity_count;
  u32 entities[MaxEntities];
  f32 rot;
  b8 is_mouse_move;
};
