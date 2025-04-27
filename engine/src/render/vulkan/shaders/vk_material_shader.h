#pragma once

#include "render/vulkan/vk_types.h"

VK_MaterialShader vk_material_shader_create();

void vk_material_shader_destroy(VK_MaterialShader* shader);

void vk_material_shader_use(VK_MaterialShader* shader);

void vk_material_shader_update_global_state(VK_MaterialShader* shader, f32 delta_time);

void vk_material_shader_set_model(VK_MaterialShader* shader, mat4 model);
void vk_material_shader_apply_material(VK_MaterialShader* shader, Material* data);

void vk_material_shader_acquire_resources(VK_MaterialShader* shader, Material* material);
void vk_material_shader_release_resources(VK_MaterialShader* shader, Material* material);

