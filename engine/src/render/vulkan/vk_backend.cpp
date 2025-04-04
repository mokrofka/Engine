#include "vk_backend.h"

#include "vk_types.h"
#include "vk_os.h"
#include "vk_device.h"
#include "vk_swapchain.h"
#include "vk_renderpass.h"
#include "vk_command_buffer.h"
#include "vk_framebuffer.h"
#include "vk_fence.h"
#include "vk_utils.h"
#include "vk_buffer.h"
#include "vk_image.h"

#include "shaders/vk_material_shader.h"

#include <strings.h>
#include <os.h>
#include <math/math_types.h>

global VK_Context* context;
global u32 cached_framebuffer_width;
global u32 cached_framebuffer_height;

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);

internal i32 find_memory_index(u32 type_filter, u32 property_flags);

internal void create_command_buffers();
internal void 
regenerate_framebuffers(VK_Swapchain* swapchain, VK_RenderPass * renderpass);
internal b8 recreate_swapchain();
internal b8 create_buffers(VK_Context* context);

internal void upload_data_range(VK_Context* context, VkCommandPool pool, VkFence fence, VkQueue queue, VK_Buffer* buffer, u64 offset, u64 size, void* data) {
  // Create a host-visible stagin buffer to upload to. Mark it as the source or the transfer
  VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  VK_Buffer staging;
  vk_buffer_create(context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, true, &staging);
  
  // Load the data into the staging buffer
  vk_buffer_load_data(context, &staging, 0, size, 0, data);
  
  // Perform the copy from staging to the device local buffer
  vk_buffer_copy_to(context, pool, fence, queue, staging.handle, 0, buffer->handle, offset, size);
  
  // Clean up the stagin buffer
  vk_buffer_destroy(context, &staging);
}

b8 vk_r_backend_init(R_Backend* backend) {
  // Function pointer
  backend->internal_context = push_struct(backend->arena, VK_Context);
  context = (VK_Context*)backend->internal_context;
  context->arena = backend->arena;
  context->find_memory_index = find_memory_index;
  
  // TODO: custom allocator.
  context->allocator = 0;
  
  os_get_framebuffer_size(&cached_framebuffer_width, &cached_framebuffer_height);
  context->framebuffer_width = (cached_framebuffer_width != 0) ? cached_framebuffer_width : 800;
  context->framebuffer_height = (cached_framebuffer_height != 0) ? cached_framebuffer_height : 600;
  cached_framebuffer_width = 0;
  cached_framebuffer_height = 0;

  VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  app_info.apiVersion = VK_API_VERSION_1_2;
  app_info.pApplicationName = "Engine.exe";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "Kohi Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  
  VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  create_info.pApplicationInfo = &app_info;
  
  // Obtain a list of required extensions
  const char* required_extensions[3]; 
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
  if (!vk_os_create_surface(context)) {
    Error("Failed to create platform surface!");
    return false;
  }
  Debug("Vulkan surface created.");

  // Device creation
  if (!vk_device_create(context)) {
    Error("Failed to create device!");
    return false;
  }
  
  // Swapchain
  vk_swapchain_create(
      context,
      context->framebuffer_width,
      context->framebuffer_height,
      &context->swapchain);
  
  vk_renderpass_create(
    context, 
    &context->main_renderpass, 
    0, 0, context->framebuffer_width, context->framebuffer_height, 
    0.0f, 0.0f, 0.2f, 1.0f, 
    1.0f, 
    0);
  
  // Swapchain framebuffers.
  context->swapchain.framebuffers = push_array(context->arena, VK_Framebuffer, context->swapchain.image_count);
  regenerate_framebuffers(&context->swapchain, &context->main_renderpass);
  
  // Create command buffers.
  create_command_buffers();
  
  // Create sync objects.
  context->image_available_semaphores = push_array(context->arena, VkSemaphore,
                                                   context->swapchain.max_frames_in_flight);
  context->queue_complete_semaphores = push_array(context->arena, VkSemaphore,
                                                   context->swapchain.max_frames_in_flight);
  context->in_flight_fences = push_array(context->arena, VK_Fence,
                                         context->swapchain.max_frames_in_flight);

  for (u8 i = 0; i < context->swapchain.max_frames_in_flight; ++i) {
    VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(context->device.logical_device, &semaphore_create_info, context->allocator,
                      &context->image_available_semaphores[i]);
    vkCreateSemaphore(context->device.logical_device, &semaphore_create_info, context->allocator,
                      &context->queue_complete_semaphores[i]);
    
    vk_fence_create(context, true, &context->in_flight_fences[i]);
  }

  context->images_in_flight = push_array(context->arena, VK_Fence*, context->swapchain.image_count);
  for (u32 i = 0; i < context->swapchain.image_count; ++i) {
    context->images_in_flight[i] = 0;
  }

  if (!vk_material_shader_create(context, &context->material_shader)) {
    Error("Error loading built-in basic_lighting shader.");
    return false;
  }
  
  create_buffers(context);
  
  // TODO temporary test code
  const u32 vert_count = 4;
  Vertex3D verts[vert_count] = {};
  
  const f32 f = 10.0f;
  
  verts[0].position.x = f*-0.5;
  verts[0].position.y = f*-0.5;
  verts[0].texcoord.x = 0.0f;
  verts[0].texcoord.y = 0.0f;
  
  verts[1].position.x = f*0.5;
  verts[1].position.y = f*0.5;
  verts[1].texcoord.x = 1.0f;
  verts[1].texcoord.y = 1.0f;
  
  verts[2].position.x = f*-0.5;
  verts[2].position.y = f*0.5;
  verts[2].texcoord.x = 0.0f;
  verts[2].texcoord.y = 1.0f;
  
  verts[3].position.x = f*0.5;
  verts[3].position.y = f*-0.5;
  verts[3].texcoord.x = 1.0f;
  verts[3].texcoord.y = 0.0f;
  
  const u32 index_count = 6;
  u32 indices[index_count] = {0,1,2, 0,3,1};
  
  u32 object_id = 0;
  if (!vk_material_shader_acquire_resources(context, &context->material_shader, &object_id)) {
    Error("Failed to acquire shader resources.");
    return false;
  }
  
  upload_data_range(context, context->device.graphics_command_pool, 0, context->device.graphics_queue, &context->object_vertex_buffer, 0, sizeof(Vertex3D) * vert_count, verts);
  upload_data_range(context, context->device.graphics_command_pool, 0, context->device.graphics_queue, &context->object_index_buffer, 0, sizeof(u32) * index_count, indices);
  // TODO end temp code

  Info("Vulkan renderer initialized successfully.");
  return true;
}

void vk_r_backend_shutdown() {
  vkDeviceWaitIdle(context->device.logical_device);
  
  // Destroy in opposite order of creation
  // buffers
  vk_buffer_destroy(context, &context->object_vertex_buffer);
  vk_buffer_destroy(context, &context->object_index_buffer);
  
  vk_material_shader_destroy(context, &context->material_shader);
  
  // Sync objects
  for (u8 i = 0; i < context->swapchain.max_frames_in_flight; ++i) {
    if (context->image_available_semaphores[i]) {
      vkDestroySemaphore(context->device.logical_device,
                         context->image_available_semaphores[i],
                         context->allocator);
    }
    if (context->queue_complete_semaphores[i]) {
      vkDestroySemaphore(context->device.logical_device,
                         context->queue_complete_semaphores[i],
                         context->allocator);
    }
    vk_fence_destroy(context, &context->in_flight_fences[i]);
  }
  context->image_available_semaphores = 0;
  context->queue_complete_semaphores = 0;
  context->in_flight_fences = 0;
  context->images_in_flight = 0;
  
  // Command buffers
  for (u32 i = 0; i < context->swapchain.image_count; ++i) {
    if (context->graphics_command_buffers[i].handle) {
      vk_command_buffer_free(
        context, 
        context->device.graphics_command_pool,
        &context->graphics_command_buffers[i]);
      context->graphics_command_buffers[i].handle = 0;
    }
  }
  // darray_destroy(context->graphics_command_buffers); // TODO
  context->graphics_command_buffers = 0;
  
  for (u32 i = 0; i < context->swapchain.image_count; ++i) {
    vk_framebuffer_destroy(context, &context->swapchain.framebuffers[i]);
  }
  
  // Renderpass
  vk_renderpass_destroy(context, &context->main_renderpass);
  
  // Swapchain
  vk_swapchain_destroy(context, &context->swapchain);
  
  Debug("Destroying Vulkan device...");
  vk_device_destroy(context);
  
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

void vk_r_backend_on_resize(u16 width, u16 height) {
  cached_framebuffer_width = width;
  cached_framebuffer_height = height;
  ++context->framebuffer_size_generation;
  
  Info("Vulkan renderer backend->resized: w/h/gen %i/%i/%llu", width, height, context->framebuffer_size_generation);
}

b8 vk_r_backend_begin_frame(f32 delta_time) {
  context->frame_delta_time = delta_time;
  VK_Device* device = &context->device;
  
  // Check if recreating swap chain and boot out.
  if (context->recreating_swapchain) {
    VkResult result = vkDeviceWaitIdle(device->logical_device);
    if (!vk_result_is_success(result)) {
      Error("vk_r_backend_begin_frame vkDeviceWaitIdle (1) failed '%s'", vk_result_string(result, true));
    }
    Info("Recreating swapchain, booting.");
    return false;
  }
  
  // Check if the framebuffer has been resized. If so, a new swapchain must be created.
  if (context->framebuffer_size_generation != context->framebuffer_size_last_generation) {
    VkResult result = vkDeviceWaitIdle(device->logical_device);
    if (!vk_result_is_success(result)) {
      Error("vk_r_backend_begin_frame vkDeviceWaitIdle (2) failed '%s'", vk_result_string(result, true));
      return false;
    }
    
    // If the swapcahin recreation failed (because, for example, the window was minimized),
    // boot out before unsetting the flag.
    if (!recreate_swapchain()) {
      return false;
    }
    
    Info("Resized, booting.");
    return false;
  }

  // Wait for the execution of the current frame to complete. The fence being free will allow this one to move on.
  if (!vk_fence_wait(
          context,
          &context->in_flight_fences[context->current_frame],
          U64_MAX)) {
    Warn("In-flight fence wait failure!");
    return false;
  }
  
  
  // Acquire the next image from the swap chain. Pass along the semaphore that should signaled when this completes.
  // This same semaphore will later be waited on by the queue submission to ensure this image is available.
  if (!vk_swapchain_acquire_next_image_index(
    context, 
    &context->swapchain, 
    U64_MAX,
    context->image_available_semaphores[context->current_frame],
    0,
    &context->image_index)) {
    return false;
  }
  
  // Begin recording commands.
  VK_CommandBuffer* command_buffer = &context->graphics_command_buffers[context->image_index];
  vkResetCommandBuffer(command_buffer->handle, 0); // TODO get rid of this
  vk_command_buffer_reset(command_buffer);
  vk_command_buffer_begin(command_buffer, false, false, false);
  
  // Dynamic state
  VkViewport viewport;
  viewport.x = 0.0f;
  viewport.y = (f32)context->framebuffer_height;
  viewport.width = (f32)context->framebuffer_width;
  viewport.height = -(f32)context->framebuffer_height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  
  // Scissor
  VkRect2D scissor;
  scissor.offset.x = scissor.offset.y = 0;
  scissor.extent.width = context->framebuffer_width;
  scissor.extent.height = context->framebuffer_height;
  
  vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
  vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);
  
  context->main_renderpass.w = context->framebuffer_width;
  context->main_renderpass.h = context->framebuffer_height;

  vk_renderpass_begin(
      command_buffer,
      &context->main_renderpass,
      context->swapchain.framebuffers[context->image_index].handle);


  return true;
}

void vk_r_update_global_state(mat4 projection, mat4 view, v3 view_position, v4 ambient_color, i32 mode) {
  VK_CommandBuffer* command_buffer = &context->graphics_command_buffers[context->image_index];

  vk_material_shader_use(context, &context->material_shader);
  
  context->material_shader.global_ubo.projection = projection;
  context->material_shader.global_ubo.view = view;
  
  // TODO other ubo properties
  
  vk_material_shader_update_global_state(context, &context->material_shader, context->frame_delta_time);
  
}

b8 vk_r_backend_end_frame(f32 delta_time) {
  
  VK_CommandBuffer* command_buffer = &context->graphics_command_buffers[context->image_index];
  
  // End renderpass
  vk_renderpass_end(command_buffer, &context->main_renderpass);
  
  vk_command_buffer_end(command_buffer);
  
  // Make sure the previous frame is not using this image (i.e. its fence is being waited on)
  if (context->images_in_flight[context->image_index] != VK_NULL_HANDLE) {
    vk_fence_wait(
      context, 
      context->images_in_flight[context->image_index], 
      U64_MAX);
  }
  
  // Mark the image fence as in-use by this frame.
  context->images_in_flight[context->image_index] = &context->in_flight_fences[context->current_frame];
  
  // Rest the fence for use on the next frame
  vk_fence_reset(context, &context->in_flight_fences[context->current_frame]);

  // Submit the queue and wait for the operation to complete.
  // Begin queue submission
  VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  
  // Command buffer(s) to be executed.
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer->handle;
  
  // The semaphore(s) to be signaled when the queue is complete.
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &context->queue_complete_semaphores[context->current_frame];

  // Wait semaphore ensures that the operation cannot begin until the image is available.
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &context->image_available_semaphores[context->current_frame];

  // Each semaphore waits on the corresponding pipeline stage to complete. 1:1 ratio.
  // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent color attachment
  // writes from executing until the semaphore signals (i.e. one frame is presented at a time)
  VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submit_info.pWaitDstStageMask = flags;

  VkResult result = vkQueueSubmit(
      context->device.graphics_queue,
      1,
      &submit_info,
      context->in_flight_fences[context->current_frame].handle);
  if (result != VK_SUCCESS) {
    Error("vkQueueSubmit failed with result: %s", vk_result_string(result, true));
    return false;
  }

  vk_command_buffer_update_submitted(command_buffer);
  // End queue submission

  // Give the image back to the swapchain.
  vk_swapchain_present(
      context,
      &context->swapchain,
      context->device.graphics_queue,
      context->device.present_queue,
      context->queue_complete_semaphores[context->current_frame],
      context->image_index);

  return true;
}

