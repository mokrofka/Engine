#include "common.h"

// f32 cube_vertices[] = {
//   // Pos                 // Normal           // Texcoord
//   // Front face (0, 0, 1)
//   -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
//    0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
//    0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
//    0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
//   -0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
//   -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,

//   // Back face (0, 0, -1)
//    0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,
//   -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,
//   -0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,
//   -0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,
//    0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f,
//    0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,

//   // Left face (-1, 0, 0)
//   -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
//   -0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
//   -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
//   -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
//   -0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
//   -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,

//   // Right face (1, 0, 0)
//    0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
//    0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
//    0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
//    0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
//    0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
//    0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,

//   // Bottom face (0, -1, 0)
//   -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,
//    0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f,
//    0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
//    0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
//   -0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f,
//   -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,

//   // Top face (0, 1, 0)
//   -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
//    0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
//    0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
//    0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
//   -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
//   -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
// };

// f32 triangle_vertices[] = {
//   // position          // color
//   0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,  0,0,  // Vertex3D 1: red
//  -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,  0,0,  // Vertex3D 2: green
//   0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,  0,0,  // Vertex3D 3: blue
// };

// f32 axis_vertices[] = {
//   // Position       // Color
//   0, 0, 0,          1, 0, 0,  // Line from (0,0,0) to (1,0,0) - X axis (red)
//   1, 0, 0,          1, 0, 0,

//   0, 0, 0,          0, 1, 0,  // Line from (0,0,0) to (0,1,0) - Y axis (green)
//   0, 1, 0,          0, 1, 0,

//   0, 0, 0,          0, 0, 1,  // Line from (0,0,0) to (0,0,1) - Z axis (blue)
//   0, 0, 1,          0, 0, 1,
// };

Vertex triangle_vertices[] = {
  {v3( 0.0,   0.5, 0), v3(), v2(0.5, 1), v3(1, 0.0, 0)},
  {v3(-0.5,  -0.5, 0), v3(), v2(0.0, 0), v3(0.0, 1, 0)},
  {v3( 0.5,  -0.5, 0), v3(), v2(1.0, 0), v3(0.0, 0.0, 1)},
};

// Vertex triangle_vertices[] = {
//   {v3( 0.0,   0.5, 0), v3(), v2(0.5, 1)},
//   {v3(-0.5,  -0.5, 0), v3(), v2(0.0, 0)},
//   {v3( 0.5,  -0.5, 0), v3(), v2(1.0, 0)},
// };

struct Entity {
  u32 id;
  Transform& trans() { return entities_transforms[id]; }
  v3& pos() { return entities_transforms[id].pos; }
  v3& rot() { return entities_transforms[id].rot; }
  v3& scale() { return entities_transforms[id].scale; }
};

struct Camera {
  mat4 view;
  mat4 projection;
  v3 pos;
  v3 dir;
  f32 yaw;
  f32 pitch;
  f32 fov;
  b8 view_dirty;
};

struct GameState {
  Arena arena;
  AllocSegList gpa;
  IdPool id_pool;
  Darray<Entity> entities;
  Camera cam;
  Timer timer;
};

global GameState* st;

////////////////////////////////////////////////////////////////////////
// Utils

void gpu_data_update() {
  ShaderGlobalState& shader_st = *vk_get_shader_state();
  Camera& cam = st->cam;
  shader_st.projection_view = cam.projection * cam.view;
  shader_st.projection = cam.projection;
  shader_st.view = cam.view;
  shader_st.time = os_timer_now();
  shader_st.ambient_color = v4(1,2,3,4);
}

