#include "common.h"

Vertex cube_vertices[] = {
  // Front face (0, 0, 1)
  {.pos = v3(-1.00, -1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3( 1.00, -1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3( 1.00,  1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3( 1.00,  1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3(-1.00, -1.00,  1.00), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(0.0f, 0.0f)},

  // Back face (0, 0, -1)
  {.pos = v3( 1.00, -1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3(-1.00, -1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3(-1.00,  1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3( 1.00,  1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3( 1.00, -1.00, -1.00), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(0.0f, 0.0f)},

  // Left face (-1, 0, 0)
  {.pos = v3(-1.00, -1.00, -1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3(-1.00, -1.00,  1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3(-1.00,  1.00,  1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00,  1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00, -1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3(-1.00, -1.00, -1.00),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},

  // Right face (1, 0, 0)
  {.pos = v3(1.00, -1.00,  1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3(1.00, -1.00, -1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3(1.00,  1.00, -1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(1.00,  1.00, -1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(1.00,  1.00,  1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3(1.00, -1.00,  1.00),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},

  // Bottom face (0, -1, 0)
  {.pos = v3(-1.00, -1.00, -1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3( 1.00, -1.00, -1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3( 1.00, -1.00,  1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3( 1.00, -1.00,  1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3(-1.00, -1.00,  1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3(-1.00, -1.00, -1.00),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(0.0f, 1.0f)},

  // Top face (0, 1, 0)
  {.pos = v3(-1.00,  1.00,  1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},
  {.pos = v3( 1.00,  1.00,  1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(1.0f, 0.0f)},
  {.pos = v3( 1.00,  1.00, -1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3( 1.00,  1.00, -1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(1.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00, -1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(0.0f, 1.0f)},
  {.pos = v3(-1.00,  1.00,  1.00), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(0.0f, 0.0f)},
};

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

struct Camera {
  v3 pos;
  v3 dir;
  f32 yaw;
  f32 pitch;
  f32 fov;
};

struct Entity {
  v3 vel;
  AABB aabb;
};

template<>
struct Handle<Entity> {
  u32 handle;
  Transform& trans() { return entities_transforms((Handle<Entity>)handle); }
  v3& pos() { return trans().pos; }
  v3& rot() { return trans().rot; }
  v3& scale() { return trans().scale; }
  Entity& get();
  AABB& aabb();
  v3& vel();
};

struct StaticEntity {
  u32 some;
};

template<>
struct Handle<StaticEntity> {
  u32 handle;
  Transform& trans() { return static_entities_transforms((Handle<StaticEntity>)handle); }
  v3& pos() { return static_entities_transforms((Handle<StaticEntity>)handle).pos; }
  v3& rot() { return static_entities_transforms((Handle<StaticEntity>)handle).rot; }
  v3& scale() { return static_entities_transforms((Handle<StaticEntity>)handle).scale; }
};

struct GameState {
  Arena arena;
  Arena persistent_arena;
  AllocSegList gpa;
  Camera cam;
  Timer timer;
  StaticObjectPool<Entity, MaxEntities> entity_pool;
  StaticObjectPool<StaticEntity, MaxStaticEntities> static_entity_pool;

  Darray<Handle<Entity>> entities;
  Darray<Handle<StaticEntity>> static_entities;
  DarrayHandler<Handle<Entity>> moving_cubes;

  Handle<Entity> axis_attached_to_cam;
  Handle<Entity> monkey;
  Handle<Entity> rotating_cube;
};  

global GameState* st;

Entity& Handle<Entity>::get() { return st->entity_pool.get((Handle<Entity>)handle); }
AABB& Handle<Entity>::aabb() { return get().aabb; }
v3& Handle<Entity>::vel() { return get().vel; }

////////////////////////////////////////////////////////////////////////
// Utils

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

Handle<Entity> entity_create(MeshId mesh_id, ShaderId shader_id, MaterialId material_id) {
  Handle<Entity> e = st->entity_pool.add();
  e.aabb() = { v3_scale(-1.2), v3_scale(1.2) };
  e.vel() = v3_rand_range(v3_scale(-1), v3_scale(1));
  e.trans() = {};
  e.scale() = v3_one();
  vk_make_renderable(e, mesh_get(mesh_id), shader_get(shader_id), material_get(material_id));
  return e;
}

Handle<StaticEntity> entity_static_create(MeshId mesh_id, ShaderId shader_id, MaterialId material_id) {
  Handle<StaticEntity> e = st->static_entity_pool.add();
  e.trans() = {};
  e.scale() = v3_one();
  vk_make_renderable_static(e, mesh_get(mesh_id), shader_get(shader_id), material_get(material_id));
  return e;
}

void entity_remove(Handle<Entity> e) {
  st->entity_pool.remove(e);
  vk_remove_renderable(e);
}

v3 ray_from_camera() {
  v2 mouse_pos = os_get_mouse_pos();
  v2i win_size = os_get_window_size();
  v2 norm_coords = v2(2 * (mouse_pos.x/win_size.x) - 1, 2 * -(mouse_pos.y/win_size.y) + 1);
  v4 clip_coords = v4(norm_coords.x, norm_coords.y, -1, 1);
  v4 eye_coord = mat4_inverse(vk_get_projection()) * clip_coords;
  eye_coord = v4(eye_coord.x, eye_coord.y, -1, 0);
  v3 world_coord = v3_of_v4(vk_get_view() * eye_coord);
  world_coord = v3_norm(world_coord);
  return world_coord;
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

void select_obj() {
  v3 dir = ray_from_camera();
  Handle<Entity> e = entity_create(Mesh_Cube, Shader_Color, Material_RedOrange);
  // e.pos() = st->cam.pos + v3_norm(mat4_forward(st->cam.view));
  e.pos() = st->cam.pos;
  e.scale() = v3_scale(0.3);
  e.vel() = dir * 4;
}

void camera_update() {
  Camera& cam = st->cam;
  v2 win_size = v2_of_v2i(os_get_window_size());
  mat4& projection = vk_get_projection();
  mat4& view = vk_get_view();
  projection = mat4_perspective(degtorad(cam.fov), win_size.x / win_size.y, 0.1f, 10000.0f);

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
      v3 forward = mat4_forward(view);
      velocity += forward;
    }
    if (os_is_key_down(Key_S)) {
      v3 backward = mat4_backward(view);
      velocity += backward;
    }
    if (os_is_key_down(Key_Q)) {
      v3 left = mat4_left(view);
      velocity += left;
    }
    if (os_is_key_down(Key_E)) {
      v3 right = mat4_right(view);
      velocity += right;
    }
    if (os_is_key_down(Key_Space)) {
      velocity.y += 1.0f;
    }
    if (os_is_key_down(Key_X)) {
      velocity.y -= 1.0f;
    }
    if (os_is_key_down(Key_Shift)) {
      speed *= 20;
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
  view = mat4_look_at(cam.pos, cam.dir, v3_up());
}

////////////////////////////////////////////////////////////////////////
// Init

void new_init() {
}

void game_init() {
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
  vk_get_view() = mat4_look_at(cam.pos, cam.dir, v3_up());
  Handle<Entity> cube = entity_create(Mesh_Cube, Shader_Color, Material_RedOrange);
  st->rotating_cube = cube;
  Handle<Entity> monkey = entity_create(Mesh_MonkeyGlb, Shader_Color, Material_GreenContainer);
  st->monkey = monkey;
  st->entities.add(monkey);
  {
    Handle<Entity> triangle = entity_create(Mesh_Triangle, Shader_Color, Material_RedOrange);
    triangle.pos() = v3_scale(3);
  }
  {
    Handle<Entity> grid = entity_create(Mesh_Grid, Shader_Grid, MaterialId(0));
    grid.pos() = v3(0,0,-5);
  }
  {
    st->axis_attached_to_cam = entity_create(Mesh_Axis, Shader_Axis, MaterialId(0));
  }
  {
  }

  Loop (i, 3) {
    Handle<Entity> cube = entity_create(Mesh_Cube, Shader_Color, Material_RedOrange);
    u32 range = 10;
    cube.pos() = v3_rand_range(-v3_scale(range), v3_scale(range));
  }

#if 1
  u64 start = os_now_ns();
  // Loop (i, MB(1)-KB(100)) {
  // Loop (i, KB(100)) {
  Loop (i, KB(1)) {
    Handle<StaticEntity> e = entity_static_create(Mesh_Cube, Shader_Color, Material_RedOrange);
    u32 range = KB(1);
    e.pos() = v3_rand_range(-v3_scale(range), v3_scale(range));
  }
  u64 end = os_now_ns();
  Info("%f64 s", f64(end - start)/Billion(1));
#endif
}

void game_deinit() {
  st->entities = {};
  st->entity_pool = {};
  st->entity_pool.clear();
  arena_clear(&st->arena);
}

////////////////////////////////////////////////////////////////////////
// Update

Darray<Handle<Entity>> arr;

intern void render_add() {
  Scratch scratch;
  Loop (i, 10) {
    Handle<Entity> e = entity_create(Mesh_Cube, Shader_Color, Material_GreenContainer);
    arr.add(e);
    f32 range = 10;
    e.pos() = v3_rand_range(v3_scale(range), -v3_scale(range));
  }
  for (u32 i = arr.count - 1; i > 0; --i) {
    u32 j = rand_range_u32(0, i);
    Swap(arr.data[i], arr.data[j]);
  }
}

intern void render_remove() {
  Scratch scratch;
  for (Handle<Entity> e : arr) {
    entity_remove(e);
  }
  arr.clear();
}

void game_update() {
  if (os_is_key_pressed(Key_1)) {
    render_add();
  }
  if (os_is_key_pressed(Key_2)) {
    render_remove();
  }
  st->timer.interval = 1;
  if (timer_tick(st->timer)) {
    // Info("%i", st->entities.count);
  }
  {
    Handle<Entity> cube = st->rotating_cube;
    Handle<Entity> monkey = st->monkey;
    monkey.pos().x += 0.1 * g_dt;
    cube.pos().x = monkey.pos().x + Sin(g_time) * 4;
    cube.pos().z = monkey.pos().z + Cos(g_time) * 4;
    cube.pos().y = monkey.pos().z + Cos(g_time) * 4;
  }
  if (os_is_button_pressed(MouseButton_Left)) {
    // select_obj();
    v3 dir = ray_from_camera();
    // debug_draw_line(st->cam.pos - v3(0,0.1,0), st->cam.pos + dir*100, ColorWhite);
    v3 max = st->cam.pos + v3_one();
    v3 min = st->cam.pos - v3_one();
  }
  for (Handle<Entity> e : st->entities) {
    debug_draw_aabb(e.pos()+e.aabb().min, e.pos()+e.aabb().max, ColorWhite);
  }
  {
    mat4& view = vk_get_view();
    v3 forward = mat4_forward(view);
    v3 right   = mat4_right(view);
    v3 up      = mat4_up(view);
    v2i win_size = os_get_window_size();
    f32 dist = 1.0f;
    f32 xoff = 0.3f;
    f32 yoff = 0.3f;
    Handle<Entity> axis = st->axis_attached_to_cam;
    axis.pos() = st->cam.pos + forward*dist + right*xoff + up*yoff;
    axis.scale() = v3_scale(0.1);
  }
  Loop (i, 0) {
    MeshId meshes[] = {
      Mesh_MonkeyGlb,
      Mesh_Triangle,
      Mesh_Cube,
    };
    MaterialId materials[] = {
      Material_RedOrange,
      Material_GreenContainer,
    };
    // Handle<Entity> cube = e_create(Mesh_Cube, Shader_Color, Material_RedOrange);
    Handle<Entity> e = entity_create(meshes[rand_range_u32(0, ArrayCount(meshes)-1)], Shader_Color, materials[rand_range_u32(0, ArrayCount(materials)-1)]);
    u32 range = 100;
    e.pos() = v3_rand_range(-v3_scale(range), v3_scale(range));
    st->moving_cubes.add(e);
  }
  for (Handle<Entity> e : st->moving_cubes) {
    // v3 max = v3_scale(3000);
    // if (e.pos().x > max.x || e.pos().y > max.y || e.pos().z > max.z) {
    //   e.dir() *= -1;
    // }
    e.pos() += e.vel() * g_dt;
    v3 center = {0, 0, 0};
    v3 dir = e.pos() - center;
    // perpendicular vector (2D style in XZ plane)
    v3 tangent = v3_norm(v3{-dir.z, 0, dir.x});
    // combine swirl + slight attraction
    e.vel() += tangent * 2.0f * g_dt;
    e.vel() += -dir * 0.5f * g_dt;
  }
}

void main_init(u8** state) {
  Scratch scratch;
  Allocator global_alloc = mem_get_global_allocator();
  GameState* game_st = push_struct_zero(global_alloc, GameState);
  *state = (u8*)game_st;
  st = game_st;
  *st = {
    .arena = arena_init(),
    .persistent_arena = arena_init(),
    .gpa{st->arena},
    .timer = timer_init(1),
    .entities{st->arena},
    .static_entities{st->arena},
  };
  Mesh triangle_mesh =  {
    .vertices = triangle_vertices,
    .vert_count = ArrayCount(triangle_vertices),
  };
  st->entity_pool.data = push_array(st->persistent_arena, Entity, MaxEntities);
  st->entity_pool.generations = push_array(st->persistent_arena, u32, MaxEntities);
  st->entity_pool.clear();
  entities_generations_set(st->entity_pool.generations);
  st->static_entity_pool.data = push_array(st->persistent_arena, StaticEntity, MaxStaticEntities);
  st->static_entity_pool.generations = push_array(st->persistent_arena, u32, MaxStaticEntities);
  st->static_entity_pool.clear();
  static_entities_generations_set(st->static_entity_pool.generations);
  Mesh cube_mesh = {.vertices = cube_vertices, .vert_count = ArrayCount(cube_vertices)};
  // mesh_set(Mesh_Cube, vk_mesh_load(cube_mesh));
  mesh_set(Mesh_Triangle, vk_mesh_load(triangle_mesh));
  Mesh grid_mesh = grid_create(scratch, 100, 1);
  mesh_set(Mesh_Grid, vk_mesh_load(grid_mesh));
  Mesh axis_mesh = {.vertices = axis_vertices, .vert_count = ArrayCount(axis_vertices)};
  mesh_set(Mesh_Axis, vk_mesh_load(axis_mesh));
  cubemap_load("night_cubemap");
  asset_load();
  game_init();
}

shared_function void main_update(u8** state) {
  Scratch scratch;
  if (*state == null) {
    main_init(state);
  }
  Assign(st, *state);
  camera_update();
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
  game_update();
}

void foo() {

}