void vk_r_update_object(GeometryRenderData data) {
  VK_CommandBuffer* command_buffer = &context->graphics_command_buffers[context->image_index];
  
  vk_material_shader_update_object(context, &context->material_shader, data);
  
  // TODO temporary test code
  vk_material_shader_use(context, &context->material_shader);
  
  // Bind vertex buffer at offset
  VkDeviceSize offsets[1] = {0};
  vkCmdBindVertexBuffers(command_buffer->handle, 0, 1, &context->object_vertex_buffer.handle, (VkDeviceSize*)offsets);
  
  // Bind index buffer at offset
  vkCmdBindIndexBuffer(command_buffer->handle, context->object_index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);
  
  // Issue the draw
  vkCmdDrawIndexed(command_buffer->handle, 6, 1, 0, 0, 0);
  // TODO end temporary test code
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

internal void create_command_buffers() {
  if (!context->graphics_command_buffers) {
    context->graphics_command_buffers = push_array(context->arena, VK_CommandBuffer, context->swapchain.image_count);
    for (u32 i = 0; i < context->swapchain.image_count; ++i) {
      MemZeroStruct(&context->graphics_command_buffers[i]);
    }
  }
  
  for (u32 i = 0; i < context->swapchain.image_count; ++i) {
    if (context->graphics_command_buffers[i].handle) {
      vk_command_buffer_free(
          context,
          context->device.graphics_command_pool,
          &context->graphics_command_buffers[i]);
    }
    MemZeroStruct(&context->graphics_command_buffers[i]);
    vk_command_buffer_alloc(
      context, 
      context->device.graphics_command_pool, 
      true, 
      &context->graphics_command_buffers[i]);
  }
  Debug("Vulkan command buffers created.");
}

internal void 
regenerate_framebuffers(VK_Swapchain* swapchain, VK_RenderPass * renderpass) {
  for (u32 i = 0; i < swapchain->image_count; ++i) {
    // TODO: make this dynamic based on the currently configured attachments
    u32 attachment_count = 2;
    VkImageView attachments[] = {
      swapchain->views[i],
      swapchain->depth_attachment.view};
      
    vk_framebuffer_create(
      context, 
      renderpass, 
      context->framebuffer_width, 
      context->framebuffer_height, 
      attachment_count, 
      attachments, 
      &context->swapchain.framebuffers[i]);
  }
}

internal b8 recreate_swapchain() {
  // If already being recreated, do not try again.
  if (context->recreating_swapchain) {
    Debug("recreate_swapchain called when already recreating. Booting.");
    return false;
  }

  // Detect if the window is too small to be drawn to
  if (context->framebuffer_width == 0 || context->framebuffer_height == 0) {
    Debug("recreate_swapchain called when window is < 1 in a dimension. Booting.");
    return false;
  }

  // Mark as recreating if the dimensions are valid.
  context->recreating_swapchain = true;

  // Wait for any operations to complete.
  vkDeviceWaitIdle(context->device.logical_device);

  // Clear these out just in case.
  for (u32 i = 0; i < context->swapchain.image_count; ++i) {
    context->images_in_flight[i] = 0;
  }

  // Requery support
  vk_device_query_swapchain_support(
      context->device.physical_device,
      context,
      &context->device.swapchain_support);
  vk_device_detect_depth_format(&context->device);

  vk_swapchain_recreate(
      context,
      cached_framebuffer_width,
      cached_framebuffer_height,
      &context->swapchain);

  // Sync the framebuffer size with the cached sizes.
  context->framebuffer_width = cached_framebuffer_width;
  context->framebuffer_height = cached_framebuffer_height;
  context->main_renderpass.w = context->framebuffer_width;
  context->main_renderpass.h = context->framebuffer_height;
  cached_framebuffer_width = 0;
  cached_framebuffer_height = 0;

  // Update framebuffer size generation.
  context->framebuffer_size_last_generation = context->framebuffer_size_generation;

  // cleanup swapchain
  for (u32 i = 0; i < context->swapchain.image_count; ++i) {
    vk_command_buffer_free(context, context->device.graphics_command_pool, &context->graphics_command_buffers[i]);
  }

  // Framebuffers.
  for (u32 i = 0; i < context->swapchain.image_count; ++i) {
    vk_framebuffer_destroy(context, &context->swapchain.framebuffers[i]);
  }

  context->main_renderpass.x = 0;
  context->main_renderpass.y = 0;
  context->main_renderpass.w = context->framebuffer_width;
  context->main_renderpass.h = context->framebuffer_height;

  regenerate_framebuffers(&context->swapchain, &context->main_renderpass);

  create_command_buffers();

  // Clear the recreating flag.
  context->recreating_swapchain = false;

  return true;
}

internal b8 create_buffers(VK_Context* context) {
  VkMemoryPropertyFlagBits memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;   
  
  const u64 vertex_buffer_size = sizeof(Vertex3D) * MB(1);
  if (!vk_buffer_create(
    context, 
    vertex_buffer_size,
    VkBufferUsageFlagBits(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
    u32(memory_property_flags),
    true,
    &context->object_vertex_buffer)) {
    
    Error("Error creating vertex buffer.");
    return false;
  }
  
  context->geometry_vertex_offset = 0;
  
  const u64 index_buffer_size = sizeof(u32) * MB(1);
  if (!vk_buffer_create(
    context, 
    vertex_buffer_size,
    VkBufferUsageFlagBits(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
    u32(memory_property_flags),
    true,
    &context->object_index_buffer)) {
    
    Error("Error creating vertex buffer.");
    return false;
  }
  context->geometry_index_offset = 0;
  
  return true;
}

void vk_r_create_texture(const u8* pixels, Texture* texture) {
  // Internal data creation
  texture->internal_data = push_struct(context->arena, VulkanTextureData);
  VulkanTextureData* data = (VulkanTextureData*)texture->internal_data;
  VkDeviceSize image_size = texture->width * texture->height * texture->channel_count;
  
  // NOTE assumes 8 bits per channel
  VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;
  
  // Create a staging buffer and load data into it
  VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  VkMemoryPropertyFlags memory_prop_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  VK_Buffer staging;
  vk_buffer_create(context, image_size, usage, memory_prop_flags, true, &staging);
  
  vk_buffer_load_data(context, &staging, 0, image_size, 0, pixels);
  
  // Note Lots of assumptions, different texture types will require
  // different options here
  vk_image_create(
    context, 
    VK_IMAGE_TYPE_2D, 
    texture->width, 
    texture->height, 
    image_format, 
    VK_IMAGE_TILING_OPTIMAL, 
    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    true,
    VK_IMAGE_ASPECT_COLOR_BIT,
    &data->image);
  
  VK_CommandBuffer temp_buffer;
  VkCommandPool pool = context->device.graphics_command_pool;
  VkQueue queue = context->device.graphics_queue;
  vk_command_buffer_alloc_and_begin_single_use(context, pool, &temp_buffer);
  
  // Transition the layout from whatever it is currently to optimal for receiving data
  vk_image_transition_layout(
    context, 
    &temp_buffer, 
    &data->image, 
    image_format, 
    VK_IMAGE_LAYOUT_UNDEFINED, 
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  
  // Copy the data from the buffer
  vk_image_copy_from_buffer(context, &data->image, staging.handle, &temp_buffer);
  
  // Copy the data from the buffer
  vk_image_transition_layout(
    context, 
    &temp_buffer, 
    &data->image, 
    image_format, 
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  
  vk_command_buffer_end_single_use(context, pool, &temp_buffer, queue);
  
  vk_buffer_destroy(context, &staging);

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
  
  VkResult result = vkCreateSampler(context->device.logical_device, &sampler_info, context->allocator, &data->sampler);
  if (!vk_result_is_success(VK_SUCCESS)) {
    Error("Error creating texture sampler: %s", vk_result_string(result, true));
    return;
  }
  
  ++texture->generation;
}

void vk_r_destroy_texture(Texture* texture) {
  vkDeviceWaitIdle(context->device.logical_device);
  
  VulkanTextureData* data = (VulkanTextureData*)texture->internal_data;
  if (data) {
    vk_image_destroy(context, &data->image);
    MemZeroStruct(&data->image);
    vkDestroySampler(context->device.logical_device, data->sampler, context->allocator);
    data->sampler = 0;
    // TODO free memory
  }
  
  MemZeroStruct(texture);
}
