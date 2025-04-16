#include "vk_command_buffer.h"

VK_CommandBuffer vk_command_buffer_alloc(VkCommandPool pool, b8 is_primary) {
  VK_CommandBuffer command_buffer = {};
  
  VkCommandBufferAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  allocate_info.commandPool = pool;
  allocate_info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
  allocate_info.commandBufferCount = 1;
  allocate_info.pNext = 0;

  command_buffer.state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
  VK_CHECK(vkAllocateCommandBuffers(vkdevice, &allocate_info, &command_buffer.handle));

  command_buffer.state = COMMAND_BUFFER_STATE_READY;
  return command_buffer;
}

void vk_cmd_free(VkCommandPool pool, VK_CommandBuffer* command_buffer) {
  vkFreeCommandBuffers(vkdevice, pool, 1, &command_buffer->handle);

  command_buffer->handle = 0;
  command_buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

void vk_cmd_begin(
  VK_CommandBuffer* command_buffer,
  b8 is_single_use,
  b8 is_renderpass_continue,
  b8 is_simultaneous_use) {
  
  VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  begin_info.flags = 0;
  if (is_single_use) {
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  }
  if (is_renderpass_continue) {
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
  }
  if (is_simultaneous_use) {
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
  }
  
  VK_CHECK(vkBeginCommandBuffer(command_buffer->handle, &begin_info));
  command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;
}

void vk_cmd_end(VK_CommandBuffer* command_buffer) {
  VK_CHECK(vkEndCommandBuffer(command_buffer->handle));
  command_buffer->state = COMMAND_BUFFER_STATE_RECORDING_ENDED;
}

void vk_cmd_update_submitted(VK_CommandBuffer* command_buffer) {
  command_buffer->state = COMMAND_BUFFER_STATE_SUBMITTED;
}

void vk_cmd_reset(VK_CommandBuffer* command_buffer) {
  command_buffer->state = COMMAND_BUFFER_STATE_READY;
}

void vk_cmd_alloc_and_begin_single_use(VkCommandPool pool, VK_CommandBuffer* out_command_buffer) {
  *out_command_buffer = vk_command_buffer_alloc(pool, true);
  vk_cmd_begin(out_command_buffer, true, false, false);
}

void vk_cmd_end_single_use(VkCommandPool pool, VK_CommandBuffer* command_buffer, VkQueue queue) {

  // End the command buffer.
  vk_cmd_end(command_buffer);
  
  // Submit the queue
  VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer->handle;
  VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, 0));
  
  // Wait for it to finish
  VK_CHECK(vkQueueWaitIdle(queue));
  
  // Free the command buffer.
  vk_cmd_free(pool, command_buffer);
}
