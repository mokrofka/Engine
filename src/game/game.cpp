#include "lib.h"

#include "common.h"
#include "event.h"
#include "profiler.h"

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

// f32 triangle_vertices_pos[] = {
//  -1.0f, -1.0f, 0,
//   0.0f,  1.0f, 0,
//   1.0f, -1.0f, 0,
// };

// void* grid_create(Arena* arena, i32 grid_size, f32 grid_step) {
//   v3* grid = push_array(arena, v3, grid_size*4);

//   i32 i = 0;
//   f32 half = (grid_size / 2.0f) * grid_step;
//   // horizontal from left to right
//   {
//     f32 z = 0;
//     f32 x = grid_size * grid_step;
//     Loop (j, grid_size) {
//       grid[i++] = v3(0 - half, 0, z + half);
//       grid[i++] = v3(x - half, 0, z + half);
//       z -= grid_step;
//     }
//   }

//   // vertical from top to down
//   {
//     f32 z = -grid_size * grid_step;
//     f32 x = 0;
//     Loop (j, grid_size) {
//       grid[i++] = v3(x - half, 0, 0 + half);
//       grid[i++] = v3(x - half, 0, z + half);
//       x += grid_step;
//     }
//   }
//   return grid;
// }

// struct Camera {
//   mat4 view;
//   mat4 projection;
//   b8 view_dirty;
//   v3 position;
//   v3 direction;
//   f32 yaw;
//   f32 pitch;
//   f32 fov;
// };

// struct Entity {
//   u32 id;
//   v3 pos;
//   v3 scale;
//   v3 rot;
//   v3 color;
//   PointLight point_light;
//   DirLight dir_light;
//   SpotLight spot_light;
// };

// struct SparseSetEntity {
//   Entity data[MaxEntities];
//   u32 entity_to_index[MaxEntities];
//   u32 entities[MaxEntities];
//   u32 count;
//   u32 capacity;
  
//   inline void insert_data(Entity e) {
//     u32 new_index = count;
//     entity_to_index[e.id] = new_index;
//     entities[new_index] = e.id;
//     data[new_index] = e;
//     ++count;
//   }
//   inline void add(u32 id) {
//     // Put new entry at end and update the maps
//     u32 new_index = count;
//     entity_to_index[id] = new_index;
//     entities[new_index] = id;
//     ++count;
//   }
//   inline void remove_data(u32 id) {
//     // Copy element at end into deleted element's place to maintain density
//     u32 index_of_removed_entity = entity_to_index[id];
//     u32 index_of_last_element = count - 1;
//     data[index_of_removed_entity] = data[index_of_last_element];

//     // Update map to point to moved spot
//     u32 entity_of_last_element = entities[index_of_last_element];
//     entity_to_index[entity_of_last_element] = index_of_removed_entity;
//     entities[index_of_removed_entity] = entity_of_last_element;

//     entity_to_index[id] = INVALID_ID;
//     entities[index_of_last_element] = INVALID_ID;

//     --count;
//   }
//   inline Entity& get_data(u32 id) {
//     return data[entity_to_index[id]];
//   }
// };

// struct GameState {
//   Arena* arena;
  
//   Camera camera;
  
//   SparseSetEntity cubes;
//   SparseSetEntity entities;
//   SparseSetEntity point_lights;
//   SparseSetEntity dir_lights;
//   SparseSetEntity spot_lights;

//   struct {
//     u32 entity_count;
//     u32 entities[MaxEntities];
//     b8 is_entities_alive[MaxEntities];
//   } e_st;

//   struct ShaderGlobalState* shader_global_state;
  
//   u32 grid_id;
//   u32 triangled_id;

//   b8 is_mouse_move;
// };

// GameState* st;
// u32 entity_create() {
//   u32 entity_id = st->e_st.entities[st->e_st.entity_count++];
//   st->e_st.is_entities_alive[entity_id] = true;
//   return entity_id;
// }

// void entity_destroy(u32 entity) {
//   b32 is_alive = st->e_st.is_entities_alive[entity];
//   Assert(st->e_st.is_entities_alive[entity]);
//   st->e_st.is_entities_alive[entity] = false;
//   st->e_st.entities[--st->e_st.entity_count] = entity;
// }

