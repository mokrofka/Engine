#include "vk_shader_utils.h"

#include "sys/res_sys.h"

VK_ShaderStage shader_module_create(String name, String type_str, VkShaderStageFlagBits shader_stage_flag) {
  Scratch scratch;
  VK_ShaderStage shader_stage = {};
  String file_path = push_strf(scratch, "shaders/%s.%s.spv", name, type_str);

  // Read the resource
  Res binary_resource;
  if (!res_sys_load(file_path, ResType_Binary, &binary_resource)) {
    Error("Unable to read shader module: %s", file_path);
  }
  // OS_Handle file = os_file_open(file_path, OS_AccessFlag_Read);

  // u64 file_size = os_file_size(file);
  // u8* file_buffer = push_buffer(scratch, u8, file_size);
  // if (!os_file_read(file, file_size, file_buffer)) {
  //   Error("Unable to binary read shader module: %s.", file_name);
  // }
  // os_file_close(file);
  shader_stage.create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shader_stage.create_info.codeSize = binary_resource.data_size;
  shader_stage.create_info.pCode = (u32*)binary_resource.data;


  VK_CHECK(vkCreateShaderModule(vkdevice, &shader_stage.create_info, vk->allocator, &shader_stage.handle));

  // Release the resource
  res_sys_unload(&binary_resource);
  
  // Shader stage info
  shader_stage.shader_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stage.shader_state_create_info.stage = shader_stage_flag;
  shader_stage.shader_state_create_info.module = shader_stage.handle;
  shader_stage.shader_state_create_info.pName = "main";
  
  return shader_stage;
}
