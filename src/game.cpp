#include "common.h"
#include "vk.h"

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

Mesh sphere_generate(Allocator arena) {
  u32 lat_steps = 10;
  u32 lon_steps = 10;
  u32 vert_count = lat_steps*lon_steps;
  u32 index_count = (lat_steps - 1) * lon_steps * 6;
  Vertex* vertices = push_array(arena, Vertex, vert_count);
  u32* indices = push_array(arena, u32, index_count);
  f32 lat_step_angle = PI / lat_steps;
  f32 lon_step_angle = 2*PI / lon_steps;
  for (u32 i = 0; i < lat_steps; ++i) {
    f32 lat_angle = -PI/2 + i*lat_step_angle;
    for (u32 j = 0; j < lon_steps; ++j) {
      f32 lon_angle = j * lon_step_angle;
      Vertex vert = {
        .pos.x = Cos(lat_angle) * Cos(lon_angle),
        .pos.y = Sin(lat_angle),
        .pos.z = Cos(lat_angle) * Sin(lon_angle),
        .uv.x = (f32)j / (lon_steps - 1),  // 0 → 1 across longitude
        .uv.y = (f32)i / (lat_steps - 1),  // 0 → 1 from bottom to top
      };
      vertices[i*lon_steps + j] = vert;
    }
  }
  var idx = [&](u32 i, u32 j) {
    return i * lon_steps + j;
  };
  u32 k = 0;
  for (u32 i = 0; i < lat_steps - 1; ++i) {
    for (u32 j = 0; j < lon_steps; ++j) {
      u32 next_j = (j + 1) % lon_steps; // wrap around
      u32 v0 = idx(i, j);
      u32 v1 = idx(i, next_j);
      u32 v2 = idx(i + 1, j);
      u32 v3 = idx(i + 1, next_j);
      indices[k++] = v0;
      indices[k++] = v2;
      indices[k++] = v1;
      indices[k++] = v1;
      indices[k++] = v2;
      indices[k++] = v3;
    }
  }
  u32 north_pole_index = vert_count - 1; // last vertex
  for (u32 j = 0; j < lon_steps; ++j) {
    u32 next_j = (j + 1) % lon_steps;
    indices[k++] = idx(lat_steps - 2, j); // last row before pole
    indices[k++] = north_pole_index;      // pole
    indices[k++] = idx(lat_steps - 2, next_j);
  }
  Mesh mesh = {
    .vert_count = lat_steps * lon_steps,
    .vertices = vertices,
    .index_count = index_count,
    .indices = indices,
  };
  return mesh;
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

////////////////////////////////////////////////////////////////////////
// Utils

Handle<Entity> entity_create(MeshId mesh_id, MaterialId material_id) {
  GameState& g = g_st->game;
  u32 e_id = g.entity_id_pool.alloc();
  Handle<Entity> e = {e_id};
  e.trans() = {};
  e.scale() = v3_one();
  vk_make_renderable(e, mesh_get(mesh_id), material_get(material_id));
  return e;
}

Handle<StaticEntity> entity_static_create(MeshId mesh_id, MaterialId material_id) {
  GameState& g = g_st->game;
  u32 e_id = g.static_entity_id_pool.alloc();
  Handle<StaticEntity> e = {e_id};
  e.trans() = {};
  e.scale() = v3_one();
  vk_make_renderable_static(e, mesh_get(mesh_id), material_get(material_id));
  return e;
}

void entity_remove(Handle<Entity> e) {
  GameState& g = g_st->game;
  g.entity_id_pool.free(e.handle);
  vk_remove_renderable(e);
}

v3 ray_from_camera() {
  v2 mouse_pos = os_get_mouse_pos();
  v2u win_size = os_get_window_size();
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
  GameState& g = g_st->game;
  v3 dir = ray_from_camera();
  Handle<Entity> e = entity_create(Mesh_Cube, Material_Orange);
  // e.pos() = st->cam.pos + v3_norm(mat4_forward(st->cam.view));
  e.pos() = g.cam.pos;
  e.scale() = v3_scale(0.3);
  e.vel() = dir * 4;
}

void camera_update() {
  GameState& g = g_st->game;
  Camera& cam = g.cam;
  v2 win_size = v2_of_v2u(os_get_window_size());
  mat4& projection = vk_get_projection();
  mat4& view = vk_get_view();
  projection = mat4_perspective(degtorad(cam.fov), win_size.x / win_size.y, 0.1f, 10000.0f);

  // Camera rotation
  {
    f32 rotation_speed = 180.0f * get_dt();
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
    if (os_is_key_down(Key_LAlt)) {
      speed *= 0.1;
    }
    if (velocity != v3_zero()) {
      velocity = v3_norm(velocity);
      cam.pos += velocity * speed * get_dt();
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

Darray<Handle<Entity>> arr;

intern void render_add() {
  Scratch scratch;
  Loop (i, 10) {
    Handle<Entity> e = entity_create(Mesh_Cube, Material_Container);
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

////////////////////////////////////////////////////////////////////////
// Init

void scene_init() {
  Scratch scratch;
  GameState& g = g_st->game;
  Camera& cam = g.cam;
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
  Handle<Entity> cube = entity_create(Mesh_Cube, Material_Orange);
  g.rotating_cube = cube;
  Handle<Entity> monkey = entity_create(Mesh_MonkeyGlb, Material_Container);
  monkey.aabb() = {v3_scale(-1.2), v3_scale(1.2)};
  g.monkey = monkey;
  {
    Handle<Entity> triangle = entity_create(Mesh_Triangle, Material_Orange);
    triangle.pos() = v3_scale(3);
  }
  {
    Handle<Entity> grid = entity_create(Mesh_Grid, Material_Line);
    g.grid = grid;
    vk_set_entity_color(grid, v4_scale(0.6));
    grid.pos() = v3(0,0,-5);
  }
  {
    g.axis_attached_to_cam = entity_create(Mesh_Axis, Material_Axis);
  }
  Loop (i, 3) {
    Handle<Entity> cube = entity_create(Mesh_Cube, Material_Orange);
    u32 range = 10;
    cube.pos() = v3_rand_range(-v3_scale(range), v3_scale(range));
  }
#if 1
  u64 start = os_now_ns();
  // Loop (i, MB(1)-KB(1)) {
  // Loop (i, KB(100)) {
  Loop (i, KB(1)) {
    // Handle<StaticEntity> e = entity_static_create(Mesh_Cube, Material_Orange);
    // u32 range = KB(1);
    // e.pos() = v3_rand_range(-v3_scale(range), v3_scale(range));

    MeshId meshes[] = {
      // Mesh_MonkeyGlb,
      // Mesh_Triangle,
      Mesh_Cube,
    };
    MaterialId materials[] = {
      Material_Orange,
      // Material_Container,
      // Material_Screen,
    };
    Handle<StaticEntity> e = entity_static_create(meshes[rand_range_u32(0, ArrayCount(meshes)-1)], materials[rand_range_u32(0, ArrayCount(materials)-1)]);
    u32 range = KB(1);
    e.pos() = v3_rand_range(-v3_scale(range), v3_scale(range));

  }
  u64 end = os_now_ns();
  Info("%f64 s", f64(end - start)/Billion(1));
#endif

  {
    g.sphere = entity_create(Mesh_Sphere, Material_Container);
    g.sphere.pos() = v3(0,0,-10);
  }
  {
    // Handle<Entity> e = entity_create(Mesh_Castle, Shader_E_Texture, Material_Castle);
    // e.pos().z = -100;
  }
  {
    // Loop (i, KB(1)) {
    //   Handle<Entity> e = entity_create(Mesh_Cube, Material_Screen);
    //   u32 range = 100;
    //   e.pos() = v3_rand_range(-v3_scale(range), v3_scale(range));
    // }
  }
  {
    // Loop (i, KB(400)) {
    // Loop (i, MB(1)-KB(1)) {
    Loop (i, 0) {
      Handle<Entity> e = entity_create(Mesh_Cube, Material_Container);
      u32 range = KB(1);
      e.pos() = v3_rand_range(-v3_scale(range), v3_scale(range));
      g.moving_cubes.add(e);
    }
  }

  Loop (i, 0) {
    MeshId meshes[] = {
      Mesh_MonkeyGlb,
      Mesh_Triangle,
      Mesh_Cube,
    };
    MaterialId materials[] = {
      // Material_Orange,
      Material_Container,
      // Material_Screen,
    };
    Handle<Entity> e = entity_create(meshes[rand_range_u32(0, ArrayCount(meshes)-1)], materials[rand_range_u32(0, ArrayCount(materials)-1)]);
    u32 range = 100;
    e.pos() = v3_rand_range(-v3_scale(range), v3_scale(range));
    g.moving_cubes.add(e);
  }
}

void scene_deinit() {
  // st->entity_pool = {};
  // st->entity_pool.clear();
  // arena_clear(&st->arena);
}

////////////////////////////////////////////////////////////////////////
// Update

void scene_update() {
  Scratch scratch;
  GameState& g = g_st->game;
  if (os_is_key_pressed(Key_1)) {
    render_add();
  }
  if (os_is_key_pressed(Key_2)) {
    render_remove();
  }
  if (os_is_key_pressed(MouseKey_Left)) {
    // select_obj();
    // v3 dir = ray_from_camera();
    // debug_draw_line(st->cam.pos - v3(0,0.1,0), st->cam.pos + dir*100, ColorWhite);
    // v3 max = st->cam.pos + v3_one();
    // v3 min = st->cam.pos - v3_one();
  }
  // moving cube and monkey
  {
    Handle<Entity> cube = g.rotating_cube;
    Handle<Entity> monkey = g.monkey;
    monkey.pos().x += 0.1 * get_dt();
    cube.pos().x = monkey.pos().x + Sin(get_time()) * 4;
    cube.pos().z = monkey.pos().z + Cos(get_time()) * 4;
    cube.pos().y = monkey.pos().z + Cos(get_time()) * 4;
  }
  {
    Handle<Entity> e = g.monkey;
    vk_draw_aabb(e.pos()+e.aabb().min, e.pos()+e.aabb().max, ColorWhite);
  }
  {
    mat4& view = vk_get_view();
    v3 forward = mat4_forward(view);
    v3 right   = mat4_right(view);
    v3 up      = mat4_up(view);
    f32 dist = 1.0f;
    f32 xoff = 0.3f;
    f32 yoff = 0.3f;
    Handle<Entity> axis = g.axis_attached_to_cam;
    axis.pos() = g.cam.pos + forward*dist + right*xoff + up*yoff;
    axis.scale() = v3_scale(0.1);
  }

  ///////////////////////////////////
  // Random creating and moving stuff
  Loop (i, 0) {
    MeshId meshes[] = {
      // Mesh_MonkeyGlb,
      // Mesh_Triangle,
      Mesh_Cube,
    };
    MaterialId materials[] = {
      Material_Orange,
      // Material_Container,
      // Material_Screen,
    };
    Handle<Entity> e = entity_create(meshes[rand_range_u32(0, ArrayCount(meshes)-1)], materials[rand_range_u32(0, ArrayCount(materials)-1)]);
    u32 range = 100;
    e.pos() = v3_rand_range(-v3_scale(range), v3_scale(range));
    g.moving_cubes.add(e);
  }
  for (Handle<Entity> e : g.moving_cubes) {
    e.pos() += e.vel() * get_dt();
    v3 center = {0, 0, 0};
    v3 dir = e.pos() - center;
    v3 tangent = v3_norm(v3{-dir.z, 0, dir.x});
    e.vel() += tangent * 2.0f * get_dt();
    e.vel() += -dir * 0.5f * get_dt();
  }
}

void game_init() {
  GameState& g = g_st->game;
  Scratch scratch;
  g.arena = arena_init_named("game arena");
  g.persistent_arena = arena_init_named("persistent game arena");
  g.gpa.init(g.arena, "game gpa");
  g.timer = timer_init(1);
  g.entity_id_pool.init(g.persistent_arena, MaxEntities);
  g.static_entity_id_pool.init(g.persistent_arena, MaxStaticEntities);
  g.entities = push_array(g.persistent_arena, Entity, MaxEntities);
  g.static_entities = push_array(g.persistent_arena, StaticEntity, MaxStaticEntities);

  g.gpa_arena0.init(g.arena, "gpu_arena0");
  g.gpa_arena1.init(g.arena, "gpu_arena1");

  // g.gpa_gpa0.init(g.gpa, "gpu_gpu0");
  // g.gpa_gpa1.init(g.gpa, "gpu_gpu1");

  // Mesh cube_mesh = {.vertices = cube_vertices, .vert_count = ArrayCount(cube_vertices)};
  // mesh_set(Mesh_Cube, vk_mesh_load(cube_mesh));
  Mesh triangle_mesh =  {.vertices = triangle_vertices, .vert_count = ArrayCount(triangle_vertices)};
  mesh_set(Mesh_Triangle, vk_mesh_load(triangle_mesh));
  Mesh grid_mesh = grid_create(scratch, 100, 1);
  mesh_set(Mesh_Grid, vk_mesh_load(grid_mesh));
  Mesh axis_mesh = {.vertices = axis_vertices, .vert_count = ArrayCount(axis_vertices)};
  mesh_set(Mesh_Axis, vk_mesh_load(axis_mesh));
  Mesh sphere = sphere_generate(scratch);
  mesh_set(Mesh_Sphere, vk_mesh_load(sphere));
  cubemap_load("night_cubemap");
  asset_load();
  scene_init();
}

void foo();
void game_update() {
  // foo();
  TimeFunction;
  var& g = g_st->game;
  if (os_is_key_pressed(Key_0)) {
    arena_clear(&g.arena);
  }
  // push_array(g.arena, u32, 100);
  if (timer_tick(g.timer)) {
    push_array(g.gpa, u32, 10000);
    // push_array(g.gpa_arena0, u32, 200);
    // push_array(g.gpa_arena1, u32, 300);
    // push_array(g.gpa_gpa0, u32, 100);
    // push_array(g.gpa_gpa1, u32, 150);
  }
  // if (os_is_key_pressed(Key_X)) {
  //   arena_clear(&g.arena);
  // }
  Scratch scratch;
  camera_update();
  if (os_is_key_down(Key_T)) {
    scene_deinit();
    scene_init();
  }
  if (os_is_key_pressed(Key_N)) {
  }
  if (os_is_key_down(Key_Escape)) {
    os_close_window();
  }
  scene_update();
}

struct Thing {
  Thing* first;
  Thing* last;
  Thing* next;
  Thing* prev;
  Thing* parent;
  i32 data;
};

void foo() {
  Scratch scratch;
  Thing* thing = push_struct_zero(scratch, Thing);
  Thing* thing1 = push_struct_zero(scratch, Thing);
  Thing* thing2 = push_struct_zero(scratch, Thing);
  Thing* thing3 = push_struct_zero(scratch, Thing);
  Thing* thing4 = push_struct_zero(scratch, Thing);
  Thing* thing5 = push_struct_zero(scratch, Thing);
  Thing* thing6 = push_struct_zero(scratch, Thing);
  Thing* thing7 = push_struct_zero(scratch, Thing);

  thing->data = 0;
  thing1->data = 1;
  thing2->data = 2;
  thing3->data = 3;
  thing4->data = 4;
  thing5->data = 5;
  thing6->data = 6;
  thing7->data = 7;

  Thing* first = null;
  Thing* last = null;

  #if 0
  SLLStackPush(head, thing1);
  SLLStackPush(head, thing2);
  SLLStackPush(head, thing3);

  for EachNode(it, Thing, head) {
    Info("%i", it->data);
  }

  SLLStackPop(head);
  SLLStackPop(head);
  for EachNode(it, Thing, head) {
    Info("%i", it->data);
  }
  #endif

  #if 0
  SLLQueuePush(first, last, thing);
  SLLQueuePush(first, last, thing1);
  SLLQueuePush(first, last, thing2);
  SLLQueuePush(first, last, thing3);

  for EachNode (it, Thing, first) {
    Info("%i", it->data);
  }
  #endif

  DLLPushBack(first, last, thing);
  DLLPushBack(first, last, thing1);
  DLLPushBack(first, last, thing2);
  DLLPushBack(first, last, thing3);

  DLLPushFront(first, last, thing4);
  DLLPushFront(first, last, thing5);
  DLLPushFront(first, last, thing6);
  DLLPushFront(first, last, thing7);

  DLLRemove(first, last, thing5);

  for EachNode (it, Thing, first) {
    Info("%i", it->data);
  }

}
