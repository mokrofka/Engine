#include "vk.h"

u64 hash(VK_KeyToShaderPipeline x) { return hash(x.name) + hash_memory(&x.state, sizeof(ShaderState)); }
b32 equal(VK_KeyToShaderPipeline a, VK_KeyToShaderPipeline b) { return equal(a.name, b.name) && MemMatchStruct(&a.state, &b.state); }

v4& get_pos() { return st->vk.gpu_global_shader_st->ambient_color; }
mat4& get_mat() { return st->vk.gpu_global_shader_st->mat; }
v4& get_matrix() { return st->vk.gpu_global_shader_st->ambient_color; }

VK_MeshBatch vk_mesh_batch_make(Allocator alloc) {
  VK_MeshBatch res = {
    .entities = darray_make<u32>(alloc),
  };
  return res;
}

VK_MeshesBatches vk_shader_batch_make(Allocator alloc) {
  VK_MeshesBatches res = {
    .mesh_batches = darray_make<VK_MeshBatch>(alloc),
    .mesh_to_batch = map_make<u32, u32>(alloc),
  };
  return res;
}

VK_PipelineBatch vk_render_batch_make(Allocator alloc) {
  VK_PipelineBatch res = {};
  for EachElement(i, res.batches) {
    res.batches[i] = vk_shader_batch_make(alloc);
  }
  return res;
}

VK_ShaderModuleEntry vk_shader_module_entry_make(Allocator alloc) {
  VK_ShaderModuleEntry res = {
    .track_pipelines = darray_make<u32>(alloc),
    .track_shader_states = darray_make<u32>(alloc),
  };
  return res;
}

intern String vk_result_string(VkResult result) {
  switch (result) {
    case VK_SUCCESS:                                            return "VK_SUCCESS";
    case VK_NOT_READY:                                          return "VK_NOT_READY";
    case VK_TIMEOUT:                                            return "VK_TIMEOUT";
    case VK_EVENT_SET:                                          return "VK_EVENT_SET";
    case VK_EVENT_RESET:                                        return "VK_EVENT_RESET";
    case VK_INCOMPLETE:                                         return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY:                           return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:                         return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:                        return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:                                  return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:                            return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:                            return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:                        return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:                          return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:                          return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:                             return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:                         return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL:                              return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_UNKNOWN:                                      return "VK_ERROR_UNKNOWN";
    case VK_ERROR_OUT_OF_POOL_MEMORY:                           return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:                      return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_FRAGMENTATION:                                return "VK_ERROR_FRAGMENTATION";
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:               return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
    case VK_PIPELINE_COMPILE_REQUIRED:                          return "VK_PIPELINE_COMPILE_REQUIRED";
    case VK_ERROR_SURFACE_LOST_KHR:                             return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:                     return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR:                                     return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:                              return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:                     return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT:                        return "VK_ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_INVALID_SHADER_NV:                            return "VK_ERROR_INVALID_SHADER_NV";
    case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:                return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:       return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:    return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:       return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:        return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:          return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
    case VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT:                return "VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT";
    case VK_ERROR_NOT_PERMITTED_KHR:                            return "VK_ERROR_NOT_PERMITTED_KHR";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:          return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case VK_THREAD_IDLE_KHR:                                    return "VK_THREAD_IDLE_KHR";
    case VK_THREAD_DONE_KHR:                                    return "VK_THREAD_DONE_KHR";
    case VK_OPERATION_DEFERRED_KHR:                             return "VK_OPERATION_DEFERRED_KHR";
    case VK_OPERATION_NOT_DEFERRED_KHR:                         return "VK_OPERATION_NOT_DEFERRED_KHR";
    case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:                    return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
    case VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT:               return "VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT";
    case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:             return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
    case VK_PIPELINE_BINARY_MISSING_KHR:                        return "VK_PIPELINE_BINARY_MISSING_KHR";
    case VK_ERROR_NOT_ENOUGH_SPACE_KHR:                         return "VK_ERROR_NOT_ENOUGH_SPACE_KHR";
    case VK_RESULT_MAX_ENUM:                                    return "VK_RESULT_MAX_ENUM";
  }
}

#if OS_WINDOW
  typedef void* HINSTANCE;
  typedef void* HWND;
  typedef void* HANDLE;
  typedef wchar_t* LPCWSTR;
  typedef u32 DWORD;
  typedef void* LPVOID;
  typedef b32 BOOL;
  typedef void* HMONITOR;
  struct SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
  };
  #include <vulkan/vulkan_win32.h>
  intern void vk_surface_create() {
    VkWin32SurfaceCreateInfoKHR surface_create_info = {
      .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
      .hinstance = (HINSTANCE)os_get_handle_info(),
      .hwnd = (HWND)os_get_window_handle()
    };
    VK_CHECK(vkCreateWin32SurfaceKHR(g_st->vk.instance, &surface_create_info, g_st->vk.allocator, &g_st->vk.surface));
    Info("Vulkan win32 surface created");
  }
  #define VK_SURFACE_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
  
#elif OS_LINUX
  #if GFX_X11
    #include <xcb/xcb.h>
    #include <vulkan/vulkan_xcb.h>
    intern void vk_surface_create() {
      VK_State& g = st->vk;
      struct VK_Surface {
        xcb_connection_t* connection;
        xcb_window_t window;
      } vk_surface; 
      os_get_gfx_api_handlers(&vk_surface);
      VkXcbSurfaceCreateInfoKHR surfaceInfo = {
        .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        .connection = vk_surface.connection,
        .window = vk_surface.window,
      };
      PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)g.GetInstanceProcAddr(g.instance, "vkCreateXcbSurfaceKHR");
      VK_CHECK(vkCreateXcbSurfaceKHR(g.instance, &surfaceInfo, g.allocator, &g.surface));
      Info("Vulkan XCB surface created");
    }
    #define VK_SURFACE_NAME VK_KHR_XCB_SURFACE_EXTENSION_NAME

  #else
    #include <vulkan/vulkan_wayland.h>
    intern void vk_surface_create() {
      struct VK_Surface{
        struct wl_display* wl_display;
        struct wl_surface* wl_surface;
      } vk_surface;
      os_get_gfx_api_handlers(&vk_surface);
      VkWaylandSurfaceCreateInfoKHR surfaceInfo = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = vk_surface.wl_display,
        .surface = vk_surface.wl_surface,
      };
      PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)g_st->vk.GetInstanceProcAddr(g_st->vk.instance, "vkCreateWaylandSurfaceKHR");
      VK_CHECK(vkCreateWaylandSurfaceKHR (g_st->vk.instance, &surfaceInfo, g_st->vk.allocator, &g_st->vk.surface));
      Info("Vulkan wayland surface created");
    }
    #define VK_SURFACE_NAME VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
  #endif
#endif

#define VulkanUseAllocator 0

#if VulkanUseAllocator
  VkAllocationCallbacks vk_allocator_create() {
    VkAllocationCallbacks callbacks = {
      .pUserData = null,
      .pfnAllocation = [](void* user_data, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope){
        void* result = mem_alloc(g_st->vk.alloc, size);
        return result;
      },
      .pfnReallocation = [](void* user_data, void* origin, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope) {
        ...
      },
      .pfnFree = [](void* user_data, void* memory){
        if (!memory) { // NOTE: it happens
          return;
        }
        mem_free(memory);
      },
      .pfnInternalAllocation = [](void* user_data, size_t size, VkInternalAllocationType allocation_type, VkSystemAllocationScope allocation_scope){},
      .pfnInternalFree = [](void* user_data, size_t size, VkInternalAllocationType allocation_type, VkSystemAllocationScope allocation_scope){},
    };
    return callbacks;
  }
#endif

////////////////////////////////////////////////////////////////////////
// @Misc

intern VkSemaphore vk_get_current_image_available_semaphore() { return st->vk.sync.image_available_semaphores[st->vk.current_frame_idx]; }
intern VkSemaphore vk_get_current_render_complete_semaphore() { return st->vk.sync.render_complete_semaphores[st->vk.current_image_idx]; }
intern VkCommandBuffer vk_get_current_cmd()                   { return st->vk.cmds[st->vk.current_frame_idx]; }

intern void vk_bind_pipeline(VkCommandBuffer cmd, VkPipeline pipeline) {
  st->vk.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

intern u32 vk_find_memory_idx(u32 type_filter, u32 property_flags) {
  VkPhysicalDeviceMemoryProperties memory_properties = st->vk.device.memory;
  u32 idx = -1;
  Loop (i, memory_properties.memoryTypeCount) {
    if (BitHas(type_filter, i) && FlagHas(memory_properties.memoryTypes[i].propertyFlags, property_flags)) {
      idx = i;
      break;
    }
  }
  Assert(idx != -1);
  return idx;
}

////////////////////////////////////////////////////////////////////////
// @Cmd

intern VkCommandBuffer vk_cmd_alloc(VkCommandPool pool) {
  VkCommandBufferAllocateInfo allocate_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1,
  };
  VkCommandBuffer cmd;
  VK_CHECK(st->vk.AllocateCommandBuffers(vkdevice, &allocate_info, &cmd));
  return cmd;
}

intern void vk_cmd_free(VkCommandPool pool, VkCommandBuffer cmd) {
  st->vk.FreeCommandBuffers(vkdevice, pool, 1, &cmd);
}

intern void vk_cmd_begin(VkCommandBuffer cmd) {
  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  VK_CHECK(st->vk.BeginCommandBuffer(cmd, &begin_info));
}

intern void vk_cmd_end(VkCommandBuffer cmd) {
  VK_CHECK(st->vk.EndCommandBuffer(cmd));
}

intern void vk_cmd_submit(VkCommandBuffer cmd) {
  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
  };
  VK_CHECK(st->vk.QueueSubmit(st->vk.device.graphics_queue, 1, &submit_info, 0));
  VK_CHECK(st->vk.QueueWaitIdle(st->vk.device.graphics_queue));
}

intern void vk_cmd_end_submit(VkCommandBuffer cmd) {
  vk_cmd_end(cmd);
  vk_cmd_submit(cmd);
}

intern VkCommandBuffer vk_cmd_alloc_begin() {
  VkCommandBuffer result = vk_cmd_alloc(st->vk.device.cmd_pool);
  vk_cmd_begin(result);
  return result;
}

intern void vk_cmd_end_free(VkCommandBuffer cmd) {
  VK_State& g = st->vk;
  vk_cmd_end(cmd);
  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
  };
  VK_CHECK(g.QueueSubmit(g.device.graphics_queue, 1, &submit_info, 0));
  VK_CHECK(g.QueueWaitIdle(g.device.graphics_queue));
  vk_cmd_free(g.device.cmd_pool, cmd);
}

////////////////////////////////////////////////////////////////////////
// @Buffer

intern VK_Memory vk_mem_alloc(VK_MemType type, u64 size) {
  VK_State& g = st->vk;
  u32 mem_idx = 0;
  switch (type) {
    case VK_MemType_Gpu: {
      mem_idx = g.device.gpu_type_idx; 
    } break;
    case VK_MemType_Cpu: {
      mem_idx = g.device.cpu_type_idx; 
    } break;
  }
  VkMemoryAllocateInfo alloc_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = size,
    .memoryTypeIndex = mem_idx,
  };
  VK_Memory res = {.cap = size};
  VK_CHECK(g.AllocateMemory(vkdevice, &alloc_info, g.allocator, &res.h));
  if (type == VK_MemType_Cpu) {
    VK_CHECK(g.MapMemory(vkdevice, res.h, 0, size, 0, (void**)&res.mapped_mem));
  }
  return res;
}

intern VK_Buffer vk_buffer_alloc(u64 size, VK_BufferUsageFlags usage, VK_MemType mem_type) {
  VK_State& g = st->vk;
  VkBufferUsageFlags buf_usage_flags = 0;
  if (FlagHas(usage, VK_BufferUsageFlag_Vert)) buf_usage_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if (FlagHas(usage, VK_BufferUsageFlag_Index)) buf_usage_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  if (FlagHas(usage, VK_BufferUsageFlag_Dst)) buf_usage_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  if (FlagHas(usage, VK_BufferUsageFlag_Src)) buf_usage_flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  if (FlagHas(usage, VK_BufferUsageFlag_Storage)) buf_usage_flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  if (FlagHas(usage, VK_BufferUsageFlag_Indirect)) buf_usage_flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
  VkBufferCreateInfo buffer_create_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = size,
    .usage = buf_usage_flags,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  VK_Buffer res = {.cap = size};
  VK_CHECK(g.CreateBuffer(vkdevice, &buffer_create_info, g.allocator, &res.h));

  u32 mem_prop_flags = 0;
  if (mem_type == VK_MemType_Gpu) mem_prop_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  if (mem_type == VK_MemType_Cpu) mem_prop_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  VkMemoryRequirements requirements;
  g.GetBufferMemoryRequirements(vkdevice, res.h, &requirements);
  // u32 mem_idx = vk_find_memory_idx(requirements.memoryTypeBits, mem_prop_flags);
  // VkMemoryAllocateInfo alloc_info = {
  //   .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
  //   .allocationSize = requirements.size,
  //   .memoryTypeIndex = mem_idx,
  // };
  // UnusedVariable(alloc_info);

  VK_Memory* mem = null;
  u64 offset = 0;
  switch (mem_type) {
    case VK_MemType_Gpu: {
      mem = &g.gpu_mem;
      offset = offset_push(mem->pos, size, requirements.alignment);
    } break;
    case VK_MemType_Cpu: {
      mem = &g.cpu_mem;
      offset = offset_push(mem->pos, size, requirements.alignment);
    } break;
  }
  VK_CHECK(st->vk.BindBufferMemory(vkdevice, res.h, mem->h, offset));
  res.base = Offset(mem->mapped_mem, offset);
  return res;
}

