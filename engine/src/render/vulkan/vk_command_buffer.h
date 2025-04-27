#pragma once
#include "vk_types.h"

VK_Cmd vk_cmd_alloc(VkCommandPool pool, b32 is_primary);

void vk_cmd_free(VkCommandPool pool, VK_Cmd* command_buffer);

void vk_cmd_begin(VK_Cmd* command_buffer, b32 is_single_use, b32 is_renderpass_continue, b32 is_simultaneous_use);

void vk_cmd_end(VK_Cmd* command_buffer);

void vk_cmd_update_submitted(VK_Cmd* command_buffer);

void vk_cmd_reset(VK_Cmd* command_buffer);

void vk_cmd_alloc_and_begin_single_use(VkCommandPool pool, VK_Cmd* out_command_buffer);

void vk_cmd_end_single_use(VkCommandPool pool, VK_Cmd* command_buffer, VkQueue queue);
