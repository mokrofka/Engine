#include "camera.h"
#include "game.h"
#include "input.h"
#include "engine.h"
#include "render/r_frontend.h"

void view_matrix_update() {
  if (st->camera.view_dirty) {
    // mat4 translation = mat4_translation(st->camera.position);
    // mat4 rotaion = mat4_euler_xyz(st->camera.direction.x, st->camera.direction.y, st->camera.direction.z);
    
    // st->camera.view = translation * rotaion;
    // st->camera.view = mat4_inverse(st->camera.view);
    
    
    Camera* camera = &st->camera;
    st->camera.pitch = Clamp(-89.0f, st->camera.pitch, 89.0f);

    st->camera.direction.x = CosD(camera->yaw) * CosD(camera->pitch);
    st->camera.direction.z = SinD(camera->yaw) * CosD(camera->pitch);
    st->camera.direction.y = SinD(camera->pitch);
    st->camera.direction = v3_normalize(st->camera.direction);
    st->camera.view_dirty = true;
    
    v3 target = st->camera.position + st->camera.direction;
    st->camera.view = mat4_look_at(st->camera.position, target, v3(0,1,0));

    st->camera.view_dirty = false;
  }
}

internal void camera_yaw(f32 amount) {
  st->camera.yaw += amount;
  st->camera.view_dirty = true;
}

internal void camera_pitch(f32 amount) {
  st->camera.pitch += amount;
  
  // Clamp to avoid Gimball lock
  f32 limit = deg_to_rad(89.0f);
  st->camera.direction.x = Clamp(-limit, st->camera.direction.x, limit);
  
  st->camera.view_dirty = true;
}

void camera_update() {
  Camera* camera = &st->camera;
  v2i frame_size = os_get_framebuffer_size();
  // v2 frame_size = get_viewport_size();
  st->camera.projection = mat4_perspective(deg_to_rad(st->camera.fov), (f32)frame_size.x / frame_size.y, 0.1f, 1000.0f);
  
  if (input_was_key_pressed(KEY_T)) {
    st->is_mouse_move = !st->is_mouse_move;
    if (st->is_mouse_move) {
      os_mouse_enable();
    } else {
      os_mouse_disable();
    }
  }

  i32 pos_x, pos_y;
  input_get_mouse_position(&pos_x, &pos_y);

  f32 rotation_speed = 180.0f;
  if (input_is_key_down(KEY_A)) {
    camera_yaw(-rotation_speed * delta_time);
  }
  if (input_is_key_down(KEY_D)) {
    camera_yaw(rotation_speed * delta_time);
  }
  if (input_is_key_down(KEY_R)) {
    camera_pitch(rotation_speed * delta_time);
  }
  if (input_is_key_down(KEY_F)) {
    camera_pitch(-rotation_speed * delta_time);
  }
  
  f32 temp_move_speed = 20.0f;
  v3 velocity = {};
  if (input_is_key_down(KEY_W)) {
    v3 forward = mat4_forward(camera->view);
    velocity += forward;
  }
  if (input_is_key_down(KEY_S)) {
    v3 backward = mat4_backward(camera->view);
    velocity += backward;
  }
  
  if (input_is_key_down(KEY_Q)) {
    v3 left = mat4_left(camera->view);
    velocity += left;
  }
  if (input_is_key_down(KEY_E)) {
    v3 right = mat4_right(camera->view);
    velocity += right;
  }
  
  if (input_is_key_down(KEY_SPACE)) {
    velocity.y += 1.0f;
  }
  if (input_is_key_down(KEY_X)) {
    velocity.y -= 1.0f;
  }
  
  v3 z = v3_zero();
  if (z != velocity) {
    // Be sure to normalize the velocity before applying speed 
    velocity = v3_normalize(velocity);
    camera->position += velocity * temp_move_speed * delta_time;
    camera->view_dirty = true;
  }
  
  view_matrix_update();
}