intern void vk_buffer_upload(VK_Buffer buffer, BufferRegion range, void* data) {
  VK_State& g = st->vk;
  MemCopy(g.stage_buffer.base, data, range.size);
  vk_cmd_begin(g.upload_cmd);
  VkBufferCopy copy_region = {
    .srcOffset = 0,
    .dstOffset = range.offset,
    .size = range.size,
  };
  g.CmdCopyBuffer(g.upload_cmd, g.stage_buffer.h, buffer.h, 1, &copy_region);
  vk_cmd_end_submit(g.upload_cmd);
}

////////////////////////////////////////////////////////////////////////
// @Shader

VK_ShaderModule vk_shader_module_create(String name) {
  Scratch scratch;
  VK_State& g = st->vk;
  VkPipelineShaderStageCreateInfo stages[2] = {};
  String entry_points[] = {"vs_main", "fs_main"};
  VkShaderStageFlagBits stage_types[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
  String filepath = push_strf(scratch, "%s/%s.spv", st->shader_compiled_dir, name);
  Slice binary = os_file_path_read_all(scratch, filepath);
  Loop(i, 2) {
    VkShaderModuleCreateInfo module_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = binary.size,
      .pCode = (u32*)binary.data,
    };
    VkShaderModule handle;
    VK_CHECK(g.CreateShaderModule(vkdevice, &module_info, st->vk.allocator, &handle));
    stages[i] = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = stage_types[i],
      .module = handle,
      .pName = (char*)entry_points[i].str,
    };
  }
  VK_ShaderModule module = {stages[0], stages[1]};
  return module;
}

intern VkPipeline vk_shader_pipeline_create(VK_ShaderModule module, ShaderState state) {
  VK_State& g = st->vk;
  // Dynamic rendering
  // VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
  VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;
  VkPipelineRenderingCreateInfo rendering_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .colorAttachmentCount = 1,
    .pColorAttachmentFormats = &color_format,
    .depthAttachmentFormat = g.device.depth_format,
  };
  
  // Vertex input
  VkVertexInputBindingDescription binding_description = {
    .binding = 0,
    .stride = sizeof(Vertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
  };

  // Attributes
  VkVertexInputAttributeDescription attribute_desriptions[] = {
    [0] = {
      .location = 0,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = (u32)OffsetOf(Vertex, pos),
    },
    [1] = {
      .location = 1,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = (u32)OffsetOf(Vertex, norm),
    },
    [2] = {
      .location = 2,
      .binding = 0,
      .format = VK_FORMAT_R32G32_SFLOAT,
      .offset = (u32)OffsetOf(Vertex, uv),
    },
    [3] = {
      .location = 3,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = (u32)OffsetOf(Vertex, color),
    },
  };
  VkPipelineVertexInputStateCreateInfo vertex_input_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &binding_description,
    .vertexAttributeDescriptionCount = ArrayCount(attribute_desriptions),
    .pVertexAttributeDescriptions = attribute_desriptions,
  };

  // Input assembly
  VkPrimitiveTopology topology;
  switch (state.topology) {
    case ShaderTopology_Triangle: topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
    case ShaderTopology_Line:     topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
    case ShaderTopology_Point:    topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; break;
  }
  VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = topology,
    .primitiveRestartEnable = VK_FALSE,
  };
  
  // Viewport
  VkViewport viewport = {
    .x = 0.0f,
    .y = (f32)g.height,
    .width = (f32)g.width,
    .height = -(f32)g.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  VkRect2D scissor = {
    .offset = { .x = 0, .y = 0 },
    .extent = {
      .width = g.width,
      .height = g.height,
    },
  };
  VkPipelineViewportStateCreateInfo viewport_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .pViewports = &viewport,
    .scissorCount = 1,
    .pScissors = &scissor,
  };
  
  // Rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_NONE,
    // .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = VK_FALSE,
    .depthBiasConstantFactor = 0.0f,
    .depthBiasClamp = 0.0f,
    .depthBiasSlopeFactor = 0.0f,
    .lineWidth = 1.0f,
  };
  
  // Multisampling
  VkPipelineMultisampleStateCreateInfo multisampling_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    // .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    .rasterizationSamples = (VkSampleCountFlagBits)state.samples,
    .sampleShadingEnable = VK_FALSE,
    .minSampleShading = 1.0f,
    .pSampleMask = 0,
    .alphaToCoverageEnable = VK_FALSE,
    .alphaToOneEnable = VK_FALSE,
  };

  // Depth and stencil testing
  VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  if (state.use_depth) {
    depth_stencil_state_info.depthTestEnable = VK_TRUE;
    if (state.type == ShaderType_Cube) {
      depth_stencil_state_info.depthWriteEnable = VK_FALSE;
      depth_stencil_state_info.depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL;
    } else {
      depth_stencil_state_info.depthWriteEnable = VK_TRUE;
      depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS;
    }
    depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state_info.stencilTestEnable = VK_FALSE;
  }
  
  // Blending
  VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
  if (state.is_transparent) {
    color_blend_attachment_state.blendEnable = VK_TRUE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
  }
  color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  VkPipelineColorBlendStateCreateInfo color_blend_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_COPY,
    .attachmentCount = 1,
    .pAttachments = &color_blend_attachment_state,
  };
   
  // Dynamic state
  VkDynamicState dynamic_states[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
  };
  VkPipelineDynamicStateCreateInfo dynamic_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = ArrayCount(dynamic_states),
    .pDynamicStates = dynamic_states,
  };
  
  // Pipeline
  VkGraphicsPipelineCreateInfo pipeline_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = &rendering_info,
    .stageCount = 2,
    .pStages = module.stages,
    .pVertexInputState = &vertex_input_state,
    .pInputAssemblyState = &input_assembly_state,
    .pTessellationState = null,
    .pViewportState = &viewport_state,
    .pRasterizationState = &rasterizer_state,
    .pMultisampleState = &multisampling_state_info,
    .pDepthStencilState = &depth_stencil_state_info,
    .pColorBlendState = &color_blend_state_info,
    .pDynamicState = &dynamic_state_info,
    .layout = g.pipeline_layout,
    .renderPass = null,
    .basePipelineHandle = VK_NULL_HANDLE,
    .basePipelineIndex = -1,
  };
  VkPipeline result;
  VK_CHECK(g.CreateGraphicsPipelines(vkdevice, VK_NULL_HANDLE, 1, &pipeline_info, st->vk.allocator, &result));
  return result;
}

intern VkPipeline vk_shader_pipeline_create(ShaderDesc shader) {
  VK_ShaderModule module = vk_shader_module_create(shader.name);
  return vk_shader_pipeline_create(module, shader.state);
}

void vk_shader_reload(String name) {
  VK_State& g = st->vk;
  Info("reload %s shader", name);
  Result module_idx = map_get(g.shader_to_module, name);
  VK_ShaderModuleEntry& entry = g.modules[module_idx.v];
  entry.module = vk_shader_module_create(name);
  Loop (i, entry.track_pipelines.count) {
    u32 pipeline_idx = entry.track_pipelines[i];
    u32 shader_state_idx = entry.track_shader_states[i];
    ShaderState state = g.shader_states[shader_state_idx];
    g.pipelines[pipeline_idx].h = vk_shader_pipeline_create(entry.module, state);
  }
}

u32 vk_shader_pipeline_alloc(String name, ShaderState state) {
  VK_State& g = st->vk;
  VK_KeyToShaderPipeline key = {name, state};
  Result module_idx = map_get(g.shader_to_module, name);
  if (module_idx.err) {
    VK_ShaderModuleEntry entry = vk_shader_module_entry_make(g.gpa);
    entry.module = vk_shader_module_create(name);
    module_idx.v = array_push(g.modules, entry);
    map_set(g.shader_to_module, name, module_idx.v);
  }
  VkPipeline pipeline = vk_shader_pipeline_create(g.modules[module_idx.v].module, state);
  u32* pipeline_idx = map_set(g.shader_to_pipeline, key, array_push(g.pipelines, {.h = pipeline}));
  VK_ShaderModuleEntry& entry = g.modules[module_idx.v];
  array_push(entry.track_pipelines, *pipeline_idx);
  array_push(entry.track_shader_states, g.shader_states.count);
  array_push(g.shader_states, state);
  return *pipeline_idx;
}

u32 vk_shader_pipeline_alloc(ShaderDesc shader) {
  return vk_shader_pipeline_alloc(shader.name, shader.state);
}

Slice<String> vk_shader_compile(Allocator arena) {
  Scratch scratch(arena);
  String cur_dir = os_get_current_directory();
  String shader_dir = st->shader_dir;
  String compiled_shader_dir = st->shader_compiled_dir;
  String saved_time_stamps = push_strf(scratch, "%s/%s", cur_dir, String("saved_time_stamps_for_shad"));

  if (!os_directory_path_exist(shader_dir)) {
    os_directory_create_p(shader_dir);
  }
  if (!os_directory_path_exist(compiled_shader_dir)) {
    os_directory_create_p(compiled_shader_dir);
  }

  String com_path = push_strf(scratch, "%s/%s", shader_dir, String("com.slang"));
  String lib_path = push_strf(scratch, "%s/%s", shader_dir, String("lib.slang"));
  FileProperties com_props = os_file_path_properties(com_path);
  FileProperties lib_props = os_file_path_properties(lib_path);
  FileProperties time_stamp_file_props = os_file_path_properties(saved_time_stamps);

  struct FileData {
    u64 com_modified;
    u64 lib_modified;
  };

  ///////////////////////////////////
  // Recompile?
  b32 recompile = false;
  if (time_stamp_file_props.size == 0) {
    recompile = true;
  } else {
    Slice buf = os_file_path_read_all(scratch, saved_time_stamps);
    FileData* data = (FileData*)buf.data;
    if (com_props.modified != data->com_modified || lib_props.modified != data->lib_modified) {
      recompile = true;
    }
  }
  if (recompile) {
    OS_Handle time_stamp_file = os_file_open(saved_time_stamps, OS_AccessFlag_Read|OS_AccessFlag_Write);
    FileData data = {
      .com_modified = com_props.modified,
      .lib_modified = lib_props.modified,
    };
    os_file_write(time_stamp_file, sizeof(FileData), &data);
  }

  struct File {
    String file_path;
    String compiled_file_path;
    String shader_name;
    String text;
    OS_Handle pid;
  };
  var files = darray_make<File>(scratch);
  
  ///////////////////////////////////
  // Which files recompile
  {
    OS_FileIter* it = os_file_iter_begin(scratch, shader_dir, OS_FileIterFlag_SkipFolders);
    for (OS_FileInfo info = {}; os_file_iter_next(scratch, it, &info);) {
      String file_path = push_strf(scratch, "%s/%s", shader_dir, info.name);
      String shader_name = str_chop_last_dot(info.name);
      String compiled_file_path = push_strf(scratch, "%s/%s.spv", compiled_shader_dir, shader_name);
      if (str_match(info.name, "com.slang") || str_match(info.name, "lib.slang")) {
        continue;
      }
      FileProperties compiled_props = os_file_path_properties(compiled_file_path);
      if (info.props.modified != compiled_props.modified || recompile) {
        File f = {
          .file_path = file_path,
          .compiled_file_path = compiled_file_path,
          .shader_name = shader_name,
        };
        array_push(files, f);
      }
    }
    os_file_iter_end(it);
  }

  var file_names = darray_make<String>(arena);
  Loop (i, files.count) {
    File& f = files[i];
    StringList list = {};
    str_list_push(scratch, &list, "slangc");
    str_list_push(scratch, &list, f.file_path);
    str_list_push(scratch, &list, "-target");
    str_list_push(scratch, &list, "spirv");
    str_list_push(scratch, &list, "-g");
    str_list_push(scratch, &list, "-o");
    str_list_push(scratch, &list, f.compiled_file_path);
    array_push(st->shader_module_compilation_pids, os_process_launch(list));
    Debug("%s", f.file_path);
    array_push(file_names, f.shader_name);
  }
  return slice(file_names);
}

void vk_shader_compile_join(Slice<String> names) {
  Scratch scratch;
  Loop (i, names.count) {
    os_process_join(st->shader_module_compilation_pids[i]);
    String shader_file_path = push_strf(scratch, "%s/%s.slang", st->shader_dir, names[i]);
    String compiled_file_path = push_strf(scratch, "%s/%s.spv", st->shader_compiled_dir, names[i]);
    os_file_path_copy_mtime(shader_file_path, compiled_file_path);
  }
}

void vk_shader_load_modules(Slice<String> file_names) {
  VK_State& g = st->vk;
  Loop (i, file_names.count) {
    String name = file_names[i];
    VK_ShaderModule modulo = vk_shader_module_create(name);
    map_set(g.shader_to_module, name, g.modules.count);
    VK_ShaderModuleEntry entry = vk_shader_module_entry_make(g.gpa);
    entry.module = modulo;
    array_push(g.modules, entry);
  }
}

////////////////////////////////////////////////////////////////////////
// @Image

intern VK_ImageInfo vk_image_info_default(u32 width, u32 height) {
  VK_ImageInfo result = {
    .image_type = VK_IMAGE_TYPE_2D,
    .width = width,
    .height = height,
    .miplevels_count = 1,
    .flags = 0,
    .format = VK_FORMAT_R8G8B8A8_UNORM,
    .array_layers_count = 1,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
    .view_type = VK_IMAGE_VIEW_TYPE_2D,
  };
  return result;
};

intern VkImageView vk_image_view_create(VkImage image, VK_ImageInfo info) {
  VK_State& g = st->vk;
  VkImageViewCreateInfo view_create_info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .image = image,
    .viewType = info.view_type,
    .format = info.format,
    .subresourceRange = {
      .aspectMask = info.aspect,
      .baseMipLevel = 0,
      .levelCount = info.miplevels_count,
      .baseArrayLayer = 0,
      .layerCount = info.array_layers_count,
    },
  };
  VkImageView result;
  VK_CHECK(g.CreateImageView(vkdevice, &view_create_info, g.allocator, &result));
  return result;
}

