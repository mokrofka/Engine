#pragma once
#include "vk_types.h"

VK_Image vk_image_create(
    VkImageType image_type,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tilling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memory_flags,
    b32 create_view,
    VkImageAspectFlags view_aspect_flags);

VkImageView vk_image_view_create(VkFormat format, VkImage image, VkImageAspectFlags);

void vk_image_transition_layout(VkCommandBuffer cmd, VK_Image image, VkImageLayout old_layout, VkImageLayout new_layout);

void vk_upload_image_to_gpu(VkCommandBuffer cmd, VK_Image image);

void vk_image_destroy(VK_Image image);
