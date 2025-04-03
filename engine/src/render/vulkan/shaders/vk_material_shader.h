#pragma once

#include "render/vulkan/vk_types.h"
#include "render/r_types.inl"

b8 vulkan_material_shader_create(VK_Context* context, VK_MaterialShader* out_shader);

void vulkan_material_shader_destroy(VK_Context* context, VK_MaterialShader* shader);

void vulkan_material_shader_use(VK_Context* context, VK_MaterialShader* shader);

void vulkan_material_shader_update_global_state(VK_Context* context, struct VK_MaterialShader* shader, f32 delta_time);

void vulkan_material_shader_update_object(VK_Context* context, VK_MaterialShader* shader, GeometryRenderData data);

b8 vulkan_material_shader_acquire_resources(VK_Context* context, struct VK_MaterialShader* shader, u32* out_object_id);
void vulkan_material_shader_release_resources(VK_Context* context, struct VK_MaterialShader* shader, u32 object_id);

