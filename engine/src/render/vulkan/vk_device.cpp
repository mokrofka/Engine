#include "vk_device.h"

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

internal void select_physical_device(VK_Device* device);
internal VK_PhysicalDeviceQueueFamilyInfo physical_device_meets_requirements(
    VkPhysicalDevice device,
    VkPhysicalDeviceProperties properties,
    VkPhysicalDeviceFeatures features,
    VK_PhysicalDeviceRequirements requirements,
    VK_SwapchainSupportInfo* out_swapchain_support);

VK_Device vk_device_create() {
  VK_Device device;
  select_physical_device(&device);
  
  Info("Creating logical device..."_);
  // NOTE: Do not create additional queues for shared indices.
  b32 present_shares_graphics_queue = device.graphics_queue_index == device.present_queue_index;
  b32 transfer_shares_graphics_queue = device.graphics_queue_index == device.transfer_queue_index;
  u32 index_count = 1;
  if (!present_shares_graphics_queue) {
    ++index_count;
  }
  if (!transfer_shares_graphics_queue) {
    ++index_count;
  }
  u32 indices[32];
  u32 index = 0;
  indices[index++] = device.graphics_queue_index;
  if (!present_shares_graphics_queue) {
    indices[index++] = device.present_queue_index;
  }
  if (!transfer_shares_graphics_queue) {
    indices[index++] = device.transfer_queue_index;
  }
  
  VkDeviceQueueCreateInfo queue_create_infos[32];
  Loop (i, index_count) {
    queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[i].queueFamilyIndex = indices[i];
    queue_create_infos[i].queueCount = 1;
    queue_create_infos[i].flags = 0;
    queue_create_infos[i].pNext = 0;
    f32 queue_priority = 1.0f;
    queue_create_infos[i].pQueuePriorities = &queue_priority;
  }
  
  // Request device features.
  // TODO: should be config driven
  VkPhysicalDeviceFeatures device_features = {};
  device_features.samplerAnisotropy = VK_TRUE; // Request anistrophy
  device_features.fillModeNonSolid = VK_TRUE; // Request wireframe
  
  VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  device_create_info.queueCreateInfoCount = index_count;
  device_create_info.pQueueCreateInfos = queue_create_infos;
  device_create_info.pEnabledFeatures = &device_features;
  device_create_info.enabledExtensionCount = 1;
  const char* extension_names[1] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };
  device_create_info.ppEnabledExtensionNames = extension_names;
  
  // Deprecated and ingnored, so pass nothing.
  device_create_info.enabledLayerCount = 0;
  device_create_info.ppEnabledLayerNames = 0;
  
  // Create the device.
  VK_CHECK(vkCreateDevice(device.physical_device, &device_create_info, vk->allocator, &device.logical_device));

  Info("Logical device created"_);
  
  // Get queues.
  vkGetDeviceQueue(device.logical_device, device.graphics_queue_index, 0, &device.graphics_queue);
  vkGetDeviceQueue(device.logical_device, device.present_queue_index, 0, &device.present_queue);
  vkGetDeviceQueue(device.logical_device, device.transfer_queue_index, 0, &device.transfer_queue);
  
  Info("Queues obtained"_);
  
  // Create command pool for graphics queue.
  VkCommandPoolCreateInfo pool_create_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  pool_create_info.queueFamilyIndex = device.graphics_queue_index;
  pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  VK_CHECK(vkCreateCommandPool(device.logical_device, &pool_create_info, vk->allocator, &device.graphics_cmd_pool));
  Info("Graphics command pool created"_);

  return device;
}

void vk_device_destroy() {
  // Unset queues
  vk->device.graphics_queue = 0;
  vk->device.present_queue = 0;
  vk->device.transfer_queue = 0;
  
  Info("Destroying command pools..."_);
  vkDestroyCommandPool(vkdevice, vk->device.graphics_cmd_pool, vk->allocator);

  // Destroy logical device
  Info("Destroying logical device..."_);
  vkDestroyDevice(vkdevice, vk->allocator);
  vkdevice = 0;
  
  // Physical device are not destroyed.
  Info("Releasing physical device resources..."_);
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

VK_SwapchainSupportInfo* vk_device_query_swapchain_support(VkPhysicalDevice physical_device, VK_SwapchainSupportInfo* support_info) {
  // Surface capabilities
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, vk->surface, &support_info->capabilities));

  // Surface formats
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk->surface, &support_info->format_count, 0));
  Assert(support_info->format_count);
  if (!support_info->formats) {
    support_info->formats = push_array(vk->arena, VkSurfaceFormatKHR, support_info->format_count);
  }
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk->surface, &support_info->format_count, support_info->formats));
  
  // Present modes
  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk->surface, &support_info->present_mode_count, 0));
  Assert(support_info->present_mode_count);
  if (!support_info->present_modes) {
    support_info->present_modes = push_array(vk->arena, VkPresentModeKHR, support_info->present_mode_count);
  }
  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk->surface, &support_info->present_mode_count, support_info->present_modes));
  
  return support_info;
}

