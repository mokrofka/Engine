#include "vk_types.h"
#include "vk_buffer.h"

#include "sys/shader_sys.h"
#include "sys/res_sys.h"
#include "asset_watch.h"

internal VK_Pipeline vk_pipeline_create(
    u32 vert_stride, u32 attribute_count,
    VkVertexInputAttributeDescription* attributes,
    u32 stage_count, VkPipelineShaderStageCreateInfo* stages, VkPrimitiveTopology topology, b32 is_depth, b32 is_alpha);

internal VkPipelineShaderStageCreateInfo vk_shader_module_create(String name, String type_str, VkShaderStageFlagBits shader_stage_flag) {
  Scratch scratch;
  String filepath = push_strf(scratch, "shaders/compiled/%s.%s.spv", name, type_str);
  
  Buffer binary = res_binary_load(scratch, filepath);
  if (!binary.data) {
    Error("Unable to read shader module: %s", filepath);
  }
  
  VkShaderModuleCreateInfo shader_module_create_info = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = binary.size,
    .pCode = (u32*)binary.data,
  };

  VkShaderModule handle;
  VK_CHECK(vkCreateShaderModule(vkdevice, &shader_module_create_info, vk.allocator, &handle));
  
  VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = shader_stage_flag,
    .module = handle,
    .pName = "main",
  };
  return pipeline_shader_stage_create_info;
}

