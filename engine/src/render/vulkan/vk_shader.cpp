#include "vk_types.h"
#include "vk_buffer.h"
#include "vk_pipeline.h"

#include "sys/shader_sys.h"
#include "sys/res_sys.h"

// The index of the global descriptor set.
const u32 DESC_SET_INDEX_GLOBAL = 0;
// The index of the instance descriptor set.
const u32 DESC_SET_INDEX_INSTANCE = 1;

// The index of the UBO binding.
const u32 BINDING_INDEX_UBO = 0;

// The index of the image sampler binding.
const u32 BINDING_INDEX_SAMPLER = 1;

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


void vk_r_shader_create(Shader* s, u32 renderpass_id, u32 stage_count, String* stage_filenames, ShaderStage* stages) {
  // s->internal_data = mem_alloc_struct(VK_Shader);

  // // TODO: dynamic renderpasses
  // VK_Renderpass* renderpass = vk_get_renderpass(renderpass_id);

  // // Translate stages
  // VkShaderStageFlags vk_stages[VK_ShaderMaxStages];
  // Loop (i, stage_count) {
  //   switch (stages[i]) {
  //   case ShaderStage_Fragment: {
  //     vk_stages[i] = VK_SHADER_STAGE_FRAGMENT_BIT;
  //   } break;
  //   case ShaderStage_Vertex: {
  //     vk_stages[i] = VK_SHADER_STAGE_VERTEX_BIT;
  //   } break;
  //   case ShaderStage_Geometry: {
  //     Warn("vk_r_shader_create: VK_SHADER_STAGE_GEOMETRY_BIT is set but not yet supported.");
  //     vk_stages[i] = VK_SHADER_STAGE_GEOMETRY_BIT;
  //   } break;
  //   case ShaderStage_Compute: {
  //     Warn("vk_r_shader_create: SHADER_STAGE_COMPUTE_BIT is set but not yet supported.");
  //     vk_stages[i] = VK_SHADER_STAGE_COMPUTE_BIT;
  //   } break;
  //   default:
  //     Error("Unsupported stage type: %i", stages[i]);
  //     break;
  //   }
  // }

  // // TODO: configurable max descriptor allocate count.

  // u32 max_descriptor_allocate_count = 1024;

  // // Take a copy of the pointer to the context.
  // VK_Shader* out_shader; Assign(out_shader, s->internal_data);

  // out_shader->renderpass = renderpass;

  // // Build out the configuration.
  // out_shader->config.max_descriptor_set_count = max_descriptor_allocate_count;

  // // Shader stages. Parse out the flags.
  // out_shader->config.stage_count = 0;
  // // Iterate provided stages.
  // Loop (i, stage_count) {
  //   // Make sure there is room enough to add the stage.
  //   if (out_shader->config.stage_count + 1 > VK_ShaderMaxStages) {
  //     Error("Shaders may have a maximum of %i stages", VK_ShaderMaxStages);
  //     return;
  //   }

  //   // Make sure the stage is a supported one.
  //   VkShaderStageFlagBits stage_flag;
  //   switch (stages[i]) {
  //   case ShaderStage_Vertex: {
  //     stage_flag = VK_SHADER_STAGE_VERTEX_BIT;
  //   } break;
  //   case ShaderStage_Fragment: {
  //     stage_flag = VK_SHADER_STAGE_FRAGMENT_BIT;
  //   } break;
  //   default:
  //     // Go to the next type.
  //     Error("vulkan_shader_create: Unsupported shader stage flagged: %i. Stage ignored.", stages[i]);
  //     continue;
  //   }

  //   // Set the stage and bump the counter.
  //   out_shader->config.stages[out_shader->config.stage_count].stage = stage_flag;
  //   str_copy(out_shader->config.stages[out_shader->config.stage_count].file_name64, stage_filenames[i]);
  //   ++out_shader->config.stage_count;
  // }

  // // For now, shaders will only ever have these 2 types of descriptor pools.
  // out_shader->config.pool_sizes[0] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024};         // HACK: max number of ubo descriptor sets.
  // out_shader->config.pool_sizes[1] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096}; // HACK: max number of image sampler descriptor sets.

  // // Global descriptor set config.
  // VK_DescriptorSetConfig global_descriptor_set_config = {};

  // // UBO is always available and first.
  // global_descriptor_set_config.bindings[BINDING_INDEX_UBO].binding = BINDING_INDEX_UBO;
  // global_descriptor_set_config.bindings[BINDING_INDEX_UBO].descriptorCount = 1;
  // global_descriptor_set_config.bindings[BINDING_INDEX_UBO].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  // global_descriptor_set_config.bindings[BINDING_INDEX_UBO].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  // ++global_descriptor_set_config.binding_count;

  // out_shader->config.descriptor_sets[DESC_SET_INDEX_GLOBAL] = global_descriptor_set_config;
  // ++out_shader->config.descriptor_set_count;
  // if (s->use_instances) {
  //   // If using instances, add a second descriptor set.
  //   VK_DescriptorSetConfig instance_descriptor_set_config = {};

  //   // Add a UBO to it, as instances should always have one available.
  //   // NOTE: Might be a good idea to only add this if it is going to be used...
  //   instance_descriptor_set_config.bindings[BINDING_INDEX_UBO].binding = BINDING_INDEX_UBO;
  //   instance_descriptor_set_config.bindings[BINDING_INDEX_UBO].descriptorCount = 1;
  //   instance_descriptor_set_config.bindings[BINDING_INDEX_UBO].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  //   instance_descriptor_set_config.bindings[BINDING_INDEX_UBO].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  //   ++instance_descriptor_set_config.binding_count;

  //   out_shader->config.descriptor_sets[DESC_SET_INDEX_INSTANCE] = instance_descriptor_set_config;
  //   ++out_shader->config.descriptor_set_count;
  // }

  // // Invalidate all instance states.
  // // TODO: dynamic
  // Loop (i, 1024) {
  //   out_shader->instance_states[i].id = INVALID_ID;
  // }
}