intern VK_Image vk_image_create(VK_ImageInfo info) {
  VK_State& g = st->vk;
  VkImageCreateInfo image_create_info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .flags = info.flags,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = info.format,
    .extent = {info.width, info.height, 1},
    .mipLevels = info.miplevels_count,
    .arrayLayers = info.array_layers_count,
    .samples = info.samples,
    .tiling = info.tiling,
    .usage = info.usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };
  VkImage handle;
  VK_CHECK(g.CreateImage(vkdevice, &image_create_info, g.allocator, &handle));
  VkMemoryRequirements requirements;
  g.GetImageMemoryRequirements(vkdevice, handle, &requirements);
  VK_Memory& mem = g.gpu_mem;
  u64 offset = offset_push(mem.pos, requirements.size, requirements.alignment);
  VK_CHECK(g.BindImageMemory(vkdevice, handle, mem.h, offset));
  VkImageView view = vk_image_view_create(handle, info);
  VK_Image result = {
    .h = handle,
    .view = view,
    .info = info,
    .offset = offset,
  };
  return result;
}

intern void vk_image_handle_update(VK_Image& image, u32 width, u32 height) {
  VK_State& g = st->vk;
  image.info.width = width;
  image.info.height = height;
  g.DestroyImageView(vkdevice, image.view, g.allocator);
  g.DestroyImage(vkdevice, image.h, g.allocator);
  VK_ImageInfo info = image.info;
  VkImageCreateInfo image_create_info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .flags = info.flags,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = info.format,
    .extent = {info.width, info.height, 1},
    .mipLevels = info.miplevels_count,
    .arrayLayers = info.array_layers_count,
    .samples = info.samples,
    .tiling = info.tiling,
    .usage = info.usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };
  VK_CHECK(g.CreateImage(vkdevice, &image_create_info, g.allocator, &image.h));
  VK_Memory& mem = g.gpu_mem;
  VK_CHECK(g.BindImageMemory(vkdevice, image.h, mem.h, image.offset));
  image.view = vk_image_view_create(image.h, image.info);
}

intern void vk_image_destroy(VK_Image image) {
  VK_State& g = st->vk;
  g.DestroyImageView(vkdevice, image.view, g.allocator);
  g.DestroyImage(vkdevice, image.h, g.allocator);
}

intern VkAccessFlags vk_image_layout_to_mem_access_flags(VkImageLayout layout) {
	switch (layout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:                return NoFlags;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:     return VK_ACCESS_TRANSFER_READ_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:     return VK_ACCESS_TRANSFER_WRITE_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:          return NoFlags;
    InvalidDefaultCase; return {};
	}
}

intern VkPipelineStageFlags vk_image_layout_to_pipeline_stage_flags(VkImageLayout layout) {
	switch (layout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:                return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:     return VK_PIPELINE_STAGE_TRANSFER_BIT; 
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:          return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    InvalidDefaultCase; return {};
	}
}

intern void vk_image_layout_transition(VkCommandBuffer cmd, VK_Image image, VkImageLayout old_layout, VkImageLayout new_layout, VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT) {
  VK_State& g = st->vk;
  VkImageMemoryBarrier barrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .srcAccessMask = vk_image_layout_to_mem_access_flags(old_layout),
    .dstAccessMask = vk_image_layout_to_mem_access_flags(new_layout),
    .oldLayout = old_layout,
    .newLayout = new_layout,
    .srcQueueFamilyIndex = g.device.graphics_queue_family_idx,
    .image = image.h,
    .subresourceRange = {
      .aspectMask = aspect_mask,
      .baseMipLevel = 0,
      .levelCount = image.info.miplevels_count,
      .baseArrayLayer = 0,
      .layerCount = image.info.array_layers_count,
    },
  };
  VkPipelineStageFlags src_stage = vk_image_layout_to_pipeline_stage_flags(old_layout);
  VkPipelineStageFlags dst_stage = vk_image_layout_to_pipeline_stage_flags(new_layout);
  g.CmdPipelineBarrier(cmd, src_stage, dst_stage, NoFlags, 0, null, 0, null, 1, &barrier);
}

intern void vk_image_upload_to_gpu(VkCommandBuffer cmd, VK_Image image) {
  VK_State& g = st->vk;
  VkBufferImageCopy region = {
    .bufferOffset = 0,
    .bufferRowLength = 0,
    .bufferImageHeight = 0,
    .imageSubresource = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .mipLevel = 0,
      .baseArrayLayer = 0,
      .layerCount = image.info.array_layers_count,
    },
    .imageExtent = { image.info.width, image.info.height, 1 },
  };
  g.CmdCopyBufferToImage(cmd, g.stage_buffer.h, image.h, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

intern void vk_texture_generate_mipmaps(VK_Image image) {
  VK_State& g = st->vk;
  VkCommandBuffer cmd = g.upload_cmd;
  VkImageMemoryBarrier barrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .srcQueueFamilyIndex = g.device.graphics_queue_family_idx,
    .image = image.h,
    .subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .levelCount = 1,
      .layerCount = 1,
    },
  };
  
  i32 width = image.info.width;
  i32 height = image.info.height;
  for (i32 i = 1; i < image.info.miplevels_count; ++i) {
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    g.CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, null, 0, null, 1, &barrier);

    VkImageBlit blit = {
      .srcOffsets = { {0, 0, 0}, {width, height, 1} },
      .srcSubresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = (u32)i - 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
      },
      .dstOffsets = { {0, 0, 0}, {width > 1 ? width / 2 : 1, height > 1 ? height / 2 : 1, 1} },
      .dstSubresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = (u32)i,
        .baseArrayLayer = 0,
        .layerCount = 1,
      },
    };
    g.CmdBlitImage(cmd, image.h, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image.h, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    g.CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, null, 0, null, 1, &barrier);

    if (width > 1) width /= 2;
    if (height > 1) height /= 2;
  }

  barrier.subresourceRange.baseMipLevel = image.info.miplevels_count - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  g.CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, null, 0, null, 1, &barrier);
}

GpuTextureId vk_texture_load(Texture texture) {
  VK_State& g = st->vk;
  u64 size = texture.width * texture.height * 4;
  MemCopy(g.stage_buffer.base, texture.data, size);
  VK_ImageInfo image_info = vk_image_info_default(texture.width, texture.height);
  image_info.miplevels_count = Floor(Log2(Max(image_info.width, image_info.height))) + 1;
  image_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  VK_Image image = vk_image_create(image_info);
  {
    VkCommandBuffer cmd = g.upload_cmd;
    vk_cmd_begin(cmd);
    vk_image_layout_transition(cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vk_image_upload_to_gpu(cmd, image);
    vk_texture_generate_mipmaps(image);
    vk_cmd_end_submit(cmd);
  }
  VkDescriptorImageInfo descriptor_image_info = {
    .sampler = g.sampler,
    .imageView = image.view,
    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
  VkWriteDescriptorSet texture_write_descriptor = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = g.descriptor_sets,
    .dstBinding = 1,
    .dstArrayElement = g.textures.count,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    .pImageInfo = &descriptor_image_info,
  };
  VkWriteDescriptorSet descriptors[] = {texture_write_descriptor};
  g.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  u32 id = g.textures.count;
  array_push(g.textures, image);
  GpuTextureId handle = {id};
  return handle;
}

intern void vk_texture_resize_target() {
  VK_State& g = st->vk;
  Debug("texture target resized: x = %u y = %u", st->vk.width, st->vk.height);
  VK_CHECK(g.DeviceWaitIdle(vkdevice));
  u32 width = g.width * g.scale;
  u32 height = g.height * g.scale;
  if (width == 0) width = 1;
  if (height == 0) height = 1;

  // Msaa
  vk_image_destroy(g.msaa_texture);
  VK_ImageInfo image_info = vk_image_info_default(width, height);
  image_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  image_info.format = VK_FORMAT_B8G8R8A8_UNORM;
  image_info.samples = VK_SAMPLE_COUNT_4_BIT;
  g.msaa_texture = vk_image_create(image_info);

  // Texture target
  Loop (i, g.images_in_flight) {
    vk_image_destroy(g.texture_targets[i]);
    VK_ImageInfo image_info = vk_image_info_default(width, height);
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    g.texture_targets[i] = vk_image_create(image_info);
    VkDescriptorImageInfo descriptor_image_info = {
      .sampler = g.sampler,
      .imageView = g.texture_targets[i].view,
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    VkWriteDescriptorSet texture_write_descriptor = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = g.descriptor_sets,
      .dstBinding = 1,
      .dstArrayElement = (u32)i,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
      .pImageInfo = &descriptor_image_info,
    };
    VkWriteDescriptorSet descriptors[] = {texture_write_descriptor};
    g.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  }

  // Offscreen depth buffer
  vk_image_destroy(g.offscreen_depth_buffer);
  VK_ImageInfo depth_info = vk_image_info_default(width, height);
  depth_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  depth_info.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
  depth_info.format = g.device.depth_format;
  depth_info.samples = VK_SAMPLE_COUNT_4_BIT;
  g.offscreen_depth_buffer = vk_image_create(depth_info);
}

GpuMaterialId vk_material_load(MaterialDesc material) {
  VK_State& g = st->vk;
  VK_KeyToShaderPipeline key = {material.shader.name, material.shader.state};
  Result pipeline_idx = map_get(g.shader_to_pipeline, key);
  if (pipeline_idx.err) {
    pipeline_idx.v = vk_shader_pipeline_alloc(material.shader.name, material.shader.state);
    array_push(g.entity_pipelines, pipeline_idx.v);
    g.pipelines[pipeline_idx.v].batch_idx = g.batches.count;
    array_push(g.batches, vk_render_batch_make(g.gpa));
    array_push(g.static_draw_calls, {});
    array_push(g.static_draw_calls, {});
  }
  Result texture_id = map_get(st->str_to_texture_id, material.texture);
  if (texture_id.err) {
    texture_id.v = {0};
  }
  g.gpu_materials[g.materials.count] = {
    .ambient = material.props.ambient,
    .diffuse = material.props.diffuse,
    .specular = material.props.specular,
    .shininess = material.props.shininess, 
    .texture_idx = texture_id.v.v,
  };
  GpuMaterial mat = {
    .pipeline_idx = pipeline_idx.v,
    .texture_idx = texture_id.v.v,
  };
  GpuMaterialId result = {g.materials.count};
  array_push(g.materials, mat);
  return result;
}

GpuCubemapId vk_cubemap_load(Texture* textures) {
  VK_State& g = st->vk;
  u32 width = textures->width;
  u32 height = textures->height;
  u64 size = width * height * 4;
  Loop (i, 6) {
    MemCopy(Offset(g.stage_buffer.base, size * i), textures[i].data, size);
  }
  VK_ImageInfo image_info = vk_image_info_default(width, height);
  image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  image_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
  image_info.array_layers_count = 6;
  image_info.view_type = VK_IMAGE_VIEW_TYPE_CUBE;
  VK_Image image = vk_image_create(image_info);
  {
    VkCommandBuffer cmd = g.upload_cmd;
    vk_cmd_begin(cmd);
    vk_image_layout_transition(cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vk_image_upload_to_gpu(cmd, image);
    vk_image_layout_transition(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vk_cmd_end_submit(cmd);
  }
  VkDescriptorImageInfo descriptor_image_info = {
    .sampler = g.sampler,
    .imageView = image.view,
    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
  VkWriteDescriptorSet texture_descriptor = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = g.descriptor_sets,
    .dstBinding = 3,
    .dstArrayElement = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    .pImageInfo = &descriptor_image_info,
  };
  VkWriteDescriptorSet descriptors[] = {texture_descriptor};
  g.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  return {};
}

////////////////////////////////////////////////////////////////////////
// @Swapchain

intern void vk_swapchain_create(b32 reuse) {
  VK_State& g = st->vk;
  if (reuse) {
    g.swapchain.h_old = g.swapchain.h;
    ArrayCopy(g.swapchain.old_images, g.swapchain.images);
  }
  VK_CHECK(g.GetPhysicalDeviceSurfaceCapabilitiesKHR(g.device.physical_device, g.surface, &g.device.swapchain_support.capabilities));
  VkExtent2D swapchain_extent = {g.width, g.height};
  if (g.device.swapchain_support.capabilities.currentExtent.width != U32_MAX) {
    swapchain_extent = g.device.swapchain_support.capabilities.currentExtent;
  }
  VkExtent2D min = g.device.swapchain_support.capabilities.minImageExtent;
  VkExtent2D max = g.device.swapchain_support.capabilities.maxImageExtent;
  swapchain_extent.width = Clamp(min.width, swapchain_extent.width, max.width);
  swapchain_extent.height = Clamp(min.height, swapchain_extent.height, max.height);
  VkSwapchainCreateInfoKHR swapchain_create_info = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = g.surface,
    .minImageCount = g.images_in_flight,
    .imageFormat = g.swapchain.format.format,
    .imageColorSpace = g.swapchain.format.colorSpace,
    .imageExtent = swapchain_extent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = g.device.swapchain_support.capabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = g.swapchain.present_mode,
    .clipped = VK_TRUE,
    .oldSwapchain = reuse ? g.swapchain.h : null,
  };
  VK_CHECK(g.CreateSwapchainKHR(vkdevice, &swapchain_create_info, g.allocator, &g.swapchain.h));
  u32 image_count = g.images_in_flight;
  VkImage images[4];
  VK_CHECK(g.GetSwapchainImagesKHR(vkdevice, g.swapchain.h, &image_count, images));
  Loop (i, image_count) {
    g.swapchain.images[i] = {
      .h = images[i],
      .info = {
        .miplevels_count = 1,
        .array_layers_count = 1,
      }
    };
    VkImageViewCreateInfo view_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = g.swapchain.images[i].h,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = g.swapchain.format.format,
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
      },
    };
    VK_CHECK(g.CreateImageView(vkdevice, &view_info, st->vk.allocator, &st->vk.swapchain.images[i].view));
  }

  ///////////////////////////////////
  // Destroy old one
  if (reuse) {
    Loop (i, g.images_in_flight) {
      g.DestroyImageView(vkdevice, g.swapchain.old_images[i].view, g.allocator);
    }
    g.DestroySwapchainKHR(vkdevice, g.swapchain.h_old, st->vk.allocator);
  }

  ///////////////////////////////////
  // Depth buffer
  vk_image_handle_update(g.swapchain.depth_attachment, swapchain_extent.width, swapchain_extent.height);
}

