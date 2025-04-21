#include "vk_fence.h"

VK_Fence vk_fence_create(b8 create_signaled) {
  VK_Fence fence;

  // Make sure to signal the fence if required.
  fence.is_signaled = create_signaled;
  VkFenceCreateInfo fence_create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  if (fence.is_signaled) {
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  }

  VK_CHECK(vkCreateFence(vkdevice, &fence_create_info, vk->allocator, &fence.handle));
  return fence;
}

void vk_fence_destroy(VK_Fence* fence) {
  if (fence->handle) {
    vkDestroyFence(vkdevice, fence->handle, vk->allocator);
    fence->handle = 0;
  }
  fence->is_signaled = false;
}

b8 vk_fence_wait(VK_Fence* fence, u64 timeout_ns) {
  if (!fence->is_signaled) {
    VkResult result = vkWaitForFences(vkdevice, 1, &fence->handle, true, timeout_ns);
    switch (result) {
      case VK_SUCCESS: {
        fence->is_signaled = true;
      } return true;
      case VK_TIMEOUT: {
        Warn("vk_fence_wait - Timed out"_);
      } break;
      case VK_ERROR_DEVICE_LOST: {
        Warn("vk_fence_wait - VK_ERROR_DEVICE_LOST"_);
      } break;
      case VK_ERROR_OUT_OF_HOST_MEMORY: {
        Warn("vk_fence_wait - VK_ERROR_OUT_OF_HOST_MEMORY"_);
      } break;
      case VK_ERROR_OUT_OF_DEVICE_MEMORY: {
        Warn("vk_fence_wait - VK_ERROR_OUT_OF_DEVICE_MEMORY"_);
      } break;
      
      default: {
        Error("vk_fence_wait - An unknown error has occured"_);
      } break;
    }
  } else {
    // if Already signaled, do not wait.
    return true;
  }
  
  return false;
}

void vk_fence_reset(VK_Fence* fence) {
  if (fence->is_signaled) {
    VK_CHECK(vkResetFences(vkdevice, 1, &fence->handle));
    fence->is_signaled = false;
  }
}
