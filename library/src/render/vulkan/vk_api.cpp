#include "vk_api.h"

////////////////////////////////////////////////////////////////////////
// Entity
KAPI void entity_make_renderable(u32 entity_id, u32 geom_id, u32 shader_id) {
  VK_Shader* shader = &vk.shaders[shader_id];
  shader->sparse_set.add(entity_id);
  
  vk.entity_to_mesh.insert_data(entity_id, geom_id);
  vk.entity_to_shader[entity_id] = shader_id;
  
  vk.push_constants.add(entity_id);
  PushConstant* push; Assign(push, vk.push_constants.get_data(entity_id));
  push->entity_index = entity_id;
}

KAPI void entity_remove_renderable(u32 entity_id) {
  VK_Shader* shader = &vk.shaders[vk.entity_to_shader[entity_id]];
  shader->sparse_set.remove(entity_id);

  vk.push_constants.remove_data(entity_id);
  vk.entity_to_mesh.remove_data(entity_id);
}

KAPI ShaderEntity* shader_get_entity(u32 entity_id) {
  return &vk.entities_data[entity_id];
}

////////////////////////////////////////////////////////////////////////
// Point light
KAPI void entity_make_point_light(u32 entity_id) {
  vk.point_light_data.add(entity_id);
  ++vk.global_shader_state->point_light_count;
}

KAPI void entity_remove_point_light(u32 entity_id) {
  vk.point_light_data.remove_data(entity_id);
  --vk.global_shader_state->point_light_count;
}

KAPI PointLight* shader_get_point_light(u32 entity_id) {
  return (PointLight*)vk.point_light_data.get_data(entity_id);
}

////////////////////////////////////////////////////////////////////////
// Directional light
KAPI void entity_make_dir_light(u32 entity_id) {
  vk.dir_light_data.add(entity_id);
  ++vk.global_shader_state->dir_light_count;
}

KAPI void entity_remove_dir_light(u32 entity_id) {
  vk.dir_light_data.remove_data(entity_id);
  --vk.global_shader_state->dir_light_count;
}

KAPI DirLight* shader_get_dir_light(u32 entity_id) {
  return (DirLight*)vk.dir_light_data.get_data(entity_id);
}

////////////////////////////////////////////////////////////////////////
// Spot light
KAPI void entity_make_spot_light(u32 entity_id) {
  vk.spot_light_data.add(entity_id);
  ++vk.global_shader_state->spot_light_count;
}

KAPI void entity_remove_spot_light(u32 entity_id) {
  vk.spot_light_data.remove_data(entity_id);
  --vk.global_shader_state->spot_light_count;
}

KAPI SpotLight* shader_get_spot_light(u32 entity_id) {
  return (SpotLight*)vk.spot_light_data.get_data(entity_id);
}

////////////////////////////////////////////////////////////////////////
// Util
KAPI PushConstant* get_push_constant(u32 entity_id) {
  return (PushConstant*)vk.push_constants.get_data(entity_id);
}

KAPI ShaderGlobalState* shader_get_global_state() {
  return (ShaderGlobalState*)vk.storage_buffer.maped_memory;
}

b32 vk_is_viewport_render() {
  return vk.is_viewport_render;
}
