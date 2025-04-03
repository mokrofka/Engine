#pragma once

#include "vk_types.h"

b8 vk_device_create(VK_Context* context);

void vk_device_destroy(VK_Context* context);

void vk_device_query_swapchain_support(
    VkPhysicalDevice physical_device,
    VK_Context* context,
    VK_SwapchainSupportInfo *out_support_info);

b8 vk_device_detect_depth_format(VK_Device* device);
