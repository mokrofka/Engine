#pragma once
#include "vk_types.h"

VK_CommandBuffer vk_command_buffer_alloc(VkCommandPool pool, b8 is_primary);

void vk_cmd_free(VkCommandPool pool, VK_CommandBuffer* command_buffer);

void vk_cmd_begin(
  VK_CommandBuffer* command_buffer,
  b8 is_single_use,
  b8 is_renderpass_continue,
  b8 is_simultaneous_use);
  
void vk_cmd_end(VK_CommandBuffer* command_buffer);

void vk_cmd_update_submitted(VK_CommandBuffer* command_buffer);

void vk_cmd_reset(VK_CommandBuffer* command_buffer);

void vk_cmd_alloc_and_begin_single_use(VkCommandPool pool, VK_CommandBuffer* out_command_buffer);

void vk_cmd_end_single_use(VkCommandPool pool, VK_CommandBuffer* command_buffer, VkQueue queue);