intern void vk_swapchain_destroy(VK_Swapchain swapchain) {
  VK_State& g = st->vk;
  VK_CHECK(g.DeviceWaitIdle(vkdevice));
  Loop (i, g.images_in_flight) {
    g.DestroyImageView(vkdevice, swapchain.images[i].view, g.allocator);
  }
  g.DestroySwapchainKHR(vkdevice, swapchain.h, st->vk.allocator);
}

////////////////////////////////////////////////////////////////////////
// @Mesh

GpuMeshId vk_mesh_load(Mesh mesh) {
  VK_State& g = st->vk;
  VK_Buffer& vert_buff = g.vert_buffer;
  u64 vert_size = mesh.vert_count*sizeof(Vertex);
  u64 vert_offset = vert_buff.pos;
  vert_buff.pos += vert_size;
  BufferRegion vert_range = { .offset = vert_offset, .size = vert_size };
  vk_buffer_upload(g.vert_buffer, vert_range, mesh.vertices);

  VK_Buffer& index_buff = g.index_buffer;
  u64 index_size = mesh.index_count*sizeof(u32);
  u64 index_offset = index_buff.pos;
  index_buff.pos += index_size;
  BufferRegion index_range = { .offset = index_offset, .size = index_size };
  if (mesh.indices) {
    vk_buffer_upload(g.index_buffer, index_range, mesh.indices);
  }

  VK_Mesh vk_mesh = {
    .vert_count = mesh.vert_count,
    .vert_mem_offset = vert_range.offset,
    .index_count = mesh.index_count,
    .index_mem_offset = index_range.offset,
  };
  u32 id = g.meshes.count;
  array_push(g.meshes, vk_mesh);
  GpuMeshId handle = {id};
  return handle;
}

////////////////////////////////////////////////////////////////////////
// @Drawing

void vk_draw() {
  ProfFunc;
  Scratch scratch;
  VK_State& g = st->vk;

  b32 rebuild_static_buffer = false;
  if (g.static_entities_count != g.static_entities_count_old) {
    g.static_entities_count_old = g.static_entities_count;
    rebuild_static_buffer = true;
  }

  VkCommandBuffer cmd = vk_get_current_cmd();
  g.CmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipeline_layout, 0, 1, &g.descriptor_sets, 0, null);
  VkDeviceSize size = 0;
  g.CmdBindVertexBuffers(cmd, 0, 1, &g.vert_buffer.h, &size);
  g.CmdBindIndexBuffer(cmd, g.index_buffer.h, 0, VK_INDEX_TYPE_UINT32);

  GlobalStateGPU& shader_st = *g.gpu_global_shader_st;
  shader_st.projection_view = st->projection * st->view;
  shader_st.projection = st->projection;
  shader_st.view = st->view;
  
  struct RenderCtx {
    u32 draw_call_count;
    u32 entities_count;
    u32 static_draw_call_count;
    u32 static_entities_count;
    EntityGPU* gpu_entities;
    u32* gpu_entities_indices;
    VK_DrawCallInfo* gpu_draw_call_infos;
  } ctx = {
    .gpu_entities = g.gpu_entities,
    .gpu_entities_indices = g.gpu_entities_indices,
    .gpu_draw_call_infos = g.gpu_draw_call_infos,
  };

  Loop (i, g.entity_pipelines.count) {
    VK_Pipeline pipeline = g.pipelines[g.entity_pipelines[i]];
    vk_bind_pipeline(cmd, pipeline.h);
    VK_PipelineBatch& batch = g.batches[pipeline.batch_idx];
    var fill_buffer = [&](VK_BatchType type) {
      VK_MeshesBatches shader_batch = batch.batches[type];
      b32 is_static = type & (Bit(1));
      u32 entity_offset = 0;
      u32 draw_call_offset = 0;
      u32* entities_count = &ctx.entities_count;
      u32* draw_call_count = &ctx.draw_call_count;
      if (is_static) {
        entity_offset = MaxEntities;
        draw_call_offset = MaxDrawCalls / 2;
        entities_count = &ctx.static_entities_count;
        draw_call_count = &ctx.static_draw_call_count;
      }
      u32 draw_calls = 0;
      Loop (i, shader_batch.mesh_batches.count) {
        VK_MeshBatch mesh_batch = shader_batch.mesh_batches[i];
        if (mesh_batch.entities.count == 0) continue;
        Loop (i, mesh_batch.entities.count) {
          u32 e_id = mesh_batch.entities[i];
          u32 entity_idx = entity_offset + id_idx(e_id);
          Transform trans = {};
          if (is_static) {
            StaticEntity& e = get_static_entity(StaticEntityId(e_id));
            trans = Transform{e.pos, e.rot, e.scale};
          } else {
            Entity& e = get_entity(EntityId(e_id));
            trans = Transform{e.pos, e.rot, e.scale};
          }
          ctx.gpu_entities[entity_idx].model = mat4_transform(trans);
          ctx.gpu_entities_indices[entity_offset + (*entities_count)++] = entity_idx;
        }
        u32 mesh_idx = id_idx(mesh_batch.mesh_id.v);
        VK_Mesh mesh = g.meshes[mesh_idx];
        VK_DrawCallInfo info = {};
        if (mesh.index_count) {
          info = {
            .index_draw_command = {
              .indexCount = (u32)mesh.index_count,
              .instanceCount = mesh_batch.entities.count,
              .firstIndex = (u32)(mesh.index_mem_offset / sizeof(u32)),
              .vertexOffset = (i32)(mesh.vert_mem_offset / sizeof(Vertex)),
              .firstInstance = 0,
            },
            .entity_inst_offset = entity_offset + *entities_count - mesh_batch.entities.count,
          };
        } else {
          info = {
            .draw_command = {
              .vertexCount = (u32)mesh.vert_count,
              .instanceCount = mesh_batch.entities.count,
              .firstVertex = (u32)(mesh.vert_mem_offset / sizeof(Vertex)),
              .firstInstance = 0,
            },
            .entity_inst_offset = entity_offset + *entities_count - mesh_batch.entities.count,
          };
        }
        ctx.gpu_draw_call_infos[draw_call_offset + (*draw_call_count)++] = info;
        ++draw_calls;
      }
      VK_IndirectDrawCall drawcall = {
        .draw_call_count = draw_calls,
        .draw_call_offset = draw_call_offset + *draw_call_count - draw_calls
      };
      return drawcall;
    };

    var make_draw = [&](VK_IndirectDrawCall draw, b32 indexed) {
      if (draw.draw_call_count) {
        VK_PushConstant push = {.drawcall_offset = draw.draw_call_offset};
        g.CmdPushConstants(cmd, g.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(VK_PushConstant), &push);
        u64 draw_call_mem_offset = push.drawcall_offset*sizeof(VK_DrawCallInfo);
        if (indexed) {
          g.CmdDrawIndexedIndirect(cmd, g.indirect_draw_buffer.h, draw_call_mem_offset, draw.draw_call_count, sizeof(VK_DrawCallInfo));
        } else {
          g.CmdDrawIndirect(cmd, g.indirect_draw_buffer.h, draw_call_mem_offset, draw.draw_call_count, sizeof(VK_DrawCallInfo));
        }
      }
    };

    make_draw(fill_buffer(VK_BatchType_Indexed), true);
    make_draw(fill_buffer(VK_BatchType_Unindexed), false);

    ///////////////////////////////////
    // Static entities
    if (rebuild_static_buffer) {
      g.static_draw_calls[i*2] = fill_buffer(VK_BatchType_StaticIndexed);
      g.static_draw_calls[i*2 + 1] = fill_buffer(VK_BatchType_StaticUnindexed);
    }
    make_draw(g.static_draw_calls[i*2], true);
    make_draw(g.static_draw_calls[i*2+1], false);
  }

  // Debug drawing
  if (g.draw_lines.count > 0) {
    g.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipelines[g.debug_line_pipeline].h);
    g.CmdBindVertexBuffers(cmd, 0, 1, &g.vert_buffer.h, (VkDeviceSize*)&g.draw_lines_offset);
    g.CmdDraw(cmd, g.draw_lines.count*2, 1, 0, 0);
    array_clear(g.draw_lines);
  }
  if (g.draw_lines_consistent.count > 0) {
    g.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipelines[g.debug_line_pipeline].h);
    g.CmdBindVertexBuffers(cmd, 0, 1, &g.vert_buffer.h, (VkDeviceSize*)&g.draw_lines_consistent_offset);
    g.CmdDraw(cmd, g.draw_lines_consistent.count*2, 1, 0, 0);
  }

  // Cube map
  g.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipelines[g.cubemap_pipeline].h);
  GpuMeshId h = mesh_get(Mesh_Cube);
  VK_Mesh mesh = g.meshes[h.v];
  g.CmdBindVertexBuffers(cmd, 0, 1, &g.vert_buffer.h, &mesh.vert_mem_offset);
  if (mesh.index_count) {
    g.CmdBindIndexBuffer(cmd, g.index_buffer.h, mesh.index_mem_offset, VK_INDEX_TYPE_UINT32);
    g.CmdDrawIndexed(cmd, mesh.index_count, 1, 0, 0, 0);
  } else {
    g.CmdDraw(cmd, mesh.vert_count, 1, 0, 0);
  }

  // Rect drawing
  g.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipelines[g.ui_pipeline].h);
  if (g.draw_rects.count > 0) {
    g.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipelines[g.ui_pipeline].h);
    g.CmdBindVertexBuffers(cmd, 0, 1, &g.vert_buffer.h, (VkDeviceSize*)&g.draw_rects_offset);
    g.CmdDraw(cmd, g.draw_rects.count*6, 1, 0, 0);
    array_clear(g.draw_rects);
  }

  // Hello world
  g.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipelines[g.triangle_pipeline].h);
  g.CmdDraw(cmd, 3, 1, 0, 0);
}

void vk_draw_screen() {
  VK_State& g = st->vk;
  VkCommandBuffer cmd = vk_get_current_cmd();
  VK_PushConstant push = {g.current_image_idx};
  g.CmdPushConstants(cmd, g.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(VK_PushConstant), &push);
  g.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipelines[g.screen_pipeline].h);
  g.CmdDraw(cmd, 3, 1, 0, 0);
}

void vk_loader_load_core() {
  VK_State& g = st->vk;
  g.lib = os_lib_open("libvulkan.so");
#define X(name) VK_GET_PROC(name)
  VK_GET_PROC_LIST
#undef X
}

void vk_loader_load_instance() {
  VK_State& g = st->vk;
#define X(name) VK_INSTANCE_GET_PROC(name)
  VK_INSTANCE_GET_PROC_LIST
#undef X
}

void vk_loader_load_device() {
  VK_State& g = st->vk;
#define X(name) VK_DEVICE_GET_PROC(name)
  VK_DEVICE_GET_PROC_LIST
#undef X
}

intern void vk_instance_init() {
  Scratch scratch;
  VK_State& g = st->vk;
  VkApplicationInfo app_info = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .apiVersion = VK_API_VERSION_1_4
  };
  var required_validation_layer_names = darray_make<const char*>(scratch);
  var required_extensions = darray_make<const char*>(scratch);
  array_push(required_extensions, VK_KHR_SURFACE_EXTENSION_NAME, VK_SURFACE_NAME);

#if BUILD_DEBUG

  // Validation layer
  array_push(required_validation_layer_names, "VK_LAYER_KHRONOS_validation");
  Debug("%Required layers:");
  Loop (i, required_validation_layer_names.count) {
    Debug(required_validation_layer_names[i]);
  }
  u32 available_layer_count = 0;
  VK_CHECK(g.EnumerateInstanceLayerProperties(&available_layer_count, null));
  VkLayerProperties* available_layers = push_array(scratch, VkLayerProperties, available_layer_count);
  VK_CHECK(g.EnumerateInstanceLayerProperties(&available_layer_count, available_layers));
  Loop (i, required_validation_layer_names.count) {
    b32 found = false;
    Loop (j, available_layer_count) {
      if (str_match(required_validation_layer_names[i], available_layers[j].layerName)) {
        found = true;
        Info("Validation layer %s found", String(required_validation_layer_names[i]));
        break;
      }
    }
    AssertMsg(found, "Required validation layer is missing: %s", String(required_validation_layer_names[i]));
  }

  // Extensions
  array_push(required_extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  Debug("Required extensions:");
  Loop (i, required_extensions.count) {
    Debug(required_extensions[i]);
  }
  u32 extension_count = 0;
  g.EnumerateInstanceExtensionProperties(null, &extension_count, null);
  VkExtensionProperties* props = push_array(scratch, VkExtensionProperties, extension_count);
  g.EnumerateInstanceExtensionProperties(null, &extension_count, props);
  Loop (i, required_extensions.count) {
    b32 found = false;
    Loop (j, extension_count) {
      if (str_match(required_extensions[i], props[j].extensionName)) {
        found = true;
        Info("extension %s found", String(required_extensions[i]));
        break;
      }
    }
    AssertMsg(found, "Required extension is missing: %s", String(required_extensions[i]));
  }
#endif

  VkInstanceCreateInfo instance_create_info = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &app_info,
    .enabledLayerCount = (u32)required_validation_layer_names.count,
    .ppEnabledLayerNames = required_validation_layer_names.data,
    .enabledExtensionCount = (u32)required_extensions.count,
    .ppEnabledExtensionNames = required_extensions.data,
  };
  VK_CHECK(g.CreateInstance(&instance_create_info, g.allocator, &g.instance));
  Info("Vulkan insance created");

#if BUILD_DEBUG
  VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
    .pfnUserCallback = [](
      VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      VkDebugUtilsMessageTypeFlagsEXT message_types,
      const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
      void* user_data) -> VkBool32 
    {
      switch (message_severity) {
        default:break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
          Trace(String(callback_data->pMessage));
        } break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
          Info(String(callback_data->pMessage));
        } break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
          String skip_warnings[] = {
            "vkCreateGraphicsPipelines(): pCreateInfos[0].pVertexInputState Vertex attribute at location",
            "vkCreateGraphicsPipelines(): pCreateInfos[0] (SPIR-V Interface) VK_SHADER_STAGE_VERTEX_BIT has an Output value declared at Location",
            "(Warning - This VUID has now been reported 10 times, which is the duplicate_message_limit value, this will be the last time reporting it).",
          };
          for EachElement(i, skip_warnings) {
            String skip = skip_warnings[i];
            String warn = callback_data->pMessage;
            if (warn.size >= skip.size) {
              if (str_match(str_prefix(warn, skip.size), skip)) {
                return false;
              }
            }
          }
          Warn(String(callback_data->pMessage));
        } break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
          ErrorArena(st->vk.arena, String(callback_data->pMessage));
        } break;
      }
      return false;
    },
  };
  PFN_vkCreateDebugUtilsMessengerEXT func = null;
  Assign(func, g.GetInstanceProcAddr(g.instance, "vkCreateDebugUtilsMessengerEXT"));
  AssertMsg(func, "Failed to create debug messenger");
  VK_CHECK(func(g.instance, &debug_create_info, st->vk.allocator, &st->vk.debug_messenger));
  Debug("Vulkan debugger created");
#endif
}

intern void vk_device_init() {
  Scratch scratch;
  VK_State& g = st->vk;

  ///////////////////////////////////
  // Physical device
  {
    u32 physical_device_count;
    VK_CHECK(g.EnumeratePhysicalDevices(g.instance, &physical_device_count, null));
    VK_Device* devices = push_array_zero(scratch, VK_Device, physical_device_count);
    VkPhysicalDevice* physical_devices = push_array(scratch, VkPhysicalDevice, physical_device_count);
    VK_CHECK(g.EnumeratePhysicalDevices(g.instance, &physical_device_count, physical_devices));
    b32 discrete_available = false;
    u32 discrete_idx = 0;
    u32 integrated_idx = 0;
    Loop(i, physical_device_count) {
      VK_Device& device = devices[i];
      device.physical_device = physical_devices[i];
      devices[i].depth_format = VK_FORMAT_D32_SFLOAT;
      g.GetPhysicalDeviceProperties(physical_devices[i], &device.properties);
      g.GetPhysicalDeviceFeatures(physical_devices[i], &device.features);
      g.GetPhysicalDeviceMemoryProperties(physical_devices[i], &device.memory);

      ///////////////////////////////////
      // Swapchain info
      {
        VK_SwapchainSupportInfo info = {};
        VK_CHECK(g.GetPhysicalDeviceSurfaceCapabilitiesKHR(device.physical_device, g.surface, &info.capabilities));
        VK_CHECK(g.GetPhysicalDeviceSurfaceFormatsKHR(device.physical_device, g.surface, &info.format_count, null));
        VK_CHECK(g.GetPhysicalDeviceSurfacePresentModesKHR(device.physical_device, g.surface, &info.present_mode_count, null));
        info.formats = push_array(g.arena, VkSurfaceFormatKHR, info.format_count);
        info.present_modes = push_array(g.arena, VkPresentModeKHR, info.present_mode_count);
        VK_CHECK(g.GetPhysicalDeviceSurfaceFormatsKHR(device.physical_device, g.surface, &info.format_count, info.formats));
        VK_CHECK(g.GetPhysicalDeviceSurfacePresentModesKHR(device.physical_device, g.surface, &info.present_mode_count, info.present_modes));
        device.swapchain_support = info;
      }

      ///////////////////////////////////
      // Select queue families
      {
        u32 queue_family_count = 0;
        g.GetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &queue_family_count, null);
        VkQueueFamilyProperties* queue_families = push_array(scratch, VkQueueFamilyProperties, queue_family_count);
        g.GetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &queue_family_count, queue_families);
        Loop (i, queue_family_count) {
          u32 flags = queue_families->queueFlags;
          if ((flags & VK_QUEUE_GRAPHICS_BIT)) {
            device.graphics_queue_family_idx = i;
          }
          if ((flags & VK_QUEUE_TRANSFER_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT)) {
            device.transfer_queue_family_idx = i;
          }
          if ((flags & VK_QUEUE_COMPUTE_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT) && !(flags & VK_QUEUE_TRANSFER_BIT)) {
            device.compute_queue_family_idx = i;
          }
        }
        Info("Queue Families: Graphics %i, Compute %i, Transfer %i", device.graphics_queue_family_idx, device.compute_queue_family_idx, device.transfer_queue_family_idx);
      }

      ///////////////////////////////////
      // Gpu Info
      Info("Available device: '%s'", String(device.properties.deviceName));
      switch (device.properties.deviceType) {
        default:{}break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: {
          integrated_idx = i;
          Info("GPU type is Integrated");
        } break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: {
          discrete_available = true;
          discrete_idx = i;
          Info("GPU type is Descrete");
        } break;
      }
      Info("GPU Driver version: %i.%i.%i", VK_VERSION_MAJOR(device.properties.driverVersion), VK_VERSION_MINOR(device.properties.driverVersion), VK_VERSION_PATCH(device.properties.driverVersion));
      Info("GPU API version: %i.%i.%i", VK_VERSION_MAJOR(device.properties.apiVersion), VK_VERSION_MINOR(device.properties.apiVersion), VK_VERSION_PATCH(device.properties.apiVersion));
      Loop (i, device.memory.memoryHeapCount) {
        f64 mem_size = (((f64)device.memory.memoryHeaps[i].size) / GB(1));
        if (device.memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
          Info("Heap %i: GPU: %.2f GiB", i, mem_size);
        else
          Info("Heap %i: System: %.2f GiB", i, mem_size);
      }

      ///////////////////////////////////
      // Find cpu/gpu memory type index
      Loop (i, device.memory.memoryTypeCount) {
        VkMemoryType t = device.memory.memoryTypes[i];
        if (FlagEquals(t.propertyFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
          device.gpu_type_idx = i;
          break;
        }
      }
      Loop (i, device.memory.memoryTypeCount) {
        VkMemoryType t = device.memory.memoryTypes[i];
        if ((t.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
          device.cpu_type_idx = i;
          break;
        }
      }
    }
    if (discrete_available) {
      g.device = devices[discrete_idx];
      Info("Discrete GPU was choosen");
    } else {
      g.device = devices[integrated_idx];
      Info("Integrated GPU was choosen");
    }
    Info("Physical device selected");
  }

  ///////////////////////////////////
  // Logical device
  {
    // Find unique queues
    const u32 queue_count = 3;
    Array<u32, queue_count> indices = {};
    array_push(indices, g.device.graphics_queue_family_idx);
    if (!array_exists(indices, g.device.transfer_queue_family_idx)) {
      array_push(indices, g.device.transfer_queue_family_idx);
    }
    if (!array_exists(indices, g.device.compute_queue_family_idx)) {
      array_push(indices, g.device.compute_queue_family_idx);
    }
    Array<VkDeviceQueueCreateInfo, queue_count> queue_create_infos = {};
    Loop (i, indices.count) {
      f32 queue_priority = 1.0f;
      VkDeviceQueueCreateInfo device_queue_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = null,
        .flags = 0,
        .queueFamilyIndex = indices[i],
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
      };
      array_push(queue_create_infos, device_queue_create_info);
    }
    
    ///////////////////////////////////
    // Features
    // Indirect drawing
    VkPhysicalDeviceShaderDrawParametersFeatures draw_features = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES,
      .shaderDrawParameters = VK_TRUE,
    };
    // Bindless
    VkPhysicalDeviceDescriptorIndexingFeatures indexing_features = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
      .pNext = &draw_features,
      .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,     // allows runtime indexing
      .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,  // allows update image descriptors
      .descriptorBindingPartiallyBound = VK_TRUE,               // allows not updated descriptors
      .runtimeDescriptorArray = VK_TRUE,                        // allows not specified size of descriptor array in shader
    };
    // Dynamic rendering
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
      .pNext = &indexing_features,
      .dynamicRendering = true
    };
    const char* extension_names[] = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    VkPhysicalDeviceFeatures device_features = {
      .multiDrawIndirect = true, // Request indirect drawing
      .fillModeNonSolid = true,  // Request anistrophy
      .samplerAnisotropy = true, // Request wireframe
    };
    VkDeviceCreateInfo device_create_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &dynamic_rendering_features,
      .queueCreateInfoCount = queue_create_infos.count,
      .pQueueCreateInfos = queue_create_infos.data,
      .enabledExtensionCount = ArrayCount(extension_names),
      .ppEnabledExtensionNames = extension_names,
      .pEnabledFeatures = &device_features,
    };
    VK_CHECK(g.CreateDevice(g.device.physical_device, &device_create_info, g.allocator, &g.device.logical_device));
    Info("Logical device created");
  }
}

