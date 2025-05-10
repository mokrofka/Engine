
#include "vendor/imgui/imgui.h"
#include "game.h"

#include <engine.h>

// HACK This should not be available outside the engine
#include <render/r_frontend.h>
#include "sys/geometry_sys.h"

#include <event.h>
#include <input.h>

struct Object {
  Geometry* geom;
  
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

void application_init(App* app) {
  Scratch scratch;
  
  app->state = push_struct(app->arena, GameState);
  Assign(state, app->state);
  state->arena = arena_alloc(app->arena, GameSize);
  
  {
    GeometryConfig config {
      .vertex_size = sizeof(Vertex),
      .vertex_count = ArrayCount(vertices),
      .vertices = vertices,
    };
    str_copy(config.name64, "triangle"_);
    state->triangle.geom = geometry_sys_acquire_from_config(config, true);
  }
}

void application_update(App* app) {
  Assign(state, app->state);
  
  
}
