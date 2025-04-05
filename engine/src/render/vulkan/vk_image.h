#pragma once
#include "vk_types.h"

VK_Image vk_image_create(
  VK_Context* context,
  VkImageType image_type,
  u32 width,
  u32 height,
  VkFormat format,
  VkImageTiling tilling,
  VkImageUsageFlags usage,
  VkMemoryPropertyFlags memory_flags,
  b32 create_view,
  VkImageAspectFlags view_aspect_flags);
  
void vk_image_view_create(
  VK_Context* context,
  VkFormat format,
  VK_Image* image,
  VkImageAspectFlags);

void vk_image_transition_layout(
  VK_Context* context,
  VK_CommandBuffer* command_buffer,
  VK_Image* image,
  VkFormat format,
  VkImageLayout old_layout,
  VkImageLayout new_layout);

void vk_image_copy_from_buffer(
  VK_Context* context,
  VK_Image* image,
  VkBuffer buffer,
  VK_CommandBuffer* command_buffer);

void vk_image_destroy(VK_Context* context, VK_Image* image);
