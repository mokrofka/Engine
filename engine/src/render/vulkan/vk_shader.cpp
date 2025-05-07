#include "vk_types.h"

void vk_r_shader_create(struct Shader* s, u32 renderpass_id, u32 stage_count, String* stage_filenames, ShaderStage* stages) {
  // shader->internal_data = mem_alloc_struct(VK_Shader);

  // // TODO: dynamic renderpasses
  // vulkan_renderpass* renderpass = renderpass_id == 1 ? &context.main_renderpass : &context.ui_renderpass;

  // // Translate stages
  // VkShaderStageFlags vk_stages[VULKAN_SHADER_MAX_STAGES];
  // for (u8 i = 0; i < stage_count; ++i) {
  //   switch (stages[i]) {
  //   case SHADER_STAGE_FRAGMENT:
  //     vk_stages[i] = VK_SHADER_STAGE_FRAGMENT_BIT;
  //     break;
  //   case SHADER_STAGE_VERTEX:
  //     vk_stages[i] = VK_SHADER_STAGE_VERTEX_BIT;
  //     break;
  //   case SHADER_STAGE_GEOMETRY:
  //     KWARN("vulkan_renderer_shader_create: VK_SHADER_STAGE_GEOMETRY_BIT is set but not yet supported.");
  //     vk_stages[i] = VK_SHADER_STAGE_GEOMETRY_BIT;
  //     break;
  //   case SHADER_STAGE_COMPUTE:
  //     KWARN("vulkan_renderer_shader_create: SHADER_STAGE_COMPUTE is set but not yet supported.");
  //     vk_stages[i] = VK_SHADER_STAGE_COMPUTE_BIT;
  //     break;
  //   default:
  //     KERROR("Unsupported stage type: %d", stages[i]);
  //     break;
  //   }
  // }

  // // TODO: configurable max descriptor allocate count.

  // u32 max_descriptor_allocate_count = 1024;

  // // Take a copy of the pointer to the context.
  // vulkan_shader* out_shader = (vulkan_shader*)shader->internal_data;

  // out_shader->renderpass = renderpass;

  // // Build out the configuration.
  // out_shader->config.max_descriptor_set_count = max_descriptor_allocate_count;

  // // Shader stages. Parse out the flags.
  // kzero_memory(out_shader->config.stages, sizeof(vulkan_shader_stage_config) * VULKAN_SHADER_MAX_STAGES);
  // out_shader->config.stage_count = 0;
  // // Iterate provided stages.
  // for (u32 i = 0; i < stage_count; i++) {
  //   // Make sure there is room enough to add the stage.
  //   if (out_shader->config.stage_count + 1 > VULKAN_SHADER_MAX_STAGES) {
  //     KERROR("Shaders may have a maximum of %d stages", VULKAN_SHADER_MAX_STAGES);
  //     return false;
  //   }

  //   // Make sure the stage is a supported one.
  //   VkShaderStageFlagBits stage_flag;
  //   switch (stages[i]) {
  //   case SHADER_STAGE_VERTEX:
  //     stage_flag = VK_SHADER_STAGE_VERTEX_BIT;
  //     break;
  //   case SHADER_STAGE_FRAGMENT:
  //     stage_flag = VK_SHADER_STAGE_FRAGMENT_BIT;
  //     break;
  //   default:
  //     // Go to the next type.
  //     KERROR("vulkan_shader_create: Unsupported shader stage flagged: %d. Stage ignored.", stages[i]);
  //     continue;
  //   }

  //   // Set the stage and bump the counter.
  //   out_shader->config.stages[out_shader->config.stage_count].stage = stage_flag;
  //   string_ncopy(out_shader->config.stages[out_shader->config.stage_count].file_name, stage_filenames[i], 255);
  //   out_shader->config.stage_count++;
  // }

  // // Zero out arrays and counts.
  // kzero_memory(out_shader->config.descriptor_sets, sizeof(vulkan_descriptor_set_config) * 2);

  // // Attributes array.
  // kzero_memory(out_shader->config.attributes, sizeof(VkVertexInputAttributeDescription) * VULKAN_SHADER_MAX_ATTRIBUTES);

  // // For now, shaders will only ever have these 2 types of descriptor pools.
  // out_shader->config.pool_sizes[0] = (VkDescriptorPoolSize){VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024};         // HACK: max number of ubo descriptor sets.
  // out_shader->config.pool_sizes[1] = (VkDescriptorPoolSize){VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096}; // HACK: max number of image sampler descriptor sets.

  // // Global descriptor set config.
  // vulkan_descriptor_set_config global_descriptor_set_config = {};

  // // UBO is always available and first.
  // global_descriptor_set_config.bindings[BINDING_INDEX_UBO].binding = BINDING_INDEX_UBO;
  // global_descriptor_set_config.bindings[BINDING_INDEX_UBO].descriptorCount = 1;
  // global_descriptor_set_config.bindings[BINDING_INDEX_UBO].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  // global_descriptor_set_config.bindings[BINDING_INDEX_UBO].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  // global_descriptor_set_config.binding_count++;

  // out_shader->config.descriptor_sets[DESC_SET_INDEX_GLOBAL] = global_descriptor_set_config;
  // out_shader->config.descriptor_set_count++;
  // if (shader->use_instances) {
  //   // If using instances, add a second descriptor set.
  //   vulkan_descriptor_set_config instance_descriptor_set_config = {};

  //   // Add a UBO to it, as instances should always have one available.
  //   // NOTE: Might be a good idea to only add this if it is going to be used...
  //   instance_descriptor_set_config.bindings[BINDING_INDEX_UBO].binding = BINDING_INDEX_UBO;
  //   instance_descriptor_set_config.bindings[BINDING_INDEX_UBO].descriptorCount = 1;
  //   instance_descriptor_set_config.bindings[BINDING_INDEX_UBO].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  //   instance_descriptor_set_config.bindings[BINDING_INDEX_UBO].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  //   instance_descriptor_set_config.binding_count++;

  //   out_shader->config.descriptor_sets[DESC_SET_INDEX_INSTANCE] = instance_descriptor_set_config;
  //   out_shader->config.descriptor_set_count++;
  // }

  // // Invalidate all instance states.
  // // TODO: dynamic
  // for (u32 i = 0; i < 1024; ++i) {
  //   out_shader->instance_states[i].id = INVALID_ID;
  // }

  // return true;
}

void vk_r_shader_destroy(struct Shader* shader) {

}

void vk_r_shader_initialize(struct Shader* shader) {

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
