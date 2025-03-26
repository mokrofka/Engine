#pragma once

#include "renderer/vulkan/vulkan_types.inl"
#include "renderer/renderer_types.inl"

b8 vulkan_object_shader_create(VulkanContext* context, VulkanObjectShader* out_shader);

void vulkan_object_shader_destroy(VulkanContext* context, VulkanObjectShader* shader);

void vulkan_object_shader_use(VulkanContext* context, VulkanObjectShader* shader);


