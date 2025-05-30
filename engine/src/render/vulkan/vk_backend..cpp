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
  VkAllocationCallbacks callbacks = {};
  callbacks.pfnAllocation = vk_alloc;
  callbacks.pfnReallocation = vk_realloc;
  callbacks.pfnFree = vk_free;
  callbacks.pfnInternalAllocation = vk_internal_alloc;
  callbacks.pfnInternalFree = vk_internal_free;
  callbacks.pUserData = 0;
  return callbacks;
}

#endif

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);

internal void recreate_swapchain(VK_Swapchain* swapchain);
internal void create_buffers(VK_Render* render);

void instance_create() {
  v2i framebuffer_extent = os_get_framebuffer_size();
  vk.frame.width =  framebuffer_extent.x;
  vk.frame.height = framebuffer_extent.y;

  VkApplicationInfo app_info = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .apiVersion = VK_API_VERSION_1_3
  };
  
  VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  create_info.pApplicationInfo = &app_info;
  
  // Obtain a list of required extensions
  char* required_extensions[3] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    "VK_KHR_win32_surface",
  };
#if _DEBUG
  Debug("Required extensions:"_);
  Loop (i, ArrayCount(required_extensions)) {
    Debug(str_cstr(required_extensions[i]));
  }
#endif

  create_info.enabledExtensionCount = ArrayCount(required_extensions);
  create_info.ppEnabledExtensionNames = required_extensions;
  
  char* required_validation_layer_names[1];
  u32 required_validation_layer_count = 0;
  
#if defined(_DEBUG)
  Info("Validation layers enabled. Enumerating..."_);
  required_validation_layer_names[0] = "VK_LAYER_KHRONOS_validation";
  
  u32 available_layer_count = 0;
  VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, 0));
  
  VkLayerProperties* available_layers = push_array(vk.arena, VkLayerProperties, available_layer_count);
  VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers));

  // Verify all required layers are available.
  Loop (i, ArrayCount(required_validation_layer_names)) {
    Info("Searching for layer: %s...", str_cstr(required_validation_layer_names[i]));
    b32 found = false;
    Loop (j, available_layer_count) {
      if (cstr_match(required_validation_layer_names[i], available_layers[j].layerName)) {
        found = true;
        Info("Found"_);
        break;
      }
    }
    if (!found) {
      Fatal("Required validation layer is missing: %s", str_cstr(required_validation_layer_names[i]));
    }
  }
  Info("All required validation layers are present"_);
#endif

  create_info.enabledLayerCount = required_validation_layer_count;
  create_info.ppEnabledLayerNames = required_validation_layer_names;
  
  VK_CHECK(vkCreateInstance(&create_info, vk.allocator, &vk.instance));
  Info("Vulkan insance created"_);

  // Debugger
#ifdef _DEBUG
  Debug("Creating Vulkan debugger..."_);
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

  PFN_vkCreateDebugUtilsMessengerEXT func; Assign(func, vkGetInstanceProcAddr(vk.instance, "vkCreateDebugUtilsMessengerEXT"));

  Assert(func && "Failed to create debug messenger");
  VK_CHECK(func(vk.instance, &debug_create_info, vk.allocator, &vk.debug_messenger));
  Debug("Vulkan debugger created"_);
#endif
}

void vk_r_backend_init(Arena* arena) {
  vk.arena = arena;
  
  // NOTE: custom allocator.
#ifdef VulkanUseAllocator
  vk._allocator = vk_allocator_create();
  vk.allocator = &vk._allocator;
#else
  vk.allocator = 0;
#endif

  instance_create();
  
  vk_surface_create();

  vk_device_create();

  vk_swapchain_create(vk.frame.width, vk.frame.height);

  Loop (i, FramesInFlight) {
    vk.cmds[i] = vk_cmd_alloc(vk.device.cmd_pool);
    vk.compute_cmds[i] = vk_cmd_alloc(vk.device.cmd_pool);
  }
  Debug("Vulkan command buffers created"_);

  Loop (i, vk.swapchain.max_frames_in_flight) {
    VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(vkdevice, &semaphore_create_info, vk.allocator, &vk.sync.image_available_semaphores[i]);
    vkCreateSemaphore(vkdevice, &semaphore_create_info, vk.allocator, &vk.sync.queue_complete_semaphores[i]);
    vkCreateSemaphore(vkdevice, &semaphore_create_info, vk.allocator, &vk.sync.compute_complete_semaphores[i]);

    VkFenceCreateInfo fence_create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VK_CHECK(vkCreateFence(vkdevice, &fence_create_info, vk.allocator, &vk.sync.in_flight_fences[i]));
  }

  create_buffers(&vk.render);
  vk_shader_init();

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

    VkSamplerCreateInfo sampler_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    // TODO These filters shoud be configurable
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.maxAnisotropy = 1;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;
    VK_CHECK(vkCreateSampler(vkdevice, &sampler_info, vk.allocator, &vk.texture_targets[i].sampler));
  }

  vk.depth = vk_image_create(
      VK_IMAGE_TYPE_2D,
      vk.frame.width,
      vk.frame.height,
      vk.device.depth_format,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      true,
      VK_IMAGE_ASPECT_DEPTH_BIT);
  vk.viewport_size = v2(vk.frame.width, vk.frame.height);

  Info("Vulkan renderer initialized successfully"_);
}

