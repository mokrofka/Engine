#include "vk_framebuffer.h"

void vk_framebuffer_create(
  VK_Context* context,
  VK_RenderPass* renderpass,
  u32 width,
  u32 height,
  u32 attachment_count,
  VkImageView* attachments,
  VK_Framebuffer* out_framebuffer) {
  
  // Take a copy of the attachments, renderpass and attachment count
  // out_framebuffer->attachments = mallocate(VkImageView, attachment_count, MEMORY_TAG_RENDERER);
  out_framebuffer->attachments = push_array(context->arena, VkImageView, attachment_count);
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
  
  VK_CHECK(vkCreateFramebuffer(
    vkdevice, 
    &framebuffer_create_info, 
    context->allocator, 
    &out_framebuffer->handle));
}

void vk_framebuffer_destroy(VK_Context* context, VK_Framebuffer* framebuffer) {
  vkDestroyFramebuffer(vkdevice, framebuffer->handle, context->allocator);
  if (framebuffer->attachments) {
     
  }
  framebuffer->handle = 0;
  framebuffer->attachment_count = 0;
  framebuffer->renderpass = 0;
}
