#include "vk_backend.h"
#include "vk_types.h"
#include "vk_device.h"
#include "vk_command_buffer.h"
#include "vk_buffer.h"
#include "vk_swapchain.h"
#include "vk_image.h"
#include "vk_draw.h"
#include "vk_shader.h"

#define VulkanUseAllocator
// #define VulkanAllocatorTrace 1

VK vk;
#ifdef VulkanUseAllocator

void* vk_alloc(void* user_data, u64 size, u64 alignment, VkSystemAllocationScope allocation_scope) {
  Assert(size)
  void* result = mem_alloc(size);
  mem_alloc(1);
  
#if VulkanAllocatorTrace
  Trace("vulkan allocate block %i bytes, %i alignment", size, alignment);
#endif
  return result;
}

void vk_free(void* user_data, void* memory) {
  if (!memory) {
    return;
  }
  mem_free(memory);
  
#if VulkanAllocatorTrace
  Trace("vulkan free %u address", (u32)memory);
#endif
}

void* vk_realloc(void* user_data, void* origin, u64 size, u64 alignment, VkSystemAllocationScope allocation_scope) {
  Assert(origin && size);
  void* result = mem_realoc(origin, size);
  
#if VulkanAllocatorTrace
  Trace("vulkan realoc block %i bytes, %i alignment", size, alignment);
#endif

  return result;
}

void vk_internal_alloc(void* user_data, u64 size, VkInternalAllocationType allocation_type, VkSystemAllocationScope allocation_scope) {
#if VulkanAllocatorTrace
  Trace("vulkan internal alloc %i bytes", size);
#endif
}
void vk_internal_free(void* user_data, u64 size, VkInternalAllocationType allocation_type, VkSystemAllocationScope allocation_scope) {
#if VulkanAllocatorTrace
  Trace("vulkan internal free %i bytes", size);
#endif
}

VkAllocationCallbacks vk_allocator_create() {
  VkAllocationCallbacks callbacks = {
    .pUserData = 0,
    .pfnAllocation = vk_alloc,
    .pfnReallocation = vk_realloc,
    .pfnFree = vk_free,
    .pfnInternalAllocation = vk_internal_alloc,
    .pfnInternalFree = vk_internal_free,
  };
  return callbacks;
}

#endif

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);

internal void buffers_create();
internal void vk_resize_viewport();

void vk_instance_create() {
  v2u framebuffer_extent = os_get_framebuffer_size();
  vk.frame.width =  framebuffer_extent.x;
  vk.frame.height = framebuffer_extent.y;

  VkApplicationInfo app_info = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .apiVersion = VK_API_VERSION_1_3
  };
  
  const char* required_extensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    vk_surface_extension_name()
  };

  const char* required_validation_layer_names[1];
  u32 required_validation_layer_count = 0;

#if _DEBUG
  Debug("Required extensions:");
  Loop (i, ArrayCount(required_extensions)) {
    Debug(String(required_extensions[i]));
  }

  required_validation_layer_names[0] = "VK_LAYER_KHRONOS_validation";
  
  u32 available_layer_count = 0;
  VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, 0));
  
  VkLayerProperties* available_layers = push_array(vk.arena, VkLayerProperties, available_layer_count);
  VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers));

  Loop (i, ArrayCount(required_validation_layer_names)) {
    b32 found = false;
    Loop (j, available_layer_count) {
      if (str_match(required_validation_layer_names[i], available_layers[j].layerName)) {
        ++available_layer_count;
        found = true;
        Info("Validation layer %s found", String(required_validation_layer_names[i]));
        break;
      }
    }
    AssertMsg(found, "Required validation layer is missing: %s", String(required_validation_layer_names[i]));
  }
#endif

  VkInstanceCreateInfo instance_create_info = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &app_info,
    .enabledLayerCount = required_validation_layer_count,
    .ppEnabledLayerNames = required_validation_layer_names,
    .enabledExtensionCount = ArrayCount(required_extensions),
    .ppEnabledExtensionNames = required_extensions,
  };
  
  VK_CHECK(vkCreateInstance(&instance_create_info, vk.allocator, &vk.instance));
  Info("Vulkan insance created");

