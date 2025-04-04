#pragma once

#include "render/vulkan/vk_types.h"

b8 vk_material_shader_create(VK_Context* context, VK_MaterialShader* out_shader);

void vk_material_shader_destroy(VK_Context* context, VK_MaterialShader* shader);

void vk_material_shader_use(VK_Context* context, VK_MaterialShader* shader);

void vk_material_shader_update_global_state(VK_Context* context, VK_MaterialShader* shader, f32 delta_time);

void vk_material_shader_update_object(VK_Context* context, VK_MaterialShader* shader, GeometryRenderData data);

b8 vk_material_shader_acquire_resources(VK_Context* context, VK_MaterialShader* shader, u32* out_object_id);
void vk_material_shader_release_resources(VK_Context* context, VK_MaterialShader* shader, u32 object_id);

