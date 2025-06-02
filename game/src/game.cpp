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

struct Position {
  f32 x,y,z; 
};
Component(Position)

struct Velocity {
  f32 value;
};
Component(Velocity)

f32 rotation_speed = 0.01;
f32 speed;

// System(PositionUpdate, "Position", "Velocity")
System(PositionUpdate, Position Velocity)
// System(PositionUpdate, Pos Vel)
void position_update() {
  BaseSystem* system = system_get(PositionUpdate);
  Loop (i, system->entity_count) {
    Position* pos = entity_get_component(system->entities[i], Position);
    Velocity* vel = entity_get_component(system->entities[i], Velocity);
    // v4 pos_v4 = (mat4_euler_y(rotation_speed) * v4(pos->x, pos->y, pos->z, 0));
    // *pos = {pos_v4.x, pos_v4.y, pos_v4.z};
    speed = 0.01;
    // pos->x += SinD(rotation_speed);
    pos->y += speed;
  }
}

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

v3 cube_position_vertices[] = {
  // Position             // Tex coord
  // Front face
  {-0.5f, -0.5f,  0.5f},
  { 0.5f, -0.5f,  0.5f},
  { 0.5f,  0.5f,  0.5f},
  { 0.5f,  0.5f,  0.5f},
  {-0.5f,  0.5f,  0.5f},
  {-0.5f, -0.5f,  0.5f},

  // Back face
  { 0.5f, -0.5f, -0.5f},
  {-0.5f, -0.5f, -0.5f},
  {-0.5f,  0.5f, -0.5f},
  {-0.5f,  0.5f, -0.5f},
  { 0.5f,  0.5f, -0.5f},
  { 0.5f, -0.5f, -0.5f},

  // Left face
  {-0.5f, -0.5f, -0.5f},
  {-0.5f, -0.5f,  0.5f},
  {-0.5f,  0.5f,  0.5f},
  {-0.5f,  0.5f,  0.5f},
  {-0.5f,  0.5f, -0.5f},
  {-0.5f, -0.5f, -0.5f},

  // Right face
  { 0.5f, -0.5f,  0.5f},
  { 0.5f, -0.5f, -0.5f},
  { 0.5f,  0.5f, -0.5f},
  { 0.5f,  0.5f, -0.5f},
  { 0.5f,  0.5f,  0.5f},
  { 0.5f, -0.5f,  0.5f},

  // Bottom face
  {-0.5f, -0.5f, -0.5f},
  { 0.5f, -0.5f, -0.5f},
  { 0.5f, -0.5f,  0.5f},
  { 0.5f, -0.5f,  0.5f},
  {-0.5f, -0.5f,  0.5f},
  {-0.5f, -0.5f, -0.5f},

  // Top face
  {-0.5f,  0.5f,  0.5f},
  { 0.5f,  0.5f,  0.5f},
  { 0.5f,  0.5f, -0.5f},
  { 0.5f,  0.5f, -0.5f},
  {-0.5f,  0.5f, -0.5f},
  {-0.5f,  0.5f,  0.5f},
};

f32 triangle_vertices[] = {
  // position          // color
  0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,  // Vertex 1: red
 -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,  // Vertex 2: green
  0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f   // Vertex 3: blue
};

Entity cube_create() {
  Entity e = entity_create();
  component_add(e, Position, Position{});
  component_add(e, Velocity, Velocity{0.01});
  entity_make_renderable(e, geometry_get("cube"_), shader_get("texture_shader"_));
  return e;
}

