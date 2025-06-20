#include "vk_image.h"
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

  VK_Image result = {
    .width = width,
    .height = height,
  };

  // Creation info.
  VkImageCreateInfo image_create_info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = format,
    .extent = {width, height, 1},
    .mipLevels = 1,
    .arrayLayers = 1,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .tiling = tiling,
    .usage = usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VK_CHECK(vkCreateImage(vkdevice, &image_create_info, vk.allocator, &result.handle));

  // Query memory requirements.
  VkMemoryRequirements memory_requirements;
  vkGetImageMemoryRequirements(vkdevice, result.handle, &memory_requirements);

  u32 memory_type = vk_find_memory_index(memory_requirements.memoryTypeBits, memory_flags);

  // Allocate memory
  VkMemoryAllocateInfo memory_allocate_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = memory_requirements.size,
    .memoryTypeIndex = memory_type,
  };
  VK_CHECK(vkAllocateMemory(vkdevice, &memory_allocate_info, vk.allocator, &result.memory));

  // Bind the memory
  VK_CHECK(vkBindImageMemory(vkdevice, result.handle, result.memory, 0));

  // Create view
  if (create_view) {
    result.view = vk_image_view_create(format, result.handle, view_aspect_flags);
  }
  
  return result;
}

VkImageView vk_image_view_create(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags) {
  VkImageViewCreateInfo view_create_info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .image = image,
    .viewType = VK_IMAGE_VIEW_TYPE_2D,
    .format = format,
    .subresourceRange = {
      .aspectMask = aspect_flags,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
    },
  };

  VkImageView result;
  VK_CHECK(vkCreateImageView(vkdevice, &view_create_info, vk.allocator, &result));
  return result;
}

void vk_image_transition_layout(VkCommandBuffer cmd, VK_Image image, VkImageLayout old_layout, VkImageLayout new_layout) {
  VkImageMemoryBarrier barrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .oldLayout = old_layout,
    .newLayout = new_layout,
    .srcQueueFamilyIndex = vk.device.graphics_queue_index,
    .dstQueueFamilyIndex = vk.device.graphics_queue_index,
    .image = image.handle,
    .subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
    },
  };
  
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
      0, null,
      0, null,
      1, &barrier);
}

void vk_upload_image_to_gpu(VkCommandBuffer cmd, VK_Image image) {
  VkBufferImageCopy region = {
    .bufferOffset = 0,
    .bufferRowLength = 0,
    .bufferImageHeight = 0,
    .imageSubresource = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .mipLevel = 0,
      .baseArrayLayer = 0,
      .layerCount = 1,
    },
    .imageExtent = { image.width, image.height, 1 },
  };
  
  vkCmdCopyBufferToImage(
    cmd,
    vk.stage_buffer.handle, 
    image.handle, 
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
    1, 
    &region);
}

void vk_image_destroy(VK_Image& image) {
  if (image.view) {
    vkDestroyImageView(vkdevice, image.view, vk.allocator);
  }
  if (image.memory) {
    vkFreeMemory(vkdevice, image.memory, vk.allocator);
  }
  if (image.handle) {
    vkDestroyImage(vkdevice, image.handle, vk.allocator);
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
  VkSamplerCreateInfo sampler_info = {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .magFilter = VK_FILTER_LINEAR,
    .minFilter = VK_FILTER_LINEAR,
    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .mipLodBias = 0.0f,
    .anisotropyEnable = VK_FALSE,
    .maxAnisotropy = 16,
    .compareEnable = VK_FALSE,
    .compareOp = VK_COMPARE_OP_ALWAYS,
    .minLod = 0.0f,
    .maxLod = 0.0f,
    .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
    .unnormalizedCoordinates = VK_FALSE,
  };
  
  VK_CHECK(vkCreateSampler(vkdevice, &sampler_info, vk.allocator, &texture->sampler));
}