// void camera_update(GameState* game) {
//   Camera* st = &game->camera;

//   // Camera Rotation
//   {
//     f32 rotation_speed = 180.0f;
//     auto camera_yaw = [st](f32 amount) {
//       st->yaw += amount;
//       st->view_dirty = true;
//     };
//     if (os_is_key_down(Key_A)) {
//       camera_yaw(-rotation_speed * delta_time);
//     }
//     if (os_is_key_down(Key_D)) {
//       camera_yaw(rotation_speed * delta_time);
//     }

//     auto camera_pitch = [st](f32 amount) {
//       st->pitch += amount;
//       st->view_dirty = true;
//     };
//     if (os_is_key_down(Key_R)) {
//       camera_pitch(rotation_speed * delta_time);
//     }
//     if (os_is_key_down(Key_F)) {
//       camera_pitch(-rotation_speed * delta_time);
//     }

//   }

//   // Camera movement
//   {
//     f32 speed = 20.0f;
//     v3 velocity = {};
//     if (os_is_key_down(Key_W)) {
//       v3 forward = mat4_forward(st->view);
//       velocity += forward;
//     }
//     if (os_is_key_down(Key_S)) {
//       v3 backward = mat4_backward(st->view);
//       velocity += backward;
//     }
    
//     if (os_is_key_down(Key_Q)) {
//       v3 left = mat4_left(st->view);
//       velocity += left;
//     }
//     if (os_is_key_down(Key_E)) {
//       v3 right = mat4_right(st->view);
//       velocity += right;
//     }
    
//     if (os_is_key_down(Key_Space)) {
//       velocity.y += 1.0f;
//     }
//     if (os_is_key_down(Key_X)) {
//       velocity.y -= 1.0f;
//     }
    
//     if (velocity != v3_zero()) {
//       velocity = v3_normalize(velocity);
//       st->position += velocity * speed * delta_time;
//       st->view_dirty = true;
//     }

//     if (os_is_key_pressed(Key_1)) {
//       st->position = v3_zero();
//       st->view_dirty = true;
//     }
//   }
  
//   // Camera Update
//   if (st->view_dirty) {
//     st->pitch = Clamp(-89.0f, st->pitch, 89.0f);
//     if (st->pitch > 89) {
//       i32 a =1;
//     }
    
//     st->direction = {
//       CosD(st->yaw) * CosD(st->pitch),
//       SinD(st->pitch),
//       SinD(st->yaw) * CosD(st->pitch)
//     };
    
//     st->view = mat4_look_at(st->position, st->position + st->direction, v3_up());
//     st->view_dirty = false;
//   }
  
// }

// u32 cube_create() {
//   Entity e = {
//     .id = entity_create(),
//     .pos = v3{},
//     .scale = v3(1),
//   };
//   st->cubes.insert_data(e);
//   entity_make_renderable(e.id, geometry_get("cube").id, shader_get("texture_shader").id);
//   return e.id;
// }

// void cube_destroy(Entity* e) {
//   entity_destroy(e->id);
//   entity_remove_renderable(e->id);
//   st->cubes.remove_data(e->id);
// };

// Entity light_create() {
//   Entity e = {
//     .id = entity_create(),
//     .pos = v3{},
//     .scale = v3(1),
//     .color = v3(0.8),
//   };
//   st->point_lights.insert_data(e);
//   entity_make_point_light(e.id);
//   entity_make_renderable(e.id, geometry_get("cube").id, shader_get("color_shader").id);

//   shader_get_point_light(e.id);
//   e.point_light = {
//     .color = v3{},
//     .pos = e.pos,
//   };
//   return e;
// }

// void light_destroy(Entity* e) {
//   entity_destroy(e->id);
//   entity_remove_renderable(e->id);
//   entity_remove_point_light(e->id);
//   st->point_lights.remove_data(e->id);
// }

// void push_constant_update() {
//   Loop (i, st->cubes.count) {
//     Entity& e = st->cubes.data[i];
//     PushConstant& push = get_push_constant(e.id);
//     push.model = mat4_transform(e.pos, e.rot, e.scale);
//   }
//   Loop (i, st->entities.count) {
//     Entity& e = st->entities.data[i];
//     PushConstant& push = get_push_constant(e.id);
//     push.model = mat4_transform(e.pos, e.rot, e.scale);
//   }
//   Loop (i, st->point_lights.count) {
//     Entity& e = st->point_lights.data[i];

