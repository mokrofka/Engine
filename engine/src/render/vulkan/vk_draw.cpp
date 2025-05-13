#include "vk_types.h"

mat4 model[10];

struct PushUbo {
  mat4 model;
  u32 index;
};

PushUbo ubos[10];

void setup_model() {
  // ubos[0].model = mat4_translation();
  // ubos[0].index = 0;
  // ubos[1].model = mat4_translation(v3(-0.5,-0.5,0));
  // ubos[1].index = 0;
  
  // model[1] = mat4_translation(v3(0.,0.,0.));
}

void descriptor_update() {
  VK_CommandBuffer cmd = vk_get_current_cmd();
  
  VkDescriptorSet descriptor_set = vk->descriptor_sets[vk->frame.image_index];
  VkDescriptorSet descriptor_set_new = vk->descriptor_sets_new[vk->frame.image_index];
  VkDescriptorSet sets[2] = {
    descriptor_set,
    descriptor_set_new
  };
  // vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->shader.pipeline.pipeline_layout, 0, 2, sets, 0, null);
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->shader.pipeline.pipeline_layout, 0, 1, &descriptor_set, 0, null);
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->shader.pipeline.pipeline_layout, 1, 1, &descriptor_set_new, 0, null);
  
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
  vkUpdateDescriptorSets(vkdevice, 1, &descriptor_write, 0, null);
  vkUpdateDescriptorSets(vkdevice, 1, &descriptor_write_new, 0, null);
}

void vk_draw() {
  VK_CommandBuffer cmd = vk_get_current_cmd();
  // vkCmdBindDescriptorSets(cmd.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->shader.pipeline.pipeline_layout, 0, 1, &vk->descriptor_set_layout, 0, null);
  local i32 loc = (setup_model(), 1);

  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->shader.pipeline.handle);
  descriptor_update();
  
  Loop (i, vk->entity_count) {
    Entity entity = vk->entities[i];
    VK_GeometryData& geom = vk->geometries[entity.mesh_id];
    // vkCmdPushConstants(cmd, vk->shader.pipeline.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushUbo), &ubos[i]);
    vkCmdBindVertexBuffers(cmd, 0, 1, &vk->vert_buffer.handle, &geom.range.offset);
    vkCmdDraw(cmd, geom.vertex_count, 1, 0, 0);
  }
}

void vk_make_renderable(u32 id, u32 geom_id, u32 shader_id) {
  vk->entities[id].mesh_id = geom_id;
  ++vk->entity_count;
}
