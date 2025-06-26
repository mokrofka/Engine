#include "vk_device.h"

struct VK_PhysicalDeviceRequirements {
  b8 graphics; 
  b8 present;
  b8 transfer;
  b8 compute;
  b8 sampler_anisotropy;  
  b8 discrete_gpu;
  const char** device_extension_names;
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

internal b32 index_in_list(u32* list, u32 count, u32 value) {
  Loop (i, count) {
    if (list[i] == value) return true;
  }
  return false;
}
void vk_device_create() {
  vk.device = select_physical_device();
  
  Info("Creating logical device...");
  // NOTE: Do not create additional queues for shared indices.
  u32 indices[4];
  u32 index_count = 0;

  // Add graphics queue index
  indices[index_count++] = vk.device.graphics_queue_index;

  // Add present queue index if unique
  if (!index_in_list(indices, index_count, vk.device.present_queue_index)) {
    indices[index_count++] = vk.device.present_queue_index;
  }

  // Add transfer queue index if unique
  if (!index_in_list(indices, index_count, vk.device.transfer_queue_index)) {
    indices[index_count++] = vk.device.transfer_queue_index;
  }

  // Add compute queue index if unique
  if (!index_in_list(indices, index_count, vk.device.compute_queue_index)) {
    indices[index_count++] = vk.device.compute_queue_index;
  }

  VkDeviceQueueCreateInfo queue_create_infos[4];
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
  VkPhysicalDeviceFeatures device_features = {
    .fillModeNonSolid = true,  // Request anistrophy
    .samplerAnisotropy = true, // Request wireframe
  };
  
  const char* extension_names[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
  };
  VkDeviceCreateInfo device_create_info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .queueCreateInfoCount = index_count,
    .pQueueCreateInfos = queue_create_infos,
    .enabledExtensionCount = ArrayCount(extension_names),
    .ppEnabledExtensionNames = extension_names,
    .pEnabledFeatures = &device_features,
  };
  
  // Create the device.
  VK_CHECK(vkCreateDevice(vk.device.physical_device, &device_create_info, vk.allocator, &vk.device.logical_device));

  Info("Logical device created");
  
  // Get queues.
  vkGetDeviceQueue(vk.device.logical_device, vk.device.graphics_queue_index, 0, &vk.device.graphics_queue);
  vkGetDeviceQueue(vk.device.logical_device, vk.device.present_queue_index, 0, &vk.device.present_queue);
  vkGetDeviceQueue(vk.device.logical_device, vk.device.transfer_queue_index, 0, &vk.device.transfer_queue);
  vkGetDeviceQueue(vk.device.logical_device, vk.device.compute_queue_index, 0, &vk.device.compute_queue);
  
  Info("Queues obtained");
  
  VkCommandPoolCreateInfo graphics_pool_create_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = vk.device.graphics_queue_index,
  };
  VK_CHECK(vkCreateCommandPool(vk.device.logical_device, &graphics_pool_create_info, vk.allocator, &vk.device.cmd_pool));

  VkCommandPoolCreateInfo transfer_pool_create_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
    .queueFamilyIndex = vk.device.transfer_queue_index,
  };
  VK_CHECK(vkCreateCommandPool(vk.device.logical_device, &transfer_pool_create_info, vk.allocator, &vk.device.transient_cmd_pool));

  Info("Graphics command pool created");
}

void vk_device_destroy() {
  Info("Destroying command pools...");
  vkDestroyCommandPool(vkdevice, vk.device.cmd_pool, vk.allocator);
  vkDestroyCommandPool(vkdevice, vk.device.transient_cmd_pool, vk.allocator);

  // Destroy logical device
  Info("Destroying logical device...");
  vkDestroyDevice(vkdevice, vk.allocator);
  
  Info("Releasing physical device resources...");
}

VK_SwapchainSupportInfo* vk_device_query_swapchain_support(VkPhysicalDevice physical_device, VK_SwapchainSupportInfo* support_info) {
  // Surface capabilities
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, vk.surface, &support_info->capabilities));

  // Surface formats
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk.surface, &support_info->format_count, 0));
  Assert(support_info->format_count);
  if (!support_info->formats) {
    support_info->formats = push_array(vk.arena, VkSurfaceFormatKHR, support_info->format_count);
  }
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk.surface, &support_info->format_count, support_info->formats));
  
  // Present modes
  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk.surface, &support_info->present_mode_count, 0));
  Assert(support_info->present_mode_count);
  if (!support_info->present_modes) {
    support_info->present_modes = push_array(vk.arena, VkPresentModeKHR, support_info->present_mode_count);
  }
  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk.surface, &support_info->present_mode_count, support_info->present_modes));
  
  return support_info;
}

