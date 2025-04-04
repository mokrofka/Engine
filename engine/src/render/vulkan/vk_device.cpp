#include "vk_device.h"

#include <str.h>

struct VK_PhysicalDeviceRequirements {
  b8 graphics; 
  b8 present;
  b8 compute;
  b8 transfer;
  // darray
  const char** device_extension_names;
  b8 sampler_anisotropy;  
  b8 discrete_gpu;
};

struct VK_PhysicalDeviceQueueFamilyInfo {
  u32 graphics_family_index;
  u32 present_family_index;
  u32 compute_family_index;
  u32 transfer_family_index;
};

internal b8 select_physical_device(VK_Context* context);
internal b8 physical_device_meets_requirements(
    VkPhysicalDevice device, VK_Context* context,
    const VkPhysicalDeviceProperties *properties,
    const VkPhysicalDeviceFeatures *features,
    const VK_PhysicalDeviceRequirements *requirements,
    VK_PhysicalDeviceQueueFamilyInfo *out_queue_family_info,
    VK_SwapchainSupportInfo *out_swapchain_support);

b8 vk_device_create(VK_Context *context) {
  if (!select_physical_device(context)) {
    return false;
  }
  
  Info("Creating logical device...");
  // NOTE: Do not create additional queues for shared indices.
  b8 present_shares_graphics_queue = context->device.graphics_queue_index == context->device.present_queue_index;
  b8 transfer_shares_graphics_queue = context->device.graphics_queue_index == context->device.transfer_queue_index;
  u32 index_count = 1;
  if (!present_shares_graphics_queue) {
    ++index_count;
  }
  if (!transfer_shares_graphics_queue) {
    ++index_count;
  }
  u32 indices[32];
  u8 index = 0;
  indices[index++] = context->device.graphics_queue_index;
  if (!present_shares_graphics_queue) {
    indices[index++] = context->device.present_queue_index;
  }
  if (!transfer_shares_graphics_queue) {
    indices[index++] = context->device.transfer_queue_index;
  }
  
  VkDeviceQueueCreateInfo queue_create_infos[32];
  for (u32 i = 0; i < index_count; ++i) {
    queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[i].queueFamilyIndex = indices[i];
    queue_create_infos[i].queueCount = 1;
    // TODO: Enable this for a future enhancement.
    // if (indices[i] == context->device.graphics_queue_index) {
    //   queue_create_infos[i].queueCount = 2;
    // }
    queue_create_infos[i].flags = 0;
    queue_create_infos[i].pNext = 0;
    f32 queue_priority = 1.0f;
    queue_create_infos[i].pQueuePriorities = &queue_priority;
  }
  
  // Request device features.
  // TODO: should be config driven
  VkPhysicalDeviceFeatures device_features = {};
  device_features.samplerAnisotropy = VK_TRUE; // Request anistrophy
  
  VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  device_create_info.queueCreateInfoCount = index_count;
  device_create_info.pQueueCreateInfos = queue_create_infos;
  device_create_info.pEnabledFeatures = &device_features;
  device_create_info.enabledExtensionCount = 1;
  const char* extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
  device_create_info.ppEnabledExtensionNames = &extension_names;
  
  // Deprecated and ingnored, so pass nothing.
  device_create_info.enabledLayerCount = 0;
  device_create_info.ppEnabledLayerNames = 0;
  
  // Create the device.
  VK_CHECK(vkCreateDevice(
    context->device.physical_device,
    &device_create_info,
    context->allocator,
    &vkdevice));

  Info("Logical device created.");
  
  // Get queues.
  vkGetDeviceQueue(
    vkdevice, 
    context->device.graphics_queue_index, 
    0, 
    &context->device.graphics_queue);
    
  vkGetDeviceQueue(
    vkdevice, 
    context->device.present_queue_index, 
    0, 
    &context->device.present_queue);
    
  vkGetDeviceQueue(
    vkdevice, 
    context->device.transfer_queue_index, 
    0, 
    &context->device.transfer_queue);
  Info("Queues obtained.");
  
  // Create command pool for graphics queue.
  VkCommandPoolCreateInfo pool_create_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  pool_create_info.queueFamilyIndex = context->device.graphics_queue_index;
  pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  VK_CHECK(vkCreateCommandPool(
    vkdevice, 
    &pool_create_info, 
    context->allocator, 
    &context->device.graphics_command_pool));
  Info("Graphics command pool created.");

  return true;
}