void vk_r_backend_shutdown() {
  vkDeviceWaitIdle(vkdevice);
  
  // Sync objects
  Loop (i, vk.swapchain.max_frames_in_flight) {
    vkDestroySemaphore(vkdevice, vk.sync.image_available_semaphores[i], vk.allocator);
    vkDestroySemaphore(vkdevice, vk.sync.queue_complete_semaphores[i], vk.allocator);
    vkDestroyFence(vkdevice, vk.sync.in_flight_fences[i], vk.allocator);
  }
  
  // Command buffers
  Loop (i, FramesInFlight) {
    vk_cmd_free(vk.device.cmd_pool, vk.cmds[i]);
    vk_cmd_free(vk.device.cmd_pool, vk.compute_cmds[i]);
  }
  
  Loop (i, FramesInFlight) {
    vkDestroyFramebuffer(vkdevice, vk.world_framebuffers[i], vk.allocator);
    vkDestroyFramebuffer(vkdevice, vk.swapchain.framebuffers[i], vk.allocator);
  }
  
  // Swapchain
  vk_swapchain_destroy(&vk.swapchain);
  
  Debug("Destroying Vulkan device..."_);
  vk_device_destroy();
  
  Debug("Destroying Vulkan surface..."_);
  if (vk.surface) {
    vkDestroySurfaceKHR(vk.instance, vk.surface, vk.allocator);
    vk.surface = 0;
  }
  
  Debug("Destroying Vulkan debugger..."_);
  if (vk.debug_messenger) {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            vk.instance, "vkDestroyDebugUtilsMessengerEXT");
    func(vk.instance, vk.debug_messenger, vk.allocator);
  }

  Debug("Destroying Vulkan instance..."_);
  vkDestroyInstance(vk.instance, vk.allocator);
}

void vk_r_backend_on_resize(u32 width, u32 height) {
  vk.frame.width = width;
  vk.frame.height = height;
  ++vk.frame.size_generation;
  
  Info("Vulkan renderer backend->resized: w/h/gen %i/%i/%i", width, height, vk.frame.size_generation);
}

