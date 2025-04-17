#include "vk_backend.h"
#include "vk_types.h"
#include "vk_device.h"
#include "vk_renderpass.h"
#include "vk_command_buffer.h"
#include "vk_fence.h"
#include "vk_utils.h"
#include "vk_buffer.h"
#include "vk_swapchain.h"
#include "vk_framebuffer.h"
#include "vk_image.h"

#include "shaders/vk_material_shader.h"

#include "sys/material_sys.h"

VK_Context* vk;

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);

internal i32 find_memory_index(u32 type_filter, u32 property_flags);

internal void regenerate_framebuffers(VK_Swapchain* swapchain, VK_RenderPass renderpass);
internal b8 recreate_swapchain(VK_Swapchain* swapchain);
internal b8 create_buffers(VK_Render* render);

internal void upload_data_range(VkCommandPool pool, VkFence fence, VkQueue queue, VK_Buffer* buffer, u64 offset, u64 size, void* data) {
  // Create a host-visible stagin buffer to upload to. Mark it as the source or the transfer
  VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  VK_Buffer staging = vk_buffer_create(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, true);
  
  // Load the data into the staging buffer
  vk_buffer_load_data(&staging, 0, size, 0, data);
  
  // Perform the copy from staging to the device local buffer
  vk_buffer_copy_to(pool, fence, queue, staging.handle, 0, buffer->handle, offset, size);
  
  // Clean up the stagin buffer
  vk_buffer_destroy(&staging);
}

void free_data_range(VK_Buffer* buffer, u64 offset, u64 size) {
  // TODO free this in the buffer
  // TODO update free list with this range being free
}

void vk_r_backend_init(R_Backend* backend) {
  // Function pointer
  vk = push_struct(backend->arena, VK_Context);
  vk->arena = backend->arena;
  vk->find_memory_index = find_memory_index;
  
  // TODO: custom allocator.
  vk->allocator = 0;
  
  v2i framebuffer_extent = os_get_framebuffer_size();
  vk->frame.width =  framebuffer_extent.x;
  vk->frame.height = framebuffer_extent.y;

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
  VkLayerProperties* available_layers = push_array(vk->arena, VkLayerProperties, available_layer_count);
  VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers));

  // Verify all required layers are available.
  for (u32 i = 0; i < required_validation_layer_count; ++i) {
    Info("Searching for layer: %s...", required_validation_layer_names[i]);
    b8 found = false;
    for (u32 j = 0; j < available_layer_count; ++j) {
      if (cstr_match(required_validation_layer_names[i], available_layers[j].layerName)) {
        found = true;
        Info("Found.");
        break;
      }
    }
    if (!found) {
      Fatal("Required validation layer is missing: %s", required_validation_layer_names[i]);
    }
  }
  Info("All required validation layers are present.");
#endif

  create_info.enabledLayerCount = required_validation_layer_count;
  create_info.ppEnabledLayerNames = required_validation_layer_names;
  
  VK_CHECK(vkCreateInstance(&create_info, vk->allocator, &vk->instance));
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

  PFN_vkCreateDebugUtilsMessengerEXT func;
  Assign(func, vkGetInstanceProcAddr(vk->instance, "vkCreateDebugUtilsMessengerEXT"));

  AssertMsg(func, "Failed to create debug messenger!");
  VK_CHECK(func(vk->instance, &debug_create_info, vk->allocator, &vk->debug_messenger));
  Debug("Vulkan debugger created.");
