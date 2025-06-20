#include "vk_buffer.h"
#include "vk_command_buffer.h"

VK_Buffer vk_buffer_create(u64 size, u32 usage, u32 memory_property_flags) {
  VkBufferCreateInfo buffer_create_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = size,
    .usage = usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };

  VK_Buffer buffer = {
    .size = size,
    .usage = usage,
    .memory_property_flags = memory_property_flags,
  };
  
  VK_CHECK(vkCreateBuffer(vkdevice, &buffer_create_info, vk.allocator, &buffer.handle));

  VkMemoryRequirements requirements;
  vkGetBufferMemoryRequirements(vkdevice, buffer.handle, &requirements);
  buffer.memory_index = vk_find_memory_index(requirements.memoryTypeBits, buffer.memory_property_flags);
  
  VkMemoryAllocateInfo allocate_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = requirements.size,
    .memoryTypeIndex = buffer.memory_index,
  };
  
  VK_CHECK(vkAllocateMemory(vkdevice, &allocate_info, vk.allocator, &buffer.memory));
  VK_CHECK(vkBindBufferMemory(vkdevice, buffer.handle, buffer.memory, 0));
  
  return buffer;
}

void vk_buffer_destroy(VK_Buffer& buffer) {
  Assert(buffer.memory && buffer.handle);
  vkFreeMemory(vkdevice, buffer.memory, vk.allocator);
  vkDestroyBuffer(vkdevice, buffer.handle, vk.allocator);
}

void vk_buffer_map_memory(VK_Buffer& buffer, u64 offset, u64 size) {
  VK_CHECK(vkMapMemory(vkdevice, buffer.memory, offset, size, 0, (void**)&buffer.maped_memory));
}

void vk_buffer_unmap_memory(VK_Buffer& buffer) {
  vkUnmapMemory(vkdevice, buffer.memory);
}

void vk_upload_to_gpu(VK_Buffer& buffer, Range range, void* data) {
  MemCopy(vk.stage_buffer.maped_memory, data, range.size);

  VkCommandBuffer temp_cmd = vk_cmd_alloc_and_begin_single_use();
  
  VkBufferCopy copy_region = {
    .srcOffset = 0,
    .dstOffset = range.offset,
    .size = range.size,
  };
  
  vkCmdCopyBuffer(temp_cmd, vk.stage_buffer.handle, buffer.handle, 1, &copy_region);
  
  vk_cmd_end_single_use(temp_cmd);
}
