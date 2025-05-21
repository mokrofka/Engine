#include "vk_shader_utils.h"

#include "sys/res_sys.h"

VK_ShaderStage shader_module_create(String name, String type_str, VkShaderStageFlagBits shader_stage_flag) {
  Scratch scratch;
  VK_ShaderStage shader_stage = {};
  String file_path = push_strf(scratch, "shaders/compiled/%s.%s.spv", name, type_str);
  
  Binary binary = res_binary_load(scratch, file_path);
  if (!binary.data) {
    Error("Unable to read shader module: %s", file_path);
  }
  
  shader_stage.create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shader_stage.create_info.codeSize = binary.size;
  shader_stage.create_info.pCode = (u32*)binary.data;

  VK_CHECK(vkCreateShaderModule(vkdevice, &shader_stage.create_info, vk.allocator, &shader_stage.handle));
  
  // Shader stage info
  shader_stage.shader_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stage.shader_state_create_info.stage = shader_stage_flag;
  shader_stage.shader_state_create_info.module = shader_stage.handle;
  shader_stage.shader_state_create_info.pName = "main";
  
  return shader_stage;
}
