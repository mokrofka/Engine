#pragma once

#include "vulkan_types.inl"

void vulkan_fence_create(VulkanContext* context, b8 create_signaled, VulkanFence* out_fence);

void vulkan_fence_destroy(VulkanContext* context, VulkanFence* fence);

b8 vulkan_fence_wait(VulkanContext* context, VulkanFence* fence, u64 timeout_ns);

void vulkan_fence_reset(VulkanContext* context, VulkanFence fence);