void vk_r_shader_create(Shader* s) {
  VK_Shader* shader = &vk.shaders[vk.shader_count];

  #define ShaderStageCount 2
  String stage_type_strs[] = { "vert"_, "frag"_};
  VkShaderStageFlagBits stage_types[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
  Loop (i, ShaderStageCount) {
    shader->stages[i] = vk_shader_module_create(s->name, stage_type_strs[i], stage_types[i]);
  }

  VkVertexInputAttributeDescription attribute_desriptions[10];
  
  VkFormat formats[] = {
    VK_FORMAT_R32_SFLOAT,
    VK_FORMAT_R32G32_SFLOAT,
    VK_FORMAT_R32G32B32_SFLOAT,
    VK_FORMAT_R32G32B32A32_SFLOAT
  };
  
  u32 vert_stride = 0;
  u32 attribute_count = 0;
  for (u32 i = 0; s->attribut[i]; ++i) {
    shader->attribute_desriptions[i] = {
      .location = i,
      .binding = 0,
      .format = formats[s->attribut[i] - 1],
      .offset = vert_stride,
    };
    vert_stride += s->attribut[i] * sizeof(f32);
    ++attribute_count;
  }
  
  VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  if (s->primitive == ShaderTopology_Line) {
    topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
  }
  
  shader->name = s->name;
  shader->attribute_count = attribute_count;
  shader->vert_stride = vert_stride;
  shader->topology = topology;
  shader->is_transparent = s->is_transparent;
  ++vk.shader_count;

  shader->pipeline = vk_pipeline_create(shader->vert_stride, shader->attribute_count, shader->attribute_desriptions,
                                        2, shader->stages, shader->topology, true, s->is_transparent);
}

VK_Pipeline vk_pipeline_create(
    u32 vert_stride, u32 attribute_count,
    VkVertexInputAttributeDescription* attributes,
    u32 stage_count, VkPipelineShaderStageCreateInfo* stages, VkPrimitiveTopology topology, b32 is_depth, b32 is_alpha) {
  
  // Dynamic rendering
  VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
  VkPipelineRenderingCreateInfo renderingCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .colorAttachmentCount = 1,
    .pColorAttachmentFormats = &color_format,
    .depthAttachmentFormat = vk.device.depth_format,
  };
  
  // Vertex input
  VkVertexInputBindingDescription binding_description = {
    .binding = 0,
    .stride = vert_stride,
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
  };

  // Attributes
  VkPipelineVertexInputStateCreateInfo vertex_input_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &binding_description,
    .vertexAttributeDescriptionCount = attribute_count,
    .pVertexAttributeDescriptions = attributes,
  };

  // Input assembly
  VkPipelineInputAssemblyStateCreateInfo input_assembly = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = topology,
    .primitiveRestartEnable = VK_FALSE,
  };
  
  // Viewport
  VkViewport viewport = {
    .x = 0.0f,
    .y = (f32)vk.frame.height,
    .width = (f32)vk.frame.width,
    .height = (f32)vk.frame.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };

  VkRect2D scissor = {
    .offset = { .x = 0, .y = 0 },
    .extent = {
      .width = vk.frame.width,
      .height = vk.frame.height,
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
  VkPipelineRasterizationStateCreateInfo rasterizer_create_info = {
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
  VkPipelineMultisampleStateCreateInfo multisampling_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    .sampleShadingEnable = VK_FALSE,
    .minSampleShading = 1.0f,
    .pSampleMask = 0,
    .alphaToCoverageEnable = VK_FALSE,
    .alphaToOneEnable = VK_FALSE,
  };

  // Depth and stencil testing
  VkPipelineDepthStencilStateCreateInfo depth_stencil = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  if (is_depth) {
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;
  }
  
  // Blending
  VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
  if (is_alpha) {
    color_blend_attachment_state.blendEnable = VK_TRUE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;

    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
  } else {
    color_blend_attachment_state.blendEnable = VK_FALSE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;

    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
  }
  color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_COPY,
    .attachmentCount = 1,
    .pAttachments = &color_blend_attachment_state,
  };
   
  // Dynamic state
  VkDynamicState dynamic_state[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
  };

  VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = ArrayCount(dynamic_state),
    .pDynamicStates = dynamic_state,
  };
  
  // Push constants
  VkPushConstantRange push_constant = {
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    .offset = sizeof(mat4) * 0,
    .size = sizeof(mat4) * 2,
  };

  VkDescriptorSetLayout set_layouts[] = {
    vk.descriptor_set_layout,
  };

  // Pipeline layout
  VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = ArrayCount(set_layouts),
    .pSetLayouts = set_layouts,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges = &push_constant,
  };
  
  VK_Pipeline result = {};
  VK_CHECK(vkCreatePipelineLayout(vkdevice, &pipeline_layout_create_info, vk.allocator, &result.pipeline_layout));

  // Pipeline create
  VkGraphicsPipelineCreateInfo pipeline_create_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = &renderingCreateInfo,
    .stageCount = stage_count,
    .pStages = stages,
    .pVertexInputState = &vertex_input_info,
    .pInputAssemblyState = &input_assembly,
    .pTessellationState = null,
    .pViewportState = &viewport_state,
    .pRasterizationState = &rasterizer_create_info,
    .pMultisampleState = &multisampling_create_info,
    .pDepthStencilState = &depth_stencil,
    .pColorBlendState = &color_blend_state_create_info,
    .pDynamicState = &dynamic_state_create_info,
    .layout = result.pipeline_layout,
    .basePipelineHandle = VK_NULL_HANDLE,
    .basePipelineIndex = -1,
  };

  VK_CHECK(vkCreateGraphicsPipelines(vkdevice, VK_NULL_HANDLE, 1, &pipeline_create_info, vk.allocator, &result.handle));

  return result;
}


f32 ease_in_exp(f32 x) {
	return x <= 0.0 ? 0.0 : Pow(2, 10.0 * x - 10.0);
}

