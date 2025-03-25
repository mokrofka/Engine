#pragma once

#include "vulkan_types.inl"

b8 create_shader_module(
    VulkanContext* context, const char* name, const char* type_str,
    VkShaderStageFlagBits shader_stage_flag,
    u32 stage_index,
    VulkanShaderStage* shader_stages);
    
    