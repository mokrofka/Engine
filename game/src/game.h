#pragma once
#include "lib.h"
#include "object.h"

C_LINKAGE_BEGIN

ExportAPI void application_update(struct App* game_inst);
ExportAPI void application_init(struct App* game_inst);
ExportAPI void application_render(struct App* game_inst);
ExportAPI void application_on_resize(struct App* game_inst);

C_LINKAGE_END

struct Object {
  u32 id;
  v3 position;
  v3 velocity;
  v3 acceleration;
};

struct GameState {
  Arena* arena;
  f32 delta;
  
  Object triangle;
  
  u32 geom_id;
  struct Shader* shader; 
  struct UBO* entities_ubo;
  struct PushConstant* entities_push_constant;
  
  mat4 view;
  mat4 projection;
  b8 camera_view_dirty;
  v3 camera_position;
  v3 camera_direction;
  Object* objs;
  u32 obj_count;
  f32 rot;
};

extern GameState* state;
