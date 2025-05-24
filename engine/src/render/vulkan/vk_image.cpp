#include "vk_image.h"
#include "vk_buffer.h"
#include "vk_command_buffer.h"

VK_Image vk_image_create(
    VkImageType image_type,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memory_flags,
    b32 create_view,
    VkImageAspectFlags view_aspect_flags) {
  // Copy params
  VK_Image result = {};
  result.width = width;
  result.height = height;

  // Creation info.
  VkImageCreateInfo image_create_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  image_create_info.imageType = VK_IMAGE_TYPE_2D;
  image_create_info.extent.width = width;
  image_create_info.extent.height = height;
  image_create_info.extent.depth = 1; // TODO: Support configurable depth.
  image_create_info.mipLevels = 1;    // TODO: Support mip mapping
  image_create_info.arrayLayers = 1;  // TODO: Support number of layers in the image.
  image_create_info.format = format;
  image_create_info.tiling = tiling;
  image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_create_info.usage = usage;
  image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;         // TODO: Configurable sample count.
  image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // TODO: Configurable sharing mode.

  VK_CHECK(vkCreateImage(vkdevice, &image_create_info, vk.allocator, &result.handle));

  // Query memory requirements.
  VkMemoryRequirements memory_requirements;
  vkGetImageMemoryRequirements(vkdevice, result.handle, &memory_requirements);

  i32 memory_type = vk_find_memory_index(memory_requirements.memoryTypeBits, memory_flags);
  Assert(memory_type != -1 && "Required memory type not found. Image not valid");

  // Allocate memory
  VkMemoryAllocateInfo memory_allocate_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  memory_allocate_info.allocationSize = memory_requirements.size;
  memory_allocate_info.memoryTypeIndex = memory_type;
  VK_CHECK(vkAllocateMemory(vkdevice, &memory_allocate_info, vk.allocator, &result.memory));

  // Bind the memory
  VK_CHECK(vkBindImageMemory(vkdevice, result.handle, result.memory, 0)); // TODO: configurable memory offset.

  // Create view
  if (create_view) {
    result.view = vk_image_view_create(format, result.handle, view_aspect_flags);
  }
  
  return result;
}

VkImageView vk_image_view_create(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags) {
  VkImageView result;
  VkImageViewCreateInfo view_create_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  view_create_info.image = image;
  view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D; // TODO: Make configurable.
  view_create_info.format = format;
  view_create_info.subresourceRange.aspectMask = aspect_flags;

  // TODO: Make configurable
  view_create_info.subresourceRange.baseMipLevel = 0;
  view_create_info.subresourceRange.levelCount = 1;
  view_create_info.subresourceRange.baseArrayLayer = 0;
  view_create_info.subresourceRange.layerCount = 1;

  VK_CHECK(vkCreateImageView(vkdevice, &view_create_info, vk.allocator, &result));
  return result;
}

void vk_image_transition_layout(
    VkCommandBuffer cmd,
    VK_Image image,
    VkImageLayout old_layout,
    VkImageLayout new_layout) {
  VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = vk.device.graphics_queue_index;
  barrier.dstQueueFamilyIndex = vk.device.graphics_queue_index;
  barrier.image = image.handle;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  
  VkPipelineStageFlags source_stage;
  VkPipelineStageFlags dest_stage;

  // Don't care about the old layout - transition to optimal layout (for the underlying implementation).
  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    // Don't care what stage the pipeline is in at the start.
    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    // Used for copying
    dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    // Transitioning from a transfer destination layout to a shader-readonly layout.
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    // From a copying stage to...
    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    // The fragment stage.
    dest_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    Fatal("unsupported layout transition"_);
    return;
  }

  vkCmdPipelineBarrier(
      cmd,
      source_stage, dest_stage,
      0,
      0, 0,
      0, 0,
      1, &barrier);
}

void vk_upload_image_to_gpu(VkCommandBuffer cmd, VK_Image image) {
  // Region to copy
  VkBufferImageCopy region = {};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  
  region.imageExtent.width = image.width;
  region.imageExtent.height = image.height;
  region.imageExtent.depth = 1;
  
  vkCmdCopyBufferToImage(
    cmd,
    vk.stage_buffer.handle, 
    image.handle, 
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
    1, 
    &region);
}

void vk_image_destroy(VK_Image* image) {
  if (image->view) {
    vkDestroyImageView(vkdevice, image->view, vk.allocator);
    image->view = 0;
  }
  if (image->memory) {
    vkFreeMemory(vkdevice, image->memory, vk.allocator);
    image->memory = 0;
  }
  if (image->handle) {
    vkDestroyImage(vkdevice, image->handle, vk.allocator);
    image->handle = 0;
  }
}

void vk_texture_load(Texture* t) {
  VK_Texture* texture = &vk.texture;
  
  u64 size = t->width * t->height * t->channel_count;
  VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;
  
  MemCopy(vk.stage_buffer.maped_memory, t->data, size);
  
  texture->image = vk_image_create(
    VK_IMAGE_TYPE_2D, 
    t->width, 
    t->height, 
    image_format, 
    VK_IMAGE_TILING_OPTIMAL, 
    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    true,
    VK_IMAGE_ASPECT_COLOR_BIT);
  
  VkCommandBuffer cmd = vk_cmd_alloc_and_begin_single_use();
  
  vk_image_transition_layout(
    cmd, 
    texture->image, 
    VK_IMAGE_LAYOUT_UNDEFINED, 
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    
  vk_upload_image_to_gpu(cmd, texture->image);
  
  vk_image_transition_layout(
    cmd, 
    texture->image, 
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
  vk_cmd_end_single_use(cmd);
  
  // Create a sampler for the texture
  VkSamplerCreateInfo sampler_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  // TODO These filters shoud be configurable
  sampler_info.magFilter = VK_FILTER_LINEAR;
  sampler_info.minFilter = VK_FILTER_LINEAR;
  sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.anisotropyEnable = VK_FALSE;
  sampler_info.maxAnisotropy = 16;
  sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sampler_info.unnormalizedCoordinates = VK_FALSE;
  sampler_info.compareEnable = VK_FALSE;
  sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
  sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_info.mipLodBias = 0.0f;
  sampler_info.minLod = 0.0f;
  sampler_info.maxLod = 0.0f;
  
  VK_CHECK(vkCreateSampler(vkdevice, &sampler_info, vk.allocator, &texture->sampler));
}

void vk_render_target_create() {
}