#endif
  
  Debug("Creating Vulkan surface...");
  vk->surface = (VkSurfaceKHR)vk_os_create_surface();
  Debug("Vulkan surface created.");

  vk->device = vk_device_create();

  vk->swapchain = vk_swapchain_create(vk->frame.width, vk->frame.height);
  vk->renderpass = vk_renderpass_create(
      {0, 0, vk->frame.width, vk->frame.height},
      {0, 0, 0.2, 1.0},
      1.0f,
      0);

  regenerate_framebuffers(&vk->swapchain, vk->renderpass);

  for (u32 i = 0; i < vk->swapchain.image_count; ++i) {
    vk->render.cmd[i] = vk_command_buffer_alloc(vk->device.gfx_cmd_pool, true);
  }
  Debug("Vulkan command buffers created.");

  vk->sync.image_available_semaphores = push_array(vk->arena, VkSemaphore,
                                                   vk->swapchain.max_frames_in_flight);
  vk->sync.queue_complete_semaphores = push_array(vk->arena, VkSemaphore,
                                                  vk->swapchain.max_frames_in_flight);
  vk->sync.in_flight_fences = push_array(vk->arena, VK_Fence,
                                         vk->swapchain.max_frames_in_flight);

  for (u8 i = 0; i < vk->swapchain.max_frames_in_flight; ++i) {
    VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(vkdevice, &semaphore_create_info, vk->allocator,
                      &vk->sync.image_available_semaphores[i]);
    vkCreateSemaphore(vkdevice, &semaphore_create_info, vk->allocator,
                      &vk->sync.queue_complete_semaphores[i]);

    vk->sync.in_flight_fences[i] = vk_fence_create(true);
  }

  vk->render.material_shader = vk_material_shader_create(vk);
  
  create_buffers(&vk->render);
  
  for (u32 i = 0; i < VK_MAX_GEOMETRY_COUNT; ++i) {
    vk->render.geometries[i].id = INVALID_ID;
  }
  
  Info("Vulkan renderer initialized successfully.");
}

void vk_r_backend_shutdown() {
  vkDeviceWaitIdle(vkdevice);
  
  // Destroy in opposite order of creation
  // buffers
  vk_buffer_destroy(&vk->render.obj_vertex_buffer);
  vk_buffer_destroy(&vk->render.obj_index_buffer);
  
  vk_material_shader_destroy(&vk->render.material_shader);
  
  // Sync objects
  for (u8 i = 0; i < vk->swapchain.max_frames_in_flight; ++i) {
    if (vk->sync.image_available_semaphores[i]) {
      vkDestroySemaphore(vkdevice,
                         vk->sync.image_available_semaphores[i],
                         vk->allocator);
    }
    if (vk->sync.queue_complete_semaphores[i]) {
      vkDestroySemaphore(vkdevice,
                         vk->sync.queue_complete_semaphores[i],
                         vk->allocator);
    }
    vk_fence_destroy(&vk->sync.in_flight_fences[i]);
  }
  vk->sync.image_available_semaphores = 0;
  vk->sync.queue_complete_semaphores = 0;
  vk->sync.in_flight_fences = 0;
  
  // Command buffers
  for (u32 i = 0; i < vk->swapchain.image_count; ++i) {
    if (vk->render.cmd[i].handle) {
      vk_cmd_free(vk->device.gfx_cmd_pool, &vk->render.cmd[i]);
      vk->render.cmd[i].handle = 0;
    }
  }
  
  // Renderpass
  vk_renderpass_destroy(vk->renderpass);
  vk->renderpass.handle = 0;
  
  // Swapchain
  vk_swapchain_destroy(&vk->swapchain);
  
  Debug("Destroying Vulkan device...");
  vk_device_destroy();
  
  Debug("Destroying Vulkan surface...");
  if (vk->surface) {
    vkDestroySurfaceKHR(vk->instance, vk->surface, vk->allocator);
    vk->surface = 0;
  }
  
  Debug("Destroying Vulkan debugger...");
  if (vk->debug_messenger) {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            vk->instance, "vkDestroyDebugUtilsMessengerEXT");
    func(vk->instance, vk->debug_messenger, vk->allocator);
  }

  Debug("Destroying Vulkan instance...");
  vkDestroyInstance(vk->instance, vk->allocator);
}

void vk_r_backend_on_resize(u16 width, u16 height) {
  // cached_framebuffer_extent.x = width;
  // cached_framebuffer_extent.y = height;
  vk->frame.width = width;
  vk->frame.height = height;
  ++vk->frame.size_generation;
  
  Info("Vulkan renderer backend->resized: w/h/gen %i/%i/%llu", width, height, vk->frame.size_generation);
}

