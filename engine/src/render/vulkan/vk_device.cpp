#include "vk_device.h"

#include "str.h"

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

internal VK_Device select_physical_device();
internal VK_PhysicalDeviceQueueFamilyInfo physical_device_meets_requirements(
    VkPhysicalDevice device,
    VkPhysicalDeviceProperties properties,
    VkPhysicalDeviceFeatures features,
    VK_PhysicalDeviceRequirements requirements,
    VK_SwapchainSupportInfo* out_swapchain_support);

VK_Device vk_device_create() {
  VK_Device device = select_physical_device();
  
  Info("Creating logical device...");
  // NOTE: Do not create additional queues for shared indices.
  b8 present_shares_graphics_queue = device.graphics_queue_index == device.present_queue_index;
  b8 transfer_shares_graphics_queue = device.graphics_queue_index == device.transfer_queue_index;
  u32 index_count = 1;
  if (!present_shares_graphics_queue) {
    ++index_count;
  }
  if (!transfer_shares_graphics_queue) {
    ++index_count;
  }
  u32 indices[32];
  u8 index = 0;
  indices[index++] = device.graphics_queue_index;
  if (!present_shares_graphics_queue) {
    indices[index++] = device.present_queue_index;
  }
  if (!transfer_shares_graphics_queue) {
    indices[index++] = device.transfer_queue_index;
  }
  
  VkDeviceQueueCreateInfo queue_create_infos[32];
  for (u32 i = 0; i < index_count; ++i) {
    queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[i].queueFamilyIndex = indices[i];
    queue_create_infos[i].queueCount = 1;
    // TODO: Enable this for a future enhancement.
    // if (indices[i] == vk->device.graphics_queue_index) {
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
    device.physical_device,
    &device_create_info,
    vk->allocator,
    &device.logical_device));

  Info("Logical device created.");
  
  // Get queues.
  vkGetDeviceQueue(
    device.logical_device, 
    device.graphics_queue_index, 
    0, 
    &device.graphics_queue);
    
  vkGetDeviceQueue(
    device.logical_device, 
    device.present_queue_index, 
    0, 
    &device.present_queue);
    
  vkGetDeviceQueue(
    device.logical_device, 
    device.transfer_queue_index, 
    0, 
    &device.transfer_queue);
  Info("Queues obtained.");
  
  // Create command pool for graphics queue.
  VkCommandPoolCreateInfo pool_create_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  pool_create_info.queueFamilyIndex = device.graphics_queue_index;
  pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  VK_CHECK(vkCreateCommandPool(
    device.logical_device, 
    &pool_create_info, 
    vk->allocator, 
    &device.gfx_cmd_pool));
  Info("Graphics command pool created.");

  return device;
}

void vk_device_destroy() {
  // Unset queues
  vk->device.graphics_queue = 0;
  vk->device.present_queue = 0;
  vk->device.transfer_queue = 0;
  
  Info("Destroying command pools...");
  vkDestroyCommandPool(
    vkdevice, 
    vk->device.gfx_cmd_pool, 
    vk->allocator);
  
  // Destroy logical device
  Info("Destroying logical device...");
  if (vkdevice) {
    vkDestroyDevice(vkdevice, vk->allocator);
    vkdevice = 0;
  }
  
  // Physical device are not destroyed.
  Info("Releasing physical device resources...");
  vk->device.physical_device = 0;
  
  if (vk->device.swapchain_support.formats) {
    vk->device.swapchain_support.formats = 0;
    vk->device.swapchain_support.format_count = 0;
  }
  
  if (vk->device.swapchain_support.present_modes) {
    vk->device.swapchain_support.present_modes = 0;
    vk->device.swapchain_support.present_mode_count = 0;
  }

  MemZeroStruct(&vk->device.swapchain_support.capabilities);

  vk->device.graphics_queue_index = -1;
  vk->device.present_queue_index = -1;
  vk->device.transfer_queue_index = -1;
}

VK_SwapchainSupportInfo vk_device_query_swapchain_support(VkPhysicalDevice physical_device) {
  VK_SwapchainSupportInfo& result = vk->device.swapchain_support;

  // Surface capabilities
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physical_device,
      vk->surface,
      &result.capabilities));

  // Surface formats 
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
    physical_device,
    vk->surface,
    &result.format_count,
    0));
  if (result.format_count != 0) {
    if (!result.formats) {
      // TODO: free memory
      result.formats = push_array(vk->arena, VkSurfaceFormatKHR, result.format_count);
    }
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device,
        vk->surface,
        &result.format_count,
        result.formats));
  }
  
  // Present modes
  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
    physical_device,
    vk->surface,
    &result.present_mode_count,
    0));
  if (result.present_mode_count != 0) {
    if (!result.present_modes) {
      // TODO: free memory
      result.present_modes = push_array(vk->arena, VkPresentModeKHR, result.present_mode_count);
    }
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device,
        vk->surface,
        &result.present_mode_count,
        result.present_modes));
  }
  
  return result;
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

