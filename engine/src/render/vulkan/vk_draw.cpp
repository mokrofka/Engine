#include "vk_types.h"

struct SparseSetIndex {
  u32 indexes[MaxEntities];
  u32 entity_to_index[MaxEntities];  
  u32 index_to_entity[MaxEntities];  
  u32 size;
  
  inline void insert_data(u32 entity, u32 id) {
    // Put new entry at end and update the maps
    u32 new_index = size;
    entity_to_index[entity] = new_index;
    index_to_entity[new_index] = entity;
    indexes[new_index] = id;
    ++size;
  }
  inline void remove_data(u32 entity) {
    // Copy element at end into deleted element's place to maintain density
    u32 index_of_removed_entity = entity_to_index[entity];
    u32 index_of_last_element = size - 1;
    indexes[index_of_removed_entity] = indexes[index_of_last_element];

    // Update map to point to moved spot
    u32 entity_of_last_element = index_to_entity[index_of_last_element];
    entity_to_index[entity_of_last_element] = index_of_removed_entity;
    index_to_entity[index_of_removed_entity] = entity_of_last_element;

    entity_to_index[entity] = INVALID_ID;
    index_to_entity[index_of_last_element] = INVALID_ID;

    --size;
  }
  inline u32 get_data(u32 entity) {
    u32 index = entity_to_index[entity];
    return indexes[index];
  }
};
SparseSetIndex sparse;
SparseSetIndex entity_shader;

void descriptor_update(u32 shader_id) {
  vk_Shader* shader = &vk->shaders[shader_id];
  VK_CommandBuffer cmd = vk_get_current_cmd();
  
  VkDescriptorSet descriptor_set = vk->descriptor_sets[vk->frame.image_index];
  
  MemRange mem_range = vk->uniform_buffer_mem_range;
  u64 range = mem_range.size;
  u64 offset = mem_range.offset;
  VkDescriptorBufferInfo buffer_info;
  buffer_info.buffer = vk->uniform_buffer.handle;
  buffer_info.offset = offset;
  buffer_info.range = range;

  // Update descriptor sets
  VkWriteDescriptorSet ubo_descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  ubo_descriptor.dstSet = descriptor_set;
  ubo_descriptor.dstBinding = 0;
  ubo_descriptor.dstArrayElement = 0;
  ubo_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  ubo_descriptor.descriptorCount = 1;
  ubo_descriptor.pBufferInfo = &buffer_info;
  
  VkDescriptorImageInfo image_info;
  image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  image_info.imageView = vk->texture.image.view;
  image_info.sampler = vk->texture.sampler;

  VkWriteDescriptorSet texture_descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  texture_descriptor.dstSet = descriptor_set;
  texture_descriptor.dstBinding = 1;
  texture_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  texture_descriptor.descriptorCount = 1;
  texture_descriptor.pImageInfo = &image_info;
  VkWriteDescriptorSet descriptors[] = {ubo_descriptor, texture_descriptor};
  
  vkUpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  // vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->shader.pipeline.pipeline_layout, 0, 1, &descriptor_set, 0, null);
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.pipeline_layout, 0, 1, &descriptor_set, 0, null);
}

void vk_draw() {
  VK_CommandBuffer cmd = vk_get_current_cmd();
  Loop (j, vk->shader_count) {
    vk_Shader* shader = &vk->shaders[j];
    SparseSetKeep* push_constants = &shader->push_constants;
    
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.handle);
    u32* entities = shader->entities;
    u32 entity_count = shader->entity_count;
    descriptor_update(j); // TODO make descriptors
    
    Loop (i, entity_count) {
      u32 entity = entities[i];
      VK_Mesh& mesh = vk->meshes[sparse.get_data(entity)];
      void* push_constant = push_constants->get_data(entity);
      u32 push_constant_size = push_constants->element_size;
      vkCmdPushConstants(cmd, shader->pipeline.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, push_constant_size, push_constant);
      vkCmdBindVertexBuffers(cmd, 0, 1, &vk->vert_buffer.handle, &mesh.offset);
      vkCmdDraw(cmd, mesh.vert_count, 1, 0, 0);
    }
  }
}

void vk_make_renderable(u32 id, u32 geom_id, u32 shader_id) {
  u32 entity_count = vk->shaders[shader_id].entity_count++;
  vk_Shader* shader = &vk->shaders[shader_id];
  shader->entities[entity_count] = id;
  shader->push_constants.insert_data(id);
  
  sparse.insert_data(id, geom_id);
  entity_shader.insert_data(id, shader_id);
}

void vk_remove_renderable(u32 id) {

}

KAPI void* vk_get_push_constant(u32 id) {
  u32 shader_id = entity_shader.get_data(id);
  return vk->shaders[shader_id].push_constants.get_data(id);
}
