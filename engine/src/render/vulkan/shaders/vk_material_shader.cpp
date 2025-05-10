#include "vk_material_shader.h"

#include "render/vulkan/vk_shader_utils.h"
#include "render/vulkan/vk_pipeline.h"
#include "render/vulkan/vk_buffer.h"

#include "sys/texture_sys.h"

#define BUILTIN_SHADER_NAME_MATERIAL "Builtin.MaterialShader"_

VK_MaterialShader vk_material_shader_create() {
  VK_MaterialShader shader = {};

  String stage_type_strs[MaterialShaderStageCount] = { "vert"_, "frag"_, };
  VkShaderStageFlagBits stage_types[MaterialShaderStageCount] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

  Loop (i, MaterialShaderStageCount) {
    shader.stages[i] = shader_module_create(BUILTIN_SHADER_NAME_MATERIAL, stage_type_strs[i], stage_types[i]);
  }
  
  // Global Descriptors
  VkDescriptorSetLayoutBinding g_ubo_layout_binding = {};
  g_ubo_layout_binding.binding = 0; // binding in shader
  g_ubo_layout_binding.descriptorCount = 1; // how much you have seperate resources you can access in shader (array)
  g_ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  g_ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  
  VkDescriptorSetLayoutCreateInfo g_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  g_layout_info.bindingCount = 1;
  g_layout_info.pBindings = &g_ubo_layout_binding;
  VK_CHECK(vkCreateDescriptorSetLayout(vkdevice, &g_layout_info, vk->allocator, &shader.global_descriptor_set_layout));
  
  // Global descriptor pool: Used for global items such as view/projection matrix
  VkDescriptorPoolSize g_pool_size = {};
  g_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  g_pool_size.descriptorCount = 3;

  VkDescriptorPoolCreateInfo g_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  g_pool_info.poolSizeCount = 1;
  g_pool_info.pPoolSizes = &g_pool_size;
  g_pool_info.maxSets = vk->swapchain.image_count;
  VK_CHECK(vkCreateDescriptorPool(vkdevice, &g_pool_info, vk->allocator, &shader.global_descriptor_pool));

  shader.sampler_uses[0] = TextureUse_MapDiffuse;

  // Local/Object Descriptors
  VkDescriptorType descriptor_types[MaterialShaderDescriptorCount] = {
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         // Binding 0 - uniform buffer
    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, // Binding 1 - Diffuse sampler layout.
  };
  
  VkDescriptorSetLayoutBinding bindings[MaterialShaderDescriptorCount] = {};
  Loop (i, MaterialShaderDescriptorCount) {
    bindings[i].binding = i;
    bindings[i].descriptorCount = 1;
    bindings[i].descriptorType = descriptor_types[i];
    bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  }

  VkDescriptorSetLayoutCreateInfo layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  layout_info.bindingCount = MaterialShaderDescriptorCount;
  layout_info.pBindings = bindings;
  VK_CHECK(vkCreateDescriptorSetLayout(vkdevice, &layout_info, vk->allocator, &shader.obj_descriptor_set_layout));
  
  // Local/Object descriptor pool: Used for object-specific items like diffuse color
  VkDescriptorPoolSize obj_pool_sizes[2];
  // The first section will be used for uniform buffers
  obj_pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  obj_pool_sizes[0].descriptorCount = VK_MaxMaterialCount; // how many descriptor in whole you have
  // The second section will be used for image samplers
  obj_pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  obj_pool_sizes[1].descriptorCount = MaterialShaderSamplerCount * VK_MaxMaterialCount;
  
  VkDescriptorPoolCreateInfo obj_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  obj_pool_info.poolSizeCount = 2;
  obj_pool_info.pPoolSizes = obj_pool_sizes;
  obj_pool_info.maxSets = VK_MaxMaterialCount;
  obj_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // allows you free 
  
  // Create object descriptor pool
  VK_CHECK(vkCreateDescriptorPool(vkdevice, &obj_pool_info, vk->allocator, &shader.obj_descriptor_pool));
  
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
  
  // Attributes
  u32 offset = 0;
  #define ATTRIBUTE_COUNT 2
  VkVertexInputAttributeDescription attribute_desriptions[ATTRIBUTE_COUNT];
  // Position, texcoord
  VkFormat formats[ATTRIBUTE_COUNT] = {
    VK_FORMAT_R32G32B32_SFLOAT, // v3
    VK_FORMAT_R32G32_SFLOAT     // v2
  };
  u64 sizes[ATTRIBUTE_COUNT] = {
    sizeof(v3),
    sizeof(v2)
  };
  Loop (i, ATTRIBUTE_COUNT) {
    attribute_desriptions[i].binding = 0;  // binding index - should match binding desc
    attribute_desriptions[i].location = i; // attrib location
    attribute_desriptions[i].format = formats[i];
    attribute_desriptions[i].offset = offset;
    offset += sizes[i];
  }
  
  // Descriptor set layouts
  const i32 descriptor_set_layout_count = 2;
  VkDescriptorSetLayout layouts[descriptor_set_layout_count] = {
    shader.global_descriptor_set_layout,
    shader.obj_descriptor_set_layout};

  // Stages
  // NOTE Should match the number of shader.stages
  VkPipelineShaderStageCreateInfo stage_create_infos[MaterialShaderStageCount] = {};
  Loop (i, MaterialShaderStageCount) {
    // stage_create_infos[i].sType = shader.stages[i].shader_state_create_info.sType;
    stage_create_infos[i] = shader.stages[i].shader_state_create_info;
  }

  shader.pipeline = vk_graphics_pipeline_create(
    vk_get_renderpass(vk->main_renderpass_id),
    sizeof(Vertex3D),
    ATTRIBUTE_COUNT,
    attribute_desriptions,
    descriptor_set_layout_count,
    layouts,
    MaterialShaderStageCount,
    stage_create_infos,
    viewport,
    scissor,
    false,
    true);

  // Create uniform buffer
  shader.global_uniform_buffer = vk_buffer_create(
    sizeof(VK_MaterialShaderGlobalUbo),
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
    true);
  
  // Allocate global descriptor sets
  VkDescriptorSetLayout global_layouts[] = {
    shader.global_descriptor_set_layout,
    shader.global_descriptor_set_layout,
    shader.global_descriptor_set_layout};

  VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  alloc_info.descriptorPool = shader.global_descriptor_pool;
  alloc_info.descriptorSetCount = 3;
  alloc_info.pSetLayouts = global_layouts;
  VK_CHECK(vkAllocateDescriptorSets(vkdevice, &alloc_info, shader.global_descriptor_sets));
  
  // Create the object uniform buffer
  shader.obj_uniform_buffer = vk_buffer_create(
    sizeof(VK_MaterialShaderInstUbo) * VK_MaxMaterialCount,
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
    true);

  return shader;
}