void compute_shader() {
  Scratch scratch;
  VK_ComputeShader* shader = &vk.compute_shader;
  shader->stage.pipeline_shader_stage_create_info = vk_shader_module_create("compute"_, "comp"_, VK_SHADER_STAGE_COMPUTE_BIT);
  
  Particle* particles = push_array(scratch, Particle, ParticleCount);
  
  v2i frame = os_get_framebuffer_size();
  f32 min = 0, max = 1;
  
  Loop (i, ParticleCount) {
    f32 r = 0.25f * rand_in_range_f32(min, max);
    f32 theta = rand_in_range_f32(min, max) * Two_PI;          // angle around Y axis
    f32 phi = Acos(2.0f * rand_in_range_f32(min, max) - 1.0f); // angle from Z axis (0..pi)

    f32 x = r * Sin(phi) * Cos(theta);
    f32 y = r * Sin(phi) * Sin(theta);
    f32 z = r * Cos(phi);

    particles[i].pos = v3(x, y, z);
    particles[i].velocity = v3_normalize(v3(x, y, z)) * 0.25f;
    particles[i].color = v4(rand_in_range_f32(min, max), rand_in_range_f32(min, max), rand_in_range_f32(min, max), 1.0f);
  }
  
  u32 num_elipses = 14;
  f32 tilt_step = rad_to_deg(0.1);
  f32 radius_step = 1.1;
  // Loop (i, num_elipses) {
  //   f32 angle_offset = i * tilt_step;
  //   f32 a = (i + 1) * radius_step / 5; // semi-major axis
  //   // f32 b = a * 0.9f * i;              // semi-minor axis (flattening)
  //   f32 b = a * 0.91f;              // semi-minor axis (flattening)

  //   u32 stars_per_ellipse = ParticleCount / num_elipses;
  //   Loop (j, stars_per_ellipse) {
  //     f32 t = 2 * PI * j / stars_per_ellipse;
  //     f32 rand = rand_in_range_f32(0,0.2);
  //     f32 x = a * Cos(t) + rand;
  //     f32 y = b * Sin(t) + rand;

  //     // Apply rotation
  //     f32 xr = Cos(angle_offset) * x - Sin(angle_offset) * y;
  //     f32 yr = Sin(angle_offset) * x + Cos(angle_offset) * y;

  //     particles[i*stars_per_ellipse + j].position = {xr,yr, 1};
  //   }
  // }

  // Loop(i, num_elipses) {
  //   f32 angle_offset = i * tilt_step;
  //   f32 a = (i + 1) * radius_step / 5.0f; // semi-major axis
  //   f32 b = a * 0.91f;                    // semi-minor axis

  //   u32 stars_per_ellipse = ParticleCount / num_elipses;

  //   Loop(j, stars_per_ellipse) {
  //     f32 t = 2.0f * PI * j / stars_per_ellipse;

  //     // Base ellipse position
  //     f32 x = a * Cos(t);
  //     f32 y = b * Sin(t);

  //     // Add random jitter (scatter)
  //     f32 rand_angle = rand_in_range_f32(-0.1f, 0.1f);  // scatter angle
  //     f32 rand_radius = rand_in_range_f32(-0.1f, 0.2f); // scatter radius

  //     x += rand_radius * Cos(t + rand_angle);
  //     y += rand_radius * Sin(t + rand_angle);

  //     // Rotate ellipse
  //     f32 xr = Cos(angle_offset) * x - Sin(angle_offset) * y;
  //     f32 yr = Sin(angle_offset) * x + Cos(angle_offset) * y;
  //     f32 z = rand_in_range_f32(-0.1, 0.1);

  //     particles[i * stars_per_ellipse + j].pos = {xr, yr, z};

  //     f32 dist = v3_length(particles[i].pos);

  //     f32 max_dist = 2;
  //     f32 brightness = Clamp(0.0f, 1.0f - dist / max_dist, 1.0);

  //     v4 baseColor = v4(0.5f, 0.5f, 0.5f, 1.0f);
      
  //     particles[i*stars_per_ellipse + j].color = v4(baseColor.r * brightness, baseColor.g * brightness, baseColor.b * brightness, baseColor.a);
      
  //     particles[i*stars_per_ellipse + j].pos = {xr,yr, 1};
  //   }
  // }

  Range range = {0, ParticleCount * sizeof(Particle)};
  vk_upload_to_gpu(vk.compute_storage_buffers[0], range, particles);
  vk_upload_to_gpu(vk.compute_storage_buffers[1], range, particles);
  
  VkDescriptorSetLayoutBinding layout_bindings[3] = {};
  layout_bindings[0].binding = 0;
  layout_bindings[0].descriptorCount = 1;
  layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  layout_bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
  
  layout_bindings[1].binding = 1;
  layout_bindings[1].descriptorCount = 1;
  layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  layout_bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
  
  layout_bindings[2].binding = 2;
  layout_bindings[2].descriptorCount = 1;
  layout_bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  layout_bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
  
  VkDescriptorSetLayoutCreateInfo layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  layout_info.bindingCount = ArrayCount(layout_bindings);
  layout_info.pBindings = layout_bindings;
  VK_CHECK(vkCreateDescriptorSetLayout(vkdevice, &layout_info, vk.allocator, &vk.compute_descriptor_set_layout));
  
  VkDescriptorSetLayout layouts[] = {
    vk.compute_descriptor_set_layout, 
    vk.compute_descriptor_set_layout, 
  };
  VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  alloc_info.descriptorPool = vk.descriptor_pool;
  alloc_info.descriptorSetCount = ArrayCount(layouts);
  alloc_info.pSetLayouts = layouts;
  VK_CHECK(vkAllocateDescriptorSets(vkdevice, &alloc_info, vk.compute_descriptor_sets));

  Loop (i, FramesInFlight) {
    VkWriteDescriptorSet descriptor_writes[3];
    
    VkDescriptorBufferInfo uniform_buffer_info = {};
    // uniform_buffer_info.buffer = vk.compute_uniform_buffer.handle;
    uniform_buffer_info.offset = 0;
    uniform_buffer_info.range = sizeof(UniformBufferObject);
    
    descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[0].dstSet = vk.compute_descriptor_sets[i];
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].dstArrayElement = 0;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].pBufferInfo = &uniform_buffer_info;

    VkDescriptorBufferInfo storage_buffer_info_last_frame{};
    storage_buffer_info_last_frame.buffer = vk.compute_storage_buffers[(i + 1) % FramesInFlight].handle;
    storage_buffer_info_last_frame.offset = 0;
    storage_buffer_info_last_frame.range = sizeof(Particle) * ParticleCount;

    descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[1].dstSet = vk.compute_descriptor_sets[i];
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].dstArrayElement = 0;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].pBufferInfo = &storage_buffer_info_last_frame;

    VkDescriptorBufferInfo storage_buffer_info_current_frame{};
    storage_buffer_info_current_frame.buffer = vk.compute_storage_buffers[i].handle;
    storage_buffer_info_current_frame.offset = 0;
    storage_buffer_info_current_frame.range = sizeof(Particle) * ParticleCount;

    descriptor_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[2].dstSet = vk.compute_descriptor_sets[i];
    descriptor_writes[2].dstBinding = 2;
    descriptor_writes[2].dstArrayElement = 0;
    descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[2].descriptorCount = 1;
    descriptor_writes[2].pBufferInfo = &storage_buffer_info_current_frame;

    vkUpdateDescriptorSets(vkdevice, 3, descriptor_writes, 0, null);
  }

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &vk.compute_descriptor_set_layout;
  VK_CHECK(vkCreatePipelineLayout(vkdevice, &pipelineLayoutInfo, vk.allocator, &vk.compute_shader.pipeline.pipeline_layout));

  VkComputePipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipeline_info.layout = vk.compute_shader.pipeline.pipeline_layout;
  pipeline_info.stage = vk.compute_shader.stage.pipeline_shader_stage_create_info;
  VK_CHECK(vkCreateComputePipelines(vkdevice, VK_NULL_HANDLE, 1, &pipeline_info, vk.allocator, &vk.compute_shader.pipeline.handle));
  

}

