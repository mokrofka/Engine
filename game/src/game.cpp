#include <vendor/imgui/imgui.h>
#include "game_types.h"
#include "game.h"

#include <render/r_frontend.h>
#include <sys/geometry.h>
#include <sys/shader.h>
#include <sys/texture.h>
#include <ui.h>
#include <entity.h>

#include "camera.h"
#include "game_primitives.h"

GameState* st;

void cubes_position_update() {
  Loop (i, st->cubes.count) {
    Entity& cube = st->cubes.data[i];
    // cube.pos.y += 0.1;
    // cube.rot.x += 0.01;

  }
}

void push_constant_update() {
  Loop (i, st->cubes.count) {
    Entity& e = st->cubes.data[i];
    PushConstant* push = get_push_constant(e.id);
    push->model = mat4_transform({.pos = e.pos, .rot = e.rot, .scale = e.scale});
  }
  Loop (i, st->entities.count) {
    Entity& e = st->entities.data[i];
    PushConstant* push = get_push_constant(e.id);
    push->model = mat4_transform({.pos = e.pos, .rot = e.rot, .scale = e.scale});
  }
  Loop (i, st->lights.count) {
    Entity& e = st->lights.data[i];

    PushConstant* push = get_push_constant(e.id);
    push->model = mat4_transform({.pos = e.pos, .rot = e.rot, .scale = e.scale});

    ShaderEntity* shader_e = shader_get_entity(e.id);
    shader_e->color = e.color;

    PointLight* point_light = shader_get_point_light(e.id);
    point_light->color = e.color;
    point_light->pos = e.pos;
  }
}

u32 cube_create() {
  Entity e = {
    .id = entity_create(),
    .pos = v3_zero(),
    .scale = v3(1),
  };
  st->cubes.insert_data(e);
  entity_make_renderable(e.id, geometry_get("cube"), shader_get("texture_shader"));
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
    .color = v3(0.8),
  };
  st->lights.insert_data(e);
  entity_make_point_light(e.id);
  entity_make_renderable(e.id, geometry_get("cube"), shader_get("color_shader"));

  e.point_light = shader_get_point_light(e.id);
  *e.point_light = {
    .color = v3(0),
    .pos = e.pos,
  };
  return e;
}

void light_destroy(Entity* e) {
  entity_destroy(e->id);
  entity_remove_renderable(e->id);
  entity_remove_point_light(e->id);
  st->lights.remove_data(e->id);
}

void app_init(u8** state) {
  Scratch scratch;

  *state = mem_alloc(sizeof(GameState));
  Assign(st, *state);
  st->arena = mem_arena_alloc(MB(1));

  st->shader_global_state = shader_get_global_state();
  st->cubes.count = 0;
  st->lights.count = 0;
  st->entities.count = 0;

  st->camera.view_dirty = true;
  st->camera.position = v3(0,0, 10);
  st->camera.yaw = -90;
  st->camera.fov = 45;

  event_register(EventCode_ViewportResized, &st->camera, [](u32 code, void* sender, void* listener_inst, EventContext context)->b32 {
    f32 width = context.u32[0];
    f32 height = context.u32[1];
    st->camera.projection = mat4_perspective(deg_to_rad(st->camera.fov), width / height, 0.1f, 1000.0f);
    return false;
  });
  entity_init();

  // Mesh
  {
    u32 vert_count = sizeof(cube_vertices) / 32;
    Geometry cube_geom = {
      .name = "cube",
      .vertex_count = vert_count,
      .vertex_size = sizeof(Vertex3D),
      .vertices = cube_vertices,
    };
    geometry_create(cube_geom);
  }
  {
    Geometry triangle_geom = {
      .name = "triangle",
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
      .name = "grid",
      .vertex_count = grid_size*4,
      .vertex_size = sizeof(v3),
      .vertices = vertices,
    };
    geometry_create(grid);
  }
  {
    Geometry axis = {
      .name = "axis",
      .vertex_count = 6,
      .vertex_size = sizeof(axis_vertices),
      .vertices = axis_vertices,
    };
    geometry_create(axis);
  }
  
  // Shader
  {
    Shader shader = {
      .name = "texture_shader",
      .attribut = {3,3,2},
    };
    shader_create(shader);
  }
  {
    Shader shader = {
      .name = "color_shader",
      .attribut = {3,3,2},
    };
    shader_create(shader);
  }
  {
    Shader shader = {
      .name = "grid_shader",
      .primitive = ShaderTopology_Line,
      .is_transparent = true,
      .attribut = {3},
    };
    shader_create(shader);
  }
  {
    Shader shader = {
      .name = "transparent_shader",
      .is_transparent = true,
      .attribut = {3,3,2},
    };
    shader_create(shader);
  }
  {
    Shader shader = {
      .name = "axis_shader",
      .primitive = ShaderTopology_Line,
      .is_transparent = true,
      .attribut = {3, 3},
    };
    shader_create(shader);
  }
  
  // Texture
  {
    texture_load("orange_lines_512.png");
  }

  // Entity
  {
    Entity e = {
      .id = entity_create(),
      .pos = v3(0,-1,0),
      .scale = 1,
    };
    entity_make_renderable(e.id, geometry_get("grid"), shader_get("grid_shader"));
    st->entities.insert_data(e);
  }

  {
    // Entity transparent_cube = entity_create();
    // entity_make_renderable(transparent_cube, geometry_get("cube_position_vertices"), shader_get("transparent_shader"));
    // PushConstant* push = push_constant(transparent_cube);
    // push->model = mat4_translation(v3(0,0,0));
  }

  {
    // Entity LargeCube = cube_create();
    // PushConstant* push = push_constant(LargeCube);
    // push->model = mat4_translation(v3(0,0,0)) * mat4_scale(v3(100,100,100));
  }

  {
    Entity axis = {
      .id = entity_create(),
      .pos = v3_zero(),
      .scale = v3(10),
    };
    st->entities.insert_data(axis);
    entity_make_renderable(axis.id, geometry_get("axis"), shader_get("axis_shader"));
  }

  // Light
  {
    Entity light = {
      .id = entity_create(),
      .pos = v3(1,1,1),
      .scale = v3(1),
      .color = v3(0.8)
    };
    st->lights.insert_data(light);
    entity_make_point_light(light.id);
    entity_make_renderable(light.id, geometry_get("cube"), shader_get("color_shader"));

    PointLight* light_data = shader_get_point_light(light.id);
    *light_data = {
      .color = {1,1,1},
      .pos = light.pos,
    };
  }
  
  // Scene
  {
    u32 cube_id = cube_create();
    Entity* cube = st->cubes.get_data(cube_id);
    cube->pos.y = -54;
    cube->scale = v3(100);
  }
}

