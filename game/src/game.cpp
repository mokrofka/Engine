#include "vendor/imgui/imgui.h"
#include "game.h"

#include <engine.h>

#include <render/r_frontend.h>
#include "sys/geometry.h"
#include "sys/shader_sys.h"
#include "sys/texture.h"
#include "ui.h"
#include "ecs.h"

#include <event.h>
#include <input.h>

#include "object.h"
#include "camera.h"

GameState* st;

struct Vertex {
  v3 position;
  v3 normal;
  v2 tex_coord;
};

f32 cube_vertices[] = {
  // Front face (0, 0, 1)
  -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
   0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
   0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
   0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
  -0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
  -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,

  // Back face (0, 0, -1)
   0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,
  -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,
  -0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,
  -0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,
   0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f,
   0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,

  // Left face (-1, 0, 0)
  -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
  -0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
  -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
  -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
  -0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,

  // Right face (1, 0, 0)
   0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
   0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
   0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
   0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
   0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
   0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,

  // Bottom face (0, -1, 0)
  -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,
   0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f,
   0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
   0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
  -0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f,
  -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,

  // Top face (0, 1, 0)
  -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
   0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
   0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
   0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
  -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
  -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
};

f32 triangle_vertices[] = {
  // position          // color
  0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,  // Vertex 1: red
 -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,  // Vertex 2: green
  0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f   // Vertex 3: blue
};

f32 axis_vertices[] = {
  // Position        // Color
   0, 0, 0,          1, 0, 0,  // Line from (0,0,0) to (1,0,0) - X axis (red)
   1, 0, 0,          1, 0, 0,

   0, 0, 0,          0, 1, 0,  // Line from (0,0,0) to (0,1,0) - Y axis (green)
   0, 1, 0,          0, 1, 0,

   0, 0, 0,          0, 0, 1,  // Line from (0,0,0) to (0,0,1) - Z axis (blue)
   0, 0, 1,          0, 0, 1,
};

Component(Transform)

System(PositionUpdate, Transform)

void position_update() {
  BaseSystem* system = system_get(PositionUpdate);
  Loop (i, system->entity_count) {
    Transform* trans = entity_get_component(system->entities[i], Transform);
    // trans->pos.y += 0.01;

    trans->rot.x += 0.01;
    trans->rot.y += 0.01;
    trans->rot.z += 0.01;
  }
}

Entity cube_create() {
  Entity e = entity_create();
  // component_add(e, Position, Position{});
  component_set(e, Transform, Transform{.pos = v3_zero(), .scale = 1});
  entity_make_renderable(e, geometry_get("cube"_), shader_get("texture_shader"_));

  ShaderEntity* entity_shader_data = shader_get_entity_data(e);
  entity_shader_data->intensity = rand_f32_01();
  return e;
}

Entity light_create() {
  Entity light = entity_create(str_lit64("light"));
  entity_make_light(light);
  entity_make_renderable(light, geometry_get("cube"_), shader_get("color_shader"_));
  component_set(light, Transform, Transform{.pos = v3_zero(), .scale = 0.3});
  tag_add(light, DirectionalLight);

  PushConstant* push = vk_get_push_constant(light);
  push->model = mat4_translation(v3(0, 0, 0)) * mat4_scale(0.2);

  ShaderEntity* entity_data = shader_get_entity_data(light);
  entity_data->intensity = 0.9;

  DirectionalLight* light_data = shader_get_light_data(light);
  light_data->pos = v3(0, 0, 0);
  light_data->color = v3(0.5, 0.5, 0.5);

  ShaderGlobalState* shader_global_state = shader_get_global_state();
  ++shader_global_state->light_count;
  return light;
}

void cube_destroy(Entity e) {
  entity_destroy(e);
  entity_remove_renderable(e);
};

