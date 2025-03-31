#include "vulkan_shader_utils.h"

#include <strings.h>
#include <platform.h>

b8 create_shader_module(
    VulkanContext* context, const char* name, const char* type_str,
    VkShaderStageFlagBits shader_stage_flag,
    u32 stage_index,
    VulkanShaderStage* shader_stages) {
  // Build file name
  char file_name[512];
  str_format((u8*)file_name, "assets/shaders/%s.%s.spv", name, type_str);
  MemZeroStruct(&shader_stages[stage_index].create_info);
  shader_stages[stage_index].create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

  // TODO: file input
  FileHandle handle;
  if (!filesystem_open(file_name, FILE_MODE_READ, &handle)) {
    Error("Unable to read shader module: %s.", file_name);
    return false;
  }
  
  {
    Temp scretch = GetScratch(0, 0);

    b8* file_buffer = 0;
    if (!filesystem_read_file(scretch.arena, &handle, &file_buffer)) {
      Error("Unable to binary read shader module: %s.", file_name);
      return false;
    }
    shader_stages[stage_index].create_info.codeSize = handle.size;
    shader_stages[stage_index].create_info.pCode = (u32*)file_buffer;

    filesystem_close(&handle);

    VK_CHECK(vkCreateShaderModule(
        context->device.logical_device,
        &shader_stages[stage_index].create_info,
        context->allocator,
        &shader_stages[stage_index].handle));
        
    ReleaseScratch(scretch);
  }

  // Shader stage info
  MemZeroStruct(&shader_stages[stage_index].shader_state_create_info);
  shader_stages[stage_index].shader_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stages[stage_index].shader_state_create_info.stage = shader_stage_flag;
  shader_stages[stage_index].shader_state_create_info.module = shader_stages[stage_index].handle;
  shader_stages[stage_index].shader_state_create_info.pName = "main";
  
  return true;
}
