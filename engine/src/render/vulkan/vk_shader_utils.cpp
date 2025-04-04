#include "vk_shader_utils.h"

#include <str.h>
#include <os.h>

b8 create_shader_module(
    VK_Context* context, const char* name, const char* type_str,
    VkShaderStageFlagBits shader_stage_flag,
    u32 stage_index,
    VK_ShaderStage* shader_stages) {
  // Build file name
  char file_name[512];
  str_format((u8*)file_name, "assets/shaders/%s.%s.spv", name, type_str);
  MemZeroStruct(&shader_stages[stage_index].create_info);
  shader_stages[stage_index].create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

  // TODO: file input
  OS_File handle = os_file_open(str_lit(file_name), FILE_MODE_READ);
  if (!handle.u64) {
  // if (!os_file_open(file_name, FILE_MODE_READ, &handle)) {
    Error("Unable to read shader module: %s.", file_name);
    return false;
  }
  
  {
    Scratch scratch;
    u64 file_size = os_file_size(handle);
    u8* file_buffer = push_buffer(scratch, u8, file_size);
    if (!os_file_read(handle, file_size, file_buffer)) {
      Error("Unable to binary read shader module: %s.", file_name);
      return false;
    }
    shader_stages[stage_index].create_info.codeSize = file_size;
    shader_stages[stage_index].create_info.pCode = (u32*)file_buffer;

    os_file_close(handle);

    VK_CHECK(vkCreateShaderModule(
        vkdevice,
        &shader_stages[stage_index].create_info,
        context->allocator,
        &shader_stages[stage_index].handle));
  }

  // Shader stage info
  MemZeroStruct(&shader_stages[stage_index].shader_state_create_info);
  shader_stages[stage_index].shader_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stages[stage_index].shader_state_create_info.stage = shader_stage_flag;
  shader_stages[stage_index].shader_state_create_info.module = shader_stages[stage_index].handle;
  shader_stages[stage_index].shader_state_create_info.pName = "main";
  
  return true;
}