void vk_device_detect_depth_format(VK_Device* device) {
  // Format candidates
  u32 candidate_count = 3;
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

internal VK_Device select_physical_device() {
  Scratch scratch;
  VK_Device result;
  u32 physical_device_count = 0;
  VK_CHECK(vkEnumeratePhysicalDevices(vk.instance, &physical_device_count, 0));
  VK_Device* devices = push_array(scratch, VK_Device, physical_device_count);
  VkPhysicalDevice* physical_devices = push_array(scratch, VkPhysicalDevice, physical_device_count);
  i32 discrete_gpu_index = -1;
  i32 fallback_gpu_index = -1;

  VK_CHECK(vkEnumeratePhysicalDevices(vk.instance, &physical_device_count, physical_devices));
  Loop (i, physical_device_count) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_devices[i], &properties);
    
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physical_devices[i], &features);
    
    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory);
    
    const char* extentions[] = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    VK_PhysicalDeviceRequirements requirements = {
      .graphics = true,
      .present = true,
      .transfer = true,
      .compute = true,
      .sampler_anisotropy = true,
      .device_extension_names = extentions,
    };

    VK_PhysicalDeviceQueueFamilyInfo queue_info = physical_device_meets_requirements(
      physical_devices[i],
      properties,
      features,
      requirements,
      &devices[i].swapchain_support);

    Info("Available device: '%s'", String(properties.deviceName));
    // GPU type, etc.
    switch (properties.deviceType) {
    default:
      case VK_PHYSICAL_DEVICE_TYPE_OTHER: {
        Info("GPU type is Unkown");
        fallback_gpu_index = i;
      } break;
      case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: {
        Info("GPU type is Integrated");
        fallback_gpu_index = i;
      } break;
      case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: {
        discrete_gpu_index = i;
      } Info("GPU type is Descrete");
        break;
      case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: {
        Info("GPU type is Virtual");
        fallback_gpu_index = i;
      } break;
      case VK_PHYSICAL_DEVICE_TYPE_CPU: {
        Info("GPU type is CPU");
        fallback_gpu_index = i;
      } break;
    }

    Info("GPU Driver version: %i.%i.%i",
         VK_VERSION_MAJOR(properties.driverVersion),
         VK_VERSION_MINOR(properties.driverVersion),
         VK_VERSION_PATCH(properties.driverVersion));

    Info("GPU API version: %i.%i.%i",
         VK_VERSION_MAJOR(properties.apiVersion),
         VK_VERSION_MINOR(properties.apiVersion),
         VK_VERSION_PATCH(properties.apiVersion));

    Loop (j, memory.memoryHeapCount) {
      f32 memory_size_gib = (((f32)memory.memoryHeaps[j].size) / GB(1));
      if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
        Info("Local GPU memory: %.2f GiB", memory_size_gib);
      } else {
        Info("Shared System memory: %.2f GiB", memory_size_gib);
      }
    }

    devices[i].physical_device = physical_devices[i];
    devices[i].graphics_queue_index = queue_info.graphics_family_index;
    devices[i].present_queue_index = queue_info.present_family_index;
    devices[i].transfer_queue_index = queue_info.transfer_family_index;
    devices[i].compute_queue_index = queue_info.compute_family_index;

    devices[i].properties = properties;
    devices[i].features = features;
    devices[i].memory = memory;
  }
  i32 selected_index;
  if (discrete_gpu_index != -1) {
    selected_index = discrete_gpu_index; 
    Info("Discrete GPU was choosen");
  } else {
    selected_index = fallback_gpu_index;
    Info("Integrated GPU was choosen");
  }
  
  Assert(devices[selected_index].physical_device && "No physical devices were found which meet the requirements");
  
  Info("Physical device selected");
  return devices[selected_index];
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
  Scratch scratch;
  
  u32 queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);
  VkQueueFamilyProperties* queue_families = push_array(scratch, VkQueueFamilyProperties, queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);
  
  // Look at each queue and see what queues it supports
  Info("Graphics | Present | Computer | Transfer | Name");
  Loop (i, queue_family_count) {
    // Graphics queue?
    if ((queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queue_info.graphics_family_index == -1)) {
      queue_info.graphics_family_index = i;
    }

    // Transfer queue?
    if ((queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && (queue_info.transfer_family_index == -1) &&
        !(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) {
      queue_info.transfer_family_index = i;
    }

    // Compute queue?
    if ((queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && (queue_info.compute_family_index == -1) &&
        !(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT)) {
      queue_info.compute_family_index = i;
    }
    
    // Present queue?
    VkBool32 supports_present;
    VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vk.surface, &supports_present));
    if (supports_present && queue_info.present_family_index == -1) {
      queue_info.present_family_index = i;
    }
    if (queue_info.graphics_family_index != -1 &&
        queue_info.transfer_family_index != -1 &&
        queue_info.compute_family_index != -1 &&
        queue_info.present_family_index != -1) {
      break;
    }
  }

  if (queue_info.transfer_family_index == -1) queue_info.transfer_family_index = queue_info.graphics_family_index;
  if (queue_info.compute_family_index == -1) queue_info.compute_family_index = queue_info.graphics_family_index;

  Info("       %i |       %i |        %i |        %i | %s",
        queue_info.graphics_family_index != -1,
        queue_info.present_family_index != -1,
        queue_info.compute_family_index != -1,
        queue_info.transfer_family_index != -1,
        String(properties.deviceName));
  if (
      (!requirements.graphics || (requirements.graphics && queue_info.graphics_family_index != -1)) &&
      (!requirements.present || (requirements.present && queue_info.present_family_index != -1)) &&
      (!requirements.compute || (requirements.compute && queue_info.compute_family_index != -1)) &&
      (!requirements.compute || (requirements.compute && queue_info.transfer_family_index != -1))) {
    Info("Device meets queue requirements");
    Trace("Grahics Family Index: %i", queue_info.graphics_family_index);
    Trace("Present Family Index: %i", queue_info.present_family_index);
    Trace("Transfer Family Index: %i", queue_info.transfer_family_index);
    Trace("Compute Family Index: %i", queue_info.compute_family_index);

    // Query swapchain support.
    *out_swapchain_support = *vk_device_query_swapchain_support(device, &vk.device.swapchain_support);
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
            if (str_match(requirements.device_extension_names[i], available_extensions[j].extensionName)) {
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
  return {};
}
