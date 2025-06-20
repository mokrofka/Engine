#include "vk_swapchain.h"

#include "vk_device.h"
#include "vk_image.h"

internal VK_Swapchain create(u32 width, u32 height, b32 reuse);
internal void destroy(VK_Swapchain* swapchain);

void vk_swapchain_create(u32 width, u32 height) {
  vk.swapchain = create(width, height, false);
}

void vk_swapchain_recreate(u32 width, u32 height) {
  if (vk.frame.width == 0 || vk.frame.height == 0) {
    Debug("recreate_swapchain called when window is < 1 in a dimension. Booting"_);
  }

  vk.old_swapchain = vk.swapchain;
  vk.swapchain = create(width, height, true);
  destroy(&vk.old_swapchain);
  
  vk.frame.size_last_generation = vk.frame.size_generation;
}

void vk_swapchain_destroy() {
  destroy(&vk.swapchain);
}

u32 vk_swapchain_acquire_next_image_index(VkSemaphore image_available_semaphore) {
  u32 image_index;
  vkAcquireNextImageKHR(vkdevice, vk.swapchain.handle, U64_MAX,
                        image_available_semaphore, 0, &image_index);
  return image_index;
}

void vk_swapchain_present(VkSemaphore render_complete_semaphore, u32 present_image_index) {
  VkPresentInfoKHR present_info = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &render_complete_semaphore,
    .swapchainCount = 1,
    .pSwapchains = &vk.swapchain.handle,
    .pImageIndices = &present_image_index,
  };

  vkQueuePresentKHR(vk.device.graphics_queue, &present_info);
  
  vk.frame.current_frame = (vk.frame.current_frame + 1) % FramesInFlight;
}

internal void destroy(VK_Swapchain* swapchain) {
  vkDeviceWaitIdle(vkdevice);
  vk_image_destroy(swapchain->depth_attachment);

  Loop (i, swapchain->image_count) {
    vkDestroyImageView(vkdevice, swapchain->views[i], vk.allocator);
  }

  vkDestroySwapchainKHR(vkdevice, swapchain->handle, vk.allocator);
}

internal VK_Swapchain create(u32 width, u32 height, b32 reuse) {
  VK_Swapchain swapchain = {};
  VkExtent2D swapchain_extent = {width, height};
  
  // Choose a swap surface format
  b32 found = false;
  Loop (i, vk.device.swapchain_support.format_count) {
    VkSurfaceFormatKHR format = vk.device.swapchain_support.formats[i];
    // Preferred formats
    if (format.format == VK_FORMAT_B8G8R8A8_UNORM && // darker
    // if (format.format == VK_FORMAT_B8G8R8A8_SRGB && // brighter
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      swapchain.image_format = format;
      found = true;
      break;
    }
  }
  
  if (!found) {
    swapchain.image_format = vk.device.swapchain_support.formats[0];
  }
  
  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
  // TODO configurable
  // Loop (i, vk.device.swapchain_support.present_mode_count) {
  //   VkPresentModeKHR mode = vk.device.swapchain_support.present_modes[i];
  //   if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
  //     present_mode = mode;
  //     break;
  //   }
  // }
  
  // Requery swapchain support.
  vk_device_query_swapchain_support(vk.device.physical_device, &vk.device.swapchain_support);
  
  // Swapchain extent
  if (vk.device.swapchain_support.capabilities.currentExtent.width != U32_MAX) {
    swapchain_extent = vk.device.swapchain_support.capabilities.currentExtent;
  }
  
  // Clamp to the value allows by the GPU.
  VkExtent2D min = vk.device.swapchain_support.capabilities.minImageExtent;
  VkExtent2D max = vk.device.swapchain_support.capabilities.maxImageExtent;
  swapchain_extent.width = Clamp(min.width, swapchain_extent.width, max.width);
  swapchain_extent.height = Clamp(min.height, swapchain_extent.height, max.height);
  
  u32 image_count = vk.device.swapchain_support.capabilities.minImageCount + 1;
  if (vk.device.swapchain_support.capabilities.maxImageCount > 0 &&
      image_count > vk.device.swapchain_support.capabilities.maxImageCount) {
        image_count = vk.device.swapchain_support.capabilities.maxImageCount;
  }
  
  swapchain.max_frames_in_flight = FramesInFlight;
  
  VkSwapchainCreateInfoKHR swapchain_create_info = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = vk.surface,
    .minImageCount = image_count,
    .imageFormat = swapchain.image_format.format,
    .imageColorSpace = swapchain.image_format.colorSpace,
    .imageExtent = swapchain_extent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = vk.device.swapchain_support.capabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = present_mode,
    .clipped = VK_TRUE,
    .oldSwapchain = reuse ? vk.old_swapchain.handle : 0,
  };

  VK_CHECK(vkCreateSwapchainKHR(vkdevice, &swapchain_create_info, vk.allocator, &swapchain.handle));
  
  // Start with a zero frame index.
  vk.frame.current_frame = 0;
  
  // Images
  swapchain.image_count = 0;
  VK_CHECK(vkGetSwapchainImagesKHR(vkdevice, swapchain.handle, &swapchain.image_count, 0));
  VK_CHECK(vkGetSwapchainImagesKHR(vkdevice, swapchain.handle, &swapchain.image_count, swapchain.images));

  // Views
  Loop (i, swapchain.image_count) {
    VkImageViewCreateInfo view_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = swapchain.images[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = swapchain.image_format.format,
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
      },
    };

    VK_CHECK(vkCreateImageView(vkdevice, &view_info, vk.allocator, &swapchain.views[i]));
  }

  // Depth resources
  vk_device_detect_depth_format(&vk.device);

  // Create depth image and its view.
  swapchain.depth_attachment = vk_image_create(
    VK_IMAGE_TYPE_2D,
    swapchain_extent.width,
    swapchain_extent.height,
    vk.device.depth_format,
    VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    true,
    VK_IMAGE_ASPECT_DEPTH_BIT);

  Info("Swapchain created successfully"_);
  return swapchain;
}

typedef void* HINSTANCE;
typedef void* HWND;
typedef void* HANDLE;
typedef wchar_t* LPCWSTR;
typedef u32 DWORD;
typedef void* LPVOID;
typedef b32 BOOL;
typedef void* HMONITOR;
struct SECURITY_ATTRIBUTES {
  DWORD nLength;
  LPVOID lpSecurityDescriptor;
  BOOL bInheritHandle;
};

#include <vulkan/vulkan_win32.h>
void vk_surface_create() {
  HINSTANCE hinstance = os_get_handle_info();
  HWND hwnd = os_get_window_handle();
  
  VkWin32SurfaceCreateInfoKHR surface_create_info = {
    .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
    .hinstance = hinstance,
    .hwnd = hwnd
  };

  vkCreateWin32SurfaceKHR(vk.instance, &surface_create_info, vk.allocator, &vk.surface);
}
