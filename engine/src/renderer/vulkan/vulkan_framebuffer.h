#pragma once

#include "vulkan_types.inl"

void vulkan_framebuffer_create(
  VulkanContext* context,
  VulkanRenderPass* renderpass,
  u32 width,
  u32 height,
  u32 attachment_count,
  VkImageView* arrachments,
  VulkanFramebuffer* out_framebuffer);
  
void vulkan_framebuffer_destroy(VulkanContext* context, VulkanFramebuffer* framebuffer);
