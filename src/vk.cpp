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

////////////////////////////////////////////////////////////////////////
// @Shader

VK_Shader vk_shader_module_create(String name) {
  Scratch scratch;
  VK_State& g = st->vk;
  VK_Shader res = {};
  String filepath = push_strf(scratch, "%s/%s.spv", st->shader_compiled_dir, name);
  Slice binary = os_file_path_read_all(scratch, filepath);
  VkShaderModuleCreateInfo module_info = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = binary.size,
    .pCode = (u32*)binary.data,
  };
  VK_CHECK(g.CreateShaderModule(vkdevice, &module_info, st->vk.allocator, &res.h));
  res.vert = true;
  res.frag = true;
  return res;
}

intern VkPipeline vk_shader_pipeline_create(VK_Shader shader, ShaderState state) {
  Scratch scratch;
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
  
  VkPipelineViewportStateCreateInfo viewport_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .scissorCount = 1,
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

  // Shader stages
  var shader_stages = darray_make<VkPipelineShaderStageCreateInfo>(scratch);
  if (shader.vert) {
    VkPipelineShaderStageCreateInfo stage_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = shader.h,
      .pName = "vs_main",
    };
    array_push(shader_stages, stage_info);
  }
  if (shader.frag) {
    VkPipelineShaderStageCreateInfo stage_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = shader.h,
      .pName = "fs_main",
    };
    array_push(shader_stages, stage_info);
  }
  if (shader.comp) {
  }
  
  // Pipeline
  VkGraphicsPipelineCreateInfo pipeline_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = &rendering_info,
    .stageCount = shader_stages.count,
    .pStages = shader_stages.data,
    .pVertexInputState = &vertex_input_state,
    .pInputAssemblyState = &input_assembly_state,
    .pViewportState = &viewport_state,
    .pRasterizationState = &rasterizer_state,
    .pMultisampleState = &multisampling_state_info,
    .pDepthStencilState = &depth_stencil_state_info,
    .pColorBlendState = &color_blend_state_info,
    .pDynamicState = &dynamic_state_info,
    .layout = g.pipeline_layout,
  };
  VkPipeline result;
  VK_CHECK(g.CreateGraphicsPipelines(vkdevice, VK_NULL_HANDLE, 1, &pipeline_info, st->vk.allocator, &result));
  return result;
}

intern VkPipeline vk_shader_pipeline_create(ShaderDesc shader) {
  VK_Shader module = vk_shader_module_create(shader.name);
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
    g.pipelines0[pipeline_idx].h = vk_shader_pipeline_create(entry.module, state);
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
  u32* pipeline_idx = map_set(g.shader_to_pipeline, key, array_push(g.pipelines0, {.h = pipeline}));
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
    os_directory_make_p(shader_dir);
  }
  if (!os_directory_path_exist(compiled_shader_dir)) {
    os_directory_make_p(compiled_shader_dir);
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
    VK_Shader modulo = vk_shader_module_create(name);
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
    // .format = VK_FORMAT_R8G8B8A8_SRGB,
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
    .mem_offset = offset,
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
  VK_CHECK(g.BindImageMemory(vkdevice, image.h, mem.h, image.mem_offset));
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

void vk_image_layout_transition(VkCommandBuffer cmd, VK_Image image, VkImageLayout old_layout, VkImageLayout new_layout, VkImageAspectFlags aspect_mask) {
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

void vk_image_upload_to_gpu(VkCommandBuffer cmd, VK_Image image) {
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

void vk_texture_generate_mipmaps(VK_Image image) {
  VK_State& g = st->vk;
  VkCommandBuffer cmd = g.cmds_upload[0];
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
    VkCommandBuffer cmd = g.cmds_upload[0];
    vk_cmd_begin(cmd);
    vk_image_layout_transition(cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vk_image_upload_to_gpu(cmd, image);
    vk_texture_generate_mipmaps(image);
    vk_cmd_end_submit(cmd);
  }
  VkDescriptorImageInfo descriptor_image_info = {
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
    g.pipelines0[pipeline_idx.v].batch_idx = g.batches.count;
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
    VkCommandBuffer cmd = g.cmds_upload[0];
    vk_cmd_begin(cmd);
    vk_image_layout_transition(cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vk_image_upload_to_gpu(cmd, image);
    vk_image_layout_transition(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vk_cmd_end_submit(cmd);
  }
  VkDescriptorImageInfo descriptor_image_info = {
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

intern void vk_swapchain_create() {
  VK_State& g = st->vk;
  if (g.swapchain.h_old) {
    g.swapchain.h_old = g.swapchain.h;
    ArrayCopy(g.swapchain.old_images, g.swapchain.images);
  }
  VK_CHECK(g.GetPhysicalDeviceSurfaceCapabilitiesKHR(g.device.physical_device, g.surface, &g.device.surface_capabilities));
  VkExtent2D swapchain_extent = {g.width, g.height};
  if (g.device.surface_capabilities.currentExtent.width != U32_MAX) {
    swapchain_extent = g.device.surface_capabilities.currentExtent;
  }
  VkExtent2D min = g.device.surface_capabilities.minImageExtent;
  VkExtent2D max = g.device.surface_capabilities.maxImageExtent;
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
    .preTransform = g.device.surface_capabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = g.swapchain.present_mode,
    .clipped = VK_TRUE,
    .oldSwapchain = g.swapchain.h_old ? g.swapchain.h : null,
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
  if (g.swapchain.h_old) {
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
  VK_Buffer& vert_buf = g.vert_buffer;
  u64 vert_size = mesh.vert_count*sizeof(Vertex);
  u64 vert_offset = offset_push(vert_buf.pos, vert_size, 4);
  Region vert_range = {.offset = vert_offset, .size = vert_size};
  vk_buffer_upload(vert_buf, vert_range, mesh.vertices);

  VK_Buffer& index_buf = g.index_buffer;
  u64 index_size = mesh.index_count*sizeof(u32);
  u64 index_offset = offset_push(index_buf.pos, index_size, 4);
  Region index_range = {.offset = index_offset, .size = index_size};
  if (mesh.indices) {
    vk_buffer_upload(index_buf, index_range, mesh.indices);
  }

  VK_Mesh vk_mesh = {
    .vert_count = mesh.vert_count,
    .base_vert = (u32)(vert_offset/sizeof(Vertex)),
    .index_count = mesh.index_count,
    .base_index = (u32)(index_offset/sizeof(u32)),
  };
  u32 id = g.meshes.count;
  array_push(g.meshes, vk_mesh);
  GpuMeshId handle = {id};
  return handle;
}

////////////////////////////////////////////////////////////////////////
// @Drawing

struct IndirectBatch {
  u32 base_entity;   // offset into gpu_entities_indices
  u32 base_drawcall; // offset into gpu_draw_call_infos
  u32 entity_count;
  u32 drawcall_count;
};

struct IndirectWriter {
  // GPU-side arrays (pointers into your existing buffers)
  EntityGPU* entities;
  u32* entity_indices;
  VK_DrawCallInfo* drawcalls;

  // Cursors — the only state that advances
  u32 entity_cursor;
  u32 drawcall_cursor;
};

void indirect_push_entity(IndirectWriter* w, u32 entity_idx, mat4 model) {
}

void indirect_push_drawcall(IndirectWriter* w, VK_Mesh mesh, u32 instance_count, u32 base_instance) {
  VK_DrawCallInfo info = {};
  if (mesh.index_count) {
    info.index_draw_command = (VkDrawIndexedIndirectCommand){
      .indexCount = mesh.index_count,
      .instanceCount = instance_count,
      .firstIndex = mesh.base_index,
      .vertexOffset = (i32)mesh.base_vert,
      .firstInstance = 0,
    };
  } else {
    info.draw_command = (VkDrawIndirectCommand){
      .vertexCount = mesh.vert_count,
      .instanceCount = instance_count,
      .firstVertex = mesh.base_vert,
      .firstInstance = 0,
    };
  }
  info.base_instance = base_instance;
  w->drawcalls[w->drawcall_cursor++] = info;
}

IndirectBatch indirect_begin_batch(IndirectWriter* w) {
  IndirectBatch res = {
    .base_entity = w->entity_cursor,
    .base_drawcall = w->drawcall_cursor,
  };
  return res;
}

IndirectBatch indirect_end_batch(IndirectWriter* w, IndirectBatch began) {
  began.entity_count = w->entity_cursor - began.base_entity;
  began.drawcall_count = w->drawcall_cursor - began.base_drawcall;
  return began;
}

void vk_draw() {
  ProfFunc;
  Scratch scratch;
  VK_State& g = st->vk;

  b32 rebuild_static_buffer = false;
  if (g.static_entities_count != g.static_entities_count_old) {
    g.static_entities_count_old = g.static_entities_count;
    rebuild_static_buffer = true;
  }

  VkCommandBuffer cmd = vk_get_cur_cmd();
  g.CmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipeline_layout, 0, 1, &g.descriptor_sets, 0, null);
  VkDeviceSize size = 0;
  g.CmdBindVertexBuffers(cmd, 0, 1, &g.vert_buffer.h, &size);
  g.CmdBindIndexBuffer(cmd, g.index_buffer.h, 0, VK_INDEX_TYPE_UINT32);

  GlobalStateGPU& shader_st = *g.gpu_global_shader_st;
  shader_st.projection_view = st->projection * st->view;
  shader_st.projection = st->projection;
  shader_st.view = st->view;

  IndirectWriter dyn_writer = {
    .entities = g.gpu_entities,
    .entity_indices = g.gpu_entities_indices,
    .drawcalls = g.gpu_draw_call_infos,
    .entity_cursor = 0,
    .drawcall_cursor = 0,
  };
  IndirectWriter static_writer = {
    .entities = g.gpu_entities + MaxEntities,
    .entity_indices = g.gpu_entities_indices + MaxEntities,
    .drawcalls = g.gpu_draw_call_infos + MaxDrawCalls / 2,
    .entity_cursor = 0,
    .drawcall_cursor = 0,
  };

  Loop (i, g.entity_pipelines.count) {
    VK_Pipeline0 pipeline = g.pipelines0[g.entity_pipelines[i]];
    vk_bind_pipeline(pipeline.h);
    VK_PipelineBatch& batch = g.batches[pipeline.batch_idx];

    var fill = [&](IndirectWriter* w, VK_MeshesBatches shader_batch) -> VK_IndirectDrawCall {
      IndirectBatch range = indirect_begin_batch(w);
      Loop (i, shader_batch.mesh_batches.count) {
        VK_MeshBatch& mb = shader_batch.mesh_batches[i];
        if (mb.entities.count == 0) continue;
        u32 base_instance = w->entity_cursor;
        Loop (j, mb.entities.count) {
          u32 e_id = mb.entities[j];
          w->entities[id_idx(e_id)].model = mat4_transform(get_entity_transform({e_id}));
          w->entity_indices[w->entity_cursor++] = id_idx(e_id);
        }
        indirect_push_drawcall(w, g.meshes[id_idx(mb.mesh_id.v)], mb.entities.count, base_instance);
      }
      range = indirect_end_batch(w, range);
      VK_IndirectDrawCall res = {
        .drawcall_count = range.drawcall_count,
        .drawcall_base = range.base_drawcall,
      };
      return res;
    };
    var fill_static = [&](IndirectWriter* w, VK_MeshesBatches shader_batch) -> VK_IndirectDrawCall {
      IndirectBatch range = indirect_begin_batch(w);
      Loop (i, shader_batch.mesh_batches.count) {
        VK_MeshBatch& mb = shader_batch.mesh_batches[i];
        if (mb.entities.count == 0) continue;
        u32 base_instance = w->entity_cursor+MaxEntities;
        Loop (j, mb.entities.count) {
          u32 e_id = mb.entities[j];
          w->entities[id_idx(e_id)].model = mat4_transform(get_static_entity_transform({e_id}));
          w->entity_indices[w->entity_cursor++] = id_idx(e_id)+MaxEntities;
        }
        indirect_push_drawcall(w, g.meshes[id_idx(mb.mesh_id.v)], mb.entities.count, base_instance);
      }
      range = indirect_end_batch(w, range);
      VK_IndirectDrawCall res = {
        .drawcall_count = range.drawcall_count,
        .drawcall_base = range.base_drawcall + MaxDrawCalls/2,
      };
      return res;
    };

    var make_draw = [&](VK_IndirectDrawCall draw, b32 indexed) {
      if (draw.drawcall_count) {
        vk_push_constants({.drawcall_base = draw.drawcall_base});
        if (indexed) {
          gfx_draw_indexed_indirect(draw.drawcall_base, draw.drawcall_count);
        } else {
          gfx_draw_indirect(draw.drawcall_base, draw.drawcall_count);
        }
      }
    };

    VK_IndirectDrawCall indexed_drawcall = fill(&dyn_writer, batch.batches[VK_BatchType_Indexed]);
    VK_IndirectDrawCall drawcall = fill(&dyn_writer, batch.batches[VK_BatchType_Unindexed]);
    make_draw(indexed_drawcall, true);
    make_draw(drawcall, false);

    ///////////////////////////////////
    // Static entities
    if (rebuild_static_buffer) {
      VK_IndirectDrawCall drawcall_indexed = fill_static(&static_writer, batch.batches[VK_BatchType_StaticIndexed]);
      VK_IndirectDrawCall drawcall = fill_static(&static_writer, batch.batches[VK_BatchType_StaticUnindexed]);
      g.static_draw_calls[i*2] = drawcall_indexed;
      g.static_draw_calls[i*2 + 1] = drawcall;
    }
    make_draw(g.static_draw_calls[i*2], true);
    make_draw(g.static_draw_calls[i*2+1], false);
  }

  // Debug drawing
  if (g.draw_lines.count > 0) {
    gfx_pipeline_bind(g.debug_line_pip);
    gfx_draw(g.draw_lines_offset/sizeof(Vertex), g.draw_lines.count*2);
    array_clear(g.draw_lines);
  }
  if (g.draw_lines_consistent.count > 0) {
    gfx_pipeline_bind(g.debug_line_pip);
    gfx_draw(g.draw_lines_consistent_offset/sizeof(Vertex), g.draw_lines_consistent.count*2);
  }

  // Hello world
  gfx_pipeline_bind(g.triangle_pip);
  gfx_draw(0, 3);

  // Cube map
  gfx_pipeline_bind(g.cubemap_pip);
  GpuMeshId h = mesh_get(Mesh_Cube);
  VK_Mesh mesh = g.meshes[h.v];
  if (mesh.index_count) {
    gfx_draw_indexed(mesh.base_index, mesh.index_count, mesh.base_vert);
  } else {
    gfx_draw(mesh.base_vert, mesh.vert_count);
  }

  // Rect drawing
  if (g.draw_rects.count > 0) {
    gfx_pipeline_bind(g.ui_pip);
    gfx_draw(g.draw_rects_offset/sizeof(Vertex), g.draw_rects.count*6);
    array_clear(g.draw_rects);
  }
}

void vk_draw_screen() {
  VK_State& g = st->vk;
  VkCommandBuffer cmd = vk_get_cur_cmd();
  VK_PushConstant push = {g.current_image_idx};
  g.CmdPushConstants(cmd, g.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(VK_PushConstant), &push);
  // g.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipelines0[g.screen_pipeline0].h);
  gfx_pipeline_bind(g.screen_pip);
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
  const char* VK1_SURFACE_NAME = "VK_KHR_xcb_surface";
  array_push(required_extensions, VK_KHR_SURFACE_EXTENSION_NAME, VK1_SURFACE_NAME);

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
        VK_CHECK(g.GetPhysicalDeviceSurfaceCapabilitiesKHR(device.physical_device, g.surface, &device.surface_capabilities));
        VK_CHECK(g.GetPhysicalDeviceSurfaceFormatsKHR(device.physical_device, g.surface, &device.surface_format_count, null));
        VK_CHECK(g.GetPhysicalDeviceSurfacePresentModesKHR(device.physical_device, g.surface, &device.present_mode_count, null));
        device.surface_formats = push_array(g.arena, VkSurfaceFormatKHR, device.surface_format_count);
        device.present_modes = push_array(g.arena, VkPresentModeKHR, device.present_mode_count);
        VK_CHECK(g.GetPhysicalDeviceSurfaceFormatsKHR(device.physical_device, g.surface, &device.surface_format_count, device.surface_formats));
        VK_CHECK(g.GetPhysicalDeviceSurfacePresentModesKHR(device.physical_device, g.surface, &device.present_mode_count, device.present_modes));
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
  g.pipelines0 = darray_make<VK_Pipeline0>(g.gpa);
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
    g.gpu_mem = vk_mem_alloc(VK_MemoryType_Gpu, MB(200));
    g.cpu_mem = vk_mem_alloc(VK_MemoryType_Cpu, MB(300));
    g.vert_buffer = vk_buffer_alloc(MB(1), VK_BufferUsage_Vert | VK_BufferUsage_Dst, VK_MemoryType_Gpu);
    g.index_buffer = vk_buffer_alloc(MB(1), VK_BufferUsage_Index | VK_BufferUsage_Dst, VK_MemoryType_Gpu);
    g.stage_buffer = vk_buffer_alloc(MB(10), VK_BufferUsage_Src, VK_MemoryType_Cpu);
    g.storage_buffer = vk_buffer_alloc(MB(200), VK_BufferUsage_Storage, VK_MemoryType_Cpu);
    g.indirect_draw_buffer = vk_buffer_alloc(MB(10), VK_BufferUsage_Indirect, VK_MemoryType_Cpu);
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
    g.images_in_flight = g.device.surface_capabilities.minImageCount;
    g.frames_in_flight = g.images_in_flight - 1;
    // Choose a image format
    g.swapchain.format = g.device.surface_formats[0];
    Loop (i, g.device.surface_format_count) {
      VkSurfaceFormatKHR format = g.device.surface_formats[i];
      if (format.format == VK_FORMAT_B8G8R8A8_UNORM && // darker
      // if (format.format == VK_FORMAT_R8G8B8A8_UNORM &&
      // if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
          format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
      {
        g.swapchain.format = format;
        break;
      }
    }
    // Choose present mode
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    Loop (i, g.device.present_mode_count) {
      VkPresentModeKHR mode = g.device.present_modes[i];
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
    vk_swapchain_create();
    Info("Swapchain created");
    g.cmds_render = push_array(g.arena, VkCommandBuffer, g.frames_in_flight);
    g.cmds_frames_upload = push_array(g.arena, VkCommandBuffer, g.frames_in_flight);
    g.cmds_upload = push_array(g.arena, VkCommandBuffer, g.frames_in_flight);
    Loop (i, g.frames_in_flight) {
      g.cmds_render[i] = vk_cmd_alloc(g.device.cmd_pool);
      g.cmds_frames_upload[i] = vk_cmd_alloc(g.device.cmd_pool);
      g.cmds_upload[i] = vk_cmd_alloc(g.device.cmd_pool);
    }
    Info("Command buffers created");
  }

  // VkSemaphoreTypeCreateInfo timelineInfo = {
  //   .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
  //   .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
  //   .initialValue = 0
  // };
  // VkSemaphoreCreateInfo createInfo = {
  //   .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  //   .pNext = &timelineInfo
  // };
  // VkSemaphore semaphore;
  // g.CreateSemaphore(vkdevice, &createInfo, g.allocator, &semaphore);

  // Sync
  {
    // NOTE: for some reasons validation layer complains about render_complete_sempahores when their number is frames_in_flight but doesn't when is images_in_flight
    g.render_complete_semaphores = push_array(g.arena, VkSemaphore, g.images_in_flight);
    Loop (i, g.images_in_flight) {
      VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
      g.CreateSemaphore(vkdevice, &semaphore_create_info, g.allocator, &g.render_complete_semaphores[i]);
    }
    g.image_available_semaphores = push_array(g.arena, VkSemaphore, g.images_in_flight);
    // g.frames_upload_semaphores = push_array(g.arena, VK_Semaphore, g.images_in_flight);
    g.in_flight_fences = push_array(g.arena, VkFence, g.frames_in_flight);
    g.fences_frames_upload = push_array(g.arena, VkFence, g.frames_in_flight);
    Loop (i, g.frames_in_flight) {
      VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
      g.CreateSemaphore(vkdevice, &semaphore_create_info, g.allocator, &g.image_available_semaphores[i]);
      VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
      };
      VK_CHECK(g.CreateFence(vkdevice, &fence_create_info, g.allocator, &g.in_flight_fences[i]));
      VK_CHECK(g.CreateFence(vkdevice, &fence_create_info, g.allocator, &g.fences_frames_upload[i]));
      // g.frames_upload_semaphores[i] = vk_semaphore_make(1);
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

  {
    g.triangle_pip = gfx_pipeline_make({
      .shader = gfx_shader_make({.shader = os_file_path_read_all_str(scratch, "../assets/shaders/compiled/triangle.spv")}),
      .depth = {
        .compare = Gfx_CompareOp_Less,
        .write_enabled = true,
      },
    });
  }

  ///////////////////////////////////
  // Load basic shaders
  g.debug_line_pip = gfx_pipeline_make({
    .shader = gfx_shader_make({.shader = os_file_path_read_all_str(scratch, "../assets/shaders/compiled/line.spv")}),
    .primitive_type = Gfx_PrimitiveType_Line,
    .depth = {
      .compare = Gfx_CompareOp_Less,
      .write_enabled = true,
    },
    .color = {
      .blend = {
        .enabled = true,
        .src_factor_rgb = Gfx_BlendFactor_SrcAlpha,
        .dst_factor_rgb = Gfx_BlendFactor_OneMinusSrcAlpha,
        .src_factor_alpha = Gfx_BlendFactor_SrcAlpha,
        .dst_factor_alpha = Gfx_BlendFactor_OneMinusSrcAlpha,
      },
    },
  });
  g.screen_pip = gfx_pipeline_make({
    .shader = gfx_shader_make({.shader = os_file_path_read_all_str(scratch, "../assets/shaders/compiled/screen.spv")}),
    .sample_count = 1,
  });
  g.cubemap_pip = gfx_pipeline_make({
    .shader = gfx_shader_make({.shader = os_file_path_read_all_str(scratch, "../assets/shaders/compiled/cubemap.spv")}),
    .depth = {
      .compare = Gfx_CompareOp_LessEqual,
    },
  });
  g.ui_pip = gfx_pipeline_make({
    .shader = gfx_shader_make({.shader = os_file_path_read_all_str(scratch, "../assets/shaders/compiled/ui.spv")}),
  });
  
  Info("Vulkan renderer initialized");
}

void vk_shutdown() {
  VK_State& g = st->vk;
  VK_CHECK(g.DeviceWaitIdle(vkdevice));
  
  Loop (i, g.frames_in_flight) {
    g.DestroySemaphore(vkdevice, g.image_available_semaphores[i], g.allocator);
    g.DestroySemaphore(vkdevice, g.render_complete_semaphores[i], g.allocator);
    g.DestroySemaphore(vkdevice, g.compute_complete_semaphores[i], g.allocator);
    g.DestroyFence(vkdevice, g.in_flight_fences[i], g.allocator);
    vk_cmd_free(g.device.cmd_pool, g.cmds_render[i]);
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

  // linear memory - ring buffer/double buffering
  // gaps in memory: ring buffer/double buffering
  // async uploading: accumulate all uploads and make one uppload at the time, return dummy or not ready yet handle
  // each frame/per n frames: be valid at the time

  {
    ProfBlock("block");
    // if (g.draw_lines.count > 0) {
    //   ProfBlock("1");
    //   u32 size = g.draw_lines.count * sizeof(DebugDrawLine);
    //   void* data = g.draw_lines.data;
      // vk_buffer_upload(g.vert_buffer, {g.draw_lines_offset, size}, data);
    // }
    // if (g.draw_lines_consistent.count > 0) {
    //   ProfBlock("2");
    //   u32 size = g.draw_lines_consistent.count * sizeof(DebugDrawLine);
    //   void* data = g.draw_lines_consistent.data;
    //   vk_buffer_upload(g.vert_buffer, {g.draw_lines_consistent_offset, size}, data);
    // }
    // if (g.draw_rects.count > 0) {
    //   ProfBlock("3");
    //   u32 size = g.draw_rects.count * sizeof(DebugDrawRect);
    //   void* data = g.draw_rects.data;
    //   vk_buffer_upload(g.vert_buffer, {g.draw_rects_offset, size}, data);
    // }

    VkFence& fence = g.fences_frames_upload[g.current_frame_idx];
    g.WaitForFences(vkdevice, 1, &fence, true, U64_MAX);
    g.ResetFences(vkdevice, 1, &fence);
    VkCommandBuffer cmd = g.cmds_frames_upload[g.current_frame_idx];
    vk_cmd_begin(cmd);
    
    u32 stage_offset = 0;
    if (g.draw_lines.count > 0) {
      ProfBlock("1");
      u32 size = g.draw_lines.count * sizeof(DebugDrawLine);
      void* data = g.draw_lines.data;
      MemCopy(g.stage_buffer.base+stage_offset, data, size);
      VkBufferCopy copy_region = {
        .srcOffset = stage_offset,
        .dstOffset = g.draw_lines_offset,
        .size = size,
      };
      g.CmdCopyBuffer(cmd, g.stage_buffer.h, g.vert_buffer.h, 1, &copy_region);
      stage_offset += size;
    }
    if (g.draw_lines_consistent.count > 0) {
      ProfBlock("2");
      u32 size = g.draw_lines_consistent.count * sizeof(DebugDrawLine);
      void* data = g.draw_lines_consistent.data;
      MemCopy(g.stage_buffer.base+stage_offset, data, size);
      VkBufferCopy copy_region = {
        .srcOffset = stage_offset,
        .dstOffset = g.draw_lines_consistent_offset,
        .size = size,
      };
      g.CmdCopyBuffer(cmd, g.stage_buffer.h, g.vert_buffer.h, 1, &copy_region);
      stage_offset += size;
    }
    if (g.draw_rects.count > 0) {
      ProfBlock("3");
      u32 size = g.draw_rects.count * sizeof(DebugDrawRect);
      void* data = g.draw_rects.data;
      MemCopy(g.stage_buffer.base+stage_offset, data, size);
      VkBufferCopy copy_region = {
        .srcOffset = stage_offset,
        .dstOffset = g.draw_rects_offset,
        .size = size,
      };
      g.CmdCopyBuffer(cmd, g.stage_buffer.h, g.vert_buffer.h, 1, &copy_region);
      stage_offset += size;
    }
    vk_cmd_end(cmd);

    VkSubmitInfo submit = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &cmd,
    };
    g.QueueSubmit(g.device.transfer_queue, 1, &submit, fence);
  }
  {
    ProfBlock("rendering waiting");
    VK_CHECK(g.WaitForFences(vkdevice, 1, &g.in_flight_fences[g.current_frame_idx], true, U64_MAX));
    VK_CHECK(g.ResetFences(vkdevice, 1, &g.in_flight_fences[g.current_frame_idx]));
  }

  VkCommandBuffer cmd = vk_get_cur_cmd();
  vk_cmd_begin(cmd);

  // Resize?
  v2u win_size = os_get_window_size();
  if (g.width != win_size.x || g.height != win_size.y || g.old_scale != g.scale) {
    g.width = win_size.x;
    g.height = win_size.y;
    g.old_scale = g.scale;
    VK_CHECK(g.DeviceWaitIdle(vkdevice));
    vk_swapchain_create();
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

  gfx_apply_viewport(0, 0, g.width*g.scale, g.height*g.scale);
  gfx_apply_scissor(0, 0, g.width, g.height);
}

void vk_end_frame() {
  ProfFunc;
  VK_State& g = st->vk;
  VkCommandBuffer cmd = vk_get_cur_cmd();
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
  VK_CHECK(g.QueueSubmit(g.device.graphics_queue, 1, &submit_info, g.in_flight_fences[g.current_frame_idx]));

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
  g.current_frame_idx_one_more = (g.current_frame_idx_one_more + 1) % (g.frames_in_flight+1);
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
  VkCommandBuffer cmd = vk_get_cur_cmd();
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
  VkCommandBuffer cmd = vk_get_cur_cmd();
  switch (renderpass) {
    case VK_RenderpassType_World: {
      g.CmdEndRendering(cmd);
      vk_image_layout_transition(cmd, g.texture_targets[g.current_image_idx], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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
    gfx_apply_viewport(0, 0, g.width, g.height);
    gfx_apply_scissor(0, 0, g.width, g.height);
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
  VK_MeshesBatches& shader_batch = g.batches[g.pipelines0[pipeline_idx].batch_idx].batches[type];
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
  VK_MeshesBatches& shader_batch = g.batches[g.pipelines0[pipeline_idx].batch_idx].batches[type];

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
  rect.min = v2_remap_01_to_11(rect.min, size);
  rect.min.y = -rect.min.y;
  rect.max = v2_remap_01_to_11(rect.max, size);
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

////////////////////////////////////////////////////////////////////////
// @Dear imgui

#if DEAR_IMGUI
#include "imgui/imgui_impl_vulkan.h"

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

    default: return ImGuiKey_None;
  }
}

u32 imgui_mouse_button_translate(MouseButton button) {
  switch (button) {
    default: return 0;
    case MouseButton_Left:   return ImGuiMouseButton_Left;
    case MouseButton_Right:  return ImGuiMouseButton_Right;
    case MouseButton_Middle: return ImGuiMouseButton_Middle;
  }
}

OS_InputEvent last_key_event;
Timer _timer_type_repeat_speed = {.interval = 1.0f/20};
Timer _timer_type_repeat_delay = {.interval = 1.0f/6};

void imgui_impl_new_frame() {
  ImGuiIO& io = ImGui::GetIO();
  io.DeltaTime = get_dt();
  v2u win_size = os_get_window_size();
  io.DisplaySize = ImVec2(win_size.x, win_size.y);
  Slice<OS_InputEvent> events = os_get_input_events();
  if (last_key_event.is_pressed) {
    _timer_type_repeat_delay.passed += get_dt();
    if (_timer_type_repeat_delay.passed >= _timer_type_repeat_delay.interval) {
      if (timer_passed(_timer_type_repeat_speed)) {
        io.AddInputCharacter(os_key_to_character(last_key_event.key, last_key_event.modifier));
      }
    }
  }
  Loop (i, events.count) {
    OS_InputEvent event = events[i];
    switch (event.type) {
      case OS_EventType_Key: {
        if (event.key < Key_COUNT && event.key != Key_Super) {
          last_key_event = event;
          _timer_type_repeat_delay.passed = 0;
        }
        ImGuiKey key = imgui_keycode_translate(event.key);
        io.AddKeyEvent(key, event.is_pressed);
        // Info("%u %u", event.key, event.is_pressed);
        if (event.is_pressed) {
          io.AddInputCharacter(os_key_to_character(event.key, event.modifier));
        }
      } break;
      case OS_EventType_MouseButton: {
        io.AddMouseButtonEvent(imgui_mouse_button_translate(event.mouse_button), event.is_pressed);
      } break;
      case OS_EventType_MouseMove: {
        io.AddMousePosEvent(event.x, event.y);
      } break;
      case OS_EventType_Scroll: {
        io.AddMouseWheelEvent(0, event.scroll);
      } break;
      case OS_EventType_Modifier:
        if (FlagHas(event.modifier, OS_Modifier_Shift)) {
          io.AddKeyEvent(ImGuiMod_Shift, true);
          // Info("mod shift true");
        } else {
          io.AddKeyEvent(ImGuiMod_Shift, false);
          // Info("mod shift false");
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

void vk_imgui_init() {
  ProfFunc;
  VK_State& g = st->vk;
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  ImGuiStyle& style = ImGui::GetStyle();
  style.FontScaleDpi = 1.3;

  ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
  platform_io.Platform_GetClipboardTextFn = imgui_platform_get_clipboard_text;
  platform_io.Platform_SetClipboardTextFn = imgui_platform_set_clipboard_text;

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
  ImGui_ImplVulkan_LoadFunctions(VK_API_VERSION_1_4, [](const char* function_name, void* user_data) {
    return (PFN_vkVoidFunction)os_lib_get_proc(st->vk.lib, function_name);
  }, null);
  ImGui_ImplVulkan_Init(&init_info);
}

void vk_imgui_begin_frame() {
  imgui_impl_new_frame();
  ImGui_ImplVulkan_NewFrame();
  ImGui::NewFrame();
}

void vk_imgui_end_frame() {
  ProfFunc;
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), st->vk.cmds_render[st->vk.current_frame_idx]);
  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();
}

#endif



