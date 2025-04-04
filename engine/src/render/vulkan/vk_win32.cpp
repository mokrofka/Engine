#include <os.h>
#include <logger.h>

#include <windows.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

struct Win32HandleInfo {
  HINSTANCE h_instance;
};

struct WindowPlatformState {
  HWND hwnd;
};

// void platform_get_required_extension_names(Arena* arena) {
//   push_str_copy(arena, cstr("VK_KHR_win32_surface"));
// }

struct VK_Context {
  struct Arena* arena;
  
  VkInstance instance;
  VkAllocationCallbacks* allocator;
  VkSurfaceKHR surface;
};

// Surface creation for Vulkan
b8 vk_os_create_surface(VK_Context* context) {
  // Simply cold-cast to the known type.
  // InternalState* state = (InternalState*)plat_state->internal_state;
  HINSTANCE h_instance = (HINSTANCE)os_get_handle_info();
  HWND hwnd = (HWND)os_get_window_handle();
  
  VkWin32SurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
  create_info.hinstance = h_instance;
  create_info.hwnd = hwnd;

  VkResult result = vkCreateWin32SurfaceKHR(context->instance, &create_info,
                                            context->allocator, &context->surface);
  if (result != VK_SUCCESS) {
    Fatal("Vulkan surface creation failed.");
    return false;
  }
  return true;
}
