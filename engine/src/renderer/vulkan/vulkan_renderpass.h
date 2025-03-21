#pragma once

#include "vulkan_types.inl"

void vulkan_renderpass_create(
    VulkanContext* context,
    VulkanRenderPass* out_renderpass,
    f32 x, f32 y, f32 w, f32 h,
    f32 r, f32 g, f32 b, f32 a,
    f32 depth,
    u32 stencil);

void vulkan_renderpass_destroy(VulkanContext* context, VulkanRenderPass* renderpass);

void vulkan_renderpass_begin(
    VulkanCommandBuffer* command_buffer, 
    VulkanRenderPass* renderpass,
    VkFramebuffer frame_buffer);

void vulkan_renderpass_end(VulkanCommandBuffer* command_buffer, VulkanRenderPass* renderpass);
