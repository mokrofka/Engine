#include "vk_command_buffer.h"

VkCommandBuffer vk_cmd_alloc(VkCommandPool pool) {
  VkCommandBufferAllocateInfo allocate_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1,
  };

  VkCommandBuffer result;
  VK_CHECK(vkAllocateCommandBuffers(vkdevice, &allocate_info, &result));
  return result;
}

void vk_cmd_free(VkCommandPool pool, VkCommandBuffer cmd) {
  vkFreeCommandBuffers(vkdevice, pool, 1, &cmd);
}

void vk_cmd_begin(VkCommandBuffer cmd) {
  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  VK_CHECK(vkBeginCommandBuffer(cmd, &begin_info));
}

void vk_cmd_end(VkCommandBuffer cmd) {
  VK_CHECK(vkEndCommandBuffer(cmd));
}

VkCommandBuffer vk_cmd_alloc_and_begin_single_use() {
  VkCommandBuffer result = vk_cmd_alloc(vk.device.transient_cmd_pool);
  vk_cmd_begin(result);
  return result;
}

void vk_cmd_end_single_use(VkCommandBuffer cmd) {
  vk_cmd_end(cmd);
  
  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
  };
  VK_CHECK(vkQueueSubmit(vk.device.graphics_queue, 1, &submit_info, 0));
  VK_CHECK(vkQueueWaitIdle(vk.device.graphics_queue));
  
  vk_cmd_free(vk.device.transient_cmd_pool, cmd);
}

