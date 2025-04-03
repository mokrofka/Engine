#pragma once
#include "vk_types.h"

void vk_command_buffer_alloc(
    VK_Context* context,
    VkCommandPool pool,
    b8 is_primary,
    VK_CommandBuffer* out_command_buffer);

void vk_command_buffer_free(
    VK_Context* context,
    VkCommandPool pool,
    VK_CommandBuffer* command_buffer);
    
void vk_command_buffer_begin(
  VK_CommandBuffer* command_buffer,
  b8 is_single_use,
  b8 is_renderpass_continue,
  b8 is_simultaneous_use);
  
void vk_command_buffer_end(VK_CommandBuffer* command_buffer);

void vk_command_buffer_update_submitted(VK_CommandBuffer* command_buffer);

void vk_command_buffer_reset(VK_CommandBuffer* command_buffer);

void vk_command_buffer_alloc_and_begin_single_use(
  VK_Context* context,
  VkCommandPool pool,
  VK_CommandBuffer* out_command_buffer);
  
void vk_command_buffer_end_single_use(
  VK_Context* context,
  VkCommandPool pool,
  VK_CommandBuffer* command_buffer,
  VkQueue queue);
  