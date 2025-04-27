#pragma once
#include "vk_types.h"

VK_Swapchain vk_swapchain_create(u32 width, u32 height);

void vk_swapchain_recreate(VK_Swapchain* swapchain, u32 width, u32 height);

void vk_swapchain_destroy(VK_Swapchain* swapchain);

u32 vk_swapchain_acquire_next_image_index(
    VK_Swapchain swapchain,
    u64 timeout_ns,
    VkSemaphore image_available_semaphore,
    VkFence fence);
    
void vk_swapchain_present(
    VK_Swapchain* swapchain,
    VkQueue graphics_queue,
    VkQueue present_queue,
    VkSemaphore render_complete_semaphore,
    u32 present_image_index);