void vk_r_shader_destroy(Shader* s) {
  // Assert(s && s->internal_data);
  // VK_Shader* shader; Assign(shader, s->internal_data);

  // // Descriptor set layouts.
  // Loop (i, shader->config.descriptor_set_count) {
  //   if (shader->descriptor_set_layouts[i]) {
  //     vkDestroyDescriptorSetLayout(vkdevice, shader->descriptor_set_layouts[i], vk->allocator);
  //     shader->descriptor_set_layouts[i] = 0;
  //   }
  // }

  // // Descriptor pool
  // if (shader->descriptor_pool) {
  //   vkDestroyDescriptorPool(vkdevice, shader->descriptor_pool, vk->allocator);
  // }

  // // Uniform buffer.
  // vk_buffer_unmap_memory(&shader->uniform_buffer);
  // shader->mapped_uniform_buffer_block = 0;
  // vk_buffer_destroy(&shader->uniform_buffer);

  // // Pipeline
  // vk_pipeline_destroy(&shader->pipeline);

  // // Shader modules
  // Loop (i, shader->config.stage_count) {
  //   vkDestroyShaderModule(vkdevice, shader->stages[i].handle, vk->allocator);
  // }

  // // Destroy the configuration.
  // MemClearStruct(&shader->config);

  // mem_free(s->internal_data);
  // s->internal_data = 0;
}