b8 vk_r_backend_begin_frame(f32 delta_time) {
  vk->frame.delta_time = delta_time;
  
  // Check if the framebuffer has been resized. If so, a new swapchain must be created.
  if (vk->frame.size_generation != vk->frame.size_last_generation) {
    VkResult result = vkDeviceWaitIdle(vkdevice);
    if (!vk_result_is_success(result)) {
      Error("vk_r_backend_begin_frame vkDeviceWaitIdle (2) failed '%s'", vk_result_string(result, true));
      return false;
    }
    
    // If the swapcahin recreation failed (because, for example, the window was minimized),
    // boot out before unsetting the flag.
    if (!recreate_swapchain(&vk->swapchain)) {
      return false;
    }
    
    Info("Resized, booting.");
    return false;
  }

  // Wait for the execution of the current frame to complete. The fence being free will allow this one to move on.
  if (!vk_fence_wait(&vk->sync.in_flight_fences[vk->frame.current_frame], U64_MAX)) {
    Warn("In-flight fence wait failure!");
    return false;
  }

  // Acquire the next image from the swap chain. Pass along the semaphore that should signaled when this completes.
  // This same semaphore will later be waited on by the queue submission to ensure this image is available.
  vk->frame.image_index = vk_swapchain_acquire_next_image_index(vk->swapchain, U64_MAX, vk->sync.image_available_semaphores[vk->frame.current_frame], 0);

  // Begin recording commands.
  VK_CommandBuffer* cmd = &vk->render.cmd[vk->frame.image_index];
  vk_cmd_begin(cmd, false, false, false);
  
  // Dynamic state
  VkViewport viewport;
  viewport.x = 0.0f;
  viewport.y = vk->frame.height;
  viewport.width = vk->frame.width;
  viewport.height = -(f32)vk->frame.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  
  // Scissor
  VkRect2D scissor;
  scissor.offset.x = scissor.offset.y = 0;
  scissor.extent.width = vk->frame.width;
  scissor.extent.height = vk->frame.height;
  
  vkCmdSetViewport(cmd->handle, 0, 1, &viewport);
  vkCmdSetScissor(cmd->handle, 0, 1, &scissor);
  
  vk->renderpass.rect.w = vk->frame.width;
  vk->renderpass.rect.h = vk->frame.height;

  vk_renderpass_begin(cmd, vk->renderpass, vk->swapchain.framebuffers[vk->frame.image_index]);

  return true;
}

void vk_r_update_global_state(mat4 projection, mat4 view, v3 view_position, v4 ambient_color, i32 mode) {
  VkCommandBuffer cmd = vk->render.cmd[vk->frame.image_index].handle;

  vk_material_shader_use(cmd, vk->render.material_shader);
  
  vk->render.material_shader.global_ubo.projection = projection;
  vk->render.material_shader.global_ubo.view = view;
  
  vk_material_shader_update_global_state(cmd, &vk->render.material_shader, vk->frame.delta_time);
}

b8 vk_r_backend_end_frame(f32 delta_time) {
  VK_CommandBuffer* cmd = &vk->render.cmd[vk->frame.image_index];
  
  vk_renderpass_end(cmd);
  
  vk_cmd_end(cmd);
  
  vk_fence_reset(&vk->sync.in_flight_fences[vk->frame.current_frame]);

  VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd->handle;
  
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &vk->sync.queue_complete_semaphores[vk->frame.current_frame];

  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &vk->sync.image_available_semaphores[vk->frame.current_frame];

  // Each semaphore waits on the corresponding pipeline stage to complete. 1:1 ratio.
  // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent color attachment
  // writes from executing until the semaphore signals (i.e. one frame is presented at a time)
  VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submit_info.pWaitDstStageMask = flags;

  VkResult result = vkQueueSubmit(
    vk->device.graphics_queue,
    1,
    &submit_info,
    vk->sync.in_flight_fences[vk->frame.current_frame].handle);
  if (result != VK_SUCCESS) {
    Error("vkQueueSubmit failed with result: %s", vk_result_string(result, true));
    return false;
  }

  vk_swapchain_present(
    &vk->swapchain, vk->device.graphics_queue,
    vk->device.present_queue,
    vk->sync.queue_complete_semaphores[vk->frame.current_frame],
    vk->frame.image_index);
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
      _log_output(LOG_LEVEL_ERROR, callback_data->pMessage);
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
  vkGetPhysicalDeviceMemoryProperties(vk->device.physical_device, &memory_properties);

  for (u32 i = 0; i < memory_properties.memoryTypeCount; ++i) {
    // Check each memory type to see if its bit is set to 1.
    if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
      return i;
    }
  }

  Warn("Unable to find suitable memory type!");
  return -1;
}

