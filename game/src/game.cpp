
#include "sys/res_sys.h"
#include "game.h"

#include <engine.h>

// HACK This should not be available outside the engine
#include <render/r_frontend.h>
#include "sys/geometry.h"
#include "sys/shader_sys.h"
#include "sys/texture.h"
#include "sys/res_sys.h"

#include <event.h>
#include <input.h>

#include "object.h"
#include "camera.h"

struct alignas(16) UBO {
  mat4 projection_view;
  mat4 model;
  f32 color;
};

struct PushConstant {
  mat4 model;
};

GameState* state;

struct Vertex {
  v3 position;
  v3 color;
  v2 tex_coord;
};

Vertex triangle_vertices[] = {
  {v3(0.5f, -0.5f, 0.0f), v3(1.0f, 0.0f, 0.0f), {1.0f, 0.0f}},
  {v3(-0.5f, -0.5f, 0.0f), v3(0.0f, 1.0f, 0.0f), {0.0f, 0.0f}},
  {v3(0.0f, 0.5f, 0.0f), v3(0.0f, 0.0f, 1.0f), {0.5, 1.0f}}
};

Vertex cube_vertices[] = {
    // Front face (red)
    {v3(-0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f)},
    {v3( 0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f)},
    {v3( 0.5f,  0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f)},

    {v3( 0.5f,  0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f)},
    {v3(-0.5f,  0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f)},
    {v3(-0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f)},

    // Back face (green)
    {v3(-0.5f, -0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f)},
    {v3( 0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f)},
    {v3( 0.5f, -0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f)},

    {v3( 0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f)},
    {v3(-0.5f, -0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f)},
    {v3(-0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f)},

    // Left face (blue)
    {v3(-0.5f, -0.5f, -0.5f), v3(0.0f, 0.0f, 1.0f)},
    {v3(-0.5f, -0.5f,  0.5f), v3(0.0f, 0.0f, 1.0f)},
    {v3(-0.5f,  0.5f,  0.5f), v3(0.0f, 0.0f, 1.0f)},

    {v3(-0.5f,  0.5f,  0.5f), v3(0.0f, 0.0f, 1.0f)},
    {v3(-0.5f,  0.5f, -0.5f), v3(0.0f, 0.0f, 1.0f)},
    {v3(-0.5f, -0.5f, -0.5f), v3(0.0f, 0.0f, 1.0f)},

    // Right face (yellow)
    {v3(0.5f, -0.5f, -0.5f), v3(1.0f, 1.0f, 0.0f)},
    {v3(0.5f,  0.5f,  0.5f), v3(1.0f, 1.0f, 0.0f)},
    {v3(0.5f, -0.5f,  0.5f), v3(1.0f, 1.0f, 0.0f)},

    {v3(0.5f,  0.5f,  0.5f), v3(1.0f, 1.0f, 0.0f)},
    {v3(0.5f, -0.5f, -0.5f), v3(1.0f, 1.0f, 0.0f)},
    {v3(0.5f,  0.5f, -0.5f), v3(1.0f, 1.0f, 0.0f)},

    // Top face (cyan)
    {v3(-0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 1.0f)},
    {v3(-0.5f,  0.5f,  0.5f), v3(0.0f, 1.0f, 1.0f)},
    {v3( 0.5f,  0.5f,  0.5f), v3(0.0f, 1.0f, 1.0f)},

    {v3( 0.5f,  0.5f,  0.5f), v3(0.0f, 1.0f, 1.0f)},
    {v3( 0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 1.0f)},
    {v3(-0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 1.0f)},

    // Bottom face (magenta)
    {v3(-0.5f, -0.5f, -0.5f), v3(1.0f, 0.0f, 1.0f)},
    {v3( 0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 1.0f)},
    {v3(-0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 1.0f)},

    {v3( 0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 1.0f)},
    {v3(-0.5f, -0.5f, -0.5f), v3(1.0f, 0.0f, 1.0f)},
    {v3( 0.5f, -0.5f, -0.5f), v3(1.0f, 0.0f, 1.0f)},
};
Object objs[1];
Texture texture;

u32 obj_count = ArrayCount(objs);
void application_init(App* app) {
  Scratch scratch;

  app->state = push_struct(app->arena, GameState);
  Assign(state, app->state);
  state->arena = arena_alloc(app->arena, GameSize);
  state->camera_view_dirty = true;
  state->camera_position = v3(0,0, 10);
  state->camera_direction = v3_zero();
  entity_init();
 
  // Geom
  {
    Geometry geom {
      .name = "triangle"_,
      .vertex_size = sizeof(Vertex),
      .vertex_count = ArrayCount(triangle_vertices),
      .vertices = triangle_vertices,
    };
    state->geom_id = geometry_create(geom);
  }
  
  // Shader
  {
    ShaderConfig config = {
      .name = "triangle"_,
      .has_position = true,
      .has_color = true,
      .has_tex_coord = true,
    };
    state->shader = shader_create(config, &state->entities_ubo, sizeof(UBO), sizeof(PushConstant));
  }
  
  texture_load("container.jpg"_);
  // texture_load("paving.png"_);
  // id = vk_texture_load();
  // Texture 
  // {
  // }
  
  Loop (i, obj_count) {
    objs[i].id = entity_create();
    object_make_renderable(objs[i].id, state->geom_id);
    f32 min = -10, max = 10;
    f32 x = rand_in_range_f32(min, max);
    f32 y = rand_in_range_f32(min, max);
    f32 z = rand_in_range_f32(min, max);
    objs[i].position = v3(x, y, z);
  }
  
  // state->triangle.id = entity_create();
  // object_make_renderable(state->triangle.id, state->geom_id);
}

void application_update(App* app) {
  Assign(state, app->state);
  state->delta = app->delta_time;
  v2i frame_sise = os_get_framebuffer_size();
  state->projection = mat4_perspective(deg_to_rad(45.0f), frame_sise.x / frame_sise.y, 0.1f, 1000.0f);
  
  state->entities_ubo[0].projection_view = state->projection * state->view;
  local f32 rot = 0;
  rot += 0.01;
  // mat4* model = (mat4*)vk_get_push_constant(state->triangle.id);
  // *model = mat4_euler_y(rot) * mat4_identity();
  
  Loop (i, ArrayCount(objs)) {
    mat4* model = (mat4*)vk_get_push_constant(objs[i].id);
    *model = mat4_translation(objs[i].position) * mat4_euler_y(rot);
  }
  
  camera_manage();
}
