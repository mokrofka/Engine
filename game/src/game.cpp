#include <vendor/imgui/imgui.h>
#include "game.h"

#include <engine.h>

#include <render/r_frontend.h>
#include "sys/geometry.h"
#include "sys/shader.h"
#include "sys/texture.h"
#include "ui.h"

#include <event.h>
#include <input.h>
#include <entity.h>

#include "camera.h"
#include "game_primitives.h"

GameState* st;

void cubes_position_update() {
  Loop (i, st->cubes.count) {
    Entity& cube = st->cubes.data[i];
    cube.pos.y += 0.001;
    cube.rot.x += 0.01;
  }
}

void push_constant_update() {
  Loop (i, st->cubes.count) {
    Entity& e = st->cubes.data[i];
    PushConstant* push = vk_get_push_constant(e.id);
    push->model = mat4_transform({.pos = e.pos, .rot = e.rot, .scale = e.scale});
  }
  Loop (i, st->entities.count) {
    Entity& e = st->entities.data[i];
    PushConstant* push = vk_get_push_constant(e.id);
    push->model = mat4_transform({.pos = e.pos, .rot = e.rot, .scale = e.scale});
  }
  Loop (i, st->lights.count) {
    Entity& e = st->lights.data[i];
    PushConstant* push = vk_get_push_constant(e.id);
    push->model = mat4_transform({.pos = e.pos, .rot = e.rot, .scale = e.scale});

    ShaderEntity* shader_e = shader_get_entity_data(e.id);
    *shader_e = {
      .color = e.color,  
    };

    DirectionalLight* dir_light = shader_get_light_data(e.id);
    *dir_light = {
      .pos = e.pos,
      .direction = e.direction,
      .color = e.color,
      // .color = v3(e.color.x, e.color.y, e.color.z),
    };
  }
}

u32 cube_create() {
  Entity e = {
    .id = entity_create(),
    .pos = v3_zero(),
    .scale = v3(1),
  };
  st->cubes.insert_data(e);
  entity_make_renderable(e.id, geometry_get("cube"_), shader_get("texture_shader"_));
  return e.id;
}

void cube_destroy(Entity* e) {
  entity_destroy(e->id);
  entity_remove_renderable(e->id);
  st->cubes.remove_data(e->id);
};

Entity light_create() {
  Entity e = {
    .id = entity_create(),
    .pos = v3_zero(),
    .scale = v3(1),
  };
  st->lights.insert_data(e);
  entity_make_light(e.id);
  entity_make_renderable(e.id, geometry_get("cube"_), shader_get("color_shader"_));

  e.dir_light = shader_get_light_data(e.id);
  *e.dir_light = {
    .pos = e.pos,
    .color = v3(0),
  };
  return e;
}

void light_destroy(Entity* e) {
  entity_destroy(e->id);
  entity_remove_renderable(e->id);
  entity_remove_light(e->id);
  st->lights.remove_data(e->id);
}