void vk_r_shader_initialize(Shader* shader) {
  // VK_Shader* s; Assign(s, shader->internal_data);

  // // Create a module for each stage.
  // for (u32 i = 0; i < s->config.stage_count; ++i) {
  //   if (!create_module(s, s->config.stages[i], &s->stages[i])) {
  //     KERROR("Unable to create %s shader module for '%s'. Shader will be destroyed.", s->config.stages[i].file_name, shader->name);
  //     return false;
  //   }
  // }

  // // Static lookup table for our types->Vulkan ones.
  // static VkFormat* types = 0;
  // static VkFormat t[11];
  // if (!types) {
  //   t[SHADER_ATTRIB_TYPE_FLOAT32] = VK_FORMAT_R32_SFLOAT;
  //   t[SHADER_ATTRIB_TYPE_FLOAT32_2] = VK_FORMAT_R32G32_SFLOAT;
  //   t[SHADER_ATTRIB_TYPE_FLOAT32_3] = VK_FORMAT_R32G32B32_SFLOAT;
  //   t[SHADER_ATTRIB_TYPE_FLOAT32_4] = VK_FORMAT_R32G32B32A32_SFLOAT;
  //   t[SHADER_ATTRIB_TYPE_INT8] = VK_FORMAT_R8_SINT;
  //   t[SHADER_ATTRIB_TYPE_UINT8] = VK_FORMAT_R8_UINT;
  //   t[SHADER_ATTRIB_TYPE_INT16] = VK_FORMAT_R16_SINT;
  //   t[SHADER_ATTRIB_TYPE_UINT16] = VK_FORMAT_R16_UINT;
  //   t[SHADER_ATTRIB_TYPE_INT32] = VK_FORMAT_R32_SINT;
  //   t[SHADER_ATTRIB_TYPE_UINT32] = VK_FORMAT_R32_UINT;
  //   types = t;
  // }

  // // Process attributes
  // u32 attribute_count = darray_length(shader->attributes);
  // u32 offset = 0;
  // for (u32 i = 0; i < attribute_count; ++i) {
  //   // Setup the new attribute.
  //   VkVertexInputAttributeDescription attribute;
  //   attribute.location = i;
  //   attribute.binding = 0;
  //   attribute.offset = offset;
  //   attribute.format = types[shader->attributes[i].type];

  //   // Push into the config's attribute collection and add to the stride.
  //   s->config.attributes[i] = attribute;

  //   offset += shader->attributes[i].size;
  // }

  // // Process uniforms.
  // u32 uniform_count = darray_length(shader->uniforms);
  // for (u32 i = 0; i < uniform_count; ++i) {
  //   // For samplers, the descriptor bindings need to be updated. Other types of uniforms don't need anything to be done here.
  //   if (shader->uniforms[i].type == SHADER_UNIFORM_TYPE_SAMPLER) {
  //     const u32 set_index = (shader->uniforms[i].scope == SHADER_SCOPE_GLOBAL ? DESC_SET_INDEX_GLOBAL : DESC_SET_INDEX_INSTANCE);
  //     vulkan_descriptor_set_config* set_config = &s->config.descriptor_sets[set_index];
  //     if (set_config->binding_count < 2) {
  //       // There isn't a binding yet, meaning this is the first sampler to be added.
  //       // Create the binding with a single descriptor for this sampler.
  //       set_config->bindings[BINDING_INDEX_SAMPLER].binding = BINDING_INDEX_SAMPLER; // Always going to be the second one.
  //       set_config->bindings[BINDING_INDEX_SAMPLER].descriptorCount = 1;             // Default to 1, will increase with each sampler added to the appropriate level.
  //       set_config->bindings[BINDING_INDEX_SAMPLER].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  //       set_config->bindings[BINDING_INDEX_SAMPLER].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  //       set_config->binding_count++;
  //     } else {
  //       // There is already a binding for samplers, so just add a descriptor to it.
  //       // Take the current descriptor count as the location and increment the number of descriptors.
  //       set_config->bindings[BINDING_INDEX_SAMPLER].descriptorCount++;
  //     }
  //   }
  // }

  // // Descriptor pool.
  // VkDescriptorPoolCreateInfo pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  // pool_info.poolSizeCount = 2;
  // pool_info.pPoolSizes = s->config.pool_sizes;
  // pool_info.maxSets = s->config.max_descriptor_set_count;
  // pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

  // // Create descriptor pool.
  // VkResult result = vkCreateDescriptorPool(logical_device, &pool_info, vk_allocator, &s->descriptor_pool);
  // if (!vulkan_result_is_success(result)) {
  //   KERROR("vulkan_shader_initialize failed creating descriptor pool: '%s'", vulkan_result_string(result, true));
  //   return false;
  // }

  // // Create descriptor set layouts.
  // kzero_memory(s->descriptor_set_layouts, s->config.descriptor_set_count);
  // for (u32 i = 0; i < s->config.descriptor_set_count; ++i) {
  //   VkDescriptorSetLayoutCreateInfo layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  //   layout_info.bindingCount = s->config.descriptor_sets[i].binding_count;
  //   layout_info.pBindings = s->config.descriptor_sets[i].bindings;
  //   result = vkCreateDescriptorSetLayout(logical_device, &layout_info, vk_allocator, &s->descriptor_set_layouts[i]);
  //   if (!vulkan_result_is_success(result)) {
  //     KERROR("vulkan_shader_initialize failed creating descriptor pool: '%s'", vulkan_result_string(result, true));
  //     return false;
  //   }
  // }

  // // TODO: This feels wrong to have these here, at least in this fashion. Should probably
  // // Be configured to pull from someplace instead.
  // // Viewport.
  // VkViewport viewport;
  // viewport.x = 0.0f;
  // viewport.y = (f32)context.framebuffer_height;
  // viewport.width = (f32)context.framebuffer_width;
  // viewport.height = -(f32)context.framebuffer_height;
  // viewport.minDepth = 0.0f;
  // viewport.maxDepth = 1.0f;

  // // Scissor
  // VkRect2D scissor;
  // scissor.offset.x = scissor.offset.y = 0;
  // scissor.extent.width = context.framebuffer_width;
  // scissor.extent.height = context.framebuffer_height;

  // VkPipelineShaderStageCreateInfo stage_create_infos[VULKAN_SHADER_MAX_STAGES];
  // kzero_memory(stage_create_infos, sizeof(VkPipelineShaderStageCreateInfo) * VULKAN_SHADER_MAX_STAGES);
  // for (u32 i = 0; i < s->config.stage_count; ++i) {
  //   stage_create_infos[i] = s->stages[i].shader_stage_create_info;
  // }

  // b8 pipeline_result = vulkan_graphics_pipeline_create(
  //     &context,
  //     s->renderpass,
  //     shader->attribute_stride,
  //     darray_length(shader->attributes),
  //     s->config.attributes, // shader->attributes,
  //     s->config.descriptor_set_count,
  //     s->descriptor_set_layouts,
  //     s->config.stage_count,
  //     stage_create_infos,
  //     viewport,
  //     scissor,
  //     false,
  //     true,
  //     shader->push_constant_range_count,
  //     shader->push_constant_ranges,
  //     &s->pipeline);

  // if (!pipeline_result) {
  //   KERROR("Failed to load graphics pipeline for object shader.");
  //   return false;
  // }

  // // Grab the UBO alignment requirement from the device.
  // shader->required_ubo_alignment = context.device.properties.limits.minUniformBufferOffsetAlignment;

  // // Make sure the UBO is aligned according to device requirements.
  // shader->global_ubo_stride = get_aligned(shader->global_ubo_size, shader->required_ubo_alignment);
  // shader->ubo_stride = get_aligned(shader->ubo_size, shader->required_ubo_alignment);

  // // Uniform  buffer.
  // u32 device_local_bits = context.device.supports_device_local_host_visible ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : 0;
  // // TODO: max count should be configurable, or perhaps long term support of buffer resizing.
  // u64 total_buffer_size = shader->global_ubo_stride + (shader->ubo_stride * VULKAN_MAX_MATERIAL_COUNT); // global + (locals)
  // if (!vulkan_buffer_create(
  //         &context,
  //         total_buffer_size,
  //         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
  //         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | device_local_bits,
  //         true,
  //         true,
  //         &s->uniform_buffer)) {
  //   KERROR("Vulkan buffer creation failed for object shader.");
  //   return false;
  // }

  // // Allocate space for the global UBO, whcih should occupy the _stride_ space, _not_ the actual size used.
  // if (!vulkan_buffer_allocate(&s->uniform_buffer, shader->global_ubo_stride, &shader->global_ubo_offset)) {
  //   KERROR("Failed to allocate space for the uniform buffer!");
  //   return false;
  // }

  // // Map the entire buffer's memory.
  // s->mapped_uniform_buffer_block = vulkan_buffer_lock_memory(&context, &s->uniform_buffer, 0, VK_WHOLE_SIZE /*total_buffer_size*/, 0);

  // // Allocate global descriptor sets, one per frame. Global is always the first set.
  // VkDescriptorSetLayout global_layouts[3] = {
  //     s->descriptor_set_layouts[DESC_SET_INDEX_GLOBAL],
  //     s->descriptor_set_layouts[DESC_SET_INDEX_GLOBAL],
  //     s->descriptor_set_layouts[DESC_SET_INDEX_GLOBAL]};

  // VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  // alloc_info.descriptorPool = s->descriptor_pool;
  // alloc_info.descriptorSetCount = 3;
  // alloc_info.pSetLayouts = global_layouts;
  // VK_CHECK(vkAllocateDescriptorSets(context.device.logical_device, &alloc_info, s->global_descriptor_sets));

  // return true;
}
void vk_r_shader_use(struct Shader* shader) {

}
void vk_r_shader_bind_globals(struct Shader* s) {

}
void vk_r_shader_bind_instance(struct Shader* s, u32 instance_id) {

}
void vk_r_shader_apply_globals(struct Shader* s) {

}
void vk_r_shader_apply_instance(struct Shader* s) {

}
void vk_r_shader_acquire_instance_resources(struct Shader* s, u32* out_instance_id) {

}
void vk_r_shader_release_instance_resources(struct Shader* s, u32 instance_id) {

}
void vk_r_set_uniform(struct Shader* frontend_shader, struct shader_uniform* uniform, const void* value) {

}
