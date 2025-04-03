#pragma once

#include "vk_types.h"

b8 vk_graphics_pipeline_create(
  VK_Context* context,
  VK_RenderPass * renderpass,
  u32 attribute_count,
  VkVertexInputAttributeDescription* attributes,
  u32 descriptor_set_layout_count,
  VkDescriptorSetLayout* descriptor_set_layouts,
  u32 stage_count,
  VkPipelineShaderStageCreateInfo* stages,
  VkViewport viewport,
  VkRect2D scissor,
  b8 is_wireframe,
  VK_Pipeline* pipeline);

void vk_pipeline_destroy(VK_Context* context, VK_Pipeline* pipeline);

void vk_pipeline_bind(VK_CommandBuffer* command_buffer, VkPipelineBindPoint bind_point, VK_Pipeline* pipeline);