f32 min = -30, max = 30;

void app_update(u8* state) {
  Assign(st, state);
  Scratch scratch;

  cubes_position_update();
  push_constant_update();
  camera_update();
  
  st->shader_global_state->projection_view = st->camera.projection * st->camera.view;
  st->shader_global_state->view = st->camera.view;
  st->shader_global_state->time += 0.01;
  
  if (input_was_key_down(Key_1)) {
    u32 id = cube_create();
    f32 x = rand_in_range_f32(min, max);
    f32 y = rand_in_range_f32(min, max);
    f32 z = rand_in_range_f32(min, max);

    Entity* e = st->cubes.get_data(id);
    e->pos = {x,y,z};
  }
  if (input_was_key_down(Key_2)) {
    Entity* e = &st->cubes.data[st->cubes.count - 1];
    cube_destroy(e);
  }

  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);
  // UI_Window(ImGui::Begin("DockSpace", null, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
  UI_Window("DockSpace") {
    // ImGui::DockSpace(ImGui::GetID("MyDockSpace"), ImVec2(0.0f, 0.0f), 0);
    ImGui::DockSpace(ImGui::GetID("MyDockSpace")) ;
  }

  ui_texture_render();
  
  UI_Window("Scene Editor") {
    ImGui::Text("Total cubes: %u", st->cubes.count);

    UI_TabBar("EntityTabs") {
      UI_TabItem("Cubes") {
        if (ImGui::Button("+ Create Cube")) {
          cube_create();
        }

        Loop (i, st->cubes.count) {
          Entity* e = &st->cubes.data[i];

          ImGui::PushID(i);
          if (ImGui::CollapsingHeader("slider", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Pos", &e->pos.x, 0.1f);
            ImGui::DragFloat3("Scale", &e->scale.x, 0.1f);
            ImGui::DragFloat3("Rot", &e->rot.x, 0.1f);

            if (ImGui::Button("Delete Cube")) {
              cube_destroy(e);
            }
          }
          ImGui::PopID();
        }
      }

      UI_TabItem("Lights") {
        if (ImGui::Button("+ Create Light")) {
          light_create();
        }

        Loop (i, st->lights.count) {
          Entity* e = &st->lights.data[i];
          PointLight* light = shader_get_point_light(e->id);
          String entity_name_c = "light";

          ImGui::PushID(i);
          if (ImGui::CollapsingHeader((char*)entity_name_c.str), ImGuiTreeNodeFlags_DefaultOpen) {
            ImGui::Text("entity id %i", e->id);
            ImGui::DragFloat3("Position", &e->pos.x, 0.1f);
            ImGui::DragFloat3("scale", &e->scale.x, 0.1f);
            ImGui::ColorEdit3("Color", &e->color.x);

            if (ImGui::Button("Delete Light")) {
              light_destroy(e);
            }
          }
          ImGui::PopID();
        }

      }
    }
  }

  // ImGui::ShowDemoWindow();
}