//     PushConstant& push = get_push_constant(e.id);
//     push.model = mat4_transform(e.pos, e.rot, e.scale);

//     ShaderEntity& shader_e = shader_get_entity(e.id);
//     shader_e.color = e.color;

//     PointLight& point_light = shader_get_point_light(e.id);
//     point_light.color = e.color;
//     point_light.pos = e.pos;
//   }
// }

// b32 perspective_projection_callback(u32 code, void* sender, void* listener_inst, EventContext context) {
//   f32 width = context.i32[0];
//   f32 height = context.i32[1];
//   st->camera.projection = mat4_perspective(deg2rad(st->camera.fov), width / height, 0.1f, 1000.0f);
//   return false;
// };

// void app_init(u8** state) {
//   Scratch scratch;
//   Assign(*state, mem_alloc_zero(sizeof(GameState)));
//   Assign(st, *state);
//   st->arena = arena_alloc();

//   st->shader_global_state = shader_get_global_state();
//   st->cubes.count = 0;
//   st->point_lights.count = 0;
//   st->entities.count = 0;

//   st->camera.view_dirty = true;
//   st->camera.position = v3(-10,0, 0);
//   st->camera.yaw = -90;
//   // st->camera.pitch = 90;
//   st->camera.fov = 45;

//   v2i frame_size = os_get_window_size();

//   // auto perspective_projection_callback = [](u32 code, void* sender, void* listener_inst, EventContext context)->b32 {
//   //   f32 width = context.i32[0];
//   //   f32 height = context.i32[1];
//   //   st->camera.projection = mat4_perspective(deg2rad(st->camera.fov), width / height, 0.1f, 1000.0f);
//   //   return false;
//   // };

//   event_register(EventCode_Resized, &st->camera, perspective_projection_callback);
//   event_fire(EventCode_Resized, null, EventContext{.i32[0] = frame_size.x, .i32[1] = frame_size.y});

//   // Entity
//   Loop (i, MaxEntities) {
//     st->e_st.entities[i] = i;
//   }
//   st->e_st.entity_count = 0;

//   // Mesh
//   {
//     u32 vert_count = sizeof(cube_vertices) / 32;
//     Geometry cube_geom = {
//       .name = "cube",
//       .vertex_count = vert_count,
//       .vertex_size = sizeof(Vertex3D),
//       .vertices = cube_vertices,
//     };
//     geometry_create(cube_geom);
//   }
//   {
//     Geometry triangle_geom = {
//       .name = "triangle",
//       .vertex_count = ArrayCount(triangle_vertices) / 8,
//       .vertex_size = sizeof(v3) + sizeof(v3) + sizeof(v2),
//       .vertices = triangle_vertices,
//     };
//     geometry_create(triangle_geom);
//   }
//   {
//     Geometry triangle_geom_pos = {
//       .name = "triangle_pos",
//       .vertex_count = ArrayCount(triangle_vertices_pos) / 3,
//       .vertex_size = sizeof(v3),
//       .vertices = triangle_vertices_pos,
//     };
//     geometry_create(triangle_geom_pos);
//   }
//   {
//     u32 grid_size = 200;
//     f32 grid_step = 1;
//     void* vertices = grid_create(scratch, grid_size, grid_step);
//     Geometry grid = {
//       .name = "grid",
//       .vertex_count = grid_size*4,
//       .vertex_size = sizeof(v3),
//       .vertices = vertices,
//     };
//     geometry_create(grid);
//   }
//   {
//     Geometry axis = {
//       .name = "axis",
//       .vertex_count = 6,
//       .vertex_size = sizeof(axis_vertices),
//       .vertices = axis_vertices,
//     };
//     geometry_create(axis);
//   }
  