// void foo() {
//   Shader s = {
//     .has_position = true,
//     .has_color = true
//   };
//   vk_Shader* shader = &vk.graphics_shader_compute;
//   String stage_type_strs[] = { "vert"_, "frag"_};
//   #define ShaderStageCount 2
//   VkShaderStageFlagBits stage_types[ShaderStageCount] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
//   Loop (i, 2) {
//     shader->stages[i] = vk_shader_module_create("compute"_, stage_type_strs[i], stage_types[i]);
//   }
  
//   #define AttributeCount 10
//   u32 stages_count = 0;
//   u32 vert_stride = 0;
//   u32 attribute_count = 0;
//   u32 offset = 0;
//   VkVertexInputAttributeDescription attribute_desriptions[AttributeCount];
//   if (s.has_position) {
//     attribute_desriptions[attribute_count].binding = 0;
//     attribute_desriptions[attribute_count].location = attribute_count;
//     attribute_desriptions[attribute_count].format = VK_FORMAT_R32G32B32_SFLOAT;
//     attribute_desriptions[attribute_count].offset = OffsetOf(Particle, pos);
//     ++attribute_count;
//   }
//   if (s.has_color) {
//     attribute_desriptions[attribute_count].binding = 0;
//     attribute_desriptions[attribute_count].location = attribute_count;
//     attribute_desriptions[attribute_count].format = VK_FORMAT_R32G32B32A32_SFLOAT;
//     attribute_desriptions[attribute_count].offset = OffsetOf(Particle, color);
//     ++attribute_count;
//   }
//   vert_stride = sizeof(Particle);
  
