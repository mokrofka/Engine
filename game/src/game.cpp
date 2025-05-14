
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

struct alignas(16) UBO {
  f32 color;
};

struct GameState {
  Arena* arena;
  
  Object triangle;
  
  Geometry* geom;
  Shader* shader; 
  UBO* entities_ubo;
  UBO* entities_ubo_new;
};

GameState* state;

struct Vertex {
  v3 position;
  v3 color;
};

Vertex vertices[] = {
  {v3(0.5f, -0.5f, 0.0f), v3(1.0f, 0.0f, 0.0f)},
  {v3(-0.5f, -0.5f, 0.0f), v3(0.0f, 1.0f, 0.0f)},
  {v3(0.0f, 0.5f, 0.0f), v3(0.0f, 0.0f, 1.0f)}
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
      .has_color = true,
    };
    state->shader = shader_create(config, &state->entities_ubo, &state->entities_ubo_new, sizeof(UBO) * 10);
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

  state->entities_ubo[0].color = 0;
   
}

void application_update(App* app) {
  Assign(state, app->state);
  state->entities_ubo[0].color += 0.001;
  state->entities_ubo[1].color -= 0.001;
  // state->entities_ubo_new[1000].color.x = Sin(os_now_seconds()) / 2 + 0.5;
  
  // Info("%f", state->entities_ubo[0].color.x);
  // Info("%f", state->entities_ubo[1].color.x);
}
