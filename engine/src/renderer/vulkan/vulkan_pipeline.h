#pragma once

#include "vulkan_types.inl"

b8 vulkan_graphics_pipeline_create(
  VulkanContext* context,
  VulkanRenderPass* renderpass,
  u32 attribute_count,
  VkVertexInputAttributeDescription* attributes,
  u32 descriptor_set_layout_count,
  VkDescriptorSetLayout* descriptor_set_layouts,
  u32 stage_count,
  VkPipelineShaderStageCreateInfo* stages,
  VkViewport viewport,
  VkRect2D scissor,
  b8 is_wireframe,
  VulkanPipeline* pipeline);

void vulkan_pipeline_destroy(VulkanContext* context, VulkanPipeline* pipeline);

void vulkan_pipeline_bind(VulkanCommandBuffer* command_buffer, VkPipelineBindPoint bind_point, VulkanPipeline* pipeline);
