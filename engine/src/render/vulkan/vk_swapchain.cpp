#include "vk_swapchain.h"

#include "vk_device.h"
#include "vk_image.h"
#include "vk_framebuffer.h"

internal VK_Swapchain create(u32 width, u32 height);
internal void destroy(VK_Swapchain* swapchain);

VK_Swapchain vk_swapchain_create(u32 width, u32 height) {
  return create(width, height);
}

void vk_swapchain_recreate(VK_Swapchain* swapchain, u32 width, u32 height) {
  destroy(swapchain);
  *swapchain = create(width, height);
}

void vk_swapchain_destroy(VK_Swapchain* swapchain) {
  destroy(swapchain);
}

u32 vk_swapchain_acquire_next_image_index(
    VK_Swapchain swapchain,
    u64 timeout_ns,
    VkSemaphore image_available_semaphore,
    VkFence fence) {
  u32 image_index;
  vkAcquireNextImageKHR(vkdevice, swapchain.handle, timeout_ns,
                        image_available_semaphore, fence, &image_index);
  return image_index;
}

void vk_swapchain_present(
    VK_Swapchain* swapchain,
    VkQueue graphics_queue,
    VkQueue present_queue,
    VkSemaphore render_complete_semaphore,
    u32 present_image_index) {
  // Return the image to the swapchain for presentation.
  VkPresentInfoKHR present_info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &render_complete_semaphore;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &swapchain->handle;
  present_info.pImageIndices = &present_image_index;
  present_info.pResults = 0;
  
  VkResult result = vkQueuePresentKHR(present_queue, &present_info);
  
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    // I didn't get the point of this!
    // Swapchain is out of date, suboptimal of a framebuffer resize has occured. Trigger swapchain recreation.
    // VK_Swapchain swapchain_copy = *swapchain;
    // *swapchain = vk_swapchain_recreate(vk->frame.width, vk->frame.height, *swapchain);
    // for (i32 i = 0; i < 3; ++i) {
    //   swapchain->framebuffers[i] = swapchain_copy.framebuffers[i];
    // }
  } else if (result != VK_SUCCESS) {
    Fatal("Failed to acquire swapchain image!");
  }
  
  // Increment (and loop) the index
  vk->frame.current_frame = (vk->frame.current_frame + 1) % swapchain->max_frames_in_flight;
}

internal VK_Swapchain create(u32 width, u32 height) {
  VK_Swapchain swapchain = {};
  VkExtent2D swapchain_extent = {width, height};
  
  // Choose a swap surface format
  b8 found = false;
  for (i32 i = 0; i < vk->device.swapchain_support.format_count; ++i) {
    VkSurfaceFormatKHR format = vk->device.swapchain_support.formats[i];
    // Preferred formats
    if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      swapchain.image_format = format;
      found = true;
      break;
    }
  }
  
  if (!found) {
    swapchain.image_format = vk->device.swapchain_support.formats[0];
  }
  
  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
  for (i32 i = 0; i < vk->device.swapchain_support.present_mode_count; ++i) {
    VkPresentModeKHR mode = vk->device.swapchain_support.present_modes[i];
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      present_mode = mode;
      break;
    }
  }
  
  // Requery swapchain support.
  vk->device.swapchain_support = vk_device_query_swapchain_support(vk->device.physical_device);
  
  // Swapchain extent
  if (vk->device.swapchain_support.capabilities.currentExtent.width != U32_MAX) {
    swapchain_extent = vk->device.swapchain_support.capabilities.currentExtent;
  }
  
  // Clamp to the value allows by the GPU.
  VkExtent2D min = vk->device.swapchain_support.capabilities.minImageExtent;
  VkExtent2D max = vk->device.swapchain_support.capabilities.maxImageExtent;
  swapchain_extent.width = Clamp(min.width, swapchain_extent.width, max.width);
  swapchain_extent.height = Clamp(min.height, swapchain_extent.height, max.height);
  
  u32 image_count = vk->device.swapchain_support.capabilities.minImageCount + 1;
  if (vk->device.swapchain_support.capabilities.maxImageCount > 0 &&
      image_count > vk->device.swapchain_support.capabilities.maxImageCount) {
        image_count = vk->device.swapchain_support.capabilities.maxImageCount;
  }
  
  swapchain.max_frames_in_flight = image_count - 1;
  
  VkSwapchainCreateInfoKHR swapchain_create_info = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
  swapchain_create_info.surface = vk->surface;
  swapchain_create_info.minImageCount = image_count;
  swapchain_create_info.imageFormat = swapchain.image_format.format;
  swapchain_create_info.imageColorSpace = swapchain.image_format.colorSpace;
  swapchain_create_info.imageExtent = swapchain_extent;
  swapchain_create_info.imageArrayLayers = 1;
  swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  // Setup the queue family indices
  if (vk->device.graphics_queue_index != vk->device.present_queue_index) {
    u32 queueFamilyIndices[] = {
        (u32)vk->device.graphics_queue_index,
        (u32)vk->device.present_queue_index};
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_create_info.queueFamilyIndexCount = 2;
    swapchain_create_info.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.queueFamilyIndexCount = 0;
    swapchain_create_info.pQueueFamilyIndices = 0;
  }

  swapchain_create_info.preTransform = vk->device.swapchain_support.capabilities.currentTransform;
  swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_create_info.presentMode = present_mode;
  swapchain_create_info.clipped = VK_TRUE;
  swapchain_create_info.oldSwapchain = 0;
  
  VK_CHECK(vkCreateSwapchainKHR(vkdevice, &swapchain_create_info, vk->allocator, &swapchain.handle));
  
  // Start with a zero frame index.
  vk->frame.current_frame = 0;
  
  // Images
  swapchain.image_count = 0;
  VK_CHECK(vkGetSwapchainImagesKHR(vkdevice, swapchain.handle, &swapchain.image_count, 0));
  VK_CHECK(vkGetSwapchainImagesKHR(vkdevice, swapchain.handle, &swapchain.image_count, swapchain.images));

  // Views
  for (i32 i = 0; i < swapchain.image_count; ++i) {
    VkImageViewCreateInfo view_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.image = swapchain.images[i];
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = swapchain.image_format.format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(vkdevice, &view_info, vk->allocator, &swapchain.views[i]));
  }

  // Depth resources
  vk_device_detect_depth_format(&vk->device);

  // Create depth image and its view.
  swapchain.depth_attachment = vk_image_create(
      VK_IMAGE_TYPE_2D,
      swapchain_extent.width,
      swapchain_extent.height,
      vk->device.depth_format,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      true,
      VK_IMAGE_ASPECT_DEPTH_BIT);

  Info("Swapchain created successfully.");
  return swapchain;
}

internal void destroy(VK_Swapchain* swapchain) {
  for (u32 i = 0; i < vk->swapchain.image_count; ++i) {
    vk_framebuffer_destroy(&swapchain->framebuffers[i]);
  }
  vkDeviceWaitIdle(vkdevice);
  vk_image_destroy(&swapchain->depth_attachment);

  for (u32 i = 0; i < swapchain->image_count; ++i) {
    vkDestroyImageView(vkdevice, swapchain->views[i], vk->allocator);
  }

  vkDestroySwapchainKHR(vkdevice, swapchain->handle, vk->allocator);
}
