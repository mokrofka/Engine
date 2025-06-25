#pragma once
#include "vk_types.h"

void vk_swapchain_create(u32 width, u32 height);
void vk_swapchain_recreate(u32 width, u32 height);
void vk_swapchain_destroy();

u32 vk_swapchain_acquire_next_image_index(VkSemaphore image_available_semaphore);

void vk_swapchain_present(VkSemaphore render_complete_semaphore, u32 present_image_index);

void vk_surface_create();