void vk_device_destroy(VK_Context* context) {
  
  // Unset queues
  context->device.graphics_queue = 0;
  context->device.present_queue = 0;
  context->device.transfer_queue = 0;
  
  Info("Destroying command pools...");
  vkDestroyCommandPool(
    vkdevice, 
    context->device.graphics_command_pool, 
    context->allocator);
  
  // Destroy logical device
  Info("Destroying logical device...");
  if (vkdevice) {
    vkDestroyDevice(vkdevice, context->allocator);
    vkdevice = 0;
  }
  
  // Physical device are not destroyed.
  Info("Releasing physical device resources...");
  context->device.physical_device = 0;
  
  if (context->device.swapchain_support.formats) {
    // kfree(context->device.swapchain_support.formats,
    //       sizeof(VkSurfaceKHR) * context->device.swapchain_support.format_count,
    //       MEMORY_TAG_RENDERER);
    context->device.swapchain_support.formats = 0;
    context->device.swapchain_support.format_count = 0;
  }
  
  if (context->device.swapchain_support.present_modes) {
    // kfree(context->device.swapchain_support.present_modes,
    //       sizeof(VkSurfaceKHR) * context->device.swapchain_support.present_mode_count,
    //       MEMORY_TAG_RENDERER);
    context->device.swapchain_support.present_modes = 0;
    context->device.swapchain_support.present_mode_count = 0;
  }

  MemZero(&context->device.swapchain_support.capabilities,
          sizeof(context->device.swapchain_support.capabilities));

  context->device.graphics_queue_index = -1;
  context->device.present_queue_index = -1;
  context->device.transfer_queue_index = -1;
}

void vk_device_query_swapchain_support(
    VkPhysicalDevice physical_device,
    VK_Context* context,
    VK_SwapchainSupportInfo* out_support_info) {

  // Surface capabilities
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physical_device,
      context->surface,
      &out_support_info->capabilities));

  // Surface formats 
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
    physical_device,
    context->surface,
    &out_support_info->format_count,
    0));

  if (out_support_info->format_count != 0) {
    if (out_support_info->format_count) {
      // TODO: free memory
      out_support_info->formats = push_array(context->arena, VkSurfaceFormatKHR, out_support_info->format_count);
    }
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device,
        context->surface,
        &out_support_info->format_count,
        out_support_info->formats));
  }
  
  // Present modes
  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
    physical_device,
    context->surface,
    &out_support_info->present_mode_count,
    0));
  if (out_support_info->present_mode_count != 0) {
    if (!out_support_info->present_modes) {
      // TODO: free memory
      out_support_info->present_modes = push_array(context->arena, VkPresentModeKHR, out_support_info->present_mode_count);
    }
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device,
        context->surface,
        &out_support_info->present_mode_count,
        out_support_info->present_modes));
  }
}

b8 vk_device_detect_depth_format(VK_Device* device) {
  // Format candidates
  const u64 candidate_count = 3;
  VkFormat candidates[3] = {
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D24_UNORM_S8_UINT};

  u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
  for (u64 i = 0; i < candidate_count; ++i) {
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(device->physical_device, candidates[i], &properties);

    if ((properties.linearTilingFeatures & flags) == flags) {
      device->depth_format = candidates[i];
      return true;
    } else if ((properties.optimalTilingFeatures & flags) == flags) {
      device->depth_format = candidates[i];
      return true;
    }
  }

  return false;
}