void app_init(App* app) {
  Scratch scratch;

  app->state = mem_alloc(sizeof(GameState));
  Assign(st, app->state);
  st->arena = arena_alloc(app->arena, GameSize);

  st->shader_global_state = shader_get_global_state();
  st->cubes.count = 0;
  st->lights.count = 0;
  st->entities.count = 0;

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
  entity_init();

  // Mesh
  {
    Geometry cube_geom = {
      .name = "cube"_,
      .vertex_count = sizeof(cube_vertices) / sizeof(Vertex3D),
      .vertex_size = sizeof(Vertex3D),
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
    Entity e = {
      .id = entity_create(),
      .pos = v3(0,-1,0),
      .scale = 1,
    };
    entity_make_renderable(e.id, geometry_get("grid"_), shader_get("grid_shader"_));
    st->entities.insert_data(e);
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
    Entity axis = {
      .id = entity_create(),
      .pos = v3_zero(),
      .scale = v3(10),
    };
    st->entities.insert_data(axis);
    entity_make_renderable(axis.id, geometry_get("axis"_), shader_get("axis_shader"_));
  }

  // Light
  {
    Entity light = {
      .id = entity_create(),
      .pos = v3(1,1,1),
      .scale = v3(1),
    };
    st->lights.insert_data(light);
    entity_make_light(light.id);
    entity_make_renderable(light.id, geometry_get("cube"_), shader_get("color_shader"_));

    DirectionalLight* light_data = shader_get_light_data(light.id);
    *light_data = {
      .pos = light.pos,
      .color = {1,1,1},
    };
  }
  
  u32 initial_cube_count = 2;
  // Loop (i, initial_cube_count) {
  //   f32 min = -1, max = 1;
  //   Entity e = cube_create();
  //   st->cubes.insert_data(e);
  //   f32 x = rand_in_range_f32(min, max);
  //   f32 y = rand_in_range_f32(min, max);
  //   f32 z = rand_in_range_f32(min, max);

  //   Entity& cube = st->cubes.data[e.id];
  //   cube.pos = {x,y,z};
  // }
}

f32 min = -30, max = 30;
b32 toggle;
void app_update(App* app) {
  Assign(st, app->state);
  Scratch scratch;

  cubes_position_update();
  push_constant_update();
  camera_update();
  
  ShaderGlobalState* shader_state = shader_get_global_state();
  shader_state->projection_view = st->camera.projection * st->camera.view;
  shader_state->view = st->camera.view;
  shader_state->time += 0.01;
  
  if (input_was_key_down(KEY_1)) {
    u32 id = cube_create();
    f32 x = rand_in_range_f32(min, max);
    f32 y = rand_in_range_f32(min, max);
    f32 z = rand_in_range_f32(min, max);

    Entity* e = st->cubes.get_data(id);
    e->pos = {x,y,z};
  }
  if (input_was_key_down(KEY_2)) {
    Entity* e = &st->cubes.data[st->cubes.count - 1];
    cube_destroy(e);
  }

  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);
  // UI_Window(ImGui::Begin("DockSpace", null, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
  UI_Window(ImGui::Begin("DockSpace")) {
    // ImGui::DockSpace(ImGui::GetID("MyDockSpace"), ImVec2(0.0f, 0.0f), 0);
    ImGui::DockSpace(ImGui::GetID("MyDockSpace")) ;
  }

  ui_texture_render();
  
  UI_Window(ImGui::Begin("Scene Editor")) {
    ImGui::Text("Total cubes: %u", st->cubes.count);

    if (ImGui::BeginTabBar("EntityTabs")) {

      if (ImGui::BeginTabItem("Cubes")) {
        if (ImGui::Button("+ Create Cube")) {
          cube_create();
        }

        Loop (i, st->cubes.count) {
          Entity* e = &st->cubes.data[i];

          ImGui::PushID(i);
          if (ImGui::CollapsingHeader("slider")) {
            ImGui::Text("Transform");

            ImGui::DragFloat3("Position", &e->pos.x, 0.1f);

            if (ImGui::Button("Delete Cube")) {
              cube_destroy(e);
            }
          }
          ImGui::PopID();
        }

        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Lights")) {
        if (ImGui::Button("+ Create Light")) {
          light_create();
        }

        Loop (i, st->lights.count) {
          Entity* e = &st->lights.data[i];
          DirectionalLight* light = shader_get_light_data(e->id);
          String entity_name_c = "light"_;

          ImGui::PushID(i);
          if (ImGui::CollapsingHeader((char*)entity_name_c.str)) {
            ImGui::Text("entity id %i", e->id);
            ImGui::DragFloat3("Position", &e->pos.x, 0.1f);
            ImGui::DragFloat3("Direction", &e->direction.x, 0.1f);
            ImGui::DragFloat3("scale", &e->scale.x, 0.1f);
            ImGui::ColorEdit3("Color", &e->color.x);

            if (ImGui::Button("Delete Light")) {
              light_destroy(e);
            }
          }
          ImGui::PopID();
        }

        ImGui::EndTabItem();
      }

      ImGui::EndTabBar();
    }
  }

  // ImGui::ShowDemoWindow();
}

void app_on_resize(App* game_inst) {

}
