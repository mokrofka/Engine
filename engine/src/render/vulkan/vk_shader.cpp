#include "vk_types.h"
#include "vk_buffer.h"

#include "sys/shader_sys.h"
#include "sys/res_sys.h"
#include "asset_watch.h"

internal VK_Pipeline vk_pipeline_create(
    u32 vert_stride, u32 attribute_count,
    VkVertexInputAttributeDescription* attributes,
    u32 stage_count, VkPipelineShaderStageCreateInfo* stages, VkPrimitiveTopology topology, b32 is_depth, b32 is_alpha);

internal VK_ShaderStage vk_shader_module_create(String name, String type_str, VkShaderStageFlagBits shader_stage_flag) {
  Scratch scratch;
  VK_ShaderStage shader_stage = {};
  String file_path = push_strf(scratch, "shaders/compiled/%s.%s.spv", name, type_str);
  
  Binary binary = res_binary_load(scratch, file_path);
  if (!binary.data) {
    Error("Unable to read shader module: %s", file_path);
  }
  
  shader_stage.create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shader_stage.create_info.codeSize = binary.size;
  shader_stage.create_info.pCode = (u32*)binary.data;

  VK_CHECK(vkCreateShaderModule(vkdevice, &shader_stage.create_info, vk.allocator, &shader_stage.handle));
  
  // Shader stage info
  shader_stage.shader_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stage.shader_state_create_info.stage = shader_stage_flag;
  shader_stage.shader_state_create_info.module = shader_stage.handle;
  shader_stage.shader_state_create_info.pName = "main";
  
  return shader_stage;
}

internal void vk_descriptor_pool_create() {
  VkDescriptorPoolSize pool_sizes[] = {
    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
  };

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.poolSizeCount = ArrayCount(pool_sizes);
  pool_info.pPoolSizes = pool_sizes;
  pool_info.maxSets = 3;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

  vkCreateDescriptorPool(vkdevice, &pool_info, null, &vk.descriptor_pool);
}

internal void vk_descriptor_set_create() {
  VkDescriptorSetLayoutBinding ubo_layout_binding = {};
  ubo_layout_binding.binding = 0;
  ubo_layout_binding.descriptorCount = 1;
  ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  
  VkDescriptorSetLayoutBinding texture_layout_binding = {};
  texture_layout_binding.binding = 1;
  texture_layout_binding.descriptorCount = 1;
  texture_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  texture_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  VkDescriptorSetLayoutBinding layout_bindings[] = {ubo_layout_binding, texture_layout_binding};
  
  VkDescriptorSetLayoutCreateInfo layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  layout_info.bindingCount = ArrayCount(layout_bindings);
  layout_info.pBindings = layout_bindings;
  VK_CHECK(vkCreateDescriptorSetLayout(vkdevice, &layout_info, vk.allocator, &vk.descriptor_set_layout));
}

internal void vk_descriptor_set_alloc() {
  VkDescriptorSetLayout layouts[] = {
    vk.descriptor_set_layout,
    vk.descriptor_set_layout};
    
  VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  alloc_info.descriptorPool = vk.descriptor_pool;
  alloc_info.descriptorSetCount = ArrayCount(layouts);
  alloc_info.pSetLayouts = layouts;
  VK_CHECK(vkAllocateDescriptorSets(vkdevice, &alloc_info, vk.descriptor_sets));
}

void vk_r_shader_create(Shader* s) {
  Scratch scratch;
  // push constants
  vk_Shader* shader = &vk.shaders[vk.shader_count];
  // SparseSetKeep* push_constants = &shader->push_constants;
  // push_constants->data = mem_alloc(push_size * MaxEntities);
  // push_constants->element_size = push_size;
  // push_constants->capacity = 1024;
  // push_constants->size = 0;

  // TODO
  // uniform buffer
  u64 uniform_buffer_offset = 0; // NOTE won't work
  // MemRange range = {uniform_buffer_offset, data_size};
  // vk.uniform_buffer_mem_range = range;
  // *(void**)data = (u8*)vk.uniform_buffer.maped_memory + range.offset;
  // *(void**)data = (u8*)vk.uniform_buffer.maped_memory;
  // *(void**)data = (u8*)vk.storage_buffer.maped_memory;
  // vk.projection_view = (mat4*)data;

  // shader
  #define ShaderStageCount 2
  String stage_type_strs[] = { "vert"_, "frag"_};
  VkShaderStageFlagBits stage_types[ShaderStageCount] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
  String assets_dir;
  Loop (i, ShaderStageCount) {
    shader->stages[i] = vk_shader_module_create(s->name, stage_type_strs[i], stage_types[i]);
  }

  #define AttributeCount 10
  VkVertexInputAttributeDescription attribute_desriptions[AttributeCount];

  VkFormat formats[] = {
    VK_FORMAT_R32_SFLOAT,
    VK_FORMAT_R32G32_SFLOAT,
    VK_FORMAT_R32G32B32_SFLOAT,
    VK_FORMAT_R32G32B32A32_SFLOAT
  };
  i32 i = 0;
  u32 vert_stride = 0;
  u32 attribute_count = 0;
  while (s->attribut[i]) {
    attribute_desriptions[i].location = i;
    attribute_desriptions[i].binding = 0;
    attribute_desriptions[i].format = formats[s->attribut[i]-1];
    attribute_desriptions[i].offset = vert_stride;
    vert_stride += s->attribut[i] * sizeof(f32);
    ++i;
    ++attribute_count;
  }
  
  VkPipelineShaderStageCreateInfo stage_create_infos[ShaderStageCount] = {};
  Loop (i, ShaderStageCount) {
    stage_create_infos[i] = shader->stages[i].shader_state_create_info;
  }
  
  VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  if (s->primitive == ShaderTopology_Line) {
    topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
  }
  MemCopy(&shader->attribute_desriptions, &attribute_desriptions, attribute_count*sizeof(VkVertexInputAttributeDescription));
  shader->pipeline = vk_pipeline_create(vert_stride, attribute_count, attribute_desriptions,
                                        2, stage_create_infos, topology, true, s->is_transparent);

  shader->name = s->name;
  shader->attribute_count = attribute_count;
  shader->vert_stride = vert_stride;
  shader->topology = topology;
  shader->is_transparent = s->is_transparent;
  ++vk.shader_count;
}

