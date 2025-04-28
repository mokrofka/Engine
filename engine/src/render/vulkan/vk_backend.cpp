#include "vk_backend.h"
#include "vk_types.h"
#include "vk_device.h"
#include "vk_renderpass.h"
#include "vk_command_buffer.h"
#include "vk_utils.h"
#include "vk_buffer.h"
#include "vk_swapchain.h"
#include "vk_image.h"

#include "shaders/vk_material_shader.h"
#include "shaders/vk_ui_shader.h"

#include "sys/material_sys.h"

VK_Context* vk;

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);

internal void regenerate_framebuffers();
internal b32 recreate_swapchain(VK_Swapchain* swapchain);
internal void create_buffers(VK_Render* render);

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
  vk = push_struct(backend->arena, VK_Context);
  vk->arena = backend->arena;
  
  // TODO: custom allocator.
  vk->allocator = 0;
  
  v2i framebuffer_extent = os_get_framebuffer_size();
  vk->frame.width =  framebuffer_extent.x;
  vk->frame.height = framebuffer_extent.y;

  VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  app_info.apiVersion = VK_API_VERSION_1_2;
  app_info.pApplicationName = "Engine.exe";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  
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
  ++required_validation_layer_count;
  
  u32 available_layer_count = 0;
  VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, 0));
  
  VkLayerProperties* available_layers = push_array(vk->arena, VkLayerProperties, available_layer_count);
  VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers));

  // Verify all required layers are available.
  Loop (i, required_validation_layer_count) {
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
  
  VK_CHECK(vkCreateInstance(&create_info, vk->allocator, &vk->instance));
  Info("Vulkan insance created"_);

  // Debugger
#if defined(_DEBUG)
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

  PFN_vkCreateDebugUtilsMessengerEXT func;
  Assign(func, vkGetInstanceProcAddr(vk->instance, "vkCreateDebugUtilsMessengerEXT"));

  Assert(func && "Failed to create debug messenger");
  VK_CHECK(func(vk->instance, &debug_create_info, vk->allocator, &vk->debug_messenger));
  Debug("Vulkan debugger created"_);
#endif
  
  Debug("Creating Vulkan surface..."_);
  vk->surface = (VkSurfaceKHR)os_vk_create_surface();
  Debug("Vulkan surface created"_);

  vk->device = vk_device_create();

  vk->swapchain = vk_swapchain_create(vk->frame.width, vk->frame.height);
  
  // World renderpass
  vk->main_renderpass = vk_renderpass_create(
    Rect{0, 0, vk->frame.width, vk->frame.height},
    v4{0, 0, 0.2, 1.0},
    1.0f,
    0,
    RenderpassClearFlag_ColorBuffer | RenderpassClearFlag_DepthBuffer | RenderpassClearFlag_StencilBuffer,
    false, true
    );
    
  // UI renderpass
  vk->ui_renderpass = vk_renderpass_create(
    Rect{0, 0, vk->frame.width, vk->frame.height},
    v4{0.0f, 0.0f, 0.0f, 0.0f},
    1.0f,
    0,
    RenderpassClearFlag_None,
    true, false
    );

  // Regenerate swapchain and world framebuffers
  regenerate_framebuffers();

  Loop (i, vk->swapchain.image_count) {
    vk->render.cmds[i] = vk_cmd_alloc(vk->device.graphics_cmd_pool, true);
  }
  Debug("Vulkan command buffers created"_);

  vk->sync.image_available_semaphores = push_array(vk->arena, VkSemaphore, vk->swapchain.max_frames_in_flight);
  vk->sync.queue_complete_semaphores = push_array(vk->arena, VkSemaphore, vk->swapchain.max_frames_in_flight);

  Loop (i, vk->swapchain.max_frames_in_flight) {
    VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(vkdevice, &semaphore_create_info, vk->allocator, &vk->sync.image_available_semaphores[i]);
    vkCreateSemaphore(vkdevice, &semaphore_create_info, vk->allocator, &vk->sync.queue_complete_semaphores[i]);

    VkFenceCreateInfo fence_create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VK_CHECK(vkCreateFence(vkdevice, &fence_create_info, vk->allocator, &vk->sync.in_flight_fences[i]));
  }

  vk->render.material_shader = vk_material_shader_create();
  vk->render.ui_shader = vk_ui_shader_create();
  
  create_buffers(&vk->render);
  
  Loop (i, MaxGeometryCount) {
    vk->render.geometries[i].id = INVALID_ID;
  }
  
  Info("Vulkan renderer initialized successfully"_);
}