//   VkPipelineShaderStageCreateInfo stage_create_infos[ShaderStageCount] = {};
//   Loop (i, ShaderStageCount) {
//     stage_create_infos[i] = shader->stages[i].shader_state_create_info;
//   }
//   MemCopy(&shader->attribute_desriptions, &attribute_desriptions, attribute_count*sizeof(VkVertexInputAttributeDescription));
//   shader->pipeline = vk_pipeline_create(vert_stride, attribute_count, attribute_desriptions,
//                                         2, stage_create_infos, shader->topology, false, false);
// }

void vk_shader_init() {
  // Pool
  {
    VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
    };

    VkDescriptorPoolCreateInfo pool_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
      .maxSets = 3,
      .poolSizeCount = ArrayCount(pool_sizes),
      .pPoolSizes = pool_sizes,
    };

    vkCreateDescriptorPool(vkdevice, &pool_info, vk.allocator, &vk.descriptor_pool);
  }

  // Setlayout
  {
    VkDescriptorSetLayoutBinding layout_bindings[] = {
      {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      },
    };

    VkDescriptorSetLayoutCreateInfo layout_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = ArrayCount(layout_bindings),
      .pBindings = layout_bindings,
    };
    VK_CHECK(vkCreateDescriptorSetLayout(vkdevice, &layout_info, vk.allocator, &vk.descriptor_set_layout));
  }

  // Descriptor
  {
    VkDescriptorSetLayout layouts[] = {
      vk.descriptor_set_layout,
      vk.descriptor_set_layout
    };
    VkDescriptorSetAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = vk.descriptor_pool,
      .descriptorSetCount = ArrayCount(layouts),
      .pSetLayouts = layouts,
    };
    VK_CHECK(vkAllocateDescriptorSets(vkdevice, &alloc_info, vk.descriptor_sets));
  }

  // Reload callback
  asset_watch_add([](String shader_name, u32 id) {
    VK_Shader* shader = &vk.shaders[id];
    vkDeviceWaitIdle(vkdevice);
    
    vkDestroyPipeline(vkdevice, shader->pipeline.handle, vk.allocator);
    Loop (i, 2) {
      vkDestroyShaderModule(vkdevice, shader->stages[i].module, vk.allocator);
    }
    
    String stage_type_strs[2] = { "vert"_, "frag"_, };
    VkShaderStageFlagBits stage_types[2] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    
    Loop (i, 2) {
      shader->stages[i] = vk_shader_module_create(shader_name, stage_type_strs[i], stage_types[i]);
    }
    
    shader->pipeline = vk_pipeline_create(shader->vert_stride, shader->attribute_count, shader->attribute_desriptions,
                                          2, shader->stages, shader->topology, true, shader->is_transparent); // TODO
  });

  // Mem for shaders
  {
    u64 size = sizeof(ShaderGlobalState) + AlignPow2(sizeof(ShaderEntity)*MaxEntities, 16) + AlignPow2(sizeof(DirectionalLight)*KB(1), 16);
    u64 offset = freelist_gpu_alloc(vk.storage_buffer.freelist, size);
    vk.storage_buffer_range = {offset, size};

    vk.push_constants = {
      .data = mem_alloc(sizeof(PushConstant) * MaxEntities),
      .element_size = sizeof(PushConstant),
    };

    vk.entities_data = {
      .data = vk.storage_buffer.maped_memory + AlignPow2(sizeof(ShaderGlobalState), 16),
      .element_size = sizeof(ShaderEntity),
    };
    
    vk.lights_data = {
      .data = vk.storage_buffer.maped_memory + AlignPow2(sizeof(ShaderGlobalState), 16) + sizeof(ShaderEntity)*MaxEntities,
      .element_size = sizeof(DirectionalLight),
    };

    Assign(vk.global_shader_state, vk.storage_buffer.maped_memory + offset);
  }
}
