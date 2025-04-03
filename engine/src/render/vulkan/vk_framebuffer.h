#pragma once

#include "vk_types.h"

void vk_framebuffer_create(
  VK_Context* context,
  VK_RenderPass * renderpass,
  u32 width,
  u32 height,
  u32 attachment_count,
  VkImageView* arrachments,
  VK_Framebuffer* out_framebuffer);
  
void vk_framebuffer_destroy(VK_Context* context, VK_Framebuffer* framebuffer);