void* grid_create(Allocator arena, i32 grid_size, f32 grid_step) {
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

Entity& entity_create(u32 mesh, u32 shader, u32 material) {
  Entity e = {
    .id = id_pool_alloc(st->id_pool) + 1,
  };
  e.pos() = v3_zero();
  e.scale() = v3_one();
  vk_make_renderable(e.id, mesh, shader, material);
  return st->entities.add(e);
}

////////////////////////////////////////////////////////////////////////
// Camera

void camera_update() {
  Camera& cam = st->cam;
  v2 win_size = v2_of_v2i(os_get_window_size());
  cam.projection = mat4_perspective(degtorad(cam.fov), win_size.x / win_size.y, 0.1f, 1000.0f);

  // Camera Rotation
  {
    f32 rotation_speed = 180.0f;
    var camera_yaw = [&](f32 amount) {
      cam.yaw += amount;
      cam.view_dirty = true;
    };
    if (os_is_key_down(Key_A)) {
      camera_yaw(rotation_speed * g_dt);
    }
    if (os_is_key_down(Key_D)) {
      camera_yaw(-rotation_speed * g_dt);
    }
    var camera_pitch = [&](f32 amount) {
      cam.pitch += amount;
      cam.view_dirty = true;
    };
    if (os_is_key_down(Key_R)) {
      camera_pitch(rotation_speed * g_dt);
    }
    if (os_is_key_down(Key_F)) {
      camera_pitch(-rotation_speed * g_dt);
    }
  }

  // Camera movement
  {
    f32 speed = 10.0f;
    v3 velocity = {};
    if (os_is_key_down(Key_W)) {
      v3 forward = mat4_forward(cam.view);
      velocity += forward;
    }
    if (os_is_key_down(Key_S)) {
      v3 backward = mat4_backward(cam.view);
      velocity += backward;
    }
    if (os_is_key_down(Key_Q)) {
      v3 left = mat4_left(cam.view);
      velocity += left;
    }
    if (os_is_key_down(Key_E)) {
      v3 right = mat4_right(cam.view);
      velocity += right;
    }
    if (os_is_key_down(Key_Space)) {
      velocity.y += 1.0f;
    }
    if (os_is_key_down(Key_X)) {
      velocity.y -= 1.0f;
    }
    if (velocity != v3_zero()) {
      velocity = v3_norm(velocity);
      cam.pos += velocity * speed * g_dt;
      cam.view_dirty = true;
    }
  }
  
  // Camera Update
  cam.pitch = Clamp(-89.0f, cam.pitch, 89.0f);
  cam.dir = {
    CosD(cam.yaw) * CosD(cam.pitch),
    SinD(cam.pitch),
    SinD(cam.yaw) * CosD(cam.pitch)
  };
  cam.view = mat4_look_at(cam.pos, cam.dir, v3_up());
  cam.view_dirty = false;
}

////////////////////////////////////////////////////////////////////////
// Init

void new_init() {
}

void game_init() {
  Camera& cam = st->cam;
  cam = {
    .pos = v3(0,0,-5),
    .dir = v3(0,0,1),
    .yaw = 90,
    .fov = 45,
  };
  cam.view = mat4_look_at(cam.pos, cam.dir, v3_up());
  Entity& cube = entity_create(meshes[Mesh_GlbCube], shaders[Shader_Color], materials[Material_RedOrange]);
  Entity& cube1 = entity_create(meshes[Mesh_GltfCube], shaders[Shader_Color], materials[Material_GreenContainer]);
  cube1.pos() = {-4,0,1};
  {
    Mesh triangle = {
      .vertices = triangle_vertices,
      .vert_count = ArrayCount(triangle_vertices),
    };
    u32 mesh = vk_mesh_load(triangle);
    Entity& e = entity_create(mesh, shaders[Shader_Color], materials[Material_RedOrange]);
    e.pos() = 3;
  }
}

void game_deinit() {
  for (Entity e : st->entities) {
    vk_remove_renderable(e.id);
  }
  st->entities.clear();
  arena_clear(&st->arena);
}

void app_init(u8** state) {
  u64 start = os_now_ns();
  Scratch scratch;
  Allocator global_alloc = mem_get_global_allocator();
  Assign(*state, push_struct_zero(global_alloc, GameState));
  Assign(st, *state);
  st->arena = arena_init();
  st->gpa.init(st->arena);
  st->id_pool.array.init(st->gpa);
  for (i32 i = 0; i < Shader_COUNT; ++i) {
    shaders[i] = shader_load(shaders_info[i].path, shaders_info[i].type);
  }
  for (i32 i = 0; i < Mesh_COUNT; ++i) {
    meshes[i] = mesh_load(meshes_path[i]);
  }
  for (i32 i = 0; i < Texture_COUNT; ++i) {
    textures[i] = texture_load(textures_path[i]);
  }
  Loop (i, Material_COUNT) {
    materials_info[i].texture = textures[materials_info[i].texture];
    materials[i] = (MaterialId)vk_material_load(materials_info[i]);
  }
  shader_load("cubemap_shader", ShaderType_Cubemap);
  cubemap_load("night_cubemap");
  game_init();
  st->timer = timer_init(1);
  u64 end = os_now_ns();
  Info("app init took: %f64 sec", f64(end - start) / Billion(1));
}

void select_obj() {
  v2 mouse_pos = os_get_mouse_pos();
  v2i win_size = os_get_window_size();
  v2 normalized_coords =  v2(2*mouse_pos.x / win_size.x - 1, -2*mouse_pos.y / win_size.y - 1);
  v4 clip_coords = v4(normalized_coords.x, normalized_coords.y, -1, 1);
  
  if (timer_tick(st->timer)) {
    // Info("%f, %f", x, y);
  }
}

////////////////////////////////////////////////////////////////////////
// Update

f32 fast_sin(f32 x) {
  x = Mod(x, Tau);
  if (x < 0) {
    x = -x;
  }
  float x2 = x * x;
  return x * (1.0f - x2 * (1.0f / 6.0f - x2 / 120.0f));
}

shared_function void app_update(u8** state) {
  Scratch scratch;
  if (*state == null) {
    app_init(state);
  }
  Assign(st, *state);
  if (os_is_key_down(Key_T)) {
    game_deinit();
    game_init();
  }
  if (os_is_key_pressed(Key_N)) {
    new_init();
  }
  if (os_is_key_down(Key_Escape)) {
    os_close_window();
  }
  st->timer.interval = 0.1;
  if (timer_tick(st->timer)) {
    f32 sin = Sin(g_time);
    f32 f_sin = fast_sin(g_time);
    // f32 basic_sin = sin_basic_approximation(g_time);
    // Info("sin = %f", sin);
    // Info("fast sin: %f, relative error: %f", f_sin, (sin - f_sin)/sin);
    // Info("basic approx sin: %f, relative error: %f", basic_sin, (sin - basic_sin)/sin);
    // v2 mouse_pos = os_get_mouse_pos();
    // Info("%f %f", mouse_pos.x, mouse_pos.y);
    // v2i win_size = os_get_window_size();
    // Info("%i %i", win_size.x, win_size.y);
  }

  Entity& e = st->entities[0];
  e.pos().x = Sin(g_time);
  // e.scale() = Sin(g_time);
  e.rot() = Sin(g_time);
  select_obj();
  camera_update();
  gpu_data_update();

}
