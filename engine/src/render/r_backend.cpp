#include "r_backend.h"

#include "vulkan/vk_backend.h"

b8 r_backend_create(R_BackendType type, R_Backend* out_r_backend) {
  if (type == RENDERER_BACKENED_TYPE_VULKAN) {
    out_r_backend->initialize = vk_r_backend_init;
    out_r_backend->shutdown = vk_r_backend_shutdown;
    out_r_backend->begin_frame = vk_r_backend_begin_frame;
    out_r_backend->update_global_state = vk_r_update_global_state;
    out_r_backend->end_frame = vk_r_backend_end_frame;
    out_r_backend->resized = vk_r_backend_on_resize;
    out_r_backend->update_object = vk_r_update_object;
    out_r_backend->create_texture = vk_r_create_texture;
    out_r_backend->destroy_texture = vk_r_destroy_texture;
    
    return true;
  }
  
  return false;
}

void r_backend_destroy(R_Backend* r_backend) {
  r_backend->initialize = 0;
  r_backend->shutdown = 0;
  r_backend->begin_frame = 0;
  r_backend->update_global_state = 0;
  r_backend->end_frame = 0;
  r_backend->resized = 0;
  r_backend->update_object = 0;
  r_backend->create_texture = 0;
  r_backend->destroy_texture = 0;
}