void vk_init() {
  ProfFunc;
  Scratch scratch;
  Arena arena = arena_make_named("vk arena");
  VK_State& g = st->vk;
  g.arena = arena;
  g.gpa = alloc_seglist_make(g.arena, "vk gpa");
  g.scale = 1;
  g.pipelines = darray_make<VK_Pipeline>(g.gpa);
  g.static_draw_calls = darray_make<VK_IndirectDrawCall>(g.gpa);
  g.entity_pipelines = darray_make<u32>(g.gpa);
  g.modules = darray_make<VK_ShaderModuleEntry>(g.gpa);
  g.batches = darray_make<VK_PipelineBatch>(g.gpa);
  g.shader_states = darray_make<ShaderState>(g.gpa);
  g.shader_to_pipeline = map_make<VK_KeyToShaderPipeline, u32>(g.gpa);
  g.shader_to_module = map_make<String, u32>(g.gpa);

#if VulkanUseAllocator
  g._allocator = vk_allocator_create();
  g.allocator = &g._allocator;
#endif

  {
    ProfBlock("vulkan loader");
    vk_loader_load_core();
    vk_instance_init();
    vk_loader_load_instance();
    vk_surface_create();
    vk_device_init();
    vk_loader_load_device();
  }

  ///////////////////////////////////
  // Buffers
  {
    g.gpu_mem = vk_mem_alloc(VK_MemType_Gpu, MB(200));
    g.cpu_mem = vk_mem_alloc(VK_MemType_Cpu, MB(300));
    g.vert_buffer = vk_buffer_alloc(MB(1), VK_BufferUsageFlag_Vert | VK_BufferUsageFlag_Dst, VK_MemType_Gpu);
    g.index_buffer = vk_buffer_alloc(MB(1), VK_BufferUsageFlag_Index | VK_BufferUsageFlag_Dst, VK_MemType_Gpu);
    g.stage_buffer = vk_buffer_alloc(MB(10), VK_BufferUsageFlag_Src, VK_MemType_Cpu);
    g.storage_buffer = vk_buffer_alloc(MB(200), VK_BufferUsageFlag_Storage, VK_MemType_Cpu);
    g.indirect_draw_buffer = vk_buffer_alloc(MB(10), VK_BufferUsageFlag_Indirect, VK_MemType_Cpu);
  }

  g.draw_lines_offset = offset_push_array(g.vert_buffer.pos, Vertex, KB(1));
  g.draw_lines_consistent_offset = offset_push_array(g.vert_buffer.pos, Vertex, KB(1));
  g.draw_rects_offset = offset_push_array(g.vert_buffer.pos, Vertex, KB(1));

  ///////////////////////////////////
  // Device stuff
  {
    g.GetDeviceQueue(vkdevice, g.device.graphics_queue_family_idx, 0, &g.device.graphics_queue);
    g.GetDeviceQueue(vkdevice, g.device.transfer_queue_family_idx, 0, &g.device.transfer_queue);
    g.GetDeviceQueue(vkdevice, g.device.compute_queue_family_idx, 0, &g.device.compute_queue);
    Info("Queues obtained");
    VkCommandPoolCreateInfo graphics_pool_create_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = g.device.graphics_queue_family_idx,
    };
    VK_CHECK(g.CreateCommandPool(vkdevice, &graphics_pool_create_info, g.allocator, &g.device.cmd_pool));
    Info("Graphics command pool created");
    v2u win_size = os_get_window_size();
    g.width = win_size.x;
    g.height = win_size.y;
    g.images_in_flight = g.device.swapchain_support.capabilities.minImageCount;
    g.frames_in_flight = g.images_in_flight - 1;
    // Choose a image format
    g.swapchain.format = g.device.swapchain_support.formats[0];
    Loop (i, g.device.swapchain_support.format_count) {
      VkSurfaceFormatKHR format = g.device.swapchain_support.formats[i];
      if (format.format == VK_FORMAT_B8G8R8A8_UNORM && // darker
      // if (format.format == VK_FORMAT_B8G8R8A8_SRGB && // brighter TODO: use
          format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
      {
        g.swapchain.format = format;
        break;
      }
    }
    // Choose present mode
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    Loop (i, g.device.swapchain_support.present_mode_count) {
      VkPresentModeKHR mode = g.device.swapchain_support.present_modes[i];
      if (mode == VK_PRESENT_MODE_FIFO_KHR) {
        present_mode = mode;
        break;
      }
    }
    g.swapchain.present_mode = present_mode;
    v2u size = os_get_screen_size();
    VK_ImageInfo depth_info = vk_image_info_default(size.x, size.y);
    depth_info.format = g.device.depth_format;
    depth_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depth_info.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    depth_info.samples = VK_SAMPLE_COUNT_4_BIT;
    g.swapchain.depth_attachment = vk_image_create(depth_info);
    vk_swapchain_create(false);
    Info("Swapchain created");
    g.cmds = push_array(g.arena, VkCommandBuffer, g.frames_in_flight);
    Loop (i, g.frames_in_flight) {
      g.cmds[i] = vk_cmd_alloc(g.device.cmd_pool);
    }
    g.upload_cmd = vk_cmd_alloc(g.device.cmd_pool);
    Info("Command buffers created");
  }

  // Sync
  {
    // NOTE: for some reasons validation layer complains about render_complete_sempahores when their number is frames_in_flight but doesn't when is images_in_flight
    g.sync.render_complete_semaphores = push_array(g.arena, VkSemaphore, g.images_in_flight);
    Loop (i, g.images_in_flight) {
      VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
      g.CreateSemaphore(vkdevice, &semaphore_create_info, g.allocator, &g.sync.render_complete_semaphores[i]);
    }
    g.sync.image_available_semaphores = push_array(g.arena, VkSemaphore, g.images_in_flight);
    g.sync.in_flight_fences = push_array(g.arena, VkFence, g.frames_in_flight);
    Loop (i, g.frames_in_flight) {
      VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
      g.CreateSemaphore(vkdevice, &semaphore_create_info, g.allocator, &g.sync.image_available_semaphores[i]);
      VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
      };
      VK_CHECK(g.CreateFence(vkdevice, &fence_create_info, g.allocator, &g.sync.in_flight_fences[i]));
    }
  }

  ///////////////////////////////////
  // Descriptors
  {
    ///////////////////////////////////
    // Pool
    #define MaxStorageBuffer 7
    #define MaxSets 1
    #define MaxSamplers 1
    #define MaxCubeTextures 1
    {
      VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MaxStorageBuffer},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MaxTextures},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MaxCubeTextures},
        {VK_DESCRIPTOR_TYPE_SAMPLER, MaxSamplers},
      };
      VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
        .maxSets = MaxSets,
        .poolSizeCount = ArrayCount(pool_sizes),
        .pPoolSizes = pool_sizes,
      };
      g.CreateDescriptorPool(vkdevice, &pool_info, g.allocator, &g.descriptor_pool);
    }

    ///////////////////////////////////
    // Setlayout
    {
      var binding_flags = darray_make<VkDescriptorBindingFlags>(scratch);
      var bindings = darray_make<VkDescriptorSetLayoutBinding>(scratch);
      var add_binding = [&](u32 binding_idx, VK_DescriptorType type, u32 count = 1) {
        VkDescriptorBindingFlags flags = {};
        if (type == VK_DescriptorType_Image) {
          flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
        }
        array_push(binding_flags, flags);
        VkDescriptorType descriptor_type;
        switch (type) {
          case VK_DescriptorType_Storage: descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
          case VK_DescriptorType_Image: descriptor_type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; break;
          case VK_DescriptorType_Sampler: descriptor_type = VK_DESCRIPTOR_TYPE_SAMPLER; break;
        }
        VkDescriptorSetLayoutBinding binding = {
          .binding = binding_idx,
          .descriptorType = descriptor_type,
          .descriptorCount = count,
          .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        };
        array_push(bindings, binding);
      };
      add_binding(Bindings::State, VK_DescriptorType_Storage);
      add_binding(Bindings::Textures, VK_DescriptorType_Image, MaxTextures);
      add_binding(Bindings::Samplers, VK_DescriptorType_Sampler);
      add_binding(Bindings::CubeTextures, VK_DescriptorType_Image);
      add_binding(Bindings::Drawinfo, VK_DescriptorType_Storage);
      add_binding(Bindings::Entities, VK_DescriptorType_Storage);
      add_binding(Bindings::Materials, VK_DescriptorType_Storage);
      add_binding(Bindings::PointLights, VK_DescriptorType_Storage);
      add_binding(Bindings::DirLights, VK_DescriptorType_Storage);
      add_binding(Bindings::SpotLights, VK_DescriptorType_Storage);
      VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .pNext = null,
        .bindingCount = binding_flags.count,
        .pBindingFlags = binding_flags.data,
      };
      VkDescriptorSetLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &binding_flags_info,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        .bindingCount = bindings.count,
        .pBindings = bindings.data,
      };
      VK_CHECK(g.CreateDescriptorSetLayout(vkdevice, &layout_info, g.allocator, &g.descriptor_set_layout));
    }

    ///////////////////////////////////
    // Descriptor set
    {
      VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = g.descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &g.descriptor_set_layout,
      };
      VK_CHECK(g.AllocateDescriptorSets(vkdevice, &alloc_info, &g.descriptor_sets));
    }
    
    ///////////////////////////////////
    // Write Descriptors
    {
      var writes = darray_make<VkWriteDescriptorSet>(scratch);
      var buffer_infos = darray_make<VkDescriptorBufferInfo>(scratch);
      #define write_descriptor(buf, binding, T, count) (T*)write_descriptor_((buf), (binding), sizeof(T) * (count), alignof(T))
      var write_descriptor_ = [&](VK_Buffer& buf, Bindings binding, u64 size, u64 align) {
        u64 off = offset_push(buf.pos, size, align);
        VkDescriptorBufferInfo buffer_info = {
          .buffer = buf.h,
          .offset = off,
          .range = size,
        };
        array_push(buffer_infos, buffer_info);
        VkWriteDescriptorSet write = {
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = g.descriptor_sets,
          .dstBinding = binding,
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        };
        array_push(writes, write);
        u8* res = Offset(buf.base, off);
        return res;
      };
      g.gpu_global_shader_st = write_descriptor(g.storage_buffer, Bindings::State, GlobalStateGPU, 1);
      g.gpu_entities = write_descriptor(g.storage_buffer, Bindings::Entities, EntityGPU, MaxEntities+MaxStaticEntities);
      g.gpu_materials = write_descriptor(g.storage_buffer, Bindings::Materials, MaterialGPU, MaxMaterials);
      g.gpu_draw_call_infos = write_descriptor(g.indirect_draw_buffer, Bindings::Drawinfo, VK_DrawCallInfo, MaxDrawCalls);
      g.gpu_entities_indices = g.gpu_global_shader_st->entity_indices;
      Loop (i, writes.count) {
        writes[i].pBufferInfo = &buffer_infos[i];
      }
      g.UpdateDescriptorSets(vkdevice, writes.count, writes.data, 0, null);
    }
  }

  ///////////////////////////////////
  // Sampler descriptor
  {
    VkSamplerCreateInfo sampler_info = {
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .mipLodBias = 0.0f,
      .anisotropyEnable = VK_TRUE,
      .maxAnisotropy = 8,
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      .minLod = 0.0f,
      .maxLod = VK_LOD_CLAMP_NONE,
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE,
    };
    VK_CHECK(g.CreateSampler(vkdevice, &sampler_info, g.allocator, &g.sampler));
    VkDescriptorImageInfo descriptor_image_info = {
      .sampler = g.sampler,
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    VkWriteDescriptorSet texture_descriptor = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = g.descriptor_sets,
      .dstBinding = Bindings::Samplers,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
      .pImageInfo = &descriptor_image_info,
    };
    VkWriteDescriptorSet descriptors[] = {texture_descriptor};
    g.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  }

  ///////////////////////////////////
  // Pipeline layout
  {
    VkPushConstantRange push_constant = {
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      .offset = 0,
      .size = sizeof(VK_PushConstant),
    };
    VkPipelineLayoutCreateInfo pipeline_layout_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &g.descriptor_set_layout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &push_constant,
    };
    VK_CHECK(g.CreatePipelineLayout(vkdevice, &pipeline_layout_info, g.allocator, &g.pipeline_layout));
  }

  // Texture targets
  {
    g.texture_targets = push_array_zero(g.arena, VK_Image, g.images_in_flight);
    g.textures.count += g.images_in_flight;
    // Msaa
    v2u size = os_get_screen_size();
    VK_ImageInfo image_info = vk_image_info_default(size.x, size.y);
    image_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_info.samples = VK_SAMPLE_COUNT_4_BIT;
    g.msaa_texture = vk_image_create(image_info);
    // Textures
    Loop (i, g.images_in_flight) {
      vk_image_destroy(g.texture_targets[i]);
      VK_ImageInfo image_info = vk_image_info_default(size.x, size.y);
      image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
      image_info.format = VK_FORMAT_B8G8R8A8_UNORM;
      g.texture_targets[i] = vk_image_create(image_info);
      VkDescriptorImageInfo descriptor_image_info = {
        .sampler = g.sampler,
        .imageView = g.texture_targets[i].view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      };
      VkWriteDescriptorSet texture_write_descriptor = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = g.descriptor_sets,
        .dstBinding = 1,
        .dstArrayElement = (u32)i,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = &descriptor_image_info,
      };
      VkWriteDescriptorSet descriptors[] = {texture_write_descriptor};
      g.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
    }
    // Depth buffer
    VK_ImageInfo depth_info = vk_image_info_default(size.x, size.y);
    depth_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depth_info.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    depth_info.format = g.device.depth_format;
    depth_info.samples = VK_SAMPLE_COUNT_4_BIT;
    g.offscreen_depth_buffer = vk_image_create(depth_info);
  }

  {
    ProfBlock("Waiting for compiling shaders");
    vk_shader_compile_join(st->shader_module_compiled_names);
  }

  ///////////////////////////////////
  // Load basic shaders
  {
    ShaderDesc shader = {
      .name = "line",
      .state = {
        .topology = ShaderTopology_Line,
        .is_transparent = true,
        .use_depth = true,
      },
    };
    g.debug_line_pipeline = vk_shader_pipeline_alloc(shader);
  }
  {
    ShaderDesc shader = {
      .name = "screen",
      .state = {
        .topology = ShaderTopology_Triangle,
        .samples = 1,
      },
    };
    g.screen_pipeline = vk_shader_pipeline_alloc(shader);
  }
  {
    ShaderDesc shader = {
      .name = "cubemap",
      .state = {
        .type = ShaderType_Cube,
        .topology = ShaderTopology_Triangle,
        .is_transparent = false,
        .use_depth = true,
      },
    };
    g.cubemap_pipeline = vk_shader_pipeline_alloc(shader);
  }
  {
    ShaderDesc shader = {
      .name = "ui",
      .state = {
        .type = ShaderType_Drawing,
        .topology = ShaderTopology_Triangle,
        .is_transparent = false,
        .use_depth = false,
      },
    };
    g.ui_pipeline = vk_shader_pipeline_alloc(shader);
  }
  {
    ShaderDesc shader = {
      .name = "triangle",
      .state = {
        .type = ShaderType_Drawing,
        .topology = ShaderTopology_Triangle,
      },
    };
    g.triangle_pipeline = vk_shader_pipeline_alloc(shader);
  }
  
  Info("Vulkan renderer initialized");
}

void vk_shutdown() {
  VK_State& g = st->vk;
  VK_CHECK(g.DeviceWaitIdle(vkdevice));
  
  Loop (i, g.frames_in_flight) {
    g.DestroySemaphore(vkdevice, g.sync.image_available_semaphores[i], g.allocator);
    g.DestroySemaphore(vkdevice, g.sync.render_complete_semaphores[i], g.allocator);
    g.DestroySemaphore(vkdevice, g.sync.compute_complete_semaphores[i], g.allocator);
    g.DestroyFence(vkdevice, g.sync.in_flight_fences[i], g.allocator);
    vk_cmd_free(g.device.cmd_pool, g.cmds[i]);
  }

  {
    g.DestroyBuffer(vkdevice, g.vert_buffer.h, g.allocator);
    g.DestroyBuffer(vkdevice, g.index_buffer.h, g.allocator);
    g.DestroyBuffer(vkdevice, g.stage_buffer.h, g.allocator);
    g.DestroyBuffer(vkdevice, g.storage_buffer.h, g.allocator);
  }
  
  vk_swapchain_destroy(g.swapchain);
  
  // Device
  {
    Debug("Destroying Vulkan device...");
    g.DestroyCommandPool(vkdevice, g.device.cmd_pool, g.allocator);
    g.DestroyCommandPool(vkdevice, g.device.cmd_pool, g.allocator);
    g.DestroyDevice(vkdevice, g.allocator);
  }
  
  Info("Releasing physical device resources...");
  
  Debug("Destroying Vulkan surface...");
  g.DestroySurfaceKHR(g.instance, g.surface, g.allocator);
  
#if BUILD_DEBUG
  Debug("Destroying Vulkan debugger...");
  PFN_vkDestroyDebugUtilsMessengerEXT func = null;
  Assign(func, g.GetInstanceProcAddr(g.instance, "vkDestroyDebugUtilsMessengerEXT"));
  func(g.instance, g.debug_messenger, g.allocator);
#endif

  Debug("Destroying Vulkan instance...");
  g.DestroyInstance(g.instance, g.allocator);
}