void vk_r_backend_shutdown() {
  vkDeviceWaitIdle(vkdevice);
  
  // Destroy in opposite order of creation
  // buffers
  vk_buffer_destroy(&vk->render.obj_vertex_buffer);
  vk_buffer_destroy(&vk->render.obj_index_buffer);
  
  vk_ui_shader_destroy(&vk->render.ui_shader);
  vk_material_shader_destroy(&vk->render.material_shader);
  
  // Sync objects
  Loop (i, vk->swapchain.max_frames_in_flight) {
    vkDestroySemaphore(vkdevice, vk->sync.image_available_semaphores[i], vk->allocator);
    vkDestroySemaphore(vkdevice, vk->sync.queue_complete_semaphores[i], vk->allocator);
    vkDestroyFence(vkdevice, vk->sync.in_flight_fences[i], vk->allocator);
  }
  vk->sync.image_available_semaphores = 0;
  vk->sync.queue_complete_semaphores = 0;
  
  // Command buffers
  Loop (i, vk->swapchain.image_count) {
    if (vk->render.cmds[i].handle) {
      vk_cmd_free(vk->device.graphics_cmd_pool, &vk->render.cmds[i]);
      vk->render.cmds[i].handle = 0;
    }
  }
  
  Loop (i, vk->swapchain.image_count) {
    vkDestroyFramebuffer(vkdevice, vk->world_framebuffers[i], vk->allocator);
    vkDestroyFramebuffer(vkdevice, vk->swapchain.framebuffers[i], vk->allocator);
  }
  
  // Renderpass
  vk_renderpass_destroy(vk->main_renderpass);
  vk->main_renderpass.handle = 0;
  vk_renderpass_destroy(vk->ui_renderpass);
  vk->ui_renderpass.handle = 0;
  
  // Swapchain
  vk_swapchain_destroy(&vk->swapchain);
  
  Debug("Destroying Vulkan device..."_);
  vk_device_destroy();
  
  Debug("Destroying Vulkan surface..."_);
  if (vk->surface) {
    vkDestroySurfaceKHR(vk->instance, vk->surface, vk->allocator);
    vk->surface = 0;
  }
  
  Debug("Destroying Vulkan debugger..."_);
  if (vk->debug_messenger) {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            vk->instance, "vkDestroyDebugUtilsMessengerEXT");
    func(vk->instance, vk->debug_messenger, vk->allocator);
  }

  Debug("Destroying Vulkan instance..."_);
  vkDestroyInstance(vk->instance, vk->allocator);
}

void vk_r_backend_on_resize(u16 width, u16 height) {
  // cached_framebuffer_extent.x = width;
  // cached_framebuffer_extent.y = height;
  vk->frame.width = width;
  vk->frame.height = height;
  ++vk->frame.size_generation;
  
  Info("Vulkan renderer backend->resized: w/h/gen %i/%i/%i", width, height, vk->frame.size_generation);
}

b32 vk_r_backend_begin_frame(f32 delta_time) {
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
    
    Info("Resized, booting"_);
    return false;
  }

  // Wait for the execution of the current frame to complete. The fence being free will allow this one to move on.
  VkResult result = vkWaitForFences(vkdevice, 1, &vk->sync.in_flight_fences[vk->frame.current_frame], true, U64_MAX);
  if (!vk_result_is_success(result)) {
    Error("In-flight fence wait failure! error: %s", vk_result_string(result, true));
    return false;
  }

  // Acquire the next image from the swap chain. Pass along the semaphore that should signaled when this completes.
  // This same semaphore will later be waited on by the queue submission to ensure this image is available.
  vk->frame.image_index = vk_swapchain_acquire_next_image_index(vk->swapchain, U64_MAX, vk->sync.image_available_semaphores[vk->frame.current_frame], 0);

  // Begin recording commands.
  VK_Cmd* cmd = &vk->render.cmds[vk->frame.image_index];
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
  
  vk->main_renderpass.render_area.w = vk->frame.width;
  vk->main_renderpass.render_area.h = vk->frame.height;

  return true;
}

