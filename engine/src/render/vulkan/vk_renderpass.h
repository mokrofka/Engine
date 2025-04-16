#pragma once
#include "vk_types.h"

VK_RenderPass vk_renderpass_create(Rect rect, v4 color, f32 depth, u32 stencil);

void vk_renderpass_destroy(VK_RenderPass renderpass);

void vk_renderpass_begin(VK_CommandBuffer* command_buffer, VK_RenderPass renderpass, VK_Framebuffer frame_buffer);

void vk_renderpass_end(VK_CommandBuffer* command_buffer);