void vk_r_backend_begin_frame() {
  // Check if the framebuffer has been resized. If so, a new swapchain must be created.
  if (vk.frame.size_generation != vk.frame.size_last_generation) {
    VK_CHECK(vkDeviceWaitIdle(vkdevice));
    
    recreate_swapchain(&vk.swapchain);
    
    Info("Resized, booting"_);
  }

  if (vk.is_viewport_sezied) {
    vk_resize_framebuffer();
  }

  // Wait for the execution of the current frame to complete. The fence being free will allow this one to move on.
  VK_CHECK(vkWaitForFences(vkdevice, 1, &vk.sync.in_flight_fences[vk.frame.current_frame], true, U64_MAX));

  // Acquire the next image from the swap chain. Pass along the semaphore that should signaled when this completes.
  // This same semaphore will later be waited on by the queue submission to ensure this image is available.
  vk.frame.image_index = vk_swapchain_acquire_next_image_index(&vk.swapchain, vk_get_current_image_available_semaphore(), 0);

  // Begin recording commands.
  VkCommandBuffer cmd = vk_get_current_cmd();
  vk_cmd_begin(cmd);
  
  // Dynamic state
  VkViewport viewport;
  viewport.x = 0.0f;
  viewport.y = vk.viewport_size.y;
  viewport.width = vk.viewport_size.x;
  viewport.height = -(f32)vk.viewport_size.y;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  
  // Scissor
  VkRect2D scissor;
  scissor.offset.x = scissor.offset.y = 0;
  scissor.extent.width = vk.viewport_size.x;
  scissor.extent.height = vk.viewport_size.y;
  
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
  VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd;
  
  submit_info.signalSemaphoreCount = 1;
  VkSemaphore queue_available = vk_get_current_queue_complete_semaphore();
  submit_info.pSignalSemaphores = &queue_available;

  VkSemaphore wait[] = {
    vk_get_current_image_available_semaphore(),
    vk.sync.compute_complete_semaphores[vk.frame.current_frame]
  };
  submit_info.waitSemaphoreCount = ArrayCount(wait);
  // VkSemaphore image_available = vk_get_current_image_available_semaphore();
  // submit_info.pWaitSemaphores = &image_available;
  submit_info.pWaitSemaphores = wait;

  // Each semaphore waits on the corresponding pipeline stage to complete. 1:1 ratio.
  // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent color attachment
  // writes from executing until the semaphore signals (i.e. one frame is presented at a time)
  VkPipelineStageFlags flags[] = {
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
  };
  submit_info.pWaitDstStageMask = flags;

  // VK_CHECK(vkQueueSubmit(vk.device.graphics_queue, 1, &submit_info, vk.sync.in_flight_fences[vk.frame.current_frame]));
  VkBool32 result = vkQueueSubmit(vk.device.graphics_queue, 1, &submit_info, vk.sync.in_flight_fences[vk.frame.current_frame]);
  VK_CHECK(result);

  vk_swapchain_present(&vk.swapchain, vk.device.present_queue, vk_get_current_queue_complete_semaphore(), vk.frame.image_index);
}

void vk_r_begin_renderpass(u32 renderpass_id) {
  VkCommandBuffer cmd = vk_get_current_cmd();

  // Choose a renderpass based on ID.
  switch (renderpass_id) {
    case BuiltinRenderpass_World: {

  {
    // Color
      VkImageMemoryBarrier barrier = {};
      barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Or whatever it currently is
      barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      // barrier.image = vk.swapchain.images[vk.frame.image_index];
      barrier.image = vk.texture_targets[vk.frame.image_index].image.handle;
      barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      barrier.subresourceRange.levelCount = 1;
      barrier.subresourceRange.layerCount = 1;

      vkCmdPipelineBarrier(
          cmd,
          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          0,
          0, null, 0, null,
          1, &barrier);

      VkRenderingAttachmentInfo color_attachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        // .imageView = vk.swapchain.views[vk.frame.image_index],
        .imageView = vk.texture_targets[vk.frame.image_index].image.view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue.color = {{0.1f, 0.1f, 0.1f, 1.0f}},
      };

    // Depth
      VkImageMemoryBarrier depthBarrier = {};
      depthBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      depthBarrier.srcAccessMask = 0;
      depthBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      depthBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; // or current layout
      depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
      depthBarrier.image = vk.depth.handle;
      // depthBarrier.image = vk.swapchain.depth_attachment.handle;
      depthBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
      depthBarrier.subresourceRange.levelCount = 1;
      depthBarrier.subresourceRange.layerCount = 1;

      vkCmdPipelineBarrier(
          cmd,
          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
          VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
          0,
          0, null,
          0, null,
          1, &depthBarrier);

      VkRenderingAttachmentInfo depth_attachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        // .imageView = vk.swapchain.depth_attachment.view,
        .imageView = vk.depth.view,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue.depthStencil = {1.0f, 0},    // typical depth clear (far plane)
      };

    // start pass
    VkRenderingInfo render_info = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      // .renderArea = {{0, 0}, {vk.frame.width, vk.frame.height}},
      .renderArea = {{0, 0}, {(u32)vk.viewport_size.x, (u32)vk.viewport_size.y}},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment,
      .pDepthAttachment = &depth_attachment
    };

    vkCmdBeginRendering(cmd, &render_info);

  }

      
    } break;
    case BuiltinRenderpass_UI: {



    // Color
      VkImageMemoryBarrier barrier = {};
      barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Or whatever it currently is
      barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      barrier.image = vk.swapchain.images[vk.frame.image_index];
      barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      barrier.subresourceRange.levelCount = 1;
      barrier.subresourceRange.layerCount = 1;

      vkCmdPipelineBarrier(
          cmd,
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
          0,
          0, null, 0, null,
          1, &barrier);

      VkRenderingAttachmentInfo color_attachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = vk.swapchain.views[vk.frame.image_index],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue.color = {{0.01f, 0.01f, 0.01f, 1.0f}},
      };


      VkRenderingInfo render_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {{0, 0}, {vk.frame.width, vk.frame.height}},
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
  VK_Renderpass* renderpass = 0;
  VkCommandBuffer cmd = vk_get_current_cmd();

  // Choose a renderpass based on ID.
  switch (renderpass_id) {
  case BuiltinRenderpass_World: {
    // NOTE transition to shader read texture
    VkImageMemoryBarrier barrier2 = {};
    barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier2.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier2.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier2.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier2.image = vk.texture_targets[vk.frame.image_index].image.handle;
    barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier2.subresourceRange.levelCount = 1;
    barrier2.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, null, 0, null,
        1, &barrier2);

    // NOTE transition to present layout
    // VkImageMemoryBarrier barrier2 = {};
    // barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    // barrier2.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    // barrier2.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    // barrier2.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // barrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // barrier2.image = vk.swapchain.images[vk.frame.image_index];
    // barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // barrier2.subresourceRange.levelCount = 1;
    // barrier2.subresourceRange.layerCount = 1;

    // vkCmdPipelineBarrier(
    //     cmd,
    //     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //     VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    //     0,
    //     0, null, 0, null,
    //     1, &barrier2);

    vkCmdEndRendering(cmd);
  }
    break;
  case BuiltinRenderpass_UI: {

    VkImageMemoryBarrier barrier2 = {};
    barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier2.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier2.dstAccessMask = 0;
    barrier2.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier2.image = vk.swapchain.images[vk.frame.image_index];
    barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier2.subresourceRange.levelCount = 1;
    barrier2.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr, 0, nullptr,
        1, &barrier2);
        
    vkCmdEndRendering(cmd);
  } break;
  default:
    Error("vk_renderer_end_renderpass called on unrecognized renderpass id:  %#02x", renderpass_id);
    return;
  }

  // vk_renderpass_end(cmd);
}

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {
  switch (message_severity) {
    default:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
      Error(str_cstr(callback_data->pMessage))
    } break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      Warn(str_cstr(callback_data->pMessage));
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      Info(str_cstr(callback_data->pMessage));
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      Trace(str_cstr(callback_data->pMessage));
      break;
    }
  return VK_FALSE;
}