void vk_material_shader_destroy(VK_MaterialShader* shader) {
  VkDevice logical_device = vkdevice;
  
  vkDestroyDescriptorPool(logical_device, shader->obj_descriptor_pool, vk->allocator);
  vkDestroyDescriptorSetLayout(logical_device, shader->obj_descriptor_set_layout, vk->allocator);
  
  // Destroy uniform buffer
  vk_buffer_destroy(&shader->global_uniform_buffer);
  vk_buffer_destroy(&shader->obj_uniform_buffer);
  
  // Destroy pipeline
  vk_pipeline_destroy(&shader->pipeline);

  // Destroy global descriptor pool
  vkDestroyDescriptorPool(logical_device, shader->global_descriptor_pool, vk->allocator);
  
  // Destroy global set layouts
  vkDestroyDescriptorSetLayout(logical_device, shader->global_descriptor_set_layout, vk->allocator);
  
  // Destroy shader modules
  Loop (i, MaterialShaderStageCount) {
    vkDestroyShaderModule(vkdevice, shader->stages[i].handle, vk->allocator);
  }
}

void vk_material_shader_use(VK_MaterialShader* shader) {
  VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
  vk_pipeline_bind(vk->render.cmds[vk->frame.image_index].handle, bind_point, shader->pipeline);
}