//   // Shader
//   {
//     Shader shader = {
//       .name = "texture_shader",
//       .attribut = {3,3,2},
//     };
//     shader_create(shader);
//   }
//   {
//     Shader shader = {
//       .name = "color_shader",
//       .attribut = {3},
//     };
//     shader_create(shader);
//   }
//   {
//     Shader shader = {
//       .name = "grid_shader",
//       .primitive = ShaderTopology_Line,
//       .is_transparent = true,
//       .attribut = {3},
//     };
//     shader_create(shader);
//   }
//   {
//     Shader shader = {
//       .name = "transparent_shader",
//       .is_transparent = true,
//       .attribut = {3,3,2},
//     };
//     shader_create(shader);
//   }
//   {
//     Shader shader = {
//       .name = "axis_shader",
//       .primitive = ShaderTopology_Line,
//       .is_transparent = true,
//       .attribut = {3, 3},
//     };
//     shader_create(shader);
//   }

//   // Texture
//   // {
//   //   texture_load("orange_lines_512.png");
//   // }

//   // // Entity
//   {
//     Entity e = {
//       .id = entity_create(),
//       .pos = v3(0,-1,0),
//       .scale = v3(1),
//     };
//     // entity_make_renderable(e.id, geometry_get("grid").id, shader_get("grid_shader").id);
//     // entity_make_renderable(e.id, "grid", "grid_shader");
//     st->entities.insert_data(e);

//     st->grid_id = e.id;
//   }
//   // {
//   //   Entity e = {
//   //     .id = entity_create(),
//   //     .pos = v3{},
//   //     .scale = 1,
//   //   };
//   //   entity_make_renderable(e.id, geometry_get("triangle_pos").id, shader_get("color_shader").id);
//   //   st->entities.insert_data(e);

//   //   st->triangled_id = e.id;
//   // }

// }

// void app_update(u8** state) {
//   if (*state == null) {
//     app_init(state);
//   }
//   Assign(st, *state);
  
//   st->camera.fov = 65;
//   // Cube position update
//   {
//     Loop (i, st->cubes.count) {
//       Entity& cube = st->cubes.data[i];
//       cube.pos.y += 0.1 * delta_time;
//       cube.rot.x += 0.1 * delta_time;
//     }
//   }

//   push_constant_update();
//   camera_update(st);

//   st->shader_global_state->projection_view = st->camera.projection * st->camera.view;
//   st->shader_global_state->view = st->camera.view;
//   st->shader_global_state->time += 0.01;

//   if (os_is_key_down(Key_L)) {
//     Entity& e = st->entities.get_data(st->triangled_id);
//     e.pos.x += 0.1;
//   }
//   if (os_is_key_down(Key_H)) {
//     Entity& e = st->entities.get_data(st->triangled_id);
//     e.pos.x -= 0.1;
//   }
//   if (os_is_key_down(Key_I)) {
//     Entity& e = st->entities.get_data(st->triangled_id);
//     e.pos.y += 0.1;
//   }
//   if (os_is_key_down(Key_K)) {
//     Entity& e = st->entities.get_data(st->triangled_id);
//     e.pos.y -= 0.1;
//   }
  
//   if (os_was_key_down(Key_1)) {
//     f32 min = -30, max = 30;
//     u32 id = cube_create();
//     f32 x = rand_range_f32(min, max);
//     f32 y = rand_range_f32(min, max);
//     f32 z = rand_range_f32(min, max);

//     Entity& e = st->cubes.get_data(id);
//     e.pos = {x,y,z};
//   }
//   if (os_was_key_down(Key_2)) {
//     Entity* e = &st->cubes.data[st->cubes.count - 1];
//     cube_destroy(e);
//   }


// }

Vertex triangle_vertices[] = {
  {v3( 0.0,   0.5, 0), v3(), v2(0.5, 1)},
  {v3(-0.5,  -0.5, 0), v3(), v2(0.0, 0)},
  {v3( 0.5,  -0.5, 0), v3(), v2(1.0, 0)},
};

struct Entity {
  u32 id;
  // union {
  //   Transform trans;
  //   struct {
  //     v3 pos;
  //     v3 rot;
  //     v3 scale;
  //   };
  // };
  Transform& trans() { return entities_transforms[id]; }
  v3& pos() { return entities_transforms[id].pos; }
  v3& rot() { return entities_transforms[id].rot; }
  v3& scale() { return entities_transforms[id].scale; }
  u32 r_handle;
  u32 mesh;
  u32 shader;
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
  Camera camera;
};

GameState* st;

////////////////////////////////////////////////////////////////////////
// Utils

void gpu_data_update() {
  ShaderGlobalState& shader_st = *vk_get_shader_state();
  Camera& cam = st->camera;
  shader_st.projection_view = cam.projection * cam.view;
  shader_st.projection = cam.projection;
  shader_st.view = cam.view;
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

Entity& entity_create(u32 mesh, u32 shader, u32 texture) {
  Entity e = {
    .id = id_pool_alloc(st->id_pool),
    // .pos = {},
    // .scale = v3_one(),
    .r_handle = vk_make_renderable(e.id, mesh, shader, texture),
  };
  e.pos() = v3_zero();
  e.scale() = v3_one();
  return st->entities.append(e);
}

////////////////////////////////////////////////////////////////////////
// Camera

void camera_update() {
  Camera& cam = st->camera;
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
      camera_yaw(rotation_speed * delta_time);
    }
    if (os_is_key_down(Key_D)) {
      camera_yaw(-rotation_speed * delta_time);
    }
    var camera_pitch = [&](f32 amount) {
      cam.pitch += amount;
      cam.view_dirty = true;
    };
    if (os_is_key_down(Key_R)) {
      camera_pitch(rotation_speed * delta_time);
    }
    if (os_is_key_down(Key_F)) {
      camera_pitch(-rotation_speed * delta_time);
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
      cam.pos += velocity * speed * delta_time;
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

void app_init(u8** state) {
  Scratch scratch;
  Assign(*state, global_alloc_struct_zero(GameState));
  st = push_struct(scratch, GameState);
  Assign(st, *state);
  st->arena = arena_init();
  st->gpa.init(st->arena);
  st->id_pool.array.init(st->gpa);
  st->camera = {
    .pos = v3(0,0,-5),
    .yaw = 90,
    .fov = 45,
  };

  {
    Mesh triangle = {
      .vertices = triangle_vertices,
      .vert_count = ArrayCount(triangle_vertices),
    };
    vk_mesh_load(triangle);
    // Entity& e = entity_create(0, u32 shader, u32 texture)
  }

  Loop (i, Shader_COUNT) {
    shaders[i] = shader_load(shaders_definition[i].path, shaders_definition[i].type);
  }
  Loop (i, Mesh_COUNT) {
    meshes[i] = mesh_load(meshs_path[i]);
  }
  Loop (i, Texture_COUNT) {
    textures[i] = texture_load(textures_path[i]);
  }

  Entity& cube = entity_create(meshes[Mesh_Cube], shaders[Shader_Color], textures[Texture_OrangeLines]);
  Entity& cube1 = entity_create(meshes[Mesh_Cube], shaders[Shader_Color], textures[Texture_Container]);
  cube1.pos() = {-3,0,1};
  // Entity& room = entity_create(meshes[Mesh_Room], shaders[Shader_Color], textures[Texture_Room]);
  // room.pos = {0,0,10};
}

////////////////////////////////////////////////////////////////////////
// Update

shared_function void app_update(u8** state) {
  Scratch scratch;

  if (*state == null) {
    app_init(state);
  }
  Assign(st, *state);
  if (os_is_key_down(Key_Escape)) {
    os_close_window();
  }
  if (os_is_key_pressed(Key_1)) {
    st->camera.fov += 5;
  }
  if (os_is_key_pressed(Key_2)) {
    st->camera.fov -= 5;
  }

  // Local(Timer, timer, timer_init(1));
  // if (timer_tick(timer, delta_time)) {
  //   Info("entity created %i", st->entities.count);
  //   Entity& cube1 = entity_create(meshes[Mesh_Cube], shaders[Shader_Color], textures[Texture_Container]);
  //   cube1.pos = v3_rand_range(v3(-10), v3(10));
  // }
  Local(Timer, timer, timer_init(0.3));
  if (timer_tick(timer, delta_time)) {
    Info("cam yaw: %f", Mod(st->camera.yaw, 360));
    Info("block x: %f", st->entities[0].pos().x);
    v3 pos = st->camera.pos;
    Info("cam pos: %f %f %f", pos.x,pos.y,pos.z);
  }

  camera_update();
  gpu_data_update();
}
