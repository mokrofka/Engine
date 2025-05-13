
#include "vendor/imgui/imgui.h"
#include "game.h"

#include <engine.h>

// HACK This should not be available outside the engine
#include <render/r_frontend.h>
#include "sys/geometry_sys.h"
#include "sys/shader_sys.h"

#include <event.h>
#include <input.h>

#include "object.h"

struct GameState {
  Arena* arena;
  
  Object triangle;
  
  Geometry* geom;
  Shader* shader; 
};

GameState* state;

struct Vertex {
  v3 position;
};

Vertex vertices[] = {
  // v3(-0.5f, -0.5f, 0.0f),
  // v3( 0.5f, -0.5f, 0.0f),
  // v3( 0.0f,  0.5f, 0.0f),
  
  // v3(0.5f, -0.5f, 0.0f ),
  // v3(-0.5f, -0.5f, 0.0f),
  // v3(-0.5f,  0.5f, 0.0f)
  
  v3(0.5f,  0.5f, 0.0f  ),// top right
  v3( 0.5f, -0.5f, 0.0f  ),// bottom right
  v3(-0.5f,  0.5f, 0.0f  ),// top left 
  
 v3(  0.5f, -0.5f, 0.0f  ),// bottom right
 v3( -0.5f, -0.5f, 0.0f  ),// bottom left
 v3( -0.5f,  0.5f, 0.0f  ),// top left
};

struct UBO {
  f32 gree_color;
};

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
    state->geom = geometry_create(config);
  }
  
  // Shader
  {
    ShaderConfig config = {
      .name = "triangle"_,
      .has_position = true,
    };
    state->shader = shader_create(config);
  }
  object_sys_init();
  // state->triangle.id = object_create();
  Object object1 = object_create(state->geom->id, state->shader->id);
  object_make_renderable(object1);
  Object object2 = object_create(state->geom->id, state->shader->id);
  object_make_renderable(object2);

  // make_render_entity(object);
  
  // state->triangle.e = entity_create();
  // component_add(state->triangle.e, Renderable, 0);

   
}

void application_update(App* app) {
  Assign(state, app->state);
  
}
