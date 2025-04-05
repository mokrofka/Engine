#pragma once

#include "vk_types.h"

VK_Device vk_device_create();

void vk_device_destroy();

void vk_device_query_swapchain_support(
    VkPhysicalDevice physical_device,
    VK_SwapchainSupportInfo *out_support_info);

b8 vk_device_detect_depth_format(VK_Device* device);
