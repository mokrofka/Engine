#pragma once

#include "render/vulkan/vk_types.h"

VK_UIShader vk_ui_shader_create();

void vk_ui_shader_destroy(VK_UIShader* shader);

void vk_ui_shader_use(VK_UIShader* shader);

void vk_ui_shader_update_global_state(VK_UIShader* shader, f32 delta_time);

void vk_ui_shader_set_model(VK_UIShader* shader, mat4 model);
void vk_ui_shader_apply_material(VK_UIShader* shader, Material* material);

void vk_ui_shader_acquire_resources(VK_UIShader* shader, Material* material);
void vk_ui_shader_release_resources(VK_UIShader* shader, Material* material);
