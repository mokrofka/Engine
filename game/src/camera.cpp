#include "camera.h"
#include "game_types.h"

Camera* camera;

void view_matrix_update() {
  camera->pitch = Clamp(-89.0f, camera->pitch, 89.0f);

  camera->direction = {
    CosD(camera->yaw) * CosD(camera->pitch),
    SinD(camera->yaw) * CosD(camera->pitch),
    SinD(camera->pitch),
  };

  camera->direction = v3_normalize(camera->direction);
  // camera->view = mat4_look_at(camera->position, camera->position + camera->direction, v3_up());
  camera->view = mat4_look_at(camera->position, v3{}, v3_up());
  camera->view_dirty = false;

}

internal void camera_yaw(f32 amount) {
  camera->yaw += amount;
  camera->view_dirty = true;
}

internal void camera_pitch(f32 amount) {
  // camera->pitch += amount;
  
  // // Clamp to avoid Gimball lock
  // f32 limit = deg_to_rad(89.0f);
  // camera->direction.x = Clamp(-limit, camera->direction.x, limit);
  
  // camera->view_dirty = true;

  camera->pitch += amount;
  camera->pitch = Clamp(-89.0f, camera->pitch, 89.0f);
  camera->view_dirty = true;
}

void camera_update() {
  camera = &st->camera;
  
  f32 rotation_speed = 180.0f;
  if (input_is_key_down(Key_A)) {
    camera_yaw(-rotation_speed * delta_time);
  }
  if (input_is_key_down(Key_D)) {
    camera_yaw(rotation_speed * delta_time);
  }
  if (input_is_key_down(Key_R)) {
    camera_pitch(rotation_speed * delta_time);
  }
  if (input_is_key_down(Key_F)) {
    camera_pitch(-rotation_speed * delta_time);
  }
  
  f32 temp_move_speed = 20.0f;
  v3 velocity = {};
  if (input_is_key_down(Key_W)) {
    v3 forward = mat4_forward(camera->view);
    velocity += forward;
  }
  if (input_is_key_down(Key_S)) {
    v3 backward = mat4_backward(camera->view);
    velocity += backward;
  }
  
  if (input_is_key_down(Key_Q)) {
    v3 left = mat4_left(camera->view);
    velocity += left;
  }
  if (input_is_key_down(Key_E)) {
    v3 right = mat4_right(camera->view);
    velocity += right;
  }
  
  if (input_is_key_down(Key_Space)) {
    velocity.y += 1.0f;
  }
  if (input_is_key_down(Key_X)) {
    velocity.y -= 1.0f;
  }
  
  v3 z = {};

  if (z != velocity) {
    // Be sure to normalize the velocity before applying speed 
    velocity = v3_normalize(velocity);
    camera->position += velocity * temp_move_speed * delta_time;
    camera->view_dirty = true;
  }
  
  if (camera->view_dirty) {
    view_matrix_update();
  }
}
