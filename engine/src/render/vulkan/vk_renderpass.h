#pragma once

#include "vk_types.h"

void vk_renderpass_create(
    VK_Context* context,
    VK_RenderPass * out_renderpass,
    f32 x, f32 y, f32 w, f32 h,
    f32 r, f32 g, f32 b, f32 a,
    f32 depth,
    u32 stencil);

void vk_renderpass_destroy(VK_Context* context, VK_RenderPass * renderpass);

void vk_renderpass_begin(
    VK_CommandBuffer* command_buffer, 
    VK_RenderPass * renderpass,
    VkFramebuffer frame_buffer);

void vk_renderpass_end(VK_CommandBuffer* command_buffer, VK_RenderPass * renderpass);
