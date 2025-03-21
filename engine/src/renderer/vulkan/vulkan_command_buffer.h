#pragma once
#include "vulkan_types.inl"

void vulkan_command_buffer_allocate(
    VulkanContext* context,
    VkCommandPool pool,
    b8 is_primary,
    VulkanCommandBuffer* out_command_buffer);

void vulkan_command_buffer_free(
    VulkanContext* context,
    VkCommandPool pool,
    VulkanCommandBuffer* command_buffer);
    
void vulkan_command_buffer_begin(
  VulkanCommandBuffer* command_buffer,
  b8 is_single_use,
  b8 is_renderpass_continue,
  b8 is_simultaneous_use);
  
void vulkan_command_buffer_end(VulkanCommandBuffer* command_buffer);

void vulkan_command_buffer_update_submitted(VulkanCommandBuffer* command_buffer);

void vulkan_command_buffer_reset(VulkanCommandBuffer* command_buffer);

void vulkan_command_buffer_allocate_and_begin_single_use(
  VulkanContext* context,
  VkCommandPool pool,
  VulkanCommandBuffer* out_command_buffer);
  
void vulkan_command_buffer_end_single_use(
  VulkanContext* context,
  VkCommandPool pool,
  VulkanCommandBuffer* command_buffer,
  VkQueue queue);
  