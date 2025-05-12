
#include "vendor/imgui/imgui.h"
#include "game.h"

#include <engine.h>

// HACK This should not be available outside the engine
#include <render/r_frontend.h>
#include "sys/geometry_sys.h"
#include "sys/shader_sys.h"

#include <event.h>
#include <input.h>

struct Object {
  Geometry* geom;
  Shader* shader; 
};


struct GameState {
  Arena* arena;
  
  Object triangle;
};

GameState* state;

struct Vertex {
  v3 position;
};

Vertex vertices[] = {
  v3(-0.5f, -0.5f, 0.0f),
  v3( 0.5f, -0.5f, 0.0f),
  v3( 0.0f,  0.5f, 0.0f)
};

#include "ecs.h"
void application_init(App* app) {
  Scratch scratch;
  
  app->state = push_struct(app->arena, GameState);
  Assign(state, app->state);
  state->arena = arena_alloc(app->arena, GameSize);
  
  // Geom
  {
    GeometryConfig config {
      .name = "triangle"_,
      .vertex_size = sizeof(Vertex),
      .vertex_count = ArrayCount(vertices),
      .vertices = vertices,
    };
    geometry_sys_acquire_from_config(config, true);
  }
  
  // Shader
  {
    ShaderConfig config = {
      .name = "triangle"_,
      .has_position = true,
    };
    // shader_sys_create(&config);
    // entity_set_shader(entity.id, shader.id);
    // entity_set_geometry(entity.id, geom.id);
  }
  
  ecs_init();
  test();
  i32 a = 1;
}

i32 main();
void application_update(App* app) {
  Assign(state, app->state);
  
  i32 a = 0;
  SetBit(a, 1);
  Info("%i", a);
   
  // main();
}