internal b8 select_physical_device(VK_Context* context) {
  u32 physical_device_count = 0;
  VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physical_device_count, 0));
  if (physical_device_count == 0) {
    Fatal("No devices which support Vulkan were found.");
    return false;
  }
  
  const u32 max_device_count = 32;
  VkPhysicalDevice physical_devices[max_device_count];
  VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physical_device_count, physical_devices));
  for (i32 i = 0; i < physical_device_count; ++i) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_devices[i], &properties);
    
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physical_devices[i], &features);
    
    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory);
    
    
    // TODO; There requirements should probably be drive by engine
    // configuration
    VK_PhysicalDeviceRequirements requirements = {};
    requirements.graphics = true;
    requirements.present = true;
    requirements.transfer = true;
    // NOTE: Enable this fi compute will be required.
    // requirements.compute = true;
    requirements.sampler_anisotropy = true;
    // requirements.discrete_gpu = false;
    // requirements.device_extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    const char* ext = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    requirements.device_extension_names = &ext;
    // requirements.device_extension_names = &VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    // requirements.device_extension_names = darray_create(const char*);
    // darray_push(requirements.device_extension_names, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    
    VK_PhysicalDeviceQueueFamilyInfo queue_info = {};
    b8 result = physical_device_meets_requirements(
        physical_devices[i],
        context,
        &properties,
        &features,
        &requirements,
        &queue_info,
        &context->device.swapchain_support);
    
    if (result) {
      Info("Selected device: '%s'.", properties.deviceName);
      // GPU type, etc.
      switch (properties.deviceType) {
        default:
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
          Info("GPU type is Unkown.");
          break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
          Info("GPU type is Integrated.");
          break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
          Info("GPU type is Descrete.");
          break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
          Info("GPU type is Virtual.");
          break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
          Info("GPU type is CPU.");
          break;
      }
      
      Info(
        "GPU Driver version: %i.%i.%i",
        VK_VERSION_MAJOR(properties.driverVersion),
        VK_VERSION_MINOR(properties.driverVersion),
        VK_VERSION_PATCH(properties.driverVersion));
        
      // Vulkan API version.
      Info(
        "GPU API version: %i.%i.%i",
        VK_VERSION_MAJOR(properties.apiVersion),
        VK_VERSION_MINOR(properties.apiVersion),
        VK_VERSION_PATCH(properties.apiVersion));

      // Memory information
      for (i32 j = 0; j < memory.memoryHeapCount; ++j) {
        f32 memory_size_gib = (((f32)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
        if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
          Info("Local GPU memory: %.2f GiB", memory_size_gib);
        } else {
          Info("Shared System memory: %.2f GiB", memory_size_gib);
        }
      }

      context->device.physical_device = physical_devices[i];
      context->device.graphics_queue_index = queue_info.graphics_family_index;
      context->device.present_queue_index = queue_info.present_family_index;
      context->device.transfer_queue_index = queue_info.transfer_family_index;
      // NOTE: set computer index here if needed.

      // Keep a copy of properties, features and memory info for later use.
      context->device.properties = properties;
      context->device.features = features;
      context->device.memory = memory;
      break;
    }
  }
  
  // Ensure a device was selected
  if (!context->device.physical_device) {
    Error("No physical devices were found which meet the requirements.");
    return false;
  }
  
  Info("Physical device selected.");
  return true;
}

