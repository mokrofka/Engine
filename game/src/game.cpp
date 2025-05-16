#include "game.h"

#include <engine.h>

// HACK This should not be available outside the engine
#include <render/r_frontend.h>
#include "sys/geometry.h"
#include "sys/shader_sys.h"
#include "sys/texture.h"

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

// Vertex cube_vertices[] = {
//     // Front face (red)
//     {v3(-0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f)},
//     {v3( 0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f)},
//     {v3( 0.5f,  0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f)},

//     {v3( 0.5f,  0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f)},
//     {v3(-0.5f,  0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f)},
//     {v3(-0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f)},

//     // Back face (green)
//     {v3(-0.5f, -0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f)},
//     {v3( 0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f)},
//     {v3( 0.5f, -0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f)},

//     {v3( 0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f)},
//     {v3(-0.5f, -0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f)},
//     {v3(-0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f)},

//     // Left face (blue)
//     {v3(-0.5f, -0.5f, -0.5f), v3(0.0f, 0.0f, 1.0f)},
//     {v3(-0.5f, -0.5f,  0.5f), v3(0.0f, 0.0f, 1.0f)},
//     {v3(-0.5f,  0.5f,  0.5f), v3(0.0f, 0.0f, 1.0f)},

//     {v3(-0.5f,  0.5f,  0.5f), v3(0.0f, 0.0f, 1.0f)},
//     {v3(-0.5f,  0.5f, -0.5f), v3(0.0f, 0.0f, 1.0f)},
//     {v3(-0.5f, -0.5f, -0.5f), v3(0.0f, 0.0f, 1.0f)},

//     // Right face (yellow)
//     {v3(0.5f, -0.5f, -0.5f), v3(1.0f, 1.0f, 0.0f)},
//     {v3(0.5f,  0.5f,  0.5f), v3(1.0f, 1.0f, 0.0f)},
//     {v3(0.5f, -0.5f,  0.5f), v3(1.0f, 1.0f, 0.0f)},

//     {v3(0.5f,  0.5f,  0.5f), v3(1.0f, 1.0f, 0.0f)},
//     {v3(0.5f, -0.5f, -0.5f), v3(1.0f, 1.0f, 0.0f)},
//     {v3(0.5f,  0.5f, -0.5f), v3(1.0f, 1.0f, 0.0f)},

//     // Top face (cyan)
//     {v3(-0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 1.0f)},
//     {v3(-0.5f,  0.5f,  0.5f), v3(0.0f, 1.0f, 1.0f)},
//     {v3( 0.5f,  0.5f,  0.5f), v3(0.0f, 1.0f, 1.0f)},

//     {v3( 0.5f,  0.5f,  0.5f), v3(0.0f, 1.0f, 1.0f)},
//     {v3( 0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 1.0f)},
//     {v3(-0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 1.0f)},

//     // Bottom face (magenta)
//     {v3(-0.5f, -0.5f, -0.5f), v3(1.0f, 0.0f, 1.0f)},
//     {v3( 0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 1.0f)},
//     {v3(-0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 1.0f)},

//     {v3( 0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 1.0f)},
//     {v3(-0.5f, -0.5f, -0.5f), v3(1.0f, 0.0f, 1.0f)},
//     {v3( 0.5f, -0.5f, -0.5f), v3(1.0f, 0.0f, 1.0f)},
// };
Vertex cube_vertices[] = {
    // Front face (red)
    {v3(-0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f), {0.0f, 0.0f}},
    {v3( 0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f), {1.0f, 0.0f}},
    {v3( 0.5f,  0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f), {1.0f, 1.0f}},

    {v3( 0.5f,  0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f), {1.0f, 1.0f}},
    {v3(-0.5f,  0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f), {0.0f, 1.0f}},
    {v3(-0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 0.0f), {0.0f, 0.0f}},

    // Back face (green)
    {v3(-0.5f, -0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f), {1.0f, 0.0f}},
    {v3( 0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f), {0.0f, 1.0f}},
    {v3( 0.5f, -0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f), {0.0f, 0.0f}},

    {v3( 0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f), {0.0f, 1.0f}},
    {v3(-0.5f, -0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f), {1.0f, 0.0f}},
    {v3(-0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f), {1.0f, 1.0f}},

    // Left face (blue)
    {v3(-0.5f, -0.5f, -0.5f), v3(0.0f, 0.0f, 1.0f), {0.0f, 0.0f}},
    {v3(-0.5f, -0.5f,  0.5f), v3(0.0f, 0.0f, 1.0f), {1.0f, 0.0f}},
    {v3(-0.5f,  0.5f,  0.5f), v3(0.0f, 0.0f, 1.0f), {1.0f, 1.0f}},

    {v3(-0.5f,  0.5f,  0.5f), v3(0.0f, 0.0f, 1.0f), {1.0f, 1.0f}},
    {v3(-0.5f,  0.5f, -0.5f), v3(0.0f, 0.0f, 1.0f), {0.0f, 1.0f}},
    {v3(-0.5f, -0.5f, -0.5f), v3(0.0f, 0.0f, 1.0f), {0.0f, 0.0f}},

    // Right face (yellow)
    {v3(0.5f, -0.5f, -0.5f), v3(1.0f, 1.0f, 0.0f), {1.0f, 0.0f}},
    {v3(0.5f,  0.5f,  0.5f), v3(1.0f, 1.0f, 0.0f), {0.0f, 1.0f}},
    {v3(0.5f, -0.5f,  0.5f), v3(1.0f, 1.0f, 0.0f), {0.0f, 0.0f}},

    {v3(0.5f,  0.5f,  0.5f), v3(1.0f, 1.0f, 0.0f), {0.0f, 1.0f}},
    {v3(0.5f, -0.5f, -0.5f), v3(1.0f, 1.0f, 0.0f), {1.0f, 0.0f}},
    {v3(0.5f,  0.5f, -0.5f), v3(1.0f, 1.0f, 0.0f), {1.0f, 1.0f}},

    // Top face (cyan)
    {v3(-0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 1.0f), {0.0f, 1.0f}},
    {v3(-0.5f,  0.5f,  0.5f), v3(0.0f, 1.0f, 1.0f), {0.0f, 0.0f}},
    {v3( 0.5f,  0.5f,  0.5f), v3(0.0f, 1.0f, 1.0f), {1.0f, 0.0f}},

    {v3( 0.5f,  0.5f,  0.5f), v3(0.0f, 1.0f, 1.0f), {1.0f, 0.0f}},
    {v3( 0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 1.0f), {1.0f, 1.0f}},
    {v3(-0.5f,  0.5f, -0.5f), v3(0.0f, 1.0f, 1.0f), {0.0f, 1.0f}},

    // Bottom face (magenta)
    {v3(-0.5f, -0.5f, -0.5f), v3(1.0f, 0.0f, 1.0f), {0.0f, 1.0f}},
    {v3( 0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 1.0f), {1.0f, 0.0f}},
    {v3(-0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 1.0f), {0.0f, 0.0f}},

    {v3( 0.5f, -0.5f,  0.5f), v3(1.0f, 0.0f, 1.0f), {1.0f, 0.0f}},
    {v3(-0.5f, -0.5f, -0.5f), v3(1.0f, 0.0f, 1.0f), {0.0f, 1.0f}},
    {v3( 0.5f, -0.5f, -0.5f), v3(1.0f, 0.0f, 1.0f), {1.0f, 1.0f}},
};
Texture texture;

