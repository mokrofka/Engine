#pragma once
#include "vk_types.h"

VK_ShaderStage shader_module_create(String name, String type_str, VkShaderStageFlagBits shader_stage_flag);
