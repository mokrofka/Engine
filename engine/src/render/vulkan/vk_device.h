#pragma once
#include "vk_types.h"

VK_Device vk_device_create();

void vk_device_destroy();

VK_SwapchainSupportInfo* vk_device_query_swapchain_support(VkPhysicalDevice physical_device, VK_SwapchainSupportInfo* support_info);

void vk_device_detect_depth_format(VK_Device* device);