Entity triangle_create() {
  Entity e = entity_create();
  component_add(e, Position, Position{});
  component_add(e, Velocity, Velocity{0.01});
  entity_make_renderable(e, geometry_get("triangle"_), shader_get("color_shader"_));
  return e;
}

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

  // app->state = push_struct(app->arena, GameState);
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
    Geometry cube_geom = {
      .name = "cube_position_vertices"_,
      .vertex_count = ArrayCount(vertices),
      .vertex_size = sizeof(Vertex),
      .vertices = cube_position_vertices,
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
    Shader shader = {
      .name = "texture_shader"_,
      .attribut = {3,2},
    };
    shader_create(shader, &st->entities_ubo, sizeof(UBO), sizeof(PushConstant));
  }
  {
    Shader shader = {
      .name = "color_shader"_,
      .attribut = {3,3},
    };
    shader_create(shader, &st->entities_ubo, sizeof(UBO), sizeof(PushConstant));
  }
  {
    Shader shader = {
      .name = "grid_shader"_,
      .primitive = ShaderTopology_Line,
      .is_transparent = true,
      .attribut = {3},
    };
    shader_create(shader, &st->entities_ubo, sizeof(UBO), sizeof(PushConstant));
  }
  {
    Shader shader = {
      .name = "transparent_shader"_,
      .is_transparent = true,
      .attribut = {3},
    };
    shader_create(shader, &st->entities_ubo, sizeof(UBO), sizeof(PushConstant));
  }
  
  // texture_load("container.jpg"_);
  // texture_load("paving.png"_);
  texture_load("orange_lines_512.png"_);

  {
    Entity grid = entity_create();
    entity_make_renderable(grid, geometry_get("grid"_), shader_get("grid_shader"_));
    mat4* position = (mat4*)vk_get_push_constant(grid);
    *position = mat4_translation(v3(0,0,0));
  }

  {
    Entity transparent_cube = entity_create();
    entity_make_renderable(transparent_cube, geometry_get("cube_position_vertices"_), shader_get("transparent_shader"_));
    mat4* position = (mat4*)vk_get_push_constant(transparent_cube);
    *position = mat4_translation(v3(0,0,0));
  }

  // {
  //   Entity LargeCube = cube_create();
  //   mat4* position = (mat4*)vk_get_push_constant(LargeCube);
  //   *position = mat4_translation(v3(0,0,0)) * mat4_scale(v3(100,100,100));
  //   st->entities[st->entity_count] = LargeCube;
  // }
  
  f32 min = -1, max = 1;
  u32 count = 2;
  for (i32 i = st->entity_count; i < st->entity_count + count; ++i) {
    st->entities[i] = cube_create();
    f32 x = rand_in_range_f32(min, max);
    f32 y = rand_in_range_f32(min, max);
    f32 z = rand_in_range_f32(min, max);
    Position* pos = entity_get_component(st->entities[i], Position);
    *pos = {x,y,z};
  }
  st->entity_count += count;

  // Global_Data_Shader* global_shader_data = shader_get_global_state();
  // PerFrame_Data_ShaderName* per_frame_shader_data = shader_get_data_per_frame(u32 shader_id);
  // PerEntity_Data_ShaderName* per_entity_shader_data = shader_get_data_per_entity(u32 entity_id);
  // PushConstant_ShaderName* push_constant = shader_get_push_constant(u32 entity_id);
}

f32 min = -30, max = 30;
b32 toggle;
void app_update(App* app) {
  Assign(st, app->state);
  Entity* entities = st->entities;
  
  st->entities_ubo[0].projection_view = st->camera.projection * st->camera.view;
  
  f32& rot = st->rot;
  rot += 0.01;
  if (input_was_key_down(KEY_1)) {
    entities[st->entity_count] = (++toggle+1)%2 ? cube_create() : triangle_create();
    f32 x = rand_in_range_f32(min, max);
    f32 y = rand_in_range_f32(min, max);
    f32 z = rand_in_range_f32(min, max);
    Position* pos = entity_get_component(entities[st->entity_count], Position);
    *pos = {x,y,z};
    ++st->entity_count;
  }
  if (input_was_key_down(KEY_2)) {
    entity_destroy(entities[st->entity_count-1]);
    entity_remove_renderable(entities[st->entity_count-1]);
    --st->entity_count;
  }
  
  Loop (i, st->entity_count) {
    mat4* model = (mat4*)vk_get_push_constant(entities[i]);
    Position* pos = entity_get_component(entities[i], Position);
    // *model = mat4_euler_y(rot / 2) * mat4_translation(v3(pos->x, pos->y, pos->z));
    *model = mat4_translation(v3(pos->x, pos->y, pos->z));
  }

  position_update();

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
    ImGui::Text("objects: %u", st->entity_count);
    // f32 framerate = ImGui::GetIO().Framerate;
    // ImGui::Text("fps: %f", framerate);
  }
  
  // ImGui::ShowDemoWindow();

  camera_update();

}

void app_on_resize(App* game_inst) {

}
