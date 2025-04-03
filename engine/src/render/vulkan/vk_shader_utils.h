#pragma once

#include "vk_types.h"

b8 create_shader_module(
    VK_Context* context, const char* name, const char* type_str,
    VkShaderStageFlagBits shader_stage_flag,
    u32 stage_index,
    VK_ShaderStage* shader_stages);
    
    