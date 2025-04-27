#pragma once
#include "vk_types.h"

String vk_result_string(VkResult result, b32 get_extended);

b32 vk_result_is_success(VkResult result);

void vk_imgui_init(void* data);
