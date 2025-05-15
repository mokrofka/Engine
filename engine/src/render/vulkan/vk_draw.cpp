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

void descriptor_update() {
  VK_CommandBuffer cmd = vk_get_current_cmd();
  
  VkDescriptorSet descriptor_set = vk->descriptor_sets[vk->frame.image_index];
  VkDescriptorSet descriptor_set_new = vk->descriptor_sets_new[vk->frame.image_index];
  VkDescriptorSet descriptor_set_texture = vk->descriptor_sets_texture[vk->frame.image_index];
  
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->shader.pipeline.pipeline_layout, 0, 1, &descriptor_set, 0, null);
  // vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->shader.pipeline.pipeline_layout, 1, 1, &descriptor_set_new, 0, null);
  // vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->shader.pipeline.pipeline_layout, 1, 1, &descriptor_set_texture, 0, null);
  
  MemRange mem_range = vk->uniform_buffer_mem_range;
  MemRange mem_range_new = vk->uniform_buffer_mem_range_new;
  u64 range = mem_range.size;
  u64 offset = mem_range.offset;
  VkDescriptorBufferInfo buffer_info;
  buffer_info.buffer = vk->uniform_buffer.handle;
  buffer_info.offset = offset;
  buffer_info.range = range;
  
  u64 range_new = mem_range_new.size;
  u64 offset_new = mem_range_new.offset;
  VkDescriptorBufferInfo buffer_info_new;
  buffer_info_new.buffer = vk->uniform_buffer.handle;
  buffer_info_new.offset = offset_new;
  buffer_info_new.range = range_new;
  
  // Update descriptor sets
  VkWriteDescriptorSet descriptor_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  descriptor_write.dstSet = descriptor_set;
  descriptor_write.dstBinding = 0;
  descriptor_write.dstArrayElement = 0;
  descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptor_write.descriptorCount = 1;
  descriptor_write.pBufferInfo = &buffer_info;
  
  VkWriteDescriptorSet descriptor_write_new = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  descriptor_write_new.dstSet = descriptor_set_new;
  descriptor_write_new.dstBinding = 0;
  descriptor_write_new.dstArrayElement = 0;
  descriptor_write_new.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptor_write_new.descriptorCount = 1;
  descriptor_write_new.pBufferInfo = &buffer_info_new;
  
  // vkUpdateDescriptorSets(vkdevice, 1, &descriptor_write, 0, null);
  // vkUpdateDescriptorSets(vkdevice, 1, &descriptor_write_new, 0, null);
  
  VkDescriptorImageInfo image_info;
  image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  image_info.imageView = vk->texture.image.view;
  image_info.sampler = vk->texture.sampler;

  VkWriteDescriptorSet descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  descriptor.dstSet = descriptor_set;
  descriptor.dstBinding = 1;
  descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptor.descriptorCount = 1;
  descriptor.pImageInfo = &image_info;
  // vkUpdateDescriptorSets(vkdevice, 1, &descriptor, 0, null);
  VkWriteDescriptorSet writes[2] = {descriptor_write, descriptor};
  vkUpdateDescriptorSets(vkdevice, 2, writes, 0, null);
}

void vk_draw() {
  VK_CommandBuffer cmd = vk_get_current_cmd();
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->shader.pipeline.handle);
  descriptor_update();
  
  Loop (i, vk->entity_count) {
    u32 entity = vk->entities[i];
    VK_Mesh& mesh = vk->meshes[sparse.get_data(entity)];
    void* push_constant = vk->sparse_push_constants.get_data(entity);
    u32 push_constant_size = vk->sparse_push_constants.element_size;
    vkCmdPushConstants(cmd, vk->shader.pipeline.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, push_constant_size, push_constant);
    vkCmdBindVertexBuffers(cmd, 0, 1, &vk->vert_buffer.handle, &mesh.offset);
    vkCmdDraw(cmd, mesh.vert_count, 1, 0, 0);
  }
}

void vk_make_renderable(u32 id, u32 geom_id) {
  sparse.insert_data(id, geom_id);
  vk->sparse_push_constants.insert_data(id);
  vk->entities[vk->entity_count] = id;
  ++vk->entity_count;
}

KAPI void* vk_get_push_constant(u32 id) {
  return vk->sparse_push_constants.get_data(id);
}