void vk_r_update_global_world_state(mat4 projection, mat4 view, v3 view_position, v4 ambient_color, i32 mode) {
  VkCommandBuffer cmd = vk->render.cmds[vk->frame.image_index].handle;

  vk_material_shader_use(&vk->render.material_shader);
  
  vk->render.material_shader.global_ubo.projection = projection;
  vk->render.material_shader.global_ubo.view = view;
  
  vk_material_shader_update_global_state(&vk->render.material_shader, vk->frame.delta_time);
}

void vk_r_update_global_ui_state(mat4 projection, mat4 view, i32 mode) {
  VkCommandBuffer cmd = vk->render.cmds[vk->frame.image_index].handle;

  vk_ui_shader_use(&vk->render.ui_shader);
  
  vk->render.ui_shader.global_ubo.projection = projection;
  vk->render.ui_shader.global_ubo.view = view;
  
  vk_ui_shader_update_global_state(&vk->render.ui_shader, vk->frame.delta_time);
}

b32 vk_r_backend_end_frame(f32 delta_time) {
  VK_Cmd* cmd = &vk->render.cmds[vk->frame.image_index];
  
  vk_cmd_end(cmd);
  
  VK_CHECK(vkResetFences(vkdevice, 1, &vk->sync.in_flight_fences[vk->frame.current_frame]));

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
    vk->sync.in_flight_fences[vk->frame.current_frame]);
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

b32 vk_r_begin_renderpass(u32 renderpass_id) {
  VK_Renderpass* renderpass = 0;
  VkFramebuffer framebuffer = 0;
  VK_Cmd* cmd = &vk->render.cmds[vk->frame.image_index];

  // Choose a renderpass based on ID.
  switch (renderpass_id) {
    case BuiltinRenderpass_World: {
      renderpass = &vk->main_renderpass;
      framebuffer = vk->world_framebuffers[vk->frame.image_index];
    } break;
    case BuiltinRenderpass_UI: {
      renderpass = &vk->ui_renderpass;
      framebuffer = vk->swapchain.framebuffers[vk->frame.image_index];
    } break;
    default:
      Error("vulkan_renderer_begin_renderpass called on unrecognized renderpass id: %i", renderpass_id);
      return false;
  }

  // Begin the render pass.
  vk_renderpass_begin(cmd, *renderpass, framebuffer);

  // Use the appropriate shader.
  switch (renderpass_id) {
    case BuiltinRenderpass_World: {
      vk_material_shader_use(&vk->render.material_shader);
    } break;
    case BuiltinRenderpass_UI: {
      vk_ui_shader_use(&vk->render.ui_shader);
    } break;
  }

  return true;
}

