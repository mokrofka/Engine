#include "vk_types.h"
#include "vk_draw.h"
#include "ecs.h"


void descriptor_update(u32 shader_id) {
  vk_Shader* shader = &vk.shaders[shader_id];
  VkCommandBuffer cmd = vk_get_current_cmd();
  
  VkDescriptorSet descriptor_set = vk.descriptor_sets[vk.frame.current_frame];
  
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
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.pipeline_layout, 0, 1, &descriptor_set, 0, null);
}

void vk_draw() {
  push_constants_update();
  light_update();

  VkCommandBuffer cmd = vk_get_current_cmd();
  Loop (j, vk.shader_count) {
    vk_Shader* shader = &vk.shaders[j];
    u32 push_size = vk.push_constants.element_size;
    u32* entities = shader->sparse_set.entities;
    u32 entity_count = shader->sparse_set.entity_count;
    
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.handle);
    descriptor_update(j); // TODO make descriptors
    
    Loop (i, entity_count) {
      u32 entity = entities[i];
      VK_Mesh& mesh = vk.meshes[vk.entity_to_mesh.get_data(entity)];
      void* push_constant = vk.push_constants.get_data(entity);
      vkCmdPushConstants(cmd, shader->pipeline.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT , 0, push_size, push_constant);
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

void vk_make_renderable(u32 entity_id, u32 geom_id, u32 shader_id) {
  vk_Shader* shader = &vk.shaders[shader_id];
  shader->sparse_set.add(entity_id);
  
  vk.push_constants.insert_data(entity_id);
  PushConstant* push = (PushConstant*)vk.push_constants.get_data(entity_id);
  push->entity_index = entity_id;
  vk.entity_to_mesh.insert_data(entity_id, geom_id);
  vk.entity_to_shader[entity_id] = shader_id;

  vk.entities_data.insert_data(entity_id);
}

void vk_make_light(u32 entity_id) {
  vk.lights_data.insert_data(entity_id);
}

void vk_remove_renderable(u32 entity_id) {
  vk_Shader* shader = &vk.shaders[vk.entity_to_shader[entity_id]];
  shader->sparse_set.remove(entity_id);

  vk.push_constants.remove_data(entity_id);
  vk.entity_to_mesh.remove_data(entity_id);

  vk.entities_data.remove_data(entity_id);
}

KAPI PushConstant* vk_get_push_constant(u32 entity_id) {
  return (PushConstant*)vk.push_constants.get_data(entity_id);
}

void compute_descriptor_update() {
  i32 i = vk.frame.current_frame;
  VkWriteDescriptorSet descriptor_writes[2];

  VkDescriptorBufferInfo storage_buffer_info_last_frame = {
    .buffer = vk.storage_buffers[i].handle,
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
    .buffer = vk.storage_buffers[(i + 1) % FramesInFlight].handle,
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
  
  UniformBufferObject* ubo; Assign(ubo, vk.compute_uniform_buffer.maped_memory);
  ubo->projection_view = *vk.projection_view;
  ubo->delta_time = delta_time;
  
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, vk.compute_shader.pipeline.handle);
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, vk.compute_shader.pipeline.pipeline_layout, 0, 1, &vk.compute_descriptor_sets[vk.frame.current_frame], 0, null);

  vkCmdDispatch(cmd, ParticleCount / 256, 1, 1);
  
  VK_CHECK(vkEndCommandBuffer(cmd))
}

KAPI ShaderGlobalState* shader_get_global_state() {
  return (ShaderGlobalState*)vk.storage_buffer.maped_memory;
}

KAPI ShaderEntity* shader_get_entity_data(u32 entity_id) {
  return (ShaderEntity*)(vk.entities_data.get_data(entity_id));
}

KAPI DirectionalLight* shader_get_light_data(u32 entity_id) {
  return (DirectionalLight*)vk.lights_data.get_data(entity_id);
}

System(PushConstantUpdate, Transform)
void push_constants_update() {
  BaseSystem* system = system_get(PushConstantUpdate);
  Loop (i, system->entity_count) {
    Entity e = system->entities[i];
    PushConstant* push = vk_get_push_constant(e);
    Transform* trans = entity_get_component(e, Transform);
    push->model = mat4_transform(*trans);
  }
}

Component(DirectionalLight)
System(LightUpdate, Transform DirectionalLight)
void light_update() {
  BaseSystem* system = system_get(LightUpdate);
  Loop (i, system->entity_count) {
    Entity e = system->entities[i];
    DirectionalLight* light_data = shader_get_light_data(e);
    Transform* trans = entity_get_component(e, Transform);
    DirectionalLight* dir_light = entity_get_component(e, DirectionalLight);
    trans->pos = dir_light->pos;
    light_data->pos = dir_light->pos;
    light_data->color = dir_light->color;
    ShaderEntity* shader_e = shader_get_entity_data(e);
    shader_e->color = light_data->color;
  }
}

b32 vk_is_viewport_render() {
  return vk.is_viewport_render;
}