#if _DEBUG
  VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
    .pfnUserCallback = vk_debug_callback,
  };

  PFN_vkCreateDebugUtilsMessengerEXT func; Assign(func, vkGetInstanceProcAddr(vk.instance, "vkCreateDebugUtilsMessengerEXT"));
  Assert(func && "Failed to create debug messenger");
  
  VK_CHECK(func(vk.instance, &debug_create_info, vk.allocator, &vk.debug_messenger));
  Debug("Vulkan debugger created");
#endif
}

void vk_r_backend_init(Arena* arena) {
  vk.arena = arena;
  
#ifdef VulkanUseAllocator
  vk._allocator = vk_allocator_create();
  vk.allocator = &vk._allocator;
#else
  vk.allocator = 0;
#endif

  vk_instance_create();
  vk_surface_create();
  vk_device_create();
  vk_swapchain_create(vk.frame.width, vk.frame.height);

  Loop (i, FramesInFlight) {
    vk.cmds[i] = vk_cmd_alloc(vk.device.cmd_pool);
    vk.compute_cmds[i] = vk_cmd_alloc(vk.device.cmd_pool);
  }
  Debug("Vulkan command buffers created");

  Loop (i, FramesInFlight) {
    VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(vkdevice, &semaphore_create_info, vk.allocator, &vk.sync.image_available_semaphores[i]);
    vkCreateSemaphore(vkdevice, &semaphore_create_info, vk.allocator, &vk.sync.queue_complete_semaphores[i]);
    vkCreateSemaphore(vkdevice, &semaphore_create_info, vk.allocator, &vk.sync.compute_complete_semaphores[i]);

    VkFenceCreateInfo fence_create_info = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    VK_CHECK(vkCreateFence(vkdevice, &fence_create_info, vk.allocator, &vk.sync.in_flight_fences[i]));
  }

  buffers_create();
  vk_shader_init();
  vk_draw_init();

  // Target texture render
  {
    Loop (i, ImagesInFlight) {
      vk.texture_targets[i].image = vk_image_create(
        VK_IMAGE_TYPE_2D,
        vk.frame.width, vk.frame.height,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_COLOR_BIT);

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
        .maxAnisotropy = 1,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
      };
      VK_CHECK(vkCreateSampler(vkdevice, &sampler_info, vk.allocator, &vk.texture_targets[i].sampler));
    }

    vk.offscreen_depth_buffer = vk_image_create(
      VK_IMAGE_TYPE_2D,
      vk.frame.width,
      vk.frame.height,
      vk.device.depth_format,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      true,
      VK_IMAGE_ASPECT_DEPTH_BIT);
  }

  // Viewport mode
  vk.is_viewport_render = true;
  {
    auto event_reg = [](u32 code, void* sender, void* listener_inst, EventContext context)->b32 {
      f32 width = context.u16[0];
      f32 height = context.u16[1];
      vk.viewport_size = {width, height};
      vk.is_viewport_resized = true;
      return false;
    };

    if (vk.is_viewport_mode) {
      event_register(EventCode_ViewportResized, &vk.is_viewport_resized, event_reg);
    } else {
      event_register(EventCode_Resized, &vk.is_viewport_resized, event_reg);
    }
  }

  Info("Vulkan renderer initialized");
}

void vk_r_backend_shutdown() {
  VK_CHECK(vkDeviceWaitIdle(vkdevice));
  
  // Sync objects
  Loop (i, FramesInFlight) {
    vkDestroySemaphore(vkdevice, vk.sync.image_available_semaphores[i], vk.allocator);
    vkDestroySemaphore(vkdevice, vk.sync.queue_complete_semaphores[i], vk.allocator);
    vkDestroyFence(vkdevice, vk.sync.in_flight_fences[i], vk.allocator);
  }
  
  // Command buffers
  Loop (i, FramesInFlight) {
    vk_cmd_free(vk.device.cmd_pool, vk.cmds[i]);
    vk_cmd_free(vk.device.cmd_pool, vk.compute_cmds[i]);
  }
  
  // Swapchain
  vk_swapchain_destroy();
  
  Debug("Destroying Vulkan device...");
  vk_device_destroy();
  
  Debug("Destroying Vulkan surface...");
  if (vk.surface) {
    vkDestroySurfaceKHR(vk.instance, vk.surface, vk.allocator);
    vk.surface = 0;
  }
  
  Debug("Destroying Vulkan debugger...");
  if (vk.debug_messenger) {
    PFN_vkDestroyDebugUtilsMessengerEXT func; Assign(func, vkGetInstanceProcAddr(vk.instance, "vkDestroyDebugUtilsMessengerEXT"));
    func(vk.instance, vk.debug_messenger, vk.allocator);
  }

  Debug("Destroying Vulkan instance...");
  vkDestroyInstance(vk.instance, vk.allocator);
}