internal b8 physical_device_meets_requirements(
    VkPhysicalDevice device, VK_Context* context,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const VK_PhysicalDeviceRequirements* requirements,
    VK_PhysicalDeviceQueueFamilyInfo* out_queue_info,
    VK_SwapchainSupportInfo* out_swapchain_support) {

  // Evalueate device properties to determina if it meets the needs of our application.
  out_queue_info->graphics_family_index = -1;  
  out_queue_info->present_family_index = -1;  
  out_queue_info->compute_family_index = -1;  
  out_queue_info->transfer_family_index = -1;  
  
  // Discrete GPU?
  if (requirements->discrete_gpu) {
    if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      Info("Device is not a discrete GPU, and one is required. Skipping.");
      return false;
    }
  }
  
  u32 queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);
  VkQueueFamilyProperties queue_families[32] = {};
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);
  
  // Look at each queue and see what queues it supports
  Info("Graphics | Present | Computer | Transfer | Name");
  u8 min_transfer_score = 255; // TODO: check type
  for (u32 i = 0; i < queue_family_count; ++i) {
    u8 current_transfer_score = 0; 
    
    // Graphics queue?
    if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      out_queue_info->graphics_family_index = i;
      ++current_transfer_score;
    }
    
    // Compute queue?
    if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      out_queue_info->compute_family_index = i;
      ++current_transfer_score;
    }
    
    // Transfer queue?
    if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
      // Take the index if it is the current lowest. This increased the
      // likegood that it is a dedicated transfer queue.
      // if (current_transfer_score <= min_transfer_score) {
      //   min_transfer_score = current_transfer_score;
      //   out_queue_info->transfer_family_index = 0;
      // }
      out_queue_info->transfer_family_index = i;
    }
    
    VkBool32 supports_present = VK_FALSE;
    VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, context->surface, &supports_present));
    if (supports_present) {
      out_queue_info->present_family_index = i;
    }
    if (out_queue_info->graphics_family_index != -1 &&
        out_queue_info->transfer_family_index != -1 &&
        out_queue_info->compute_family_index != -1 &&
        out_queue_info->present_family_index != -1) {
      break;
    }
  }

  Info("       %d |       %d |        %d |        %d | %s",
        out_queue_info->graphics_family_index != -1,
        out_queue_info->present_family_index != -1,
        out_queue_info->compute_family_index != -1,
        out_queue_info->transfer_family_index != -1,
        properties->deviceName);
  if (
      (!requirements->graphics || (requirements->graphics && out_queue_info->graphics_family_index != -1)) &&
      (!requirements->present || (requirements->present && out_queue_info->present_family_index != -1)) &&
      (!requirements->compute || (requirements->compute && out_queue_info->compute_family_index != -1)) &&
      (!requirements->compute || (requirements->compute && out_queue_info->transfer_family_index != -1))) {
    Info("Device meets queue requirements.");
    Trace("Grahics Family Index: %i", out_queue_info->graphics_family_index);
    Trace("Present Family Index: %i", out_queue_info->present_family_index);
    Trace("Transfer Family Index: %i", out_queue_info->transfer_family_index);
    Trace("Compute Family Index: %i", out_queue_info->compute_family_index);

    // Query swapchain support.
    vk_device_query_swapchain_support(device, context, out_swapchain_support);

    if (out_swapchain_support->format_count < 1 || out_swapchain_support->present_mode_count < 1) {
      Info("Required swapchain support not present, skipping device.");
      return false;
    }

    // Device extensions.
    {
      Scratch scratch;
      if (requirements->device_extension_names) {
        u32 available_extension_count = 0;
        VkExtensionProperties* available_extensions = 0;
        VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &available_extension_count, 0))
        if (available_extension_count != 0) {
          available_extensions = push_array(scratch, VkExtensionProperties, available_extension_count);
          VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &available_extension_count, available_extensions));
        }

        u32 required_extension_cunt = 1;
        for (i32 i = 0; i < required_extension_cunt; ++i) {
          b8 found = false;
          for (u32 j = 0; j < available_extension_count; ++j) {
            if (cstr_equal(requirements->device_extension_names[i], available_extensions[j].extensionName)) {
              found = true;
              break;
            }
          }

          if (!found) {
            Info("Required extension not found: '%s', skipping device.", requirements->device_extension_names);
            // kfree(available_extensions, sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
            return false;
          }
        }
      }
    }    

    // Sample anisotropy
    if (requirements->sampler_anisotropy && !features->samplerAnisotropy) {
      Info("Device does not support sampleAnisotropy, skipping.");
      return false;
    }
    
    // Device meets all requirements.
    return true;
  }
  
  return false;
}
