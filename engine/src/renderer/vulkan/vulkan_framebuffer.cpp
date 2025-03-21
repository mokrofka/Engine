#include "vulkan_framebuffer.h"

#include "memory.h"

void vulkan_framebuffer_create(
  VulkanContext* context,
  VulkanRenderPass* renderpass,
  u32 width,
  u32 height,
  u32 attachment_count,
  VkImageView* attachments,
  VulkanFramebuffer* out_framebuffer) {
  
  // Take a copy of the attachments, renderpass and attachment count
  // out_framebuffer->attachments = mallocate(VkImageView, attachment_count, MEMORY_TAG_RENDERER);
  for (u32 i = 0; i < attachment_count; ++i) {
    out_framebuffer->attachments[i] = attachments[i];
  }
  
  out_framebuffer->renderpass = renderpass;
  out_framebuffer->attachment_count = attachment_count;
  
  // Creation info
  VkFramebufferCreateInfo framebuffer_create_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
  framebuffer_create_info.renderPass = renderpass->handle;
  framebuffer_create_info.attachmentCount = attachment_count;
  framebuffer_create_info.pAttachments = out_framebuffer->attachments;
  framebuffer_create_info.width = width;
  framebuffer_create_info.height = height;
  framebuffer_create_info.layers = 1;
}

void vulkan_framebuffer_destroy(VulkanContext* context, VulkanFramebuffer* framebuffer);