void vk_device_detect_depth_format(VK_Device* device) {
  // Format candidates
  u64 candidate_count = 3;
  VkFormat candidates[3] = {
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D24_UNORM_S8_UINT
  };

  u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
  Loop (i, candidate_count) {
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(device->physical_device, candidates[i], &properties);

    if ((properties.linearTilingFeatures & flags) == flags) {
      device->depth_format = candidates[i];
      return;
    } else if ((properties.optimalTilingFeatures & flags) == flags) {
      device->depth_format = candidates[i];
      return;
    }
  }

  Assert(!"Failed to find a supported format!");
}

internal void select_physical_device(VK_Device* device) {
  u32 physical_device_count = 0;
  VK_CHECK(vkEnumeratePhysicalDevices(vk->instance, &physical_device_count, 0));
  
  const u32 max_device_count = 32;
  VkPhysicalDevice physical_devices[max_device_count];
  VK_CHECK(vkEnumeratePhysicalDevices(vk->instance, &physical_device_count, physical_devices));
  Loop (i, physical_device_count) {
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
    
    const char* extentions[1] = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    requirements.device_extension_names = extentions;

    VK_PhysicalDeviceQueueFamilyInfo queue_info = physical_device_meets_requirements(
      physical_devices[i],
      properties,
      features,
      requirements,
      &device->swapchain_support);

    Info("Selected device: '%s'", str_cstr(properties.deviceName));
    // GPU type, etc.
    switch (properties.deviceType) {
    default:
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
      Info("GPU type is Unkown"_);
      break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      Info("GPU type is Integrated"_);
      break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      Info("GPU type is Descrete"_);
      break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      Info("GPU type is Virtual"_);
      break;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
      Info("GPU type is CPU"_);
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
    Loop (j, memory.memoryHeapCount) {
      f32 memory_size_gib = (((f32)memory.memoryHeaps[j].size) / GB(1));
      if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
        Info("Local GPU memory: %.2f GiB", memory_size_gib);
      } else {
        Info("Shared System memory: %.2f GiB", memory_size_gib);
      }
    }

    device->physical_device = physical_devices[i];
    device->graphics_queue_index = queue_info.graphics_family_index;
    device->present_queue_index = queue_info.present_family_index;
    device->transfer_queue_index = queue_info.transfer_family_index;
    // NOTE: set computer index here if needed.

    // Keep a copy of properties, features and memory info for later use.
    device->properties = properties;
    device->features = features;
    device->memory = memory;
  }
  
  // Ensure a device was selected
  Assert(device->physical_device && "No physical devices were found which meet the requirements");
  
  Info("Physical device selected"_);
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
      Info("Device is not a discrete GPU, and one is required. Skipping"_);
    }
  }
  
  u32 queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);
  VkQueueFamilyProperties queue_families[32] = {};
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);
  
  // Look at each queue and see what queues it supports
  Info("Graphics | Present | Computer | Transfer | Name"_);
  Loop (i, queue_family_count) {
    // Graphics queue?
    if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      queue_info.graphics_family_index = i;
    }
    
    // Compute queue?
    if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      queue_info.compute_family_index = i;
    }
    
    // Transfer queue?
    if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
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

  Info("       %i |       %i |        %i |        %i | %s",
        queue_info.graphics_family_index != -1,
        queue_info.present_family_index != -1,
        queue_info.compute_family_index != -1,
        queue_info.transfer_family_index != -1,
        str_cstr(properties.deviceName));
  if (
      (!requirements.graphics || (requirements.graphics && queue_info.graphics_family_index != -1)) &&
      (!requirements.present || (requirements.present && queue_info.present_family_index != -1)) &&
      (!requirements.compute || (requirements.compute && queue_info.compute_family_index != -1)) &&
      (!requirements.compute || (requirements.compute && queue_info.transfer_family_index != -1))) {
    Info("Device meets queue requirements"_);
    Trace("Grahics Family Index: %i", queue_info.graphics_family_index);
    Trace("Present Family Index: %i", queue_info.present_family_index);
    Trace("Transfer Family Index: %i", queue_info.transfer_family_index);
    Trace("Compute Family Index: %i", queue_info.compute_family_index);

    // Query swapchain support.
    *out_swapchain_support = *vk_device_query_swapchain_support(device, &vk->device.swapchain_support);
    Assert(!(out_swapchain_support->format_count < 1) || !(out_swapchain_support->present_mode_count < 1) &&
             "Required swapchain support not present, skipping device");

    // Device extensions.
    {
      Scratch scratch;
      if (requirements.device_extension_names) {
        u32 available_extension_count = 0;
        VkExtensionProperties* available_extensions = 0;
        VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &available_extension_count, 0))
        if (available_extension_count != 0) {
          available_extensions = push_array(scratch.arena, VkExtensionProperties, available_extension_count);
          VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &available_extension_count, available_extensions));
        }

        u32 required_extension_count = 1;
        Loop (i, required_extension_count) {
          b32 found = false;
          Loop (j, available_extension_count) {
            if (cstr_match(requirements.device_extension_names[i], available_extensions[j].extensionName)) {
              found = true;
              break;
            }
          }

          Assert(found);
        }
      }
    }    

    Assert(requirements.sampler_anisotropy && features.samplerAnisotropy);
    
    // Device meets all requirements.
    return queue_info;
  }
  
  Assert(!"Device doesn't have needed queues");
  VK_PhysicalDeviceQueueFamilyInfo r = {}; return r;
}