internal VK_Device select_physical_device() {
  VK_Device device = {};
  u32 physical_device_count = 0;
  VK_CHECK(vkEnumeratePhysicalDevices(vk->instance, &physical_device_count, 0));
  
  const u32 max_device_count = 32;
  VkPhysicalDevice physical_devices[max_device_count];
  VK_CHECK(vkEnumeratePhysicalDevices(vk->instance, &physical_device_count, physical_devices));
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

    VK_PhysicalDeviceQueueFamilyInfo queue_info = physical_device_meets_requirements(
        physical_devices[i],
        properties,
        features,
        requirements,
        &device.swapchain_support);

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

    device.physical_device = physical_devices[i];
    device.graphics_queue_index = queue_info.graphics_family_index;
    device.present_queue_index = queue_info.present_family_index;
    device.transfer_queue_index = queue_info.transfer_family_index;
    // NOTE: set computer index here if needed.

    // Keep a copy of properties, features and memory info for later use.
    device.properties = properties;
    device.features = features;
    device.memory = memory;
  }
  
  // Ensure a device was selected
  if (!device.physical_device) {
    Error("No physical devices were found which meet the requirements.");
  }
  
  Info("Physical device selected.");
  return device;
}

internal VK_PhysicalDeviceQueueFamilyInfo physical_device_meets_requirements(
    VkPhysicalDevice device,
    VkPhysicalDeviceProperties properties,
    VkPhysicalDeviceFeatures features,
    VK_PhysicalDeviceRequirements requirements,
    VK_SwapchainSupportInfo* out_swapchain_support) {
  VK_PhysicalDeviceQueueFamilyInfo queue_info = {};
  // Evalueate device properties to determina if it meets the needs of our application.
  queue_info.graphics_family_index = -1;  
  queue_info.present_family_index = -1;  
  queue_info.compute_family_index = -1;  
  queue_info.transfer_family_index = -1;  
  
  // Discrete GPU?
  if (requirements.discrete_gpu) {
    if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      Info("Device is not a discrete GPU, and one is required. Skipping.");
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
      queue_info.graphics_family_index = i;
      ++current_transfer_score;
    }
    
    // Compute queue?
    if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      queue_info.compute_family_index = i;
      ++current_transfer_score;
    }
    
    // Transfer queue?
    if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
      // Take the index if it is the current lowest. This increased the
      // likegood that it is a dedicated transfer queue.
      // if (current_transfer_score <= min_transfer_score) {
      //   min_transfer_score = current_transfer_score;
      //   queue_info.transfer_family_index = 0;
      // }
      queue_info.transfer_family_index = i;
    }
    
    VkBool32 supports_present = VK_FALSE;
    VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vk->surface, &supports_present));
    if (supports_present) {
      queue_info.present_family_index = i;
    }
    if (queue_info.graphics_family_index != -1 &&
        queue_info.transfer_family_index != -1 &&
        queue_info.compute_family_index != -1 &&
        queue_info.present_family_index != -1) {
      break;
    }
  }

  Info("       %d |       %d |        %d |        %d | %s",
        queue_info.graphics_family_index != -1,
        queue_info.present_family_index != -1,
        queue_info.compute_family_index != -1,
        queue_info.transfer_family_index != -1,
        properties.deviceName);
  if (
      (!requirements.graphics || (requirements.graphics && queue_info.graphics_family_index != -1)) &&
      (!requirements.present || (requirements.present && queue_info.present_family_index != -1)) &&
      (!requirements.compute || (requirements.compute && queue_info.compute_family_index != -1)) &&
      (!requirements.compute || (requirements.compute && queue_info.transfer_family_index != -1))) {
    Info("Device meets queue requirements.");
    Trace("Grahics Family Index: %i", queue_info.graphics_family_index);
    Trace("Present Family Index: %i", queue_info.present_family_index);
    Trace("Transfer Family Index: %i", queue_info.transfer_family_index);
    Trace("Compute Family Index: %i", queue_info.compute_family_index);

    // Query swapchain support.
    *out_swapchain_support = vk_device_query_swapchain_support(device);
    if (out_swapchain_support->format_count < 1 || out_swapchain_support->present_mode_count < 1) {
      Error("Required swapchain support not present, skipping device.");
    }

    // Device extensions.
    {
      Scratch scratch;
      if (requirements.device_extension_names) {
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
            if (cstr_equal(requirements.device_extension_names[i], available_extensions[j].extensionName)) {
              found = true;
              break;
            }
          }

          if (!found) {
            Error("Required extension not found: '%s', skipping device.", requirements.device_extension_names);
          }
        }
      }
    }    

    // Sample anisotropy
    if (requirements.sampler_anisotropy && !features.samplerAnisotropy) {
      Error("Device does not support sampleAnisotropy, skipping.");
    }
    
    // Device meets all requirements.
    return queue_info;
  }
  
  Error("Device doesn't have needed queues.");
  ReturnZeroStruct(&queue_info);
}
