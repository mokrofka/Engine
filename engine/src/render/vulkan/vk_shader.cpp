#include "render/vulkan/vk_utils.h"
#include "vk_types.h"
#include "vk_buffer.h"

#include "sys/shader_sys.h"
#include "sys/res_sys.h"

VK_Pipeline vk_pipeline_create(
    u32 vert_stride, u32 attribute_count,
    VkVertexInputAttributeDescription* attributes,
    u32 stage_count, VkPipelineShaderStageCreateInfo* stages);
      
internal VK_ShaderStage shader_module_create(String name, String type_str, VkShaderStageFlagBits shader_stage_flag) {
  Scratch scratch;
  VK_ShaderStage shader_stage = {};
  String file_path = push_strf(scratch, "shaders/%s.%s.spv", name, type_str);
  
  Binary binary = res_binary_load(scratch, file_path);
  if (!binary.data) {
    Error("Unable to read shader module: %s", file_path);
  }
  
  shader_stage.create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shader_stage.create_info.codeSize = binary.size;
  shader_stage.create_info.pCode = (u32*)binary.data;

  VK_CHECK(vkCreateShaderModule(vkdevice, &shader_stage.create_info, vk->allocator, &shader_stage.handle));
  
  // Shader stage info
  shader_stage.shader_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stage.shader_state_create_info.stage = shader_stage_flag;
  shader_stage.shader_state_create_info.module = shader_stage.handle;
  shader_stage.shader_state_create_info.pName = "main";
  
  return shader_stage;
}

