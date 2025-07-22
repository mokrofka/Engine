#include "vk_types.h"
#include "vk_draw.h"

void vk_draw_init() {
  Loop (i, FramesInFlight) {
    VkDescriptorSet descriptor_set = vk.descriptor_sets[i];
    
    // Storage
    VkDescriptorBufferInfo buffer_info = {
      .buffer = vk.storage_buffer.handle,
      .offset = vk.storage_buffer_range.offset,
      .range = vk.storage_buffer_range.size,
    };

    VkWriteDescriptorSet ubo_descriptor = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = descriptor_set,
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .pBufferInfo = &buffer_info,
    };
    
    // Texture
    VkDescriptorImageInfo image_info = {
      .sampler = vk.texture.sampler,
      .imageView = vk.texture.image.view,
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    VkWriteDescriptorSet texture_descriptor = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = descriptor_set,
      .dstBinding = 1,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = &image_info,
    };
    VkWriteDescriptorSet descriptors[] = {ubo_descriptor, texture_descriptor};
    vkUpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  }
}

void vk_screen_descriptor_update(u32 id) {
  // Loop (i, FramesInFlight) {
    VkDescriptorSet descriptor_set = vk.screen_descriptor_sets[vk.frame.current_frame];
    
    // Texture
    VkDescriptorImageInfo image_info = {
      .sampler = vk.texture_targets[id].sampler,
      .imageView = vk.texture_targets[id].image.view,
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    VkWriteDescriptorSet texture_descriptor = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = descriptor_set,
      .dstBinding = 1,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = &image_info,
    };

    VkWriteDescriptorSet descriptors[] = {texture_descriptor};
    vkUpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  // }
}

void vk_draw() {
  VkCommandBuffer cmd = vk_get_current_cmd();
  vk_draw_init(); // TODO add textures
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.shaders[0].pipeline.pipeline_layout, 0, 1, &vk.descriptor_sets[vk.frame.current_frame], 0, null);
  
  Loop (i, vk.shader_count) {
    VK_Shader shader = vk.shaders[i];
    
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.pipeline.handle);
    
    Loop (j, shader.sparse_set_entities.count) {
      u32 entity = shader.sparse_set_entities[j];
      
      u32 id = vk.entity_to_mesh.get_data(entity);
      VK_Mesh mesh = vk.meshes[vk.entity_to_mesh.get_data(entity)];
      // mesh.vert_count = 3;
      // Info("%i", mesh.vert_count);
      void* push_constant = vk.push_constants.get_data(entity);
      vkCmdPushConstants(cmd, shader.pipeline.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, vk.push_constants.element_size, push_constant);
      vkCmdBindVertexBuffers(cmd, 0, 1, &vk.vert_buffer.handle, &mesh.offset);
      vkCmdDraw(cmd, mesh.vert_count, 1, 0, 0);
      
    }
  }

}

void compute_descriptor_update() {
  i32 i = vk.frame.current_frame;
  VkWriteDescriptorSet descriptor_writes[2];

  VkDescriptorBufferInfo storage_buffer_info_last_frame = {
    .buffer = vk.compute_storage_buffers[i].handle,
    .offset = 0,
    .range = sizeof(Particle) * ParticleCount,
  };
  descriptor_writes[0] = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = vk.compute_descriptor_sets[i],
    .dstBinding = 1,
    .dstArrayElement = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    .pBufferInfo = &storage_buffer_info_last_frame,
  };

  VkDescriptorBufferInfo storage_buffer_info_current_frame = {
    .buffer = vk.compute_storage_buffers[(i + 1) % FramesInFlight].handle,
    .offset = 0,
    .range = sizeof(Particle) * ParticleCount,
  };
  descriptor_writes[1] = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = vk.compute_descriptor_sets[i],
    .dstBinding = 2,
    .dstArrayElement = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    .pBufferInfo = &storage_buffer_info_current_frame,
  };

  vkUpdateDescriptorSets(vkdevice, ArrayCount(descriptor_writes), descriptor_writes, 0, null);
}

void vk_compute_draw() {
  compute_descriptor_update();
  VkCommandBuffer cmd = vk.compute_cmds[vk.frame.current_frame];

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo))
  
  // UniformBufferObject* ubo; Assign(ubo, vk.compute_uniform_buffer.maped_memory);
  // ubo->projection_view = *vk.projection_view;
  // ubo->delta_time = delta_time;
  
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, vk.compute_shader.pipeline.handle);
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, vk.compute_shader.pipeline.pipeline_layout, 0, 1, &vk.compute_descriptor_sets[vk.frame.current_frame], 0, null);

  vkCmdDispatch(cmd, ParticleCount / 256, 1, 1);
  
  VK_CHECK(vkEndCommandBuffer(cmd))
}

void vk_draw_screen() {
  VkCommandBuffer cmd = vk_get_current_cmd();

  vk_screen_descriptor_update(vk.frame.image_index);
  // vk_draw_init();
  VkPipelineLayout pipeline_layout = vk.screen_shader.pipeline.pipeline_layout;
  VkPipeline pipeline = vk.screen_shader.pipeline.handle;

  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &vk.screen_descriptor_sets[vk.frame.current_frame], 0, null);
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  vkCmdDraw(cmd, 3, 1, 0, 0);
  
}