void vk_r_on_resized(u32 width, u32 height) {
  vk.frame.width = width;
  vk.frame.height = height;
  ++vk.frame.size_generation;
  
  // Debug("Vulkan renderer backend->resized: w/h/gen %i/%i/%i", width, height, vk.frame.size_generation);
}

void vk_r_backend_begin_frame() {
  if (vk.frame.size_generation != vk.frame.size_last_generation) {
    VK_CHECK(vkDeviceWaitIdle(vkdevice));
    vk_swapchain_recreate(vk.frame.width, vk.frame.height);
  }

  if (vk.is_viewport_resized) {
    if (vk.viewport_size.x == 0 || vk.viewport_size.y == 0) {
      vk.is_viewport_render = false;
    } else {
      vk_resize_viewport();

      vk.is_viewport_resized = false;
      vk.is_viewport_render = true;
    }
  }

  VK_CHECK(vkWaitForFences(vkdevice, 1, &vk.sync.in_flight_fences[vk.frame.current_frame], true, U64_MAX));

  vk.frame.image_index = vk_swapchain_acquire_next_image_index(vk_get_current_image_available_semaphore());

  VkCommandBuffer cmd = vk_get_current_cmd();
  vk_cmd_begin(cmd);
  
  VkViewport viewport = {
    .x = 0.0f,
    .y = (f32)vk.frame.height,
    .width = (f32)vk.frame.width,
    .height = -(f32)vk.frame.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  
  VkRect2D scissor = {
    .offset = {.x = 0, .y = 0},
    .extent = {.width = (u32)vk.frame.width, .height = (u32)vk.frame.height},
  };
  
  vkCmdSetViewport(cmd, 0, 1, &viewport);
  vkCmdSetScissor(cmd, 0, 1, &scissor);
}

void vk_r_backend_end_frame() {
  VkCommandBuffer cmd = vk_get_current_cmd();

  vk_cmd_end(cmd);
  
  VK_CHECK(vkResetFences(vkdevice, 1, &vk.sync.in_flight_fences[vk.frame.current_frame]));
  
  // // compute
  // VkSubmitInfo compute_submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  // compute_submit_info.commandBufferCount = 1;
  // compute_submit_info.pCommandBuffers = &vk.compute_cmds[vk.frame.current_frame].handle;
  // compute_submit_info.signalSemaphoreCount = 1;
  // compute_submit_info.pSignalSemaphores = &vk.sync.compute_complete_semaphores[vk.frame.current_frame];
  
  // VK_CHECK(vkQueueSubmit(vk.device.graphics_queue, 1, &compute_submit_info, null));

  // graphics
  VkSemaphore semaphores_wait[] = {
    vk_get_current_image_available_semaphore(),
    vk.sync.compute_complete_semaphores[vk.frame.current_frame]
  };
  VkPipelineStageFlags sync_flags[] = {
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
  };
  VkSemaphore semaphore_queue_available = vk_get_current_queue_complete_semaphore();

  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = ArrayCount(semaphores_wait),
    .pWaitSemaphores = semaphores_wait,
    .pWaitDstStageMask = sync_flags,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &semaphore_queue_available,
  };

  VK_CHECK(vkQueueSubmit(vk.device.graphics_queue, 1, &submit_info, vk.sync.in_flight_fences[vk.frame.current_frame]));

  vk_swapchain_present(vk_get_current_queue_complete_semaphore(), vk.frame.image_index);
}

