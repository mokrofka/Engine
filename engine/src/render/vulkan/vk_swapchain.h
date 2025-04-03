#pragma once

#include "vk_types.h"

void vk_swapchain_create(
  VK_Context* context,
  u32 width,
  u32 height,
  VK_Swapchain * out_swapchain);

void vk_swapchain_recreate(
  VK_Context* context,
  u32 width,
  u32 height,
  VK_Swapchain * swapchain);
  
void vk_swapchain_destroy(
  VK_Context* context,
  VK_Swapchain * swapchain);

b8 vk_swapchain_acquire_next_image_index(
  VK_Context* context,
  VK_Swapchain * swapchain,
  u64 timeout_ns,
  VkSemaphore image_available_semaphore,
  VkFence fence,
  u32* out_image_index);

void vk_swapchain_present(
  VK_Context* context,
  VK_Swapchain * swapchain,
  VkQueue graphics_queue,
  VkQueue present_queue,
  VkSemaphore render_complete_semaphore,
  u32 present_image_index);
  
  