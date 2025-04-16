#include "vk_shader_utils.h"

VK_ShaderStage shader_module_create(char* name, char* type_str, VkShaderStageFlagBits shader_stage_flag) {
  Scratch scratch;
  VK_ShaderStage shader_stage = {};
  // u8 file_name[512];
  // str_format(file_name, "assets/shaders/%s.%s.spv", name, type_str);
  String file_name = push_strf(scratch.arena, "assets/shaders/%s.%s.spv", name, type_str);
  shader_stage.create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

  // TODO: file input
  OS_Handle file = os_file_open(file_name, OS_AccessFlag_Read);
  
  {
    Scratch scratch;
    u64 file_size = os_file_size(file);
    u8* file_buffer = push_buffer(scratch.arena, u8, file_size);
    if (!os_file_read(file, file_size, file_buffer)) {
      Error("Unable to binary read shader module: %s.", file_name);
    }
    shader_stage.create_info.codeSize = file_size;
    shader_stage.create_info.pCode = (u32*)file_buffer;

    os_file_close(file);

    VK_CHECK(vkCreateShaderModule(vkdevice, &shader_stage.create_info, vk->allocator, &shader_stage.handle));
  }

  // Shader stage info
  shader_stage.shader_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stage.shader_state_create_info.stage = shader_stage_flag;
  shader_stage.shader_state_create_info.module = shader_stage.handle;
  shader_stage.shader_state_create_info.pName = "main";
  
  return shader_stage;
}