void vk_material_shader_update_global_state(VK_MaterialShader* shader, f32 delta_time) {
  u32 image_index = vk->frame.image_index;
  VkDescriptorSet global_descriptor = shader->global_descriptor_sets[image_index];
  VK_CommandBuffer cmd = vk_get_current_cmd();
  
  // Bind the global descriptor set to be updated
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.pipeline_layout, 0, 1, &global_descriptor, 0, 0);
  
  // Configure the descriptors for the given index
  u32 range = sizeof(VK_MaterialShaderGlobalUbo);
  u64 offset = 0;

  // Copy data to buffer
  vk_buffer_load_data(&shader->global_uniform_buffer, offset, range, 0, &shader->global_ubo);
  
  VkDescriptorBufferInfo buffer_info;
  buffer_info.buffer = shader->global_uniform_buffer.handle;
  buffer_info.offset = offset;
  buffer_info.range = range;
  
  // Update descriptor sets
  VkWriteDescriptorSet descriptor_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  descriptor_write.dstSet = global_descriptor;
  descriptor_write.dstBinding = 0;
  descriptor_write.dstArrayElement = 0;
  descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptor_write.descriptorCount = 1;
  descriptor_write.pBufferInfo = &buffer_info;
  
  vkUpdateDescriptorSets(vkdevice, 1, &descriptor_write, 0, 0);
}

void vk_material_shader_set_model(VK_MaterialShader* shader, mat4 model) {
  Assert(shader);
  u32 image_index = vk->frame.image_index;
  vkCmdPushConstants(vk->render.cmds[vk->frame.image_index].handle, shader->pipeline.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &model);
}