void vk_r_begin_renderpass(u32 renderpass_id) {
  VkCommandBuffer cmd = vk_get_current_cmd();

  switch (renderpass_id) {
    case Renderpass_World: {
      // Color
      VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .image = vk.texture_targets[vk.frame.image_index].image.handle,
        // .image = vk.swapchain.images[vk.frame.image_index],
        .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .layerCount = 1,
        },
      };

      vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        NoFlags,
        0, null,
        0, null,
        1, &barrier);

      VkRenderingAttachmentInfo color_attachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = vk.texture_targets[vk.frame.image_index].image.view,
        // .imageView = vk.swapchain.views[vk.frame.image_index],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        // .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, // NOTE: maybe when add cubemap
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {
          .color = {0.1f, 0.1f, 0.1f, 1.0f}
        },
      };

      // Depth
      VkImageMemoryBarrier depth_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .image = vk.offscreen_depth_buffer.handle,
        // .image = vk.swapchain.depth_attachment.handle,
        .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .layerCount = 1,
        },
      };

      vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        NoFlags,
        0, null,
        0, null,
        1, &depth_barrier);

      VkRenderingAttachmentInfo depth_attachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = vk.offscreen_depth_buffer.view,
        // .imageView = vk.swapchain.depth_attachment.view,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue = {
          .depthStencil = {1.0f, 0}
        },
      };

      // Start pass
      VkRenderingInfo render_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {
          .offset = {.x = 0, .y = 0}, 
          // .extent = {.width = (u32)vk.viewport_size.x, .height = (u32)vk.viewport_size.y}
          .extent = {.width = (u32)vk.frame.width, .height = (u32)vk.frame.height}
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment,
        .pDepthAttachment = &depth_attachment
      };

      vkCmdBeginRendering(cmd, &render_info);
    } break;
    case Renderpass_UI: {

      // // Color
      // VkImageMemoryBarrier barrier = {
      //   .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      //   .srcAccessMask = 0,
      //   .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      //   .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      //   .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      //   .image = vk.swapchain.images[vk.frame.image_index],
      //   .subresourceRange = {
      //     .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
      //     .baseMipLevel = 0,
      //     .levelCount = 1,
      //     .layerCount = 1,
      //   },
      // };

      // vkCmdPipelineBarrier(
      //   cmd,
      //   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      //   VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
      //   NoFlags,
      //   0, null, 0, null,
      //   1, &barrier);

      // VkRenderingAttachmentInfo color_attachment = {
      //   .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      //   .imageView = vk.swapchain.views[vk.frame.image_index],
      //   .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      //   .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      //   .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      //   .clearValue = { 
      //     .color = {0.01f, 0.01f, 0.01f, 1.0f},
      //   },
      // };

      // // Start pass
      // VkRenderingInfo render_info = {
      //   .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      //   .renderArea = {
      //     .offset = {.x = 0, .y = 0}, 
      //     .extent = { .width = vk.frame.width, .height = vk.frame.height }
      //   },
      //   .layerCount = 1,
      //   .colorAttachmentCount = 1,
      //   .pColorAttachments = &color_attachment,
      // };


      // vkCmdBeginRendering(cmd, &render_info);
    } break;

    case Renderpass_Screen: {
      VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .image = vk.swapchain.images[vk.frame.image_index],
        .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .layerCount = 1,
        },
      };

      vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        NoFlags,
        0, null,
        0, null,
        1, &barrier);


      VkRenderingAttachmentInfo color_attachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = vk.swapchain.views[vk.frame.image_index],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = { 
          .color = {0.01f, 0.01f, 0.01f, 1.0f},
        },
      };

      // Start pass
      VkRenderingInfo render_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {
          .offset = {.x = 0, .y = 0}, 
          .extent = { .width = vk.frame.width, .height = vk.frame.height }
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment,
      };


      vkCmdBeginRendering(cmd, &render_info);
    } break;
    default:
      Error("vulkan_renderer_begin_renderpass called on unrecognized renderpass id: %i", renderpass_id);
      return;
  }
}

