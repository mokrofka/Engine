#include "lib.h"

struct Object {
  u32 id;
  v3 position;
  v3 velocity;
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
  f32 delta;
  
  Object triangle;
  
  u32 cube_geom_id;
  u32 triangle_geom_id;
  struct Shader* shader; 
  struct UBO* entities_ubo;
  struct PushConstant* entities_push_constant;
  Camera camera;
  
  Object objs[100];
  u32 obj_count;
  f32 rot;
  b8 is_mouse_move;
};