VK_Pipeline vk_pipeline_create(
    u32 vert_stride, u32 attribute_count,
    VkVertexInputAttributeDescription* attributes,
    u32 stage_count, VkPipelineShaderStageCreateInfo* stages, VkPrimitiveTopology topology, b32 is_depth, b32 is_alpha) {
  VK_Pipeline result = {};
  
  // Pipeline creation
  VkViewport viewport;
  viewport.x = 0.0f;
  viewport.y = (f32)vk.frame.height;
  viewport.width = (f32)vk.frame.width;
  viewport.height = (f32)vk.frame.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  // Scissor
  VkRect2D scissor;
  scissor.offset.x = scissor.offset.y = 0;
  scissor.extent.width = vk.frame.width;
  scissor.extent.height = vk.frame.height;

  VkPipelineViewportStateCreateInfo viewport_state = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;
  
  // Rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer_create_info = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  rasterizer_create_info.depthClampEnable = VK_FALSE;
  rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;
  rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer_create_info.lineWidth = 1.0f;
  rasterizer_create_info.cullMode = VK_CULL_MODE_NONE;
  // rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer_create_info.depthBiasEnable = VK_FALSE;
  rasterizer_create_info.depthBiasConstantFactor = 0.0f;
  rasterizer_create_info.depthBiasClamp = 0.0f;
  rasterizer_create_info.depthBiasSlopeFactor = 0.0f;
  
  // Multisampling
  VkPipelineMultisampleStateCreateInfo multisampling_create_info = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  multisampling_create_info.sampleShadingEnable = VK_FALSE;
  multisampling_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling_create_info.minSampleShading = 1.0f;
  multisampling_create_info.pSampleMask = 0;
  multisampling_create_info.alphaToCoverageEnable = VK_FALSE;
  multisampling_create_info.alphaToOneEnable = VK_FALSE;

  // Depth and stencil testing
  VkPipelineDepthStencilStateCreateInfo depth_stencil = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  if (is_depth) {
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;
  }
  
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


  VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  color_blend_state_create_info.logicOpEnable = VK_FALSE;
  color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
  color_blend_state_create_info.attachmentCount = 1;
  color_blend_state_create_info.pAttachments = &color_blend_attachment_state;
   
  // Dynamic state
  const u32 dynamic_state_count = 3;
  VkDynamicState dynamic_state[dynamic_state_count] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
    VK_DYNAMIC_STATE_LINE_WIDTH};

  VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
  dynamic_state_create_info.dynamicStateCount = dynamic_state_count;
  dynamic_state_create_info.pDynamicStates = dynamic_state;
  
  // Vertex input
  VkVertexInputBindingDescription binding_description;
  binding_description.binding = 0; // Binding index
  binding_description.stride = vert_stride;
  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Move to next data entry for each vertex

  // Attributes
  VkPipelineVertexInputStateCreateInfo vertex_input_info = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
  vertex_input_info.vertexBindingDescriptionCount = 1;
  vertex_input_info.pVertexBindingDescriptions = &binding_description;
  vertex_input_info.vertexAttributeDescriptionCount = attribute_count;
  vertex_input_info.pVertexAttributeDescriptions = attributes;
  
  // Input assembly
  VkPipelineInputAssemblyStateCreateInfo input_assembly = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  input_assembly.topology = topology;
  input_assembly.primitiveRestartEnable = VK_FALSE;
  
  // Pipeline layout
  VkPipelineLayoutCreateInfo pipeline_layout_create_info = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  
  // Push constants
  VkPushConstantRange push_constant;
  push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  push_constant.offset = sizeof(mat4) * 0;
  push_constant.size = sizeof(mat4) * 2;
  pipeline_layout_create_info.pushConstantRangeCount = 1;
  pipeline_layout_create_info.pPushConstantRanges = &push_constant;
  
  // Descriptor set layouts
  VkDescriptorSetLayout set_layouts[] = {
    vk.descriptor_set_layout,
  };
  pipeline_layout_create_info.setLayoutCount = ArrayCount(set_layouts);
  pipeline_layout_create_info.pSetLayouts = set_layouts;
  
  // Create the pipeline layout.
  VK_CHECK(vkCreatePipelineLayout(vkdevice, &pipeline_layout_create_info, vk.allocator, &result.pipeline_layout));

  // Pipeline create
  VkGraphicsPipelineCreateInfo pipeline_create_info = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  pipeline_create_info.stageCount = stage_count;
  pipeline_create_info.pStages = stages;
  pipeline_create_info.pVertexInputState = &vertex_input_info;
  pipeline_create_info.pInputAssemblyState = &input_assembly;

  pipeline_create_info.pViewportState = &viewport_state;
  pipeline_create_info.pRasterizationState = &rasterizer_create_info;
  pipeline_create_info.pMultisampleState = &multisampling_create_info;
  pipeline_create_info.pDepthStencilState = &depth_stencil;
  pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
  pipeline_create_info.pDynamicState = &dynamic_state_create_info;
  pipeline_create_info.pTessellationState = null;
  
  pipeline_create_info.layout = result.pipeline_layout;
  
  pipeline_create_info.renderPass = 0;
  pipeline_create_info.subpass = 0;
  pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_create_info.basePipelineIndex = -1;

  VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
  // VkFormat color_format = VK_FORMAT_R8G8B8A8_SRGB;
  VkPipelineRenderingCreateInfo renderingCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .colorAttachmentCount = 1,
    .pColorAttachmentFormats = &color_format,
    .depthAttachmentFormat = vk.device.depth_format,
  };
  pipeline_create_info.pNext = &renderingCreateInfo;

  VK_CHECK(vkCreateGraphicsPipelines(vkdevice, VK_NULL_HANDLE, 1, &pipeline_create_info, vk.allocator, &result.handle));

  return result;
}

