#include "vk_command_buffer.h"

VK_CommandBuffer vk_cmd_alloc(VkCommandPool pool, b32 is_primary) {
  VK_CommandBuffer command_buffer = {};
  
  VkCommandBufferAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  allocate_info.commandPool = pool;
  allocate_info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
  allocate_info.commandBufferCount = 1;
  allocate_info.pNext = 0;

  command_buffer.state = VK_CmdState_NotAllocated;
  VK_CHECK(vkAllocateCommandBuffers(vkdevice, &allocate_info, &command_buffer.handle));
  command_buffer.state = VK_CmdState_Ready;
  return command_buffer;
}

void vk_cmd_free(VkCommandPool pool, VK_CommandBuffer* command_buffer) {
  vkFreeCommandBuffers(vkdevice, pool, 1, &command_buffer->handle);

  command_buffer->handle = 0;
  command_buffer->state = VK_CmdState_NotAllocated;
}

void vk_cmd_begin(VK_CommandBuffer* cmd, b32 is_single_use, b32 is_renderpass_continue, b32 is_simultaneous_use) {

  VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  if (is_single_use) {
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  }
  if (is_renderpass_continue) {
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
  }
  if (is_simultaneous_use) {
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
  }
  
  VK_CHECK(vkBeginCommandBuffer(cmd->handle, &begin_info));
  cmd->state = VK_CmdState_Recording;
}

void vk_cmd_end(VK_CommandBuffer* cmd) {
  VK_CHECK(vkEndCommandBuffer(cmd->handle));
  cmd->state = VK_CmdState_RecordingEnded;
}

void vk_cmd_update_submitted(VK_CommandBuffer* cmd) {
  cmd->state = VK_CmdState_Submitted;
}

void vk_cmd_reset(VK_CommandBuffer* cmd) {
  cmd->state = VK_CmdState_Ready;
}

VK_CommandBuffer vk_cmd_alloc_and_begin_single_use() {
  VK_CommandBuffer out_cmd = vk_cmd_alloc(vk->device.graphics_cmd_pool, true);
  vk_cmd_begin(&out_cmd, true, false, false);
  return out_cmd;
}

void vk_cmd_end_single_use(VK_CommandBuffer* command_buffer) {

  // End the command buffer.
  vk_cmd_end(command_buffer);
  
  // Submit the queue
  VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer->handle;
  VK_CHECK(vkQueueSubmit(vk->device.graphics_queue, 1, &submit_info, 0));
  
  // Wait for it to finish
  VK_CHECK(vkQueueWaitIdle(vk->device.graphics_queue));
  
  // Free the command buffer.
  vk_cmd_free(vk->device.graphics_cmd_pool, command_buffer);
}

