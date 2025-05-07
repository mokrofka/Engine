#include "vk_types.h"

VkSemaphore vk_get_current_image_available_semaphore() {
  return vk->sync.image_available_semaphores[vk->frame.current_frame];
}

VkSemaphore vk_get_current_queue_complete_semaphore() {
  return vk->sync.queue_complete_semaphores[vk->frame.current_frame];
}
