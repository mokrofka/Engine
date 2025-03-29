#pragma once

#include "renderer/vulkan/vulkan_types.inl"
#include "renderer/renderer_types.inl"

b8 vulkan_object_shader_create(VulkanContext* context, Texture* default_diffuse, VulkanObjectShader* out_shader);

void vulkan_object_shader_destroy(VulkanContext* context, VulkanObjectShader* shader);

void vulkan_object_shader_use(VulkanContext* context, VulkanObjectShader* shader);

void vulkan_object_shader_update_global_state(VulkanContext* context, struct VulkanObjectShader* shader, f32 delta_time);

void vulkan_object_shader_update_object(VulkanContext* context, VulkanObjectShader* shader, GeometryRenderData data);

b8 vulkan_object_shader_acquire_resources(VulkanContext* context, struct VulkanObjectShader* shader, u32* out_object_id);
void vulkan_object_shader_release_resources(VulkanContext* context, struct VulkanObjectShader* shader, u32 object_id);

