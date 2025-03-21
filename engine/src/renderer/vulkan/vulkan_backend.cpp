#include "vulkan_backend.h"

#include "vulkan_types.inl"
#include "vulkan_platform.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_renderpass.h"
#include "vulkan_command_buffer.h"

#include "logger.h"
#include "strings.h"
#include "memory.h"

global VulkanContext* context;

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);

internal i32 find_memory_index(u32 type_filter, u32 property_flags);
internal void create_command_buffers(RendererBackend* backend);

b8 vulkan_renderer_backend_initialize(RendererBackend* backend) {
  // Function pointer
  backend->internal_context = push_struct(backend->arena, VulkanContext);
  context = (VulkanContext*)backend->internal_context;
  context->arena = backend->arena;
  context->find_memory_index = find_memory_index;
  
  // TODO: custom allocator.
  context->allocator = 0;

  VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  app_info.apiVersion = VK_API_VERSION_1_2;
  app_info.pApplicationName = "Engine.exe";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "Kohi Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  
  VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  create_info.pApplicationInfo = &app_info;
  
  // Obtain a list of required extensions
  char* required_extensions[3]; 
  required_extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
  required_extensions[1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  required_extensions[2] = "VK_KHR_win32_surface";
#if defined(_DEBUG)
  
  Debug("Required extensions:");
  for (u32 i = 0; i < ArrayCount(required_extensions); ++i) {
    Debug(required_extensions[i]);
  }
#endif

  create_info.enabledExtensionCount = ArrayCount(required_extensions);
  create_info.ppEnabledExtensionNames = required_extensions;
  
  // Validation layers
  // const char** required_validation_layer_names = 0;
  const char* required_validation_layer_names[1];
  u32 required_validation_layer_count = ArrayCount(required_validation_layer_names);
// If validation should be done, get a list of the required validation layert names
// and make sure they exist. Validation layers should only be enabled on non-release builds. 
#if defined(_DEBUG)
    Info("Validation layers enabled. Enumerating...");
    // The list of validation layers required.
    required_validation_layer_names[0] = "VK_LAYER_KHRONOS_validation";
    
    // Obtain a list of available validation layers
    u32 available_layer_count = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, 0));
    // VkLayerProperties* available_layers = darray_reserve(VkLayerProperties, available_layer_count);
    VkLayerProperties* available_layers = push_array(context->arena, VkLayerProperties, available_layer_count);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers));
    
    // Verify all required layers are available.
    for (u32 i = 0; i < required_validation_layer_count; ++i) {
        Info("Searching for layer: %s...", required_validation_layer_names[i]);
        b8 found = false;
        for (u32 j = 0; j < available_layer_count; ++j) {
            if (cstr_equal(required_validation_layer_names[i], available_layers[j].layerName)) {
                found = true;
                Info("Found.");
                break;
            }
        }
        if (!found) {
            Fatal("Required validation layer is missing: %s", required_validation_layer_names[i]);
            return false;
        }
    }
    Info("All required validation layers are present.");
#endif
  
  create_info.enabledLayerCount = required_validation_layer_count;
  create_info.ppEnabledLayerNames = required_validation_layer_names;
  
  VK_CHECK(vkCreateInstance(&create_info, context->allocator, &context->instance));
  Info("Vulkan insance created.");

  // Debugger
#if defined(_DEBUG)
  Debug("Creating Vulkan debugger...");
  u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;  //|
                                                                    //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
  VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
  debug_create_info.messageSeverity = log_severity;
  debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
  debug_create_info.pfnUserCallback = vk_debug_callback;

  PFN_vkCreateDebugUtilsMessengerEXT func =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
          context->instance, "vkCreateDebugUtilsMessengerEXT");

  AssertMsg(func, "Failed to create debug messenger!");
  VK_CHECK(func(context->instance, &debug_create_info, context->allocator, &context->debug_messenger));
  Debug("Vulkan debugger created.");
