#pragma once
#include "vk_types.h"

VK_ShaderStage shader_module_create(char* name, char* type_str, VkShaderStageFlagBits shader_stage_flag);
