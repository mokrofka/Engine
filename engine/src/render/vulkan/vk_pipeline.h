#pragma once

#include "vk_types.h"

VK_Pipeline vk_graphics_pipeline_create(
  VK_RenderPass renderpass,
  u32 attribute_count,
  VkVertexInputAttributeDescription* attributes,
  u32 descriptor_set_layout_count,
  VkDescriptorSetLayout* descriptor_set_layouts,
  u32 stage_count,
  VkPipelineShaderStageCreateInfo* stages,
  VkViewport viewport,
  VkRect2D scissor,
  b8 is_wireframe);

void vk_pipeline_destroy(VK_Pipeline pipeline);

void vk_pipeline_bind(VkCommandBuffer cmd, VkPipelineBindPoint bind_point, VK_Pipeline pipeline);