#endif
  
  // Surface
  Debug("Creating Vulkan surface...");
  if (!platform_create_vulkan_surface(context)) {
    Error("Failed to create platform surface!");
    return false;
  }
  Debug("Vulkan surface created.");

  // Device creation
  if (!vulkan_device_create(context)) {
    Error("Failed to create device!");
    return false;
  }
  
  // Swapchain
  vulkan_swapchain_create(
      context,
      context->framebuffer_width,
      context->framebuffer_height,
      &context->swapchain);
  
  vulkan_renderpass_create(
    context, 
    &context->main_renderpass, 
    0, 0, context->framebuffer_width, context->framebuffer_height, 
    0.0f, 0.0f, 0.2f, 1.0f, 
    1.0f, 
    0);
  
  // Create command buffers.
  create_command_buffers(backend);

  Info("Vulkan renderer initialized successfully.");
  return true;
}

void vulkan_renderer_backend_shutdown(RendererBackend* backend) {
  // Destroy in opposite order of creation
  
  // Command buffers
  for (u32 i = 0; i < context->swapchain.image_count; ++i) {
    if (context->graphics_command_buffers[i].handle) {
      vulkan_command_buffer_free(
        context, 
        context->device.graphics_command_pool,
        &context->graphics_command_buffers[i]);
      context->graphics_command_buffers[i].handle = 0;
    }
  }
  // darray_destroy(context->graphics_command_buffers); // TODO
  context->graphics_command_buffers = 0;
  
  // Renderpass
  vulkan_renderpass_destroy(context, &context->main_renderpass);
  
  // Swapchain
  vulkan_swapchain_destroy(context, &context->swapchain);
  
  Debug("Destroying Vulkan device...");
  vulkan_device_destroy(context);
  
  Debug("Destroying Vulkan surface...");
  if (context->surface) {
    vkDestroySurfaceKHR(context->instance, context->surface, context->allocator);
    context->surface = 0;
  }
  
  Debug("Destroying Vulkan debugger...");
  if (context->debug_messenger) {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            context->instance, "vkDestroyDebugUtilsMessengerEXT");
    func(context->instance, context->debug_messenger, context->allocator);
  }

  Debug("Destroying Vulkan instance...");
  vkDestroyInstance(context->instance, context->allocator);
}

void vulkan_renderer_backend_on_resize(RendererBackend* backend, u16 width, u16 height) {

}

b8 vulkan_renderer_backend_begin_frame(RendererBackend* backend, f32 delta_time) {
  return true;
}

b8 vulkan_renderer_backend_end_frame(RendererBackend* backend, f32 delta_time) {
  return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {
    switch (message_severity) {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            Error(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            Warn(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            Info(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            Trace(callback_data->pMessage);
            break;
    }
    return VK_FALSE;
}

internal i32 find_memory_index(u32 type_filter, u32 property_flags) {
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(context->device.physical_device, &memory_properties);

  for (u32 i = 0; i < memory_properties.memoryTypeCount; ++i) {
    // Check each memory type to see if its bit is set to 1.
    if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
      return i;
    }
  }

  Warn("Unable to find suitable memory type!");
  return -1;
}

internal void create_command_buffers(RendererBackend* backend) {
  if (!context->graphics_command_buffers) {
    context->graphics_command_buffers = push_array(context->arena, VulkanCommandBuffer, context->swapchain.image_count);
    for (u32 i = 0; i < context->swapchain.image_count; ++i) {
      MemZeroStruct(&context->graphics_command_buffers[i]);
    }
  }
  
  for (u32 i = 0; i < context->swapchain.image_count; ++i) {
    if (context->graphics_command_buffers[i].handle) {
      vulkan_command_buffer_free(
          context,
          context->device.graphics_command_pool,
          &context->graphics_command_buffers[i]);
    }
    MemZeroStruct(&context->graphics_command_buffers[i]);
    vulkan_command_buffer_allocate(
      context, 
      context->device.graphics_command_pool, 
      true, 
      &context->graphics_command_buffers[i]);
  }
  Debug("Vulkan command buffers created.");
}