void application_init(App* app) {
  Scratch scratch;

  app->state = push_struct(app->arena, GameState);
  Assign(state, app->state);
  state->arena = arena_alloc(app->arena, GameSize);
  state->obj_count = KB(10);
  state->objs = (Object*)mem_alloc(state->obj_count * sizeof(Object));
  
  state->camera_view_dirty = true;
  state->camera_position = v3(0,0, 10);
  state->camera_direction = v3_zero();
  entity_init();
 
  // Geom
  {
    Geometry triangle_geom {
      .name = "triangle"_,
      .vertex_size = sizeof(Vertex),
      .vertex_count = ArrayCount(triangle_vertices),
      .vertices = triangle_vertices,
    };
    Geometry cube_geom {
      .name = "cube"_,
      .vertex_size = sizeof(Vertex),
      .vertex_count = ArrayCount(cube_vertices),
      .vertices = triangle_vertices,
    };
    state->geom_id = geometry_create(cube_geom);
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
  
  texture_load("super.jpg"_);
  
  Loop (i, state->obj_count) {
    state->objs[i].id = entity_create();
    object_make_renderable(state->objs[i].id, state->geom_id);
    f32 min = -100, max = 100;
    f32 x = rand_in_range_f32(min, max);
    f32 y = rand_in_range_f32(min, max);
    f32 z = rand_in_range_f32(min, max);
    state->objs[i].position = v3(x/100, y/100, z/100);
    
    state->objs[i].velocity = v3(x, y, z);
    state->objs[i].acceleration = v3(x, y, z);
  }
  
  // state->triangle.id = entity_create();
  // object_make_renderable(state->triangle.id, state->geom_id);
}

void application_update(App* app) {
  Assign(state, app->state);
  state->delta = app->delta_time;
  v2i frame_sise = os_get_framebuffer_size();
  state->projection = mat4_perspective(deg_to_rad(45.0f), frame_sise.x / frame_sise.y, 0.1f, 1000.0f);
  Object* objs = state->objs;
  
  state->entities_ubo[0].projection_view = state->projection * state->view;
  f32& rot = state->rot;
  rot += 0.01;
  // mat4* model = (mat4*)vk_get_push_constant(state->triangle.id);
  // *model = mat4_euler_y(rot) * mat4_identity();
  
  f32 wave = Sin(os_now_seconds() / 5);
  Loop(i, state->obj_count) {

    v3 to_center = v3(0, 0, 0) - objs[i].position; // direction to origin
    f32 dist = v3_length(to_center);
    v3 dir = v3_normal(to_center);
    f32 strength = 1.01f;

    objs[i].acceleration *= 1.0f / (dist + 1.0f);
    
    objs[i].velocity += objs[i].acceleration * state->delta;
    if (wave >= 0) {
      objs[i].position += objs[i].velocity * state->delta;
    } else {
      objs[i].position -= objs[i].velocity * state->delta;
    }

    mat4* model = (mat4*)vk_get_push_constant(objs[i].id);
    *model = mat4_translation(objs[i].position) * mat4_euler_y(rot);
    *model = mat4_euler_y(rot / 2) * mat4_translation(objs[i].position);
  }

  camera_manage();
}
