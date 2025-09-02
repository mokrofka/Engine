#pragma once
#include "vk_types.h"

VkCommandBuffer vk_cmd_alloc(VkCommandPool pool);
void vk_cmd_free(VkCommandPool pool, VkCommandBuffer cmd);

void vk_cmd_begin(VkCommandBuffer cmd);
void vk_cmd_end(VkCommandBuffer cmd);

VkCommandBuffer vk_cmd_alloc_and_begin_single_use();
void vk_cmd_end_single_use(VkCommandBuffer cmd);
