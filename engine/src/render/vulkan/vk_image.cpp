#include "vk_image.h"

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
  image_create_info.mipLevels = 4;    // TODO: Support mip mapping
  image_create_info.arrayLayers = 1;  // TODO: Support number of layers in the image.
  image_create_info.format = format;
  image_create_info.tiling = tiling;
  image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_create_info.usage = usage;
  image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;         // TODO: Configurable sample count.
  image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // TODO: Configurable sharing mode.

  VK_CHECK(vkCreateImage(vkdevice, &image_create_info, vk->allocator, &result.handle));

  // Query memory requirements.
  VkMemoryRequirements memory_requirements;
  vkGetImageMemoryRequirements(vkdevice, result.handle, &memory_requirements);

  i32 memory_type = vk_find_memory_index(memory_requirements.memoryTypeBits, memory_flags);
  Assert(memory_type != -1 && "Required memory type not found. Image not valid");

  // Allocate memory
  VkMemoryAllocateInfo memory_allocate_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  memory_allocate_info.allocationSize = memory_requirements.size;
  memory_allocate_info.memoryTypeIndex = memory_type;
  VK_CHECK(vkAllocateMemory(vkdevice, &memory_allocate_info, vk->allocator, &result.memory));

  // Bind the memory
  VK_CHECK(vkBindImageMemory(vkdevice, result.handle, result.memory, 0)); // TODO: configurable memory offset.

  // Create view
  if (create_view) {
    result.view = 0;
    vk_image_view_create(format, &result, view_aspect_flags);
  }
  
  return result;
}

void vk_image_view_create(VkFormat format, VK_Image* image, VkImageAspectFlags aspect_flags) {
  VkImageViewCreateInfo view_create_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  view_create_info.image = image->handle;
  view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D; // TODO: Make configurable.
  view_create_info.format = format;
  view_create_info.subresourceRange.aspectMask = aspect_flags;

  // TODO: Make configurable
  view_create_info.subresourceRange.baseMipLevel = 0;
  view_create_info.subresourceRange.levelCount = 1;
  view_create_info.subresourceRange.baseArrayLayer = 0;
  view_create_info.subresourceRange.layerCount = 1;

  VK_CHECK(vkCreateImageView(vkdevice, &view_create_info, vk->allocator, &image->view));
}

void vk_image_transition_layout(
    VK_Cmd* command_buffer,
    VK_Image* image,
    VkFormat format,
    VkImageLayout old_layout,
    VkImageLayout new_layout) {
  VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = vk->device.graphics_queue_index;
  barrier.dstQueueFamilyIndex = vk->device.graphics_queue_index;
  barrier.image = image->handle;
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
      command_buffer->handle,
      source_stage, dest_stage,
      0,
      0, 0,
      0, 0,
      1, &barrier);
}

void vk_image_copy_from_buffer(VK_Image* image, VkBuffer buffer, VK_Cmd* command_buffer) {
  // Region to copy
  VkBufferImageCopy region = {};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  
  region.imageExtent.width = image->width;
  region.imageExtent.height = image->height;
  region.imageExtent.depth = 1;
  
  vkCmdCopyBufferToImage(
    command_buffer->handle, 
    buffer, 
    image->handle, 
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
    1, 
    &region);
}

void vk_image_destroy(VK_Image* image) {
    if (image->view) {
        vkDestroyImageView(vkdevice, image->view, vk->allocator);
        image->view = 0;
    }
    if (image->memory) {
        vkFreeMemory(vkdevice, image->memory, vk->allocator);
        image->memory = 0;
    }
    if (image->handle) {
        vkDestroyImage(vkdevice, image->handle, vk->allocator);
        image->handle = 0;
    }
}