internal void regenerate_framebuffers(VK_Swapchain* swapchain, VK_RenderPass renderpass) {
  for (u32 i = 0; i < swapchain->image_count; ++i) {
    // TODO: make this dynamic based on the currently configured attachments
    u32 attachment_count = 2;
    VkImageView attachments[] = {
        swapchain->views[i],
        swapchain->depth_attachment.view};
        
   swapchain->framebuffers[i] = vk_framebuffer_create(
       renderpass,
       vk->frame.width,
       vk->frame.height,
       attachment_count,
       attachments);
  }
}

internal b8 recreate_swapchain(VK_Swapchain* swapchain) {
  // If already being recreated, do not try again.
  if (vk->recreating_swapchain) {
    Debug("recreate_swapchain called when already recreating. Booting.");
    return false;
  }

  // Detect if the window is too small to be drawn to
  if (vk->frame.width == 0 || vk->frame.height == 0) {
    Debug("recreate_swapchain called when window is < 1 in a dimension. Booting.");
    return false;
  }

  // Mark as recreating if the dimensions are valid.
  vk->recreating_swapchain = true;

  vk_swapchain_recreate(swapchain, vk->frame.width, vk->frame.height);

  vk->renderpass.rect.w = vk->frame.width;
  vk->renderpass.rect.h = vk->frame.height;

  // Update framebuffer size generation.
  vk->frame.size_last_generation = vk->frame.size_generation;

  vk->renderpass.rect.x = 0;
  vk->renderpass.rect.y = 0;
  vk->renderpass.rect.w = vk->frame.width;
  vk->renderpass.rect.h = vk->frame.height;

  regenerate_framebuffers(swapchain, vk->renderpass);

  // Clear the recreating flag.
  vk->recreating_swapchain = false;

  return true;
}

