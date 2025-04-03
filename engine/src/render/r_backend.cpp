#include "r_backend.h"

#include "vulkan/vk_backend.h"

b8 renderer_backend_create(R_BackendType type, R_Backend* out_renderer_backend) {
  if (type == RENDERER_BACKENED_TYPE_VULKAN) {
    out_renderer_backend->initialize = vk_r_backend_init;
    out_renderer_backend->shutdown = vk_r_backend_shutdown;
    out_renderer_backend->begin_frame = vk_r_backend_begin_frame;
    out_renderer_backend->update_global_state = vk_r_update_global_state;
    out_renderer_backend->end_frame = vk_r_backend_end_frame;
    out_renderer_backend->resized = vk_r_backend_on_resize;
    out_renderer_backend->update_object = vk_r_update_object;
    out_renderer_backend->create_texture = vk_r_create_texture;
    out_renderer_backend->destroy_texture = vk_r_destroy_texture;
    
    return true;
  }
  
  return false;
}

void renderer_backend_destroy(R_Backend* renderer_backend) {
  renderer_backend->initialize = 0;
  renderer_backend->shutdown = 0;
  renderer_backend->begin_frame = 0;
  renderer_backend->update_global_state = 0;
  renderer_backend->end_frame = 0;
  renderer_backend->resized = 0;
  renderer_backend->update_object = 0;
  renderer_backend->create_texture = 0;
  renderer_backend->destroy_texture = 0;
}
