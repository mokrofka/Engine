#include "vk_types.h"

SparseSetIndex sparse_entity_to_mesh;
u32 entity_to_shader[1024];

void descriptor_update(u32 shader_id) {
  vk_Shader* shader = &vk.shaders[shader_id];
  VkCommandBuffer cmd = vk_get_current_cmd();
  
  VkDescriptorSet descriptor_set = vk.descriptor_sets[vk.frame.current_frame];
  
  MemRange mem_range = vk.uniform_buffer_mem_range;
  u64 range = mem_range.size;
  u64 offset = mem_range.offset;
  VkDescriptorBufferInfo buffer_info;
  buffer_info.buffer = vk.uniform_buffer.handle;
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
  image_info.imageView = vk.texture.image.view;
  image_info.sampler = vk.texture.sampler;

  VkWriteDescriptorSet texture_descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  texture_descriptor.dstSet = descriptor_set;
  texture_descriptor.dstBinding = 1;
  texture_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  texture_descriptor.descriptorCount = 1;
  texture_descriptor.pImageInfo = &image_info;
  VkWriteDescriptorSet descriptors[] = {ubo_descriptor, texture_descriptor};
  
  vkUpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.pipeline_layout, 0, 1, &descriptor_set, 0, null);
}

void vk_draw() {
  VkCommandBuffer cmd = vk_get_current_cmd();
  Loop (j, vk.shader_count) {
    vk_Shader* shader = &vk.shaders[j];
    SparseSetKeep* push_constants = &shader->push_constants;
    u32 push_size = push_constants->element_size;
    u32* entities = push_constants->entities;
    u32 entity_count = push_constants->size;
    
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.handle);
    descriptor_update(j); // TODO make descriptors
    
    Loop (i, entity_count) {
      u32 entity = entities[i];
      VK_Mesh& mesh = vk.meshes[sparse_entity_to_mesh.get_data(entity)];
      void* push_constant = push_constants->get_data(entity);
      vkCmdPushConstants(cmd, shader->pipeline.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, push_size, push_constant);
      vkCmdBindVertexBuffers(cmd, 0, 1, &vk.vert_buffer.handle, &mesh.offset);
      vkCmdDraw(cmd, mesh.vert_count, 1, 0, 0);
    }
  }
  
  // vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.graphics_shader_compute.pipeline.handle);
  
  // mat4 rotate = mat4_euler_x(deg_to_rad(90)) * mat4_identity();
  // vkCmdPushConstants(cmd, vk.graphics_shader_compute.pipeline.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &rotate);
  // VkDeviceSize offsets[] = {0};
  // vkCmdBindVertexBuffers(cmd, 0, 1, &vk.storage_buffers[vk.frame.current_frame].handle, offsets);

  // vkCmdDraw(cmd, ParticleCount, 1, 0, 0);
}

void vk_make_renderable(u32 id, u32 geom_id, u32 shader_id) {
  vk_Shader* shader = &vk.shaders[shader_id];
  shader->push_constants.insert_data(id);
  
  sparse_entity_to_mesh.insert_data(id, geom_id);
  entity_to_shader[id] = shader_id;
}

void vk_remove_renderable(u32 id) {
  vk_Shader* shader = &vk.shaders[entity_to_shader[id]];
  shader->push_constants.remove_data(id);
  sparse_entity_to_mesh.remove_data(id);
}

KAPI void* vk_get_push_constant(u32 id) {
  u32 shader_id = entity_to_shader[id];
  return vk.shaders[shader_id].push_constants.get_data(id);
}

void compute_descriptor_update() {
  i32 i = vk.frame.current_frame;
  VkWriteDescriptorSet descriptor_writes[2];

  VkDescriptorBufferInfo storage_buffer_info_last_frame{};
  storage_buffer_info_last_frame.buffer = vk.storage_buffers[i].handle;
  storage_buffer_info_last_frame.offset = 0;
  storage_buffer_info_last_frame.range = sizeof(Particle) * ParticleCount;

  descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor_writes[0].dstSet = vk.compute_descriptor_sets[i];
  descriptor_writes[0].dstBinding = 1;
  descriptor_writes[0].dstArrayElement = 0;
  descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptor_writes[0].descriptorCount = 1;
  descriptor_writes[0].pBufferInfo = &storage_buffer_info_last_frame;

  VkDescriptorBufferInfo storage_buffer_info_current_frame{};
  storage_buffer_info_current_frame.buffer = vk.storage_buffers[(i + 1) % FramesInFlight].handle;
  storage_buffer_info_current_frame.offset = 0;
  storage_buffer_info_current_frame.range = sizeof(Particle) * ParticleCount;

  descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor_writes[1].dstSet = vk.compute_descriptor_sets[i];
  descriptor_writes[1].dstBinding = 2;
  descriptor_writes[1].dstArrayElement = 0;
  descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptor_writes[1].descriptorCount = 1;
  descriptor_writes[1].pBufferInfo = &storage_buffer_info_current_frame;

  vkUpdateDescriptorSets(vkdevice, 2, descriptor_writes, 0, null);
}

void vk_compute_draw() {
  compute_descriptor_update();
  VkCommandBuffer cmd = vk.compute_cmds[vk.frame.current_frame];

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo))
  
  UniformBufferObject* ubo; Assign(ubo, vk.compute_uniform_buffer.maped_memory);
  ubo->projection_view = *vk.projection_view;
  ubo->delta_time = delta_time;
  
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, vk.compute_shader.pipeline.handle);
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, vk.compute_shader.pipeline.pipeline_layout, 0, 1, &vk.compute_descriptor_sets[vk.frame.current_frame], 0, null);

  vkCmdDispatch(cmd, ParticleCount / 256, 1, 1);
  
  VK_CHECK(vkEndCommandBuffer(cmd))
}