void vk_descriptor_pool_create() {
  VkDescriptorPoolSize pool_sizes[] = {
    {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
    {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.poolSizeCount = ArrayCount(pool_sizes);
  pool_info.pPoolSizes = pool_sizes;
  pool_info.maxSets = 3;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

  vkCreateDescriptorPool(vkdevice, &pool_info, null, &vk->descriptor_pool);
}

void vk_descriptor_set_create() {
  VkDescriptorSetLayoutBinding ubo_layout_binding = {};
  ubo_layout_binding.binding = 0;           // binding in shader
  ubo_layout_binding.descriptorCount = 1; // how much you have seperate resources you can access in shader (array)
  ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  
  VkDescriptorSetLayoutBinding texture_layout_binding = {};
  texture_layout_binding.binding = 1;           // binding in shader
  texture_layout_binding.descriptorCount = 1; // how much you have seperate resources you can access in shader (array)
  texture_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  texture_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  VkDescriptorSetLayoutBinding layout_bindings[] = {ubo_layout_binding, texture_layout_binding};
  
  VkDescriptorSetLayoutCreateInfo layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  layout_info.bindingCount = ArrayCount(layout_bindings);
  layout_info.pBindings = layout_bindings;
  VK_CHECK(vkCreateDescriptorSetLayout(vkdevice, &layout_info, vk->allocator, &vk->descriptor_set_layout));
}

void vk_descriptor_set_alloc() {
  VkDescriptorSetLayout layouts[] = {
    vk->descriptor_set_layout,
    vk->descriptor_set_layout,
    vk->descriptor_set_layout};
    
  VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  alloc_info.descriptorPool = vk->descriptor_pool;
  alloc_info.descriptorSetCount = ArrayCount(layouts);
  alloc_info.pSetLayouts = layouts;
  VK_CHECK(vkAllocateDescriptorSets(vkdevice, &alloc_info, vk->descriptor_sets));
}

void vk_r_shader_create(Shader* s, void* data, u64 data_size, u64 push_size) {
  // push constants
  vk->sparse_push_constants.data = mem_alloc(MB(10));
  vk->sparse_push_constants.element_size = push_size;
  vk->sparse_push_constants.capacity = MaxEntities;
  vk->sparse_push_constants.size = 0;

  // uniform buffer
  u64 uniform_buffer_offset = vk_buffer_alloc(&vk->uniform_buffer, data_size*10, 64);
  MemRange range = {uniform_buffer_offset, data_size};
  vk->uniform_buffer_mem_range = range;
  *(void**)data = (u8*)vk->uniform_buffer.maped_memory + range.offset;

  // shader
  local u32 yes = (vk_descriptor_pool_create(), vk_descriptor_set_create(), vk_descriptor_set_alloc(), 1);
  String stage_type_strs[3] = { "vert"_, "frag"_, };
  #define ShaderStageCount 3
  VkShaderStageFlagBits stage_types[ShaderStageCount] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

  Loop (i, 2) {
    vk->shader.stages[i] = shader_module_create(s->name, stage_type_strs[i], stage_types[i]);
  }
  
  #define AttributeCount 10
  u32 stages_count = 0;
  u32 vert_stride = 0;
  u32 attribute_counter = 0;
  u32 offset = 0;
  VkVertexInputAttributeDescription attribute_desriptions[AttributeCount];
  if (s->has_position) {
    attribute_desriptions[attribute_counter].binding = 0;
    attribute_desriptions[attribute_counter].location = attribute_counter;
    attribute_desriptions[attribute_counter].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_desriptions[attribute_counter].offset = vert_stride;
    vert_stride += sizeof(v3);
    ++attribute_counter;
  }
  if (s->has_color) {
    attribute_desriptions[attribute_counter].binding = 0;
    attribute_desriptions[attribute_counter].location = attribute_counter;
    attribute_desriptions[attribute_counter].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_desriptions[attribute_counter].offset = vert_stride;
    vert_stride += sizeof(v3);
    ++attribute_counter;
  }
  if (s->has_tex_coord) {
    attribute_desriptions[attribute_counter].binding = 0;
    attribute_desriptions[attribute_counter].location = attribute_counter;
    attribute_desriptions[attribute_counter].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_desriptions[attribute_counter].offset = vert_stride;
    vert_stride += sizeof(v2);
    ++attribute_counter;
  }
  
  VkPipelineShaderStageCreateInfo stage_create_infos[ShaderStageCount] = {};
  Loop (i, ShaderStageCount) {
    stage_create_infos[i] = vk->shader.stages[i].shader_state_create_info;
  }
  vk->shader.pipeline = vk_pipeline_create(vert_stride, attribute_counter, attribute_desriptions,
                                           2, stage_create_infos);
}

VK_Pipeline vk_pipeline_create(
    u32 vert_stride, u32 attribute_count,
    VkVertexInputAttributeDescription* attributes,
    u32 stage_count, VkPipelineShaderStageCreateInfo* stages) {
  VK_Pipeline pipeline = {};
  
  // Pipeline creation
  VkViewport viewport;
  viewport.x = 0.0f;
  viewport.y = (f32)vk->frame.height;
  viewport.width = (f32)vk->frame.width;
  viewport.height = (f32)vk->frame.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  // Scissor
  VkRect2D scissor;
  scissor.offset.x = scissor.offset.y = 0;
  scissor.extent.width = vk->frame.width;
  scissor.extent.height = vk->frame.height;

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
  if (true) {
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;
  }
  
  VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
  color_blend_attachment_state.blendEnable = VK_TRUE;
  color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
  color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
  color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

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
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;
  
  // Pipeline layout
  VkPipelineLayoutCreateInfo pipeline_layout_create_info = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  
  // Push constants
  VkPushConstantRange push_constant;
  push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  push_constant.offset = sizeof(mat4) * 0;
  push_constant.size = sizeof(mat4) * 2;
  pipeline_layout_create_info.pushConstantRangeCount = 1;
  pipeline_layout_create_info.pPushConstantRanges = &push_constant;
  
  // Descriptor set layouts
  VkDescriptorSetLayout set_layouts[] = {
    vk->descriptor_set_layout,
  };
  pipeline_layout_create_info.setLayoutCount = ArrayCount(set_layouts);
  pipeline_layout_create_info.pSetLayouts = set_layouts;
  
  // Create the pipeline layout.
  VK_CHECK(vkCreatePipelineLayout(vkdevice, &pipeline_layout_create_info, vk->allocator, &pipeline.pipeline_layout));

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
  
  pipeline_create_info.layout = pipeline.pipeline_layout;
  
  pipeline_create_info.renderPass = vk_get_renderpass(vk->main_renderpass_id)->handle;
  pipeline_create_info.subpass = 0;
  pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_create_info.basePipelineIndex = -1;
  
  VkResult result = vkCreateGraphicsPipelines(
    vkdevice, 
    VK_NULL_HANDLE, 
    1, 
    &pipeline_create_info, 
    vk->allocator, 
    &pipeline.handle);
  
  if (vk_result_is_success(result)) {
    Debug("Graphics pipeline created"_);
    return pipeline;
  } 
  
  Error("vkCreateGraphicsPipelines failed with %s.", vk_result_string(result, true));
  VK_Pipeline p = {}; return p;
}