b32 vk_r_end_renderpass(u32 renderpass_id) {
  VK_Renderpass* renderpass = 0;
  VK_Cmd* cmd = &vk->render.cmds[vk->frame.image_index];

  // Choose a renderpass based on ID.
  switch (renderpass_id) {
  case BuiltinRenderpass_World:
    renderpass = &vk->main_renderpass;
    break;
  case BuiltinRenderpass_UI:
    renderpass = &vk->ui_renderpass;
    break;
  default:
    Error("vk_renderer_end_renderpass called on unrecognized renderpass id:  %#02x", renderpass_id);
    return false;
  }

  vk_renderpass_end(cmd);
  return true;
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

internal void regenerate_framebuffers() {
  Loop (i, vk->swapchain.image_count) {
    VkImageView world_attachments[2] = {vk->swapchain.views[i], vk->swapchain.depth_attachment.view};
    VkFramebufferCreateInfo framebuffer_info = {
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = vk->main_renderpass.handle,
      .attachmentCount = 2,
      .pAttachments = world_attachments,
      .width = vk->frame.width,
      .height = vk->frame.height,
      .layers = 1,
    };

    VK_CHECK(vkCreateFramebuffer(vkdevice, &framebuffer_info, vk->allocator, &vk->world_framebuffers[i]));
    
    VkImageView ui_attachments[1] = {vk->swapchain.views[i]};
    VkFramebufferCreateInfo sc_framebuffer_info = {
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = vk->ui_renderpass.handle,
      .attachmentCount = 1,
      .pAttachments = ui_attachments,
      .width = vk->frame.width,
      .height = vk->frame.height,
      .layers = 1,
    };
    
    VK_CHECK(vkCreateFramebuffer(vkdevice, &sc_framebuffer_info, vk->allocator, &vk->swapchain.framebuffers[i]));
  }
}

internal b32 recreate_swapchain(VK_Swapchain* swapchain) {
  // If already being recreated, do not try again.
  if (vk->recreating_swapchain) {
    Debug("recreate_swapchain called when already recreating. Booting"_);
    return false;
  }

  // Detect if the window is too small to be drawn to
  if (vk->frame.width == 0 || vk->frame.height == 0) {
    Debug("recreate_swapchain called when window is < 1 in a dimension. Booting"_);
    return false;
  }

  // Mark as recreating if the dimensions are valid.
  vk->recreating_swapchain = true;

  Loop (i, vk->swapchain.image_count) {
    vkDestroyFramebuffer(vkdevice, vk->world_framebuffers[i], vk->allocator);
    vkDestroyFramebuffer(vkdevice, vk->swapchain.framebuffers[i], vk->allocator);
  }

  vk_swapchain_recreate(swapchain, vk->frame.width, vk->frame.height);
  
  vk->main_renderpass.render_area.w = vk->frame.width;
  vk->main_renderpass.render_area.h = vk->frame.height;

  // Update framebuffer size generation.
  vk->frame.size_last_generation = vk->frame.size_generation;

  vk->main_renderpass.render_area.x = 0;
  vk->main_renderpass.render_area.y = 0;
  vk->main_renderpass.render_area.w = vk->frame.width;
  vk->main_renderpass.render_area.h = vk->frame.height;

  regenerate_framebuffers();

  // Clear the recreating flag.
  vk->recreating_swapchain = false;

  return true;
}

internal void create_buffers(VK_Render* render) {
  VkMemoryPropertyFlagBits memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;   
  
  // Geometry vertex buffer
  const u64 vertex_buffer_size = sizeof(Vertex3D) * MB(1);
  render->obj_vertex_buffer = vk_buffer_create(
    vertex_buffer_size,
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    memory_property_flags,
    true);
  render->geometry_vertex_offset = 0;
  
  // Geometry index buffer
  const u64 index_buffer_size = sizeof(u32) * MB(1);
  render->obj_index_buffer = vk_buffer_create(
    vertex_buffer_size,
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    memory_property_flags,
    true);
  render->geometry_index_offset = 0;
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
  
  VkCommandPool pool = vk->device.graphics_cmd_pool;
  VkQueue queue = vk->device.graphics_queue;
  VK_Cmd temp_buffer = vk_cmd_alloc_and_begin_single_use(pool);
  
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
  switch (material->type) {
    case MaterialType_World: {
      vk_material_shader_acquire_resources(&vk->render.material_shader, material);
    } break;
    case MaterialType_UI: {
      vk_ui_shader_acquire_resources(&vk->render.ui_shader, material);
    } break;
    default:
      Error("vk_r_create_material - unknown material type"_);
      return;
  }
  Trace("Render: Material created"_);
}

void vk_r_destroy_material(Material* material) {
  if (material) {
    if (material->internal_id != INVALID_ID) {
      switch (material->type) {
        case MaterialType_World: {
          vk_material_shader_release_resources(&vk->render.material_shader, material);
        } break;
        case MaterialType_UI: {
          vk_ui_shader_release_resources(&vk->render.ui_shader, material); } break;
        default:
          Error("vk_r_destroy_material - unknown material type"_);
          return;
      }
    } else {
      Warn("vk_r_destroy_material called with internal_id=INVALID_ID. Nothing was done"_);
    }
  } else {
    Warn("vk_r_destroy_material called with null. Nothing was done"_);
  }
}

void vk_r_create_geometry(Geometry* geometry, u32 vertex_size, u32 vertex_count, void* vertices, u32 index_size, u32 index_count, void* indices) {
  Assert(vertex_count && vertices);

  // Check if this is a re-upload. If it is, need to free old data afterward.
  b32 is_reupload = geometry->internal_id != INVALID_ID;
  VK_GeometryData old_range;

  VK_GeometryData* internal_data = 0;
  if (is_reupload) {
    internal_data = &vk->render.geometries[geometry->internal_id];

    // Take a copy of the old range.
    old_range.index_buffer_offset = internal_data->index_buffer_offset;
    old_range.index_count = internal_data->index_count;
    old_range.index_element_size = internal_data->index_element_size;
    old_range.vertex_buffer_offset = internal_data->vertex_buffer_offset;
    old_range.vertex_count = internal_data->vertex_count;
    old_range.vertex_element_size = internal_data->vertex_element_size;
  } else {
    Loop (i, MaxGeometryCount) {
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
    Error("vulkan_renderer_create_geometry failed to find a free index for a new geometry upload. Adjust config to allow for more"_);
  }

  VkCommandPool pool = vk->device.graphics_cmd_pool;
  VkQueue queue = vk->device.graphics_queue;

  // Vertex data.
  internal_data->vertex_buffer_offset = vk->render.geometry_vertex_offset;
  internal_data->vertex_count = vertex_count;
  internal_data->vertex_element_size = sizeof(Vertex3D);
  u32 total_size = vertex_count * vertex_size;
  upload_data_range(pool, 0, queue, &vk->render.obj_vertex_buffer, internal_data->vertex_buffer_offset, total_size, vertices);
  // TODO: should maintain a free list instead of this.
  vk->render.geometry_vertex_offset += total_size;

  // Index data, if applicable
  if (index_count && indices) {
    internal_data->index_buffer_offset = vk->render.geometry_index_offset;
    internal_data->index_count = index_count;
    internal_data->index_element_size = sizeof(u32);
    total_size = index_count * index_size;
    upload_data_range(pool, 0, queue, &vk->render.obj_index_buffer, internal_data->index_buffer_offset, total_size, indices);
    // TODO: should maintain a free list instead of this.
    vk->render.geometry_index_offset += total_size;
  }

  if (internal_data->generation == INVALID_ID) {
    internal_data->generation = 0;
  } else {
    ++internal_data->generation;
  }

  if (is_reupload) {
    // Free vertex data
    free_data_range(&vk->render.obj_vertex_buffer, old_range.vertex_buffer_offset, old_range.vertex_element_size * old_range.vertex_count);

    // Free index data, if applicable
    if (old_range.index_element_size > 0) {
      free_data_range(&vk->render.obj_index_buffer, old_range.index_buffer_offset, old_range.index_element_size * old_range.index_count);
    }
  }
}

void vk_r_destroy_geometry(Geometry* geometry) {
  if (geometry && geometry->internal_id != INVALID_ID) {
    vkDeviceWaitIdle(vk->device.logical_device);
    VK_GeometryData* internal_data = &vk->render.geometries[geometry->internal_id];

    // Free vertex data
    free_data_range(&vk->render.obj_vertex_buffer, internal_data->vertex_buffer_offset, internal_data->vertex_element_size * internal_data->vertex_count);

    // Free index data, if applicable
    if (internal_data->index_element_size > 0) {
      free_data_range(&vk->render.obj_index_buffer, internal_data->index_buffer_offset, internal_data->index_element_size * internal_data->index_count);
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
  VkCommandBuffer cmd = vk->render.cmds[vk->frame.image_index].handle;
  
  Material* m = 0;
  if (data.geometry->material) {
    m = data.geometry->material;
  } else {
    m = material_sys_get_default(); 
  }
  
  switch (m->type) {
    case MaterialType_World: {
      vk_material_shader_set_model(&vk->render.material_shader, data.model);
      vk_material_shader_apply_material(&vk->render.material_shader, m);
    } break;
    case MaterialType_UI: {
      vk_ui_shader_set_model(&vk->render.ui_shader, data.model);
      vk_ui_shader_apply_material(&vk->render.ui_shader, m);
    } break;
      default:
        Error("vk_r_draw_geometry - unknown material type %i", m->type);
  }

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