void* grid_create(Arena* arena, i32 grid_size, f32 grid_step) {
  v3* grid = push_array(arena, v3, grid_size*4);

  i32 i = 0;
  f32 half = (grid_size / 2.0f) * grid_step;
  // horizontal from left to right
  {
    f32 z = 0;
    f32 x = grid_size * grid_step;
    Loop (j, grid_size) {
      grid[i++] = v3(0 - half, 0, z + half);
      grid[i++] = v3(x - half, 0, z + half);
      z -= grid_step;
    }
  }

  // vertical from top to down
  {
    f32 z = -grid_size * grid_step;
    f32 x = 0;
    Loop (j, grid_size) {
      grid[i++] = v3(x - half, 0, 0 + half);
      grid[i++] = v3(x - half, 0, z + half);
      x += grid_step;
    }
  }
  return grid;
}

void app_init(App* app) {
  Scratch scratch;

  app->state = mem_alloc(sizeof(GameState));
  Assign(st, app->state);
  st->arena = arena_alloc(app->arena, GameSize);

  st->entity_count = 0;
  st->camera.view_dirty = true;
  st->camera.position = v3(0,0, 10);
  st->camera.yaw = -90;
  st->camera.fov = 45;

  event_register(EventCode_ViewportResized, &st->camera, [](u32 code, void* sender, void* listener_inst, EventContext context)->b32 {
    f32 width = context.data.u32[0];
    f32 height = context.data.u32[1];
    st->camera.projection = mat4_perspective(deg_to_rad(st->camera.fov), width / height, 0.1f, 1000.0f);
    return false;
  });
 
  // Mesh
  {
    Geometry cube_geom = {
      .name = "cube"_,
      .vertex_count = sizeof(cube_vertices) / sizeof(Vertex),
      .vertex_size = sizeof(Vertex),
      .vertices = cube_vertices,
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
  {
    u32 grid_size = 200;
    f32 grid_step = 1;
    void* vertices = grid_create(scratch, grid_size, grid_step);
    Geometry grid = {
      .name = "grid"_,
      .vertex_count = grid_size*4,
      .vertex_size = sizeof(v3),
      .vertices = vertices,
    };
    geometry_create(grid);
  }
  {
    Geometry axis = {
      .name = "axis"_,
      .vertex_count = 6,
      .vertex_size = sizeof(axis_vertices),
      .vertices = axis_vertices,
    };
    geometry_create(axis);
  }
  
  // Shader
  {
    Shader shader = {
      .name = "texture_shader"_,
      .attribut = {3,3,2},
    };
    shader_create(shader);
  }
  {
    Shader shader = {
      .name = "color_shader"_,
      .attribut = {3,3,2},
    };
    shader_create(shader);
  }
  {
    Shader shader = {
      .name = "grid_shader"_,
      .primitive = ShaderTopology_Line,
      .is_transparent = true,
      .attribut = {3},
    };
    shader_create(shader);
  }
  {
    Shader shader = {
      .name = "transparent_shader"_,
      .is_transparent = true,
      .attribut = {3,3,2},
    };
    shader_create(shader);
  }
  {
    Shader shader = {
      .name = "axis_shader"_,
      .primitive = ShaderTopology_Line,
      .is_transparent = true,
      .attribut = {3, 3},
    };
    shader_create(shader);
  }
  
  // Texture
  {
    texture_load("orange_lines_512.png"_);
  }

  // Entity
  {
    Entity grid = entity_create();
    entity_make_renderable(grid, geometry_get("grid"_), shader_get("grid_shader"_));
    PushConstant* push = vk_get_push_constant(grid);
    push->model = mat4_translation(v3(0,-1,0));
  }

  {
    // Entity transparent_cube = entity_create();
    // entity_make_renderable(transparent_cube, geometry_get("cube_position_vertices"_), shader_get("transparent_shader"_));
    // PushConstant* push = vk_get_push_constant(transparent_cube);
    // push->model = mat4_translation(v3(0,0,0));
  }

  {
    // Entity LargeCube = cube_create();
    // PushConstant* push = vk_get_push_constant(LargeCube);
    // push->model = mat4_translation(v3(0,0,0)) * mat4_scale(v3(100,100,100));
  }

  {
    Entity axis = entity_create();
    entity_make_renderable(axis, geometry_get("axis"_), shader_get("axis_shader"_));
    PushConstant* push = vk_get_push_constant(axis);
    push->model = mat4_translation(v3(0,0,0)) * mat4_scale(10);
  }

  // Light
  {
    Entity light = entity_create(str_lit64("light"));
    entity_make_light(light);
    entity_make_renderable(light, geometry_get("cube"_), shader_get("color_shader"_));
    component_set(light, Transform, Transform{.pos = v3_zero(), .scale = 0.3});
    tag_add(light, DirectionalLight);

    PushConstant* push = vk_get_push_constant(light);
    push->model = mat4_translation(v3(0,0,0)) * mat4_scale(0.2);

    ShaderEntity* entity_data = shader_get_entity_data(light);
    entity_data->intensity = 0.9;

    DirectionalLight* light_data = shader_get_light_data(light);
    light_data->direction = v3(-10,0,0);
      light_data->pos = v3(0,0,0);
      light_data->color = v3(0.5,0.5,0.5);
      light_data->direction = v3(0,0,1);

    ShaderGlobalState* shader_global_state = shader_get_global_state();
    shader_global_state->light_count = 1;
  }
  
  Loop (i, 2) {
    f32 min = -1, max = 1;
    st->entities[i] = cube_create();
    f32 x = rand_in_range_f32(min, max);
    f32 y = rand_in_range_f32(min, max);
    f32 z = rand_in_range_f32(min, max);
    Transform* trans = entity_get_component(st->entities[i], Transform);
    trans->pos = {x,y,z};
    ++st->entity_count;
  }

}

f32 min = -30, max = 30;
b32 toggle;
void app_update(App* app) {
  Assign(st, app->state);
  Scratch scratch;
  
  ShaderGlobalState* shader_state = shader_get_global_state();
  shader_state->g_projection_view = st->camera.projection * st->camera.view;
  shader_state->time += 0.01;
  
  if (input_was_key_down(KEY_1)) {
    st->entities[st->entity_count] = cube_create();
    f32 x = rand_in_range_f32(min, max);
    f32 y = rand_in_range_f32(min, max);
    f32 z = rand_in_range_f32(min, max);
    Transform* trans = entity_get_component(st->entities[st->entity_count], Transform);
    trans->pos = {x,y,z};

    ++st->entity_count;
  }
  if (input_was_key_down(KEY_2)) {
    entity_destroy(st->entities[st->entity_count-1]);
    entity_remove_renderable(st->entities[st->entity_count-1]);
    --st->entity_count;
  }

  position_update();

  camera_update();

  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);
  UI_Window(ImGui::Begin("DockSpace", null, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
  // UI_Window(ImGui::Begin("DockSpace")) {
    // ImGui::DockSpace(ImGui::GetID("MyDockSpace"), ImVec2(0.0f, 0.0f), 0);
    ImGui::DockSpace(ImGui::GetID("MyDockSpace")) ;
  }

  ui_texture_render();
  
  UI_Window(ImGui::Begin("window")) {
    ImGui::Text("entity count: %u", ecs.entity_count);
    if (ImGui::Button("create cube")) {
      cube_create();
    }
    if (ImGui::Button("create light")) {
      light_create();
    }
    ComponentArray* transform_array = component_get_array(component_get_id(Transform));

    Loop (i, transform_array->size) {
      Entity e = transform_array->index_to_entity[i];

      ImGui::PushID(i); // Make ImGui ID unique per entity

      // Expandable tree node per entity
      String entity_name_c = push_str_copy(scratch, ecs.entity_names[e]);
      if (ImGui::TreeNode((char*)entity_name_c.str)) {
        Transform* trans = entity_get_component(e, Transform);

        ImGui::DragFloat("x: ", &trans->pos.x, 0.05f);
        ImGui::DragFloat("y: ", &trans->pos.y, 0.05f);
        ImGui::DragFloat("z: ", &trans->pos.z, 0.05f);

        ImGui::TreePop();
        if (ImGui::Button("delete cube")) {
          cube_destroy(e);
        }
      }

      ImGui::PopID();
    }
  }

  // ImGui::ShowDemoWindow();
}

void app_on_resize(App* game_inst) {

}