void vk_r_end_renderpass(u32 renderpass_id) {
  VkCommandBuffer cmd = vk_get_current_cmd();

  switch (renderpass_id) {
    case Renderpass_World: {
      // NOTE transition to shader read texture
      VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .image = vk.texture_targets[vk.frame.image_index].image.handle,
        .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .layerCount = 1,
        },
      };

      vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        NoFlags,
        0, null,
        0, null,
        1, &barrier);

      // NOTE transition to present layout
      // VkImageMemoryBarrier barrier = {
      //   .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      //   .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      //   .dstAccessMask = 0,
      //   .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      //   .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      //   .image = vk.swapchain.images[vk.frame.image_index],
      //   .subresourceRange = {
      //     .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      //     .baseMipLevel = 0,
      //     .levelCount = 1,
      //     .layerCount = 1,
      //   },
      // };

      // vkCmdPipelineBarrier(
      //   cmd,
      //   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      //   VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
      //   0,
      //   0, null, 0, null,
      //   1, &barrier);

        vkCmdEndRendering(cmd);
    } break;
    case Renderpass_UI: {

      // VkImageMemoryBarrier barrier = {
      //   .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      //   .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      //   .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
      //   .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      //   .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      //   .image = vk.swapchain.images[vk.frame.image_index],
      //   .subresourceRange = {
      //     .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      //     .baseMipLevel = 0,
      //     .levelCount = 1,
      //     .layerCount = 1,
      //   },
      // };

      // vkCmdPipelineBarrier(
      //   cmd,
      //   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      //   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
      //   NoFlags,
      //   0, null, 0, null,
      //   1, &barrier);
          
    } break;

    case Renderpass_Screen: {
      VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image = vk.swapchain.images[vk.frame.image_index],
        .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .layerCount = 1,
        },
      };

      vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        NoFlags,
        0, null,
        0, null,
        1, &barrier);

    } break;
    default:
      Assert(true && "wrong id");
      return;
  }
}

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {
      
  switch (message_severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
      Error(String(callback_data->pMessage))
    } break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      Warn(String(callback_data->pMessage));
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      Info(String(callback_data->pMessage));
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      Trace(String(callback_data->pMessage));
      break;
    default: break;
  }

  return false;
}

internal void buffers_create() {
  // Vert
  vk.vert_buffer = vk_buffer_create(
    MB(1),
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  vk.vert_buffer.freelist = freelist_gpu_create(vk.arena, vk.vert_buffer.size);
  
  // Index
  vk.index_buffer = vk_buffer_create(
    MB(1),
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  vk.index_buffer.freelist = freelist_gpu_create(vk.arena, vk.index_buffer.size);
  
  // Stage
  vk.stage_buffer = vk_buffer_create(
    MB(8),
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  vk_buffer_map_memory(vk.stage_buffer, 0, vk.stage_buffer.size);
  
  // Storage
  vk.storage_buffer = vk_buffer_create(
    MB(1),
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  vk_buffer_map_memory(vk.storage_buffer, 0, vk.storage_buffer.size);
  vk.storage_buffer.freelist = freelist_gpu_create(vk.arena, vk.storage_buffer.size);
}

internal void vk_resize_viewport() {
  VK_CHECK(vkDeviceWaitIdle(vkdevice));
  Debug("texture target resized: x = %f y = %f", vk.viewport_size.x, vk.viewport_size.y);

  Loop (i, ImagesInFlight) {
    vk_image_destroy(vk.texture_targets[i].image);
    vk_image_destroy(vk.offscreen_depth_buffer);

    vk.texture_targets[i].image = vk_image_create(
      VK_IMAGE_TYPE_2D,
      vk.viewport_size.x, vk.viewport_size.y,
      VK_FORMAT_R8G8B8A8_UNORM,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      true,
      VK_IMAGE_ASPECT_COLOR_BIT);

    vk.offscreen_depth_buffer = vk_image_create(
      VK_IMAGE_TYPE_2D,
      vk.viewport_size.x,
      vk.viewport_size.y,
      vk.device.depth_format,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      true,
      VK_IMAGE_ASPECT_DEPTH_BIT);
  }
}

v2 vk_get_viewport_size() {
  return vk.viewport_size;
}