void vk_material_shader_apply_material(VK_MaterialShader* shader, Material* material) {
  u32 image_index = vk->frame.image_index;

  // Obtain material data
  VK_MaterialShaderInstState* object_state = &shader->instance_states[material->internal_id];
  VkDescriptorSet object_descriptor_set = object_state->descriptor_sets[image_index];

  // TODO if needs update
  VkWriteDescriptorSet descriptor_writes[MaterialShaderDescriptorCount] = {};
  u32 descriptor_count = 0;
  u32 descriptor_index = 0;

  // Descriptor 0 - Uniform buffer
  u32 range = sizeof(VK_MaterialShaderInstUbo);
  u32 offset = sizeof(VK_MaterialShaderInstUbo) * material->internal_id;
  VK_MaterialShaderInstUbo inst_ubo;

  // Get diffuse color from a material
  inst_ubo.diffuse_color = material->diffuse_color;

  // Load the data into the buffer
  vk_buffer_load_data(&shader->obj_uniform_buffer, offset, range, 0, &inst_ubo);

  // Only do this if the descriptor has not yet been updated
  u32* global_ubo_generation = &object_state->descriptor_states[descriptor_count].generations[image_index];
  if (*global_ubo_generation == INVALID_ID || *global_ubo_generation != material->generation) {
    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = shader->obj_uniform_buffer.handle;
    buffer_info.offset = offset;
    buffer_info.range = range;

    VkWriteDescriptorSet descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descriptor.dstSet = object_descriptor_set;
    descriptor.dstBinding = descriptor_index;
    descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor.descriptorCount = 1;
    descriptor.pBufferInfo = &buffer_info;

    descriptor_writes[descriptor_count] = descriptor;
    ++descriptor_count;

    // Update the frame generation. In this case it is only needed since this is a buffer
    *global_ubo_generation = material->generation;
  }
  ++descriptor_index;

  // Samplers
  u32 sampler_count = 1;
  VkDescriptorImageInfo image_infos[1];
  Loop(sampler_index, sampler_count) {
    TextureUse use = shader->sampler_uses[sampler_index];
    Texture* t = null;
    switch (use) {
    case TextureUse_MapDiffuse: {
      t = material->diffuse_map.texture;
    } break;
    default:
      Error("Unable to bind sample to unknown use"_);
      return;
    }
    u32* descriptor_generation = &object_state->descriptor_states[descriptor_index].generations[image_index];
    u32* descriptor_id = &object_state->descriptor_states[descriptor_index].ids[image_index];

    // If the texture hasn't been loaded yet, use the default
    if (t->generation == INVALID_ID) {
      t = texture_system_get_default_texture();

      // Reset the descriptor generation if using the default texture
      *descriptor_generation = INVALID_ID;
    }

    // Check if the descriptor needs updating firs1t
    if (t && (*descriptor_id != t->id || *descriptor_generation != t->generation || *descriptor_generation == INVALID_ID)) {
      VK_TextureData* internal_data = (VK_TextureData*)t->internal_data;

      // Assign view and sampler
      image_infos[sampler_index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      image_infos[sampler_index].imageView = internal_data->image.view;
      image_infos[sampler_index].sampler = internal_data->sampler;

      VkWriteDescriptorSet descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
      descriptor.dstSet = object_descriptor_set;
      descriptor.dstBinding = descriptor_index;
      descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptor.descriptorCount = 1;
      descriptor.pImageInfo = &image_infos[sampler_index];

      descriptor_writes[descriptor_count] = descriptor;
      descriptor_count++;

      // Sync frame generation if not using a default texture
      if (t->generation != INVALID_ID) {
        *descriptor_generation = t->generation;
        *descriptor_id = t->id;
      }
      ++descriptor_index;
    }
  }

  if (descriptor_count > 0) {
    vkUpdateDescriptorSets(vkdevice, descriptor_count, descriptor_writes, 0, 0);
  }

  // Bind the descriptor set to be updated, or in case the shader changed
  vkCmdBindDescriptorSets(vk->render.cmds[vk->frame.image_index].handle, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.pipeline_layout, 1, 1, &object_descriptor_set, 0, 0);
}

void vk_material_shader_acquire_resources(VK_MaterialShader* shader, Material* material) {
  // TODO free list
  material->internal_id = shader->obj_uniform_buffer_index;
  ++shader->obj_uniform_buffer_index;
  
  VK_MaterialShaderInstState* object_state = &shader->instance_states[material->internal_id];
  Loop (i, MaterialShaderDescriptorCount) {
    Loop (j, 3) {
      object_state->descriptor_states[i].generations[j] = INVALID_ID;
      object_state->descriptor_states[i].ids[j] = INVALID_ID;
    }
  }
  
  // Allocate descriptor sets
  VkDescriptorSetLayout layouts[3] = {
    shader->obj_descriptor_set_layout,
    shader->obj_descriptor_set_layout,
    shader->obj_descriptor_set_layout};
  
  VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  alloc_info.descriptorPool = shader->obj_descriptor_pool;
  alloc_info.descriptorSetCount = 3; // one per frame
  alloc_info.pSetLayouts = layouts;
  VkResult result = vkAllocateDescriptorSets(vkdevice, &alloc_info, object_state->descriptor_sets);
  if (result != VK_SUCCESS) {
    Error("Error allocating descriptor sets in shader"_);
  }
}

void vk_material_shader_release_resources(VK_MaterialShader* shader, Material* material) {
  VK_MaterialShaderInstState* inst_state = &shader->instance_states[material->internal_id];
  
  u32 descriptor_set_count = 3;
  
  // Wait for any pending operations using the descriptor set to finish
  vkDeviceWaitIdle(vkdevice);

  // Release object descriptor sets
  VkResult result = vkFreeDescriptorSets(vkdevice, shader->obj_descriptor_pool, descriptor_set_count, inst_state->descriptor_sets);
  if (result != VK_SUCCESS) {
    Error("Error freeing object shader descriptor sets"_);
  }
  
  Loop (i, MaterialShaderDescriptorCount) {
    Loop (j, 3) {
      inst_state->descriptor_states[i].generations[j] = INVALID_ID;
      inst_state->descriptor_states[i].ids[j] = INVALID_ID;
    }
  }
  
  material->internal_id = INVALID_ID;
  
  // TODO add the object_id to the free list
}