internal b8 create_buffers(VK_Render* render) {
  VkMemoryPropertyFlagBits memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;   
  
  const u64 vertex_buffer_size = sizeof(Vertex3D) * MB(1);
  render->obj_vertex_buffer = vk_buffer_create(
    vertex_buffer_size,
    VkBufferUsageFlagBits(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
    u32(memory_property_flags),
    true);
  render->geometry_vertex_offset = 0;
  
  const u64 index_buffer_size = sizeof(u32) * MB(1);
  render->obj_index_buffer = vk_buffer_create(
    vertex_buffer_size,
    VkBufferUsageFlagBits(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
    u32(memory_property_flags),
    true);
  render->geometry_index_offset = 0;
  
  return true;
}

void vk_r_create_texture(u8* pixels, Texture* texture) {
  // Internal data creation
  texture->internal_data = push_struct(vk->arena, VK_TextureData);
  VK_TextureData* data = (VK_TextureData*)texture->internal_data;
  VkDeviceSize image_size = texture->width * texture->height * texture->channel_count;
  
  // NOTE assumes 8 bits per channel
  VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;
  
  // Create a staging buffer and load data into it
  VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  VkMemoryPropertyFlags memory_prop_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  VK_Buffer staging = vk_buffer_create(image_size, usage, memory_prop_flags, true);
  
  vk_buffer_load_data(&staging, 0, image_size, 0, pixels);
  
  // Note Lots of assumptions, different texture types will require
  // different options here
  data->image = vk_image_create(
    VK_IMAGE_TYPE_2D, 
    texture->width, 
    texture->height, 
    image_format, 
    VK_IMAGE_TILING_OPTIMAL, 
    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    true,
    VK_IMAGE_ASPECT_COLOR_BIT);
  
  VK_CommandBuffer temp_buffer;
  VkCommandPool pool = vk->device.gfx_cmd_pool;
  VkQueue queue = vk->device.graphics_queue;
  vk_cmd_alloc_and_begin_single_use(pool, &temp_buffer);
  
  // Transition the layout from whatever it is currently to optimal for receiving data
  vk_image_transition_layout(
    &temp_buffer, 
    &data->image, 
    image_format, 
    VK_IMAGE_LAYOUT_UNDEFINED, 
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  
  // Copy the data from the buffer
  vk_image_copy_from_buffer(&data->image, staging.handle, &temp_buffer);
  
  // Copy the data from the buffer
  vk_image_transition_layout(
    &temp_buffer, 
    &data->image, 
    image_format, 
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  
  vk_cmd_end_single_use(pool, &temp_buffer, queue);
  
  vk_buffer_destroy(&staging);

  // Create a sampler for the texture
  VkSamplerCreateInfo sampler_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  // TODO These filters shoud be configurable
  sampler_info.magFilter = VK_FILTER_LINEAR;
  sampler_info.minFilter = VK_FILTER_LINEAR;
  sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.anisotropyEnable = VK_TRUE;
  sampler_info.maxAnisotropy = 16;
  sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sampler_info.unnormalizedCoordinates = VK_FALSE;
  sampler_info.compareEnable = VK_FALSE;
  sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
  sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_info.mipLodBias = 0.0f;
  sampler_info.minLod = 0.0f;
  sampler_info.maxLod = 0.0f;
  
  VkResult result = vkCreateSampler(vkdevice, &sampler_info, vk->allocator, &data->sampler);
  if (!vk_result_is_success(VK_SUCCESS)) {
    Error("Error creating texture sampler: %s", vk_result_string(result, true));
    return;
  }
  
  ++texture->generation;
}

void vk_r_destroy_texture(Texture* texture) {
  vkDeviceWaitIdle(vkdevice);
  
  VK_TextureData* data = (VK_TextureData*)texture->internal_data;
  if (data) {
    vk_image_destroy(&data->image);
    vkDestroySampler(vkdevice, data->sampler, vk->allocator);
    MemZeroStruct(&data);
    // TODO free memory
  }

  MemZeroStruct(texture);
}

void vk_r_create_material(Material* material) {
  Assert(material);
  vk_material_shader_acquire_resources(&vk->render.material_shader, material);
  Trace("Render: Material created.");
}

void vk_r_destroy_material(Material* material) {
  if (material) {
    if (material->internal_id != INVALID_ID) {
      vk_material_shader_release_resources(&vk->render.material_shader, material);
    } else {
      Warn("vk_r_destroy_material called with internal_id=INVALID_ID. Nothing was done");
    }
  } else {
    Warn("vk_r_destroy_material called with null. Nothing was done.");
  }
}

void vk_r_create_geometry(Geometry* geometry, u32 vertex_count, Vertex3D* vertices, u32 index_count, u32* indices) {
  if (!vertex_count || !vertices) {
    Error("vulkan_renderer_create_geometry requires vertex data, and none was supplied. vertex_count=%d, vertices=%p", vertex_count, vertices);
  }

  // Check if this is a re-upload. If it is, need to free old data afterward.
  b8 is_reupload = geometry->internal_id != INVALID_ID;
  VK_GeometryData old_range;

  VK_GeometryData* internal_data = 0;
  if (is_reupload) {
    internal_data = &vk->render.geometries[geometry->internal_id];

    // Take a copy of the old range.
    old_range.index_buffer_offset = internal_data->index_buffer_offset;
    old_range.index_count = internal_data->index_count;
    old_range.index_size = internal_data->index_size;
    old_range.vertex_buffer_offset = internal_data->vertex_buffer_offset;
    old_range.vertex_count = internal_data->vertex_count;
    old_range.vertex_size = internal_data->vertex_size;
  } else {
    for (u32 i = 0; i < VK_MAX_GEOMETRY_COUNT; ++i) {
      if (vk->render.geometries[i].id == INVALID_ID) {
        // Found a free index.
        geometry->internal_id = i;
        vk->render.geometries[i].id = i;
        internal_data = &vk->render.geometries[i];
        break;
      }
    }
  }
  if (!internal_data) {
    Error("vulkan_renderer_create_geometry failed to find a free index for a new geometry upload. Adjust config to allow for more.");
  }

  VkCommandPool pool = vk->device.gfx_cmd_pool;
  VkQueue queue = vk->device.graphics_queue;

  // Vertex data.
  internal_data->vertex_buffer_offset = vk->render.geometry_vertex_offset;
  internal_data->vertex_count = vertex_count;
  internal_data->vertex_size = sizeof(Vertex3D) * vertex_count;
  upload_data_range(pool, 0, queue, &vk->render.obj_vertex_buffer, internal_data->vertex_buffer_offset, internal_data->vertex_size, vertices);
  // TODO: should maintain a free list instead of this.
  vk->render.geometry_vertex_offset += internal_data->vertex_size;

  // Index data, if applicable
  if (index_count && indices) {
    internal_data->index_buffer_offset = vk->render.geometry_index_offset;
    internal_data->index_count = index_count;
    internal_data->index_size = sizeof(u32) * index_count;
    upload_data_range(pool, 0, queue, &vk->render.obj_index_buffer, internal_data->index_buffer_offset, internal_data->index_size, indices);
    // TODO: should maintain a free list instead of this.
    vk->render.geometry_index_offset += internal_data->index_size;
  }

  if (internal_data->generation == INVALID_ID) {
    internal_data->generation = 0;
  } else {
    internal_data->generation++;
  }

  if (is_reupload) {
    // Free vertex data
    free_data_range(&vk->render.obj_vertex_buffer, old_range.vertex_buffer_offset, old_range.vertex_size);

    // Free index data, if applicable
    if (old_range.index_size > 0) {
      free_data_range(&vk->render.obj_index_buffer, old_range.index_buffer_offset, old_range.index_size);
    }
  }
}

void vk_r_destroy_geometry(Geometry* geometry) {
  if (geometry && geometry->internal_id != INVALID_ID) {
    vkDeviceWaitIdle(vk->device.logical_device);
    VK_GeometryData* internal_data = &vk->render.geometries[geometry->internal_id];

    // Free vertex data
    free_data_range(&vk->render.obj_vertex_buffer, internal_data->vertex_buffer_offset, internal_data->vertex_size);

    // Free index data, if applicable
    if (internal_data->index_size > 0) {
      free_data_range(&vk->render.obj_index_buffer, internal_data->index_buffer_offset, internal_data->index_size);
    }

    // Clean up data.
    MemZeroStruct(internal_data);
    internal_data->id = INVALID_ID;
    internal_data->generation = INVALID_ID;
  }
}

void vk_r_draw_geometry(GeometryRenderData data) {
  if (data.geometry && data.geometry->internal_id == INVALID_ID) {
    return;
  }
  
  VK_GeometryData* buffer_data = &vk->render.geometries[data.geometry->internal_id];
  VkCommandBuffer cmd = vk->render.cmd[vk->frame.image_index].handle;
  
  // TODO check if this is actually needed
  vk_material_shader_use(cmd, vk->render.material_shader);
  
  vk_material_shader_set_model(cmd, &vk->render.material_shader, data.model);
  
  Material* m = 0;
  if (data.geometry->material) {
    m = data.geometry->material;
  } else {
    m = material_sys_get_default(); 
  }
  vk_material_shader_apply_material(cmd, &vk->render.material_shader, data.geometry->material);

  // Bind vertex buffer at offset
  VkDeviceSize offsets[1] = {buffer_data->vertex_buffer_offset};
  vkCmdBindVertexBuffers(cmd, 0, 1, &vk->render.obj_vertex_buffer.handle, offsets);
  
  // Draw indexed or non-indexed
  if (buffer_data->index_count > 0) {
    // Bind index buffer at offset
    vkCmdBindIndexBuffer(cmd, vk->render.obj_index_buffer.handle, buffer_data->index_buffer_offset, VK_INDEX_TYPE_UINT32);

    // Issue the draw
    vkCmdDrawIndexed(cmd, buffer_data->index_count, 1, 0, 0, 0);
  } else {
    vkCmdDraw(cmd, buffer_data->vertex_count, 1, 0, 0);
  }
}