void vk_begin_frame() {
  ProfFunc;
  VK_State& g = st->vk;
  
  // for (i32 i = 0; i < g.debug_lines_remain.count; ) {
  //   g_st->vk.debug_line_times[i] -= g_dt;
  //   if (g_st->vk.debug_line_times[i] <= 0) {
  //     g_st->vk.debug_lines_remain.swap_remove(i);
  //   } else {
  //     ++i;
  //   }
  // }
  {
    ProfBlock("block");
    if (g.draw_lines.count > 0) {
      ProfBlock("1");
      u32 size = g.draw_lines.count * sizeof(DebugDrawLine);
      void* data = g.draw_lines.data;
      vk_buffer_upload(g.vert_buffer, {g.draw_lines_offset, size}, data);
    }
    if (g.draw_lines_consistent.count > 0) {
      ProfBlock("2");
      u32 size = g.draw_lines_consistent.count * sizeof(DebugDrawLine);
      void* data = g.draw_lines_consistent.data;
      vk_buffer_upload(g.vert_buffer, {g.draw_lines_consistent_offset, size}, data);
    }
    if (g.draw_rects.count > 0) {
      ProfBlock("3");
      u32 size = g.draw_rects.count * sizeof(DebugDrawRect);
      void* data = g.draw_rects.data;
      vk_buffer_upload(g.vert_buffer, {g.draw_rects_offset, size}, data);
    }
  }

  {
    ProfBlock("rendering waiting");
    VK_CHECK(g.WaitForFences(vkdevice, 1, &g.sync.in_flight_fences[g.current_frame_idx], true, U64_MAX));
    VK_CHECK(g.ResetFences(vkdevice, 1, &g.sync.in_flight_fences[g.current_frame_idx]));
  }

  VkCommandBuffer cmd = vk_get_current_cmd();
  vk_cmd_begin(cmd);

  // Resize?
  v2u win_size = os_get_window_size();
  if (g.width != win_size.x || g.height != win_size.y || g.old_scale != g.scale) {
    g.width = win_size.x;
    g.height = win_size.y;
    g.old_scale = g.scale;
    VK_CHECK(g.DeviceWaitIdle(vkdevice));
    vk_swapchain_create(true);
    Info("Swapchain recreated x: %i y: %i", g.width, g.height);
    // vk_texture_resize_target();
    vk_image_handle_update(g.msaa_texture, g.width, g.height);
    Loop (i, g.images_in_flight) {
      vk_image_handle_update(g.texture_targets[i], g.width, g.height);
      VkDescriptorImageInfo descriptor_image_info = {
        .sampler = g.sampler,
        .imageView = g.texture_targets[i].view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      };
      VkWriteDescriptorSet texture_write_descriptor = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = g.descriptor_sets,
        .dstBinding = 1,
        .dstArrayElement = (u32)i,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = &descriptor_image_info,
      };
      VkWriteDescriptorSet descriptors[] = {texture_write_descriptor};
      g.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
    }
    vk_image_handle_update(g.offscreen_depth_buffer, g.width, g.height);
  }

  // Next image
  {
  ProfBlock("swapchain flip waiting");
#if GFX_X11 // NOTE: on x11 errors
  VkResult res = g.AcquireNextImageKHR(vkdevice, g.swapchain.h, U64_MAX, vk_get_current_image_available_semaphore(), null, &g.current_image_idx);
  if (res != VK_SUCCESS) {
    // Warn("%s", vk_result_string(res));
  }
#else
  VK_CHECK(g_g.AcquireNextImageKHR(vkdevice, g_g.swapchain.handle, U64_MAX, image_available_semaphore, null, &image_index));
#endif
  }

  // NOTE: we flip Y coordinate so Y:0 is on bottom of screen
  VkViewport viewport = {
    .x = 0.0f,
    .y = (f32)g.height*g.scale,
    .width = (f32)g.width*g.scale,
    .height = -(f32)g.height*g.scale,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  g.CmdSetViewport(cmd, 0, 1, &viewport);
  VkRect2D scissor = {
    .offset = {.x = 0, .y = 0},
    .extent = {
      .width = (u32)(g.width*g.scale), 
      .height = (u32)(g.height*g.scale)
    },
  };
  g.CmdSetScissor(cmd, 0, 1, &scissor);
}

void vk_end_frame() {
  ProfFunc;
  VK_State& g = st->vk;
  VkCommandBuffer cmd = vk_get_current_cmd();
  vk_cmd_end(cmd);

  ///////////////////////////////////
  // Render
  VkSemaphore semaphores_wait[] = {
    vk_get_current_image_available_semaphore(),
  };
  VkPipelineStageFlags sync_flags[] = {
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
  };
  VkSemaphore render_complete = vk_get_current_render_complete_semaphore();
  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = ArrayCount(semaphores_wait),
    .pWaitSemaphores = semaphores_wait,
    .pWaitDstStageMask = sync_flags,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &render_complete,
  };
  VK_CHECK(g.QueueSubmit(g.device.graphics_queue, 1, &submit_info, g.sync.in_flight_fences[g.current_frame_idx]));

  ///////////////////////////////////
  // Present
  {
    VkSemaphore render_complete = vk_get_current_render_complete_semaphore();
    VkPresentInfoKHR present_info = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &render_complete,
      .swapchainCount = 1,
      .pSwapchains = &g.swapchain.h,
      .pImageIndices = &g.current_image_idx,
    };
#if GFX_X11 // NOTE: on x11 errors
    VkResult res = g.QueuePresentKHR(g.device.graphics_queue, &present_info);
    UnusedVariable(res);
    // if (res != VK_SUCCESS) {
    //   Error("%s", vk_result_string(res));
    // }
#else
    VK_CHECK(g_g.QueuePresentKHR(g_g.device.graphics_queue, &present_info));
#endif
  }
  g.current_frame_idx = (g.current_frame_idx + 1) % g.frames_in_flight;
}

intern VkRenderingAttachmentInfo vk_default_color_attachment_info(VkImageView view) {
  VkRenderingAttachmentInfo result = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView = view,
    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .clearValue = {
      .color = {0.1f, 0.1f, 0.1f, 1.0f},
    }
  };
  return result;
}

intern VkRenderingAttachmentInfo vk_default_depth_attachment_info(VkImageView view) {
  VkRenderingAttachmentInfo result = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView = view,
    .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .clearValue = { 
      .depthStencil = {1, 0} 
    }
  };
  return result;
}

intern VkRenderingInfo vk_default_rendering_info(VkRenderingAttachmentInfo* color_attachment, VkRenderingAttachmentInfo* depth_attachment = null) {
  VK_State& g = st->vk;
  VkRenderingInfo result = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
    .renderArea = {
      .offset = {.x = 0, .y = 0}, 
      .extent = {.width = g.width, .height = g.height}
    },
    .layerCount = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments = color_attachment,
    .pDepthAttachment = depth_attachment
  };
  return result;
}

