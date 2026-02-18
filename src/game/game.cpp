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

Vertex triangle_vertices[] = {
  {.pos = v3( 0.0,   0.5, 0), .uv = v2(0.5, 1), .color = v3(1,0,0)},
  {.pos = v3(-0.5,  -0.5, 0), .uv = v2(0.0, 0), .color = v3(0,1,0)},
  {.pos = v3( 0.5,  -0.5, 0), .uv = v2(1.0, 0), .color = v3(0,0,1)},
};

Vertex axis_vertices[] = {
  {.pos = v3_zero(), .color = v3(1,0,0)},
  {.pos = v3(1,0,0), .color = v3(1,0,0)},
  {.pos = v3_zero(), .color = v3(0,1,0)},
  {.pos = v3(0,1,0), .color = v3(0,1,0)},
  {.pos = v3_zero(), .color = v3(0,0,1)},
  {.pos = v3(0,0,1), .color = v3(0,0,1)},
};

struct AABB {
  v3 min;
  v3 max;
};

struct Ray {
  v3 origin;
  v3 dir;
};

struct Entity {
  u32 id;
  v3 dir;
  AABB aabb;
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
};

struct GameState {
  Arena arena;
  AllocSegList gpa;
  IdPool id_pool;
  DarrayHandler<Entity> entities;
  Camera cam;
  Timer timer;
  u32 axis_attached_to_cam;
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

Mesh grid_create(Allocator arena, u32 size, f32 step) {
  Vertex* vertices = push_array(arena, Vertex, size*4);
  v3 pos_offset = v3(-(i32)size/2, 0, -(i32)size/2);
  for (i32 i = 0; i < size; ++i) {
    vertices[i*2].pos = pos_offset + v3(0, 0, i*step);
    vertices[i*2+1].pos = pos_offset + v3(size*step, 0, i*step);
  }
  Vertex* vertical_vertices = vertices + size*2;
  for (i32 i = 0; i < size; ++i) {
    vertical_vertices[i*2].pos = pos_offset + v3(i*step, 0, 0);
    vertical_vertices[i*2+1].pos = pos_offset + v3(i*step, 0, size*step);
  }
  Mesh mesh = {
    .vertices = vertices,
    .vert_count = size*4,
  };
  return mesh;
}

u32 entity_create(u32 mesh, u32 shader, u32 material) {
  Entity e = {
    .id = id_pool_alloc(st->id_pool) + 1,
    .aabb = {
      v3_scale(-1),
      v3_scale(1),
    }
  };
  e.trans() = {};
  e.scale() = v3_one();
  vk_make_renderable(e.id, mesh, shader, material);
  u32 id = st->entities.add(e);
  return id;
}

void camera_update() {
  Camera& cam = st->cam;
  v2 win_size = v2_of_v2i(os_get_window_size());
  cam.projection = mat4_perspective(degtorad(cam.fov), win_size.x / win_size.y, 0.1f, 1000.0f);

  // Camera rotation
  {
    f32 rotation_speed = 180.0f * g_dt;
    if (os_is_key_down(Key_A)) {
      cam.yaw += -rotation_speed;
    }
    if (os_is_key_down(Key_D)) {
      cam.yaw += rotation_speed;
    }
    if (os_is_key_down(Key_R)) {
      cam.pitch += rotation_speed;
    }
    if (os_is_key_down(Key_F)) {
      cam.pitch += -rotation_speed;
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
    }
  }
  
  // Camera update
  cam.pitch = Clamp(-89.0f, cam.pitch, 89.0f);
  cam.dir = {
    CosD(cam.yaw) * CosD(cam.pitch),
    SinD(cam.pitch),
    SinD(cam.yaw) * CosD(cam.pitch)
  };
  cam.view = mat4_look_at(cam.pos, cam.dir, v3_up());
}

v3 ray_from_camera() {
  v2 mouse_pos = os_get_mouse_pos();
  v2i win_size = os_get_window_size();
  v2 norm_coords = v2(2 * (mouse_pos.x/win_size.x) - 1, 2 * -(mouse_pos.y/win_size.y) + 1);
  v4 clip_coords = v4(norm_coords.x, norm_coords.y, -1, 1);
  v4 eye_coord = mat4_inverse(st->cam.projection) * clip_coords;
  eye_coord = v4(eye_coord.x, eye_coord.y, -1, 0);
  v3 world_coord = v3_of_v4(st->cam.view * eye_coord);
  world_coord = v3_norm(world_coord);
  return world_coord;
}

void foo() {
  debug_draw_line(v3_zero(), v3_one(), v3_one());
}

void select_obj() {
  v3 dir = ray_from_camera();

  u32 e_id = entity_create(meshes[Mesh_GlbCube], shaders[Shader_Color], materials[Material_RedOrange]);
  Entity& e = st->entities.get(e_id);
  // e.pos() = st->cam.pos + v3_norm(mat4_forward(st->cam.view));
  e.pos() = st->cam.pos;
  e.scale() = v3_scale(0.3);
  e.dir = dir * 4;

}

////////////////////////////////////////////////////////////////////////
// Init

void new_init() {
}

void scene_init() {
  Scratch scratch;
  Camera& cam = st->cam;
  cam = {
    .pos = v3(0,0,5),
    .yaw = -90,
    .fov = 45,
  };
  cam.dir = {
    CosD(cam.yaw) * CosD(cam.pitch),
    SinD(cam.pitch),
    SinD(cam.yaw) * CosD(cam.pitch)
  };
  cam.view = mat4_look_at(cam.pos, cam.dir, v3_up());
  u32 cube_id = entity_create(meshes[Mesh_GlbCube], shaders[Shader_Color], materials[Material_RedOrange]);
  Entity& cube = st->entities.get(cube_id);
  cube.pos().x = 0;
  u32 cube1_id = entity_create(meshes[Mesh_GltfCube], shaders[Shader_Color], materials[Material_GreenContainer]);
  Entity& cube1 = st->entities.get(cube1_id);
  cube1.pos() = {0,0,0};
  {
    u32 triangle_id = entity_create(meshes[Mesh_Triangle], shaders[Shader_Color], materials[Material_RedOrange]);
    Entity& e = st->entities.get(triangle_id);
    e.pos() = v3_scale(3);
  }
  {
    u32 grid_id = entity_create(meshes[Mesh_Grid], Shader_Grid, 0);
    Entity& grid = st->entities.get(grid_id);
    grid.pos() = v3(0,0,-5);
  }
  {
    st->axis_attached_to_cam = entity_create(meshes[Mesh_Axis], shaders[Shader_Axis], 0);
  }
}

void scene_deinit() {
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
  for (i32 i = 0; i < Mesh_Load_COUNT; ++i) {
    meshes[i] = mesh_load(meshes_path[i]);
  }
  Mesh triangle_mesh =  {
    .vertices = triangle_vertices,
    .vert_count = ArrayCount(triangle_vertices),
  };
  meshes[Mesh_Triangle] = vk_mesh_load(triangle_mesh);
  Mesh grid_mesh =  grid_create(scratch, 100, 1);
  meshes[Mesh_Grid] = vk_mesh_load(grid_mesh);
  Mesh axis_mesh = {
    .vertices = axis_vertices,
    .vert_count = ArrayCount(axis_vertices),
  };
  meshes[Mesh_Axis] = vk_mesh_load(axis_mesh);
  for (i32 i = 0; i < Texture_COUNT; ++i) {
    textures[i] = texture_load(textures_path[i]);
  }
  Loop (i, Material_COUNT) {
    materials_info[i].texture = textures[materials_info[i].texture];
    materials[i] = vk_material_load(materials_info[i]);
  }
  shader_load("cubemap_shader", ShaderType_Cubemap);
  cubemap_load("night_cubemap");
  scene_init();
  st->timer = timer_init(1);
  u64 end = os_now_ns();
  Info("app init took: %f64 sec", f64(end - start) / Billion(1));
}

// b32 ray_intersect_AABB(Ray ray, AABB aabb) {
//   v3 tMin = v3_hadamard_div(aabb.min - ray.origin, ray.dir);
//   v3 tMax = v3_hadamard_div(aabb.max - ray.origin, ray.dir);
//   v3 t1 = v3_less(tMin, tMax);
//   v3 t2 = v3_greater(tMin, tMax);
//   f32 tNear = Max3(t1.x, t1.y, t1.z);
//   f32 tFar = Min3(t2.x, t2.y, t2.z);
//   if (tNear > tFar) {
//     return false;
//   }
//   return true;
// };

////////////////////////////////////////////////////////////////////////
// Update

void scene_update() {
  st->timer.interval = 0.1;
  if (timer_tick(st->timer)) {
  }
  {
    Entity& e = st->entities.get(0);
    Entity& e1 = st->entities.get(1);
    // e1.pos().z += 0.1 * g_dt;
    e1.pos().x += 0.1 * g_dt;
    e.pos().x = e1.pos().x + Sin(g_time) * 4;
    e.pos().z = e1.pos().z + Cos(g_time) * 4;
    e.pos().y = e1.pos().z + Cos(g_time) * 4;
  }
  if (os_is_button_pressed(MouseButton_Left)) {
    select_obj();
  }
  for (Entity& e : st->entities) {
    e.pos() += e.dir * g_dt;
  }
  {
    v3 forward = mat4_forward(st->cam.view);
    v3 right   = mat4_right(st->cam.view);
    v3 up      = mat4_up(st->cam.view);
    v2i win_size = os_get_window_size();
    f32 dist = 1.0f;
    f32 xoff = 0.3f;
    f32 yoff = 0.3f;
    Entity& axis = st->entities.get(st->axis_attached_to_cam);
    axis.pos() = st->cam.pos + forward*dist + right*xoff + up*yoff;
    axis.scale() = v3_scale(0.1);
  }
}

shared_function void app_update(u8** state) {
  Scratch scratch;
  if (*state == null) {
    app_init(state);
  }
  Assign(st, *state);
  camera_update();
  if (os_is_key_down(Key_T)) {
    scene_deinit();
    scene_init();
  }
  if (os_is_key_pressed(Key_N)) {
    new_init();
  }
  if (os_is_key_down(Key_Escape)) {
    os_close_window();
  }
  scene_update();
  gpu_data_update();
}

void bar() {
  // a.init

  // Info("%i", arr[0]);
}
