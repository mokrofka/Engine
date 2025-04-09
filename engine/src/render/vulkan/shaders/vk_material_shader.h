#pragma once

#include "render/vulkan/vk_types.h"

VK_MaterialShader vk_material_shader_create(VK_Context* vk);

void vk_material_shader_destroy(VK_MaterialShader* shader);

void vk_material_shader_use(VkCommandBuffer cmd, VK_MaterialShader shader);

void vk_material_shader_update_global_state(VkCommandBuffer cmd, VK_MaterialShader* shader, f32 delta_time);

void vk_material_shader_update_object(VkCommandBuffer cmd, VK_MaterialShader* shader, GeometryRenderData data);

void vk_material_shader_acquire_resources(VK_MaterialShader* shader, Material* material);
void vk_material_shader_release_resources(VK_MaterialShader* shader, Material* material);