void vk_reload_shader(String shader_name, u32 id) {
  vk_Shader* shader = &vk.shaders[id];
  vkDeviceWaitIdle(vkdevice);
  
  vkDestroyPipeline(vkdevice, shader->pipeline.handle, vk.allocator);
  Loop (i, 2) {
    vkDestroyShaderModule(vkdevice, shader->stages[i].handle, vk.allocator);
  }
  
  String stage_type_strs[2] = { "vert"_, "frag"_, };
  VkShaderStageFlagBits stage_types[2] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
  
  Loop (i, 2) {
    shader->stages[i] = vk_shader_module_create(shader_name, stage_type_strs[i], stage_types[i]);
  }
  
  VkPipelineShaderStageCreateInfo stage_create_infos[ShaderStageCount] = {};
  Loop (i, ShaderStageCount) {
    stage_create_infos[i] = shader->stages[i].shader_state_create_info;
  }
  shader->pipeline = vk_pipeline_create(shader->vert_stride, shader->attribute_count, shader->attribute_desriptions,
                                        2, stage_create_infos, shader->topology, true, shader->is_transparent); // TODO
}

f32 ease_in_exp(f32 x) {
	return x <= 0.0 ? 0.0 : Pow(2, 10.0 * x - 10.0);
}

void compute_shader() {
  Scratch scratch;
  VK_ComputeShader* shader = &vk.compute_shader;
  shader->stage = vk_shader_module_create("compute"_, "comp"_, VK_SHADER_STAGE_COMPUTE_BIT);
  
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

  MemRange range = {0, ParticleCount * sizeof(Particle)};
  vk_upload_to_gpu(&vk.storage_buffers[0], range, particles);
  vk_upload_to_gpu(&vk.storage_buffers[1], range, particles);
  
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
    uniform_buffer_info.buffer = vk.compute_uniform_buffer.handle;
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
    storage_buffer_info_last_frame.buffer = vk.storage_buffers[(i + 1) % FramesInFlight].handle;
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
    storage_buffer_info_current_frame.buffer = vk.storage_buffers[i].handle;
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
  pipeline_info.stage = vk.compute_shader.stage.shader_state_create_info;
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
  vk_descriptor_pool_create();
  vk_descriptor_set_create();
  vk_descriptor_set_alloc();
  asset_watch_add(vk_reload_shader);

  vk.push_constants.data = mem_alloc(sizeof(PushConstant) * MaxEntities);
  vk.push_constants.element_size = sizeof(PushConstant);
  vk.push_constants.capacity = 1024;
  vk.push_constants.entity_count = 0;

  u64 size = sizeof(GlobalShaderState) + sizeof(EntityShader)*MaxEntities;
  u64 offset = freelist_gpu_alloc(vk.storage_buffer.freelist, size);
  MemRange range = {offset, size};
  Assign(vk.global_shader_state, vk.storage_buffer.maped_memory + offset);
}
