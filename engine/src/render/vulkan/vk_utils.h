#pragma once
#include "vk_types.h"

const char* vk_result_string(VkResult result, b8 get_extended);

b8 vk_result_is_success(VkResult result);
