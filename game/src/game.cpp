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

GameState* st;

struct Vertex {
  v3 position;
  // v3 color;
  v2 tex_coord;
};

Vertex vertices[] = {
  // Position             // Tex coord
  // Front face
  {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
  {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}},
  {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
  {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
  {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},
  {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},

  // Back face
  {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
  {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
  {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
  {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
  {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},
  {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},

  // Left face
  {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
  {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}},
  {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
  {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
  {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},
  {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},

  // Right face
  {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
  {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
  {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
  {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
  {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},
  {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},

  // Bottom face
  {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
  {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},
  {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}},
  {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}},
  {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
  {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},

  // Top face
  {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}},
  {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}},
  {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
  {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
  {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},
  {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}},
};

f32 triangle_vertices[] = {
  // position          // color
  0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,  // Vertex 1: red
 -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,  // Vertex 2: green
  0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f   // Vertex 3: blue
};

u32 cube_create() {
  u32 id = entity_create();
  entity_make_renderable(id, geometry_get("cube"_), shader_get("texture_shader"_));
  return id;
}

u32 triangle_create() {
  u32 id = entity_create();
  entity_make_renderable(id, geometry_get("triangle"_), shader_get("color_shader"_));
  return id;
}

void application_init(App* app) {
  Scratch scratch;

  app->state = push_struct(app->arena, GameState);
  Assign(st, app->state);
  st->arena = arena_alloc(app->arena, GameSize);

  st->obj_count = 2;

  st->camera.view_dirty = true;
  st->camera.position = v3(0,0, 10);
  st->camera.yaw = -90;
  st->camera.fov = 45;
  entity_init();
 
  {
    Geometry cube_geom = {
      .name = "cube"_,
      .vertex_count = ArrayCount(vertices),
      .vertex_size = sizeof(Vertex),
      .vertices = vertices,
    };
    geometry_create(cube_geom);
  }
  {
    Geometry triangle_geom = {
      .name = "triangle"_,
      .vertex_count = ArrayCount(triangle_vertices) / 6,
      .vertex_size = sizeof(v3) + sizeof(v3),
      .vertices = triangle_vertices,
    };
    geometry_create(triangle_geom);
  }
  // {
  //   Geometry grid = {
  //     .name = "grid"_,
  //     .vertex_count = ,
  //     .vertex_size = ,
  //     .vertices = ,
  //   }
  // }
  
  {
    ShaderConfig config = {
      .name = "texture_shader"_,
      .has_position = true,
      .has_tex_coord = true,
    };
    shader_create(config, &st->entities_ubo, sizeof(UBO), sizeof(PushConstant));
  }
  {
    ShaderConfig config = {
      .name = "color_shader"_,
      .has_position = true,
      .has_color = true,
    };
    shader_create(config, &st->entities_ubo, sizeof(UBO), sizeof(PushConstant));
  }
  
  texture_load("container.jpg"_);
  
  f32 min = -1, max = 1;
  Loop (i, st->obj_count) {
    st->objs[i].id = cube_create();
    f32 x = rand_in_range_f32(min, max);
    f32 y = rand_in_range_f32(min, max);
    f32 z = rand_in_range_f32(min, max);
    st->objs[i].position = v3(x, y, z);
  }
}

f32 min = -30, max = 30;
b32 toggle;
void application_update(App* app) {
  Assign(st, app->state);
  st->delta = app->delta_time;
  Object* objs = st->objs;
  
  st->entities_ubo[0].projection_view = st->camera.projection * st->camera.view;
  
  f32& rot = st->rot;
  rot += 0.01;
  u32 index = st->obj_count;
  if (input_was_key_down(KEY_1)) {
    objs[index].id = (++toggle+1)%2 ? cube_create() : triangle_create();
    f32 x = rand_in_range_f32(min, max);
    f32 y = rand_in_range_f32(min, max);
    f32 z = rand_in_range_f32(min, max);
    objs[index].position = v3(x, y, z);
    st->obj_count++;
  }
  if (input_was_key_down(KEY_2)) {
    entity_destroy(objs[index-1].id);
    entity_remove_renderable(objs[index-1].id);
    st->obj_count--;
  }
  
  Loop(i, st->obj_count) {
    mat4* model = (mat4*)vk_get_push_constant(objs[i].id);
    *model = mat4_translation(objs[i].position) * mat4_euler_y(rot);
    *model = mat4_euler_y(rot / 2) * mat4_translation(objs[i].position);
  }

  camera_update();
}
