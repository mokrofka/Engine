#pragma once

#include "renderer/vulkan/vulkan_types.inl"
#include "renderer/renderer_types.inl"

b8 vulkan_material_shader_create(VulkanContext* context, VulkanMaterialShader* out_shader);

void vulkan_material_shader_destroy(VulkanContext* context, VulkanMaterialShader* shader);

void vulkan_material_shader_use(VulkanContext* context, VulkanMaterialShader* shader);

void vulkan_material_shader_update_global_state(VulkanContext* context, struct VulkanMaterialShader* shader, f32 delta_time);

void vulkan_material_shader_update_object(VulkanContext* context, VulkanMaterialShader* shader, GeometryRenderData data);

b8 vulkan_material_shader_acquire_resources(VulkanContext* context, struct VulkanMaterialShader* shader, u32* out_object_id);
void vulkan_material_shader_release_resources(VulkanContext* context, struct VulkanMaterialShader* shader, u32 object_id);

