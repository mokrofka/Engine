#pragma once

#include "vulkan_types.inl"

b8 vulkan_device_create(VulkanContext* context);

void vulkan_device_destroy(VulkanContext* context);

void vulkan_device_query_swapchain_support(
    VkPhysicalDevice physical_device,
    VulkanContext* context,
    VulkanSwapchainSupportInfo *out_support_info);

b8 vulkan_device_detect_depth_format(VulkanDevice* device);