void vk_begin_renderpass(VK_RenderpassType renderpass_id) {
  VK_State& g = st->vk;
  VkCommandBuffer cmd = vk_get_current_cmd();
  switch (renderpass_id) {
    case VK_RenderpassType_World: {
      vk_image_layout_transition(cmd, g.msaa_texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      vk_image_layout_transition(cmd, g.offscreen_depth_buffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
      vk_image_layout_transition(cmd, g.texture_targets[g.current_image_idx], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

      VkRenderingAttachmentInfo color_attachment = vk_default_color_attachment_info(g.msaa_texture.view);
      color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
      color_attachment.resolveImageView = g.texture_targets[g.current_image_idx].view;
      color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      VkRenderingAttachmentInfo depth_attachment = vk_default_depth_attachment_info(g.offscreen_depth_buffer.view);
      
      VkRenderingInfo render_info = vk_default_rendering_info(&color_attachment, &depth_attachment);
      render_info.renderArea.extent = {g.texture_targets->info.width, g.texture_targets->info.height};
      g.CmdBeginRendering(cmd, &render_info);

      ///////////////////////////////////
      // vk_image_layout_transition(cmd, g_g.msaa_texture_target, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      // vk_image_layout_transition(cmd, g_g.swapchain.depth_attachment, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
      // vk_image_layout_transition(cmd, g_g.swapchain.images[g_g.current_image_idx], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

      // VkRenderingAttachmentInfo color_attachment = vk_default_color_attachment_info(g_g.msaa_texture_target.view);
      // color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
      // color_attachment.resolveImageView = g_g.swapchain.views[g_g.current_image_idx];
      // color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

      // VkRenderingAttachmentInfo depth_attachment = vk_default_depth_attachment_info(g_g.swapchain.depth_attachment.view);
      // // depth_attachment
      
      // VkRenderingInfo render_info = vk_default_rendering_info(&color_attachment, &depth_attachment);
      // g_g.CmdBeginRendering(cmd, &render_info);

      ///////////////////////////////////
      // // OLD
      // // Color
      // vk_image_layout_transition(cmd, g_g.swapchain.images[g_g.current_image_idx], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      // // Depth
      // vk_image_layout_transition(cmd, g_g.swapchain.depth_attachment, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
      // // Render
      // VkRenderingAttachmentInfo color_attachment = vk_default_color_attachment_info(g_g.swapchain.views[g_g.current_image_idx]);
      // VkRenderingAttachmentInfo depth_attachment = vk_default_depth_attachment_info(g_g.swapchain.depth_attachment.view);
      // VkRenderingInfo render_info = vk_default_rendering_info(&color_attachment, &depth_attachment);
      // g_g.CmdBeginRendering(cmd, &render_info);
    } break;
    case VK_RenderpassType_UI: {
    } break;
    case VK_RenderpassType_Screen: {
      vk_image_layout_transition(cmd, g.swapchain.images[g.current_image_idx], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      VkRenderingAttachmentInfo color_attachment = vk_default_color_attachment_info(g.swapchain.images[g.current_image_idx].view);
      VkRenderingInfo render_info = vk_default_rendering_info(&color_attachment);
      g.CmdBeginRendering(cmd, &render_info);
    } break;
  }
  return;
}

void vk_end_renderpass(VK_RenderpassType renderpass) {
  VK_State& g = st->vk;
  VkCommandBuffer cmd = vk_get_current_cmd();
  switch (renderpass) {
    case VK_RenderpassType_World: {
      g.CmdEndRendering(cmd);
      vk_image_layout_transition(cmd, g.texture_targets[g.current_image_idx], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      ///////////////////////////////////
      // vk_image_layout_transition(cmd, g_g.swapchain.images[g_g.current_image_idx], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    } break;
    case VK_RenderpassType_UI: {
    } break;
    case VK_RenderpassType_Screen: {
      g.CmdEndRendering(cmd);
      vk_image_layout_transition(cmd, g.swapchain.images[g.current_image_idx], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    } break;
  }
  return;
}

void vk_begin_draw_frame() {
  vk_imgui_begin_frame();
}

void vk_end_draw_frame() {
  ProfFunc;
  VK_State& g = st->vk;
  vk_begin_frame();
  if (0) {
  
  }
  // World
  {
    vk_begin_renderpass(VK_RenderpassType_World);
    vk_draw();
    vk_end_renderpass(VK_RenderpassType_World);
  }
  // {
  //   vk_begin_renderpass(Renderpass_UI);
  //   ui_begin_frame();
  //   ui_end_frame();
  //   vk_end_renderpass(Renderpass_UI);
  // }
  // Screen
  {
    VkCommandBuffer cmd = vk_get_current_cmd();
    VkViewport viewport = {
      .x = 0.0f,
      .y = (f32)g.height,
      .width = (f32)g.width,
      .height = -(f32)g.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
    };
    g.CmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor = {
      .offset = {.x = 0, .y = 0},
      .extent = {
        .width = (u32)g.width, 
        .height = (u32)g.height
      },
    };
    g.CmdSetScissor(cmd, 0, 1, &scissor);
    vk_begin_renderpass(VK_RenderpassType_Screen);
    vk_draw_screen();
    vk_imgui_end_frame();
    vk_end_renderpass(VK_RenderpassType_Screen);
  }
  vk_end_frame();
}

////////////////////////////////////////////////////////////////////////
// Entity

void vk_register_entity(u32 entity_id, GpuMeshId mesh_id, GpuMaterialId material_id, b32 is_static) {
  VK_State& g = st->vk;
  u32 offset = 0;
  if (is_static) {
    offset = MaxEntities;
    ++g.static_entities_count;
  }
  u32 entity_idx = id_idx(entity_id) + offset;
  Assert(g.entities[entity_idx].is_init == false);
  DebugDo(g.entities[entity_idx].is_init = true);
  u32 material_idx = id_idx(material_id.v);
  u32 pipeline_idx = g.materials[material_idx].pipeline_idx;
  u32 mesh_idx = id_idx(mesh_id.v);
  VK_Mesh mesh = g.meshes[mesh_idx];
  VK_BatchType type = (mesh.index_count == 0) | (is_static << 1);
  VK_MeshesBatches& shader_batch = g.batches[g.pipelines[pipeline_idx].batch_idx].batches[type];
  Result mesh_idx_in_array = map_get(shader_batch.mesh_to_batch, mesh_idx);
  if (mesh_idx_in_array.err) {
    VK_MeshBatch mesh_batch = vk_mesh_batch_make(g.gpa);
    mesh_batch.mesh_id = mesh_id;
    mesh_idx_in_array.v = array_push(shader_batch.mesh_batches, mesh_batch);
    map_set(shader_batch.mesh_to_batch, id_idx(mesh_id.v), mesh_idx_in_array.v);
  }
  VK_MeshBatch& mesh_batch = shader_batch.mesh_batches[mesh_idx_in_array.v];
  u32 entity_idx_in_array = array_push(mesh_batch.entities, entity_id);
  g.entities[entity_idx].entity_idx_in_mesh_batch = entity_idx_in_array;
  g.gpu_entities[entity_idx].material_idx = material_idx;
}

void vk_make_renderable(EntityId entity_id, GpuMeshId mesh_id, GpuMaterialId material_id) {
  vk_register_entity(entity_id.v, mesh_id, material_id, false);
}
void vk_make_renderable_static(StaticEntityId entity_id, GpuMeshId mesh_id, GpuMaterialId material_id) {
  vk_register_entity(entity_id.v, mesh_id, material_id, true);
}

void vk_unregister_entity(u32 entity_id, b32 is_static) {
  VK_State& g = st->vk;
  u32 offset = 0;
  if (is_static) {
    offset = MaxEntities;
    --g.static_entities_count;
  }
  u32 entity_idx = id_idx(entity_id) + offset;
  Assert(g.entities[entity_idx].is_init == true);
  DebugDo(g.entities[entity_idx].is_init = false);
  u32 pipeline_idx = 0;
  u32 mesh_idx = 0;
  if (is_static) {
    StaticEntity& entity = get_static_entity(StaticEntityId(entity_id));
    pipeline_idx = g.materials[id_idx(entity.material_id.v)].pipeline_idx;
    mesh_idx = id_idx(entity.mesh_id.v);
  } else {
    Entity& entity = get_entity(EntityId(entity_id));
    pipeline_idx = g.materials[id_idx(entity.material_id.v)].pipeline_idx;
    mesh_idx = id_idx(entity.mesh_id.v);
  }
  VK_Mesh mesh = g.meshes[mesh_idx];
  VK_BatchType type = (mesh.index_count == 0) | (is_static << 1);
  VK_MeshesBatches& shader_batch = g.batches[g.pipelines[pipeline_idx].batch_idx].batches[type];

  Result mesh_batch_idx = map_get(shader_batch.mesh_to_batch, mesh_idx);
  VK_MeshBatch& mesh_batch = shader_batch.mesh_batches[mesh_batch_idx.v];

  u32 idx = g.entities[entity_idx].entity_idx_in_mesh_batch;
  u32 last_idx = mesh_batch.entities.count-1;
  EntityId swapped = {mesh_batch.entities[last_idx]};
  u32 swapped_idx = id_idx(swapped.v);

  mesh_batch.entities[idx] = swapped.v;
  array_pop(mesh_batch.entities);
  g.entities[swapped_idx+offset].entity_idx_in_mesh_batch = idx;
}

void vk_remove_renderable(EntityId entity_id) {
  vk_unregister_entity(entity_id.v, false);
}

void vk_remove_static_renderable(StaticEntityId entity_id) {
  vk_unregister_entity(entity_id.v, true);
}

void vk_set_entity_color(EntityId entity_handle, v4 color) {
  st->vk.gpu_entities[id_idx(entity_handle)].color = color;
}

void vk_draw_line(v3 a, v3 b, v4 color) {
  Vertex vert[] = {
    {.pos = a, .color = v3_of_v4(color)},
    {.pos = b, .color = v3_of_v4(color)},
  };
  array_push(st->vk.draw_lines, {vert[0], vert[1]});
}

void vk_draw_line_consistent(v3 a, v3 b, v4 color) {
  Vertex vert[] = {
    {.pos = a, .color = v3_of_v4(color)},
    {.pos = b, .color = v3_of_v4(color)},
  };
  array_push(st->vk.draw_lines_consistent, {vert[0], vert[1]});
}

void vk_draw_cuboid(Rng3 rng, v4 color) {
  v3 p000 = {rng.min.x, rng.min.y, rng.min.z};
  v3 p001 = {rng.min.x, rng.min.y, rng.max.z};
  v3 p010 = {rng.min.x, rng.max.y, rng.min.z};
  v3 p011 = {rng.min.x, rng.max.y, rng.max.z};

  v3 p100 = {rng.max.x, rng.min.y, rng.min.z};
  v3 p101 = {rng.max.x, rng.min.y, rng.max.z};
  v3 p110 = {rng.max.x, rng.max.y, rng.min.z};
  v3 p111 = {rng.max.x, rng.max.y, rng.max.z};

  vk_draw_line(p000, p001, color);
  vk_draw_line(p000, p010, color);
  vk_draw_line(p000, p100, color);

  vk_draw_line(p111, p110, color);
  vk_draw_line(p111, p101, color);
  vk_draw_line(p111, p011, color);

  vk_draw_line(p001, p011, color);
  vk_draw_line(p001, p101, color);

  vk_draw_line(p010, p011, color);
  vk_draw_line(p010, p110, color);

  vk_draw_line(p100, p101, color);
  vk_draw_line(p100, p110, color);
}

void vk_draw_rect(Rng2 rect, v4 color) {
  v2 size = v2_of_v2u(os_get_window_size());
  rect.min = v2_map_to_11(rect.min, size);
  rect.min.y = -rect.min.y;
  rect.max = v2_map_to_11(rect.max, size);
  rect.max.y = -rect.max.y;
  DebugDrawRect square = {
    .vert = {
      {.pos = v2_to_v3(rect.min, 0), .color = v3_of_v4(color)},
      {.pos = v2_to_v3(v2(rect.min.x, rect.max.y), 0), .color = v3_of_v4(color)},
      {.pos = v2_to_v3(rect.max, 0), .color = v3_of_v4(color)},
      {.pos = v2_to_v3(rect.max, 0), .color = v3_of_v4(color)},
      {.pos = v2_to_v3(v2(rect.max.x, rect.min.y), 0), .color = v3_of_v4(color)},
      {.pos = v2_to_v3(rect.min, 0), .color = v3_of_v4(color)},
    }
  };
  array_push(st->vk.draw_rects, square);
}

#if DEAR_IMGUI
#include "imgui/imgui_impl_vulkan.h"

void imgui_impl_init() {
  ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
  platform_io.Platform_GetClipboardTextFn = [](ImGuiContext*)->const char* {
    Scratch scratch;
    String str = push_str_copy(scratch, os_clipboard_read());
    return (const char*)str.str;
  };
  platform_io.Platform_SetClipboardTextFn = [](ImGuiContext*, const char* text) { 
    os_clipboard_write(text);
  };
}

ImGuiKey imgui_keycode_translate(Key key) {
  switch (key) {
    case Key_Tab:         return ImGuiKey_Tab;
    case Key_Left:        return ImGuiKey_LeftArrow;
    case Key_Right:       return ImGuiKey_RightArrow;
    case Key_Up:          return ImGuiKey_UpArrow;
    case Key_Down:        return ImGuiKey_DownArrow;

    case Key_Pageup:      return ImGuiKey_PageUp;
    case Key_Pagedown:    return ImGuiKey_PageDown;
    case Key_Home:        return ImGuiKey_Home;
    case Key_End:         return ImGuiKey_End;
    case Key_Delete:      return ImGuiKey_Delete;

    case Key_Backspace:   return ImGuiKey_Backspace;
    case Key_Space:       return ImGuiKey_Space;
    case Key_Enter:       return ImGuiKey_Enter;
    case Key_Escape:      return ImGuiKey_Escape;

    case Key_Capslock:    return ImGuiKey_CapsLock;
    case Key_Numlock:     return ImGuiKey_NumLock;
    case Key_Printscreen: return ImGuiKey_PrintScreen;
    case Key_Pause:       return ImGuiKey_Pause;

    case Key_LShift:      return ImGuiKey_LeftShift;
    case Key_RShift:      return ImGuiKey_RightShift;
    case Key_LControl:    return ImGuiKey_LeftCtrl;
    case Key_RControl:    return ImGuiKey_RightCtrl;
    case Key_LAlt:        return ImGuiKey_LeftAlt;
    case Key_RAlt:        return ImGuiKey_RightAlt;
    case Key_Lsuper:      return ImGuiKey_LeftSuper;
    case Key_Rsuper:      return ImGuiKey_RightSuper;

    // Digits
    case Key_0: return ImGuiKey_0;
    case Key_1: return ImGuiKey_1;
    case Key_2: return ImGuiKey_2;
    case Key_3: return ImGuiKey_3;
    case Key_4: return ImGuiKey_4;
    case Key_5: return ImGuiKey_5;
    case Key_6: return ImGuiKey_6;
    case Key_7: return ImGuiKey_7;
    case Key_8: return ImGuiKey_8;
    case Key_9: return ImGuiKey_9;

    // Letters
    case Key_A: return ImGuiKey_A;
    case Key_B: return ImGuiKey_B;
    case Key_C: return ImGuiKey_C;
    case Key_D: return ImGuiKey_D;
    case Key_E: return ImGuiKey_E;
    case Key_F: return ImGuiKey_F;
    case Key_G: return ImGuiKey_G;
    case Key_H: return ImGuiKey_H;
    case Key_I: return ImGuiKey_I;
    case Key_J: return ImGuiKey_J;
    case Key_K: return ImGuiKey_K;
    case Key_L: return ImGuiKey_L;
    case Key_M: return ImGuiKey_M;
    case Key_N: return ImGuiKey_N;
    case Key_O: return ImGuiKey_O;
    case Key_P: return ImGuiKey_P;
    case Key_Q: return ImGuiKey_Q;
    case Key_R: return ImGuiKey_R;
    case Key_S: return ImGuiKey_S;
    case Key_T: return ImGuiKey_T;
    case Key_U: return ImGuiKey_U;
    case Key_V: return ImGuiKey_V;
    case Key_W: return ImGuiKey_W;
    case Key_X: return ImGuiKey_X;
    case Key_Y: return ImGuiKey_Y;
    case Key_Z: return ImGuiKey_Z;

    // Function keys
    case Key_F1:  return ImGuiKey_F1;
    case Key_F2:  return ImGuiKey_F2;
    case Key_F3:  return ImGuiKey_F3;
    case Key_F4:  return ImGuiKey_F4;
    case Key_F5:  return ImGuiKey_F5;
    case Key_F6:  return ImGuiKey_F6;
    case Key_F7:  return ImGuiKey_F7;
    case Key_F8:  return ImGuiKey_F8;
    case Key_F9:  return ImGuiKey_F9;
    case Key_F10: return ImGuiKey_F10;
    case Key_F11: return ImGuiKey_F11;
    case Key_F12: return ImGuiKey_F12;

    // Symbols
    case Key_Semicolon:   return ImGuiKey_Semicolon;
    case Key_Equal:       return ImGuiKey_Equal;
    case Key_Comma:       return ImGuiKey_Comma;
    case Key_Minus:       return ImGuiKey_Minus;
    case Key_Dot:         return ImGuiKey_Period;
    case Key_Slash:       return ImGuiKey_Slash;
    case Key_Grave:       return ImGuiKey_GraveAccent;
    case Key_LBracket:    return ImGuiKey_LeftBracket;
    case Key_Backslash:   return ImGuiKey_Backslash;
    case Key_RBracket:    return ImGuiKey_RightBracket;
    case Key_Apostrophe:  return ImGuiKey_Apostrophe;

    // Mouse
    case MouseKey_Left:   return (ImGuiKey)ImGuiMouseButton_Left;
    case MouseKey_Right:  return (ImGuiKey)ImGuiMouseButton_Right;
    case MouseKey_Middle: return (ImGuiKey)ImGuiMouseButton_Middle;

    default: return ImGuiKey_None;
  }
}

void imgui_impl_new_frame() {
  ImGuiIO& io = ImGui::GetIO();
  io.DeltaTime = get_dt();
  v2u win_size = os_get_window_size();
  io.DisplaySize = ImVec2(win_size.x, win_size.y);
  Slice<OS_InputEvent> events = os_get_events();
  Loop (i, events.count) {
    OS_InputEvent event = events[i];
    switch (event.type) {
      case OS_EventKind_Key: {
        ImGuiKey key = imgui_keycode_translate(event.key);
        io.AddKeyEvent(key, event.is_pressed);
          if (event.is_pressed) {
            // io.AddInputCharacter(os_key_to_str(event.key, event.modifier));
            io.AddInputCharacter(os_key_to_str(event.key, event.modifier));
          }
      } break;
      case OS_EventKind_MouseButton: {
        ImGuiKey key = imgui_keycode_translate(event.key);
        io.AddMouseButtonEvent(key, event.is_pressed);
      } break;
      case OS_EventKind_MouseMove: {
        io.AddMousePosEvent(event.x, event.y);
      } break;
      case OS_EventKind_Scroll: {
        io.AddMouseWheelEvent(0, event.scroll);
      } break;
      case OS_EventKind_Modifier:
        if (FlagHas(event.modifier, OS_Modifier_Shift)) {
          io.AddKeyEvent(ImGuiMod_Shift, true);
        } else {
          io.AddKeyEvent(ImGuiMod_Shift, false);
        }
        if (FlagHas(event.modifier, OS_Modifier_Alt)) {
          io.AddKeyEvent(ImGuiMod_Alt, true);
        } else {
          io.AddKeyEvent(ImGuiMod_Alt, false);
        }
        if (FlagHas(event.modifier, OS_Modifier_Ctrl)) {
          io.AddKeyEvent(ImGuiMod_Ctrl, true);
        } else {
          io.AddKeyEvent(ImGuiMod_Ctrl, false);
        } break;
      }
  }
}

PFN_vkVoidFunction imgui_load_fn(const char* function_name, void* user_data) {
  return (PFN_vkVoidFunction)os_lib_get_proc(st->vk.lib, function_name);
}

void vk_imgui_init() {
  ProfFunc;
  VK_State& g = st->vk;
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  ImGuiStyle& style = ImGui::GetStyle();
  style.FontScaleDpi = 1.3;
  imgui_impl_init();
  VkDescriptorPoolSize pool_sizes[] = {
    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
  };
  VkDescriptorPoolCreateInfo pool_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .poolSizeCount = ArrayCount(pool_sizes),
    .pPoolSizes = pool_sizes,
    .maxSets = 1000,
    .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
  };
  VkDescriptorPool descriptor_pool;
  VK_CHECK(g.CreateDescriptorPool(vkdevice, &pool_info, null, &descriptor_pool));
  VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
  ImGui_ImplVulkan_InitInfo init_info = {
    .Instance = g.instance,
    .PhysicalDevice = g.device.physical_device,
    .Device = g.device.logical_device,
    .QueueFamily = g.device.graphics_queue_family_idx,
    .Queue = g.device.graphics_queue,
    .DescriptorPool = descriptor_pool,
    .MinImageCount = g.frames_in_flight,
    .ImageCount = g.images_in_flight,
    .PipelineInfoMain = {
      .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
      .PipelineRenderingCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &format,
      },
    },
    .UseDynamicRendering = true,
  };
  ImGui_ImplVulkan_LoadFunctions(VK_API_VERSION_1_4, imgui_load_fn, null);
  ImGui_ImplVulkan_Init(&init_info);
}

void vk_imgui_begin_frame() {
  ImGui_ImplVulkan_NewFrame();
  imgui_impl_new_frame();
  ImGui::NewFrame();
}

void vk_imgui_end_frame() {
  ProfFunc;
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), st->vk.cmds[st->vk.current_frame_idx]);
  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();
}

#endif



