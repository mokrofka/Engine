#include "vk_draw.h"

mat4 model[10];

void setup_model() {
  model[0] = mat4_translation(v3(0.5,0.5,0));
  model[1] = mat4_translation(v3(-0.5,-0.5,0));
  // model[1] = mat4_translation(v3(0.,0.,0.));
}

void vk_draw() {
  VK_CommandBuffer cmd = vk_get_current_cmd();
  // vkCmdBindDescriptorSets(cmd.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->shader.pipeline.pipeline_layout, 0, 1, &vk->descriptor_set_layout, 0, null);
  local i32 loc = (setup_model(), 1);

  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->shader.pipeline.handle);
  
  Loop (i, vk->entity_count) {
    Entity entity = vk->entities[i];
    VK_GeometryData& geom = vk->geometries[entity.mesh_id];
    vkCmdPushConstants(cmd, vk->shader.pipeline.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &model[i]);
    vkCmdBindVertexBuffers(cmd, 0, 1, &vk->vert_buffer.handle, &geom.range.offset);
    vkCmdDraw(cmd, geom.vertex_count, 1, 0, 0);
  }
}

void vk_make_renderable(u32 id, u32 geom_id, u32 shader_id) {
  vk->entities[id].mesh_id = geom_id;
  ++vk->entity_count;
}