internal void recreate_swapchain(VK_Swapchain* swapchain) {
  // Detect if the window is too small to be drawn to
  if (vk.frame.width == 0 || vk.frame.height == 0) {
    Debug("recreate_swapchain called when window is < 1 in a dimension. Booting"_);
  }

  // Mark as recreating if the dimensions are valid.
  vk.recreating_swapchain = true;

  vk_swapchain_recreate(swapchain, vk.frame.width, vk.frame.height);
  

  // Update framebuffer size generation.
  vk.frame.size_last_generation = vk.frame.size_generation;

  // Clear the recreating flag.
  vk.recreating_swapchain = false;
}

internal void create_buffers(VK_Render* render) {
  // vert
  vk.vert_buffer = vk_buffer_create(
    MB(1),
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  vk.vert_buffer.freelist = freelist_gpu_create(vk.arena, vk.vert_buffer.size);
  
  // index
  vk.index_buffer = vk_buffer_create(
    MB(1),
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  vk.index_buffer.freelist = freelist_gpu_create(vk.arena, vk.index_buffer.size);
  
  // stage
  vk.stage_buffer = vk_buffer_create(
    MB(8),
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  vk_buffer_map_memory(&vk.stage_buffer, 0, vk.stage_buffer.size);
  
  // uniform
  vk.uniform_buffer = vk_buffer_create(
    MB(1),
    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  vk_buffer_map_memory(&vk.uniform_buffer, 0, vk.uniform_buffer.size);
  vk.uniform_buffer.freelist = freelist_gpu_create(vk.arena, vk.index_buffer.size);
}

void vk_resize_framebuffer() {
  vkDeviceWaitIdle(vkdevice);

  Info("%f %f", vk.viewport_size.x, vk.viewport_size.y);
  Loop (i, ImagesInFlight) {
    vk_image_destroy(vk.texture_targets[i].image);
    vk_image_destroy(vk.depth);

    vk.texture_targets[i].image = vk_image_create(
        VK_IMAGE_TYPE_2D,
        vk.viewport_size.x, vk.viewport_size.y,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_COLOR_BIT);

  vk.depth = vk_image_create(
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
