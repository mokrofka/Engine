#pragma once

#include "vk_types.h"

VK_Framebuffer vk_framebuffer_create(
    VK_RenderPass renderpass,
    u32 width, u32 height, u32 attachment_count,
    VkImageView* in_attachments);
void vk_framebuffer_destroy(VK_Framebuffer* frambuffer);
