#include "vk_types.h"

VK_Framebuffer vk_framebuffer_create(
    VK_RenderPass renderpass,
    u32 width, u32 height, u32 attachment_count,
    VkImageView* attachments) {
  VK_Framebuffer framebuffer = {};
  Loop (i, attachment_count) {
    framebuffer.attachments[i] = attachments[i];
  }

  framebuffer.attachment_count = attachment_count;
  
  // Creation info
  VkFramebufferCreateInfo framebuffer_create_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
  framebuffer_create_info.renderPass = renderpass.handle;
  framebuffer_create_info.attachmentCount = attachment_count;
  framebuffer_create_info.pAttachments = framebuffer.attachments;
  framebuffer_create_info.width = width;
  framebuffer_create_info.height = height;
  framebuffer_create_info.layers = 1;
  
  VK_CHECK(vkCreateFramebuffer(
    vkdevice, 
    &framebuffer_create_info, 
    vk->allocator, 
    &framebuffer.handle)); 

  return framebuffer;
}

void vk_framebuffer_destroy(VK_Framebuffer* frambuffer) {
  if (!frambuffer->handle) {
    Error("You don't have frame buffer handle"_);
  }
  vkDestroyFramebuffer(vkdevice, frambuffer->handle, vk->allocator);
  MemZeroStruct(frambuffer);
}
