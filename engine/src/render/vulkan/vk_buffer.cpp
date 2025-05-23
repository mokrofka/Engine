#include "vk_buffer.h"
#include "vk_command_buffer.h"

VK_Buffer vk_buffer_create(u64 size, u32 usage, u32 memory_property_flags) {
  VK_Buffer buffer = {};
  buffer.size = size;
  buffer.usage = usage;
  buffer.memory_property_flags = memory_property_flags;
  
  VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  buffer_info.size = size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // NOTE only used in one queue
  
  VK_CHECK(vkCreateBuffer(vkdevice, &buffer_info, vk.allocator, &buffer.handle));

  // Gather memory requirements
  VkMemoryRequirements requirements;
  vkGetBufferMemoryRequirements(vkdevice, buffer.handle, &requirements);
  buffer.memory_index = vk_find_memory_index(requirements.memoryTypeBits, buffer.memory_property_flags);
  
  // Allocate memory info
  VkMemoryAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  allocate_info.allocationSize = requirements.size;
  allocate_info.memoryTypeIndex = (u32)buffer.memory_index;
  
  // Allocate the memory.
  VkResult result = vkAllocateMemory(vkdevice, &allocate_info, vk.allocator, &buffer.memory);
  if (result != VK_SUCCESS) {
    Error("Unable to create vulkan buffer because the required memory allocation failed. Error: %i", result);
  }
  
  VK_CHECK(vkBindBufferMemory(vkdevice, buffer.handle, buffer.memory, 0));
  
  return buffer;
}

void vk_buffer_destroy(VK_Buffer* buffer) {
  Assert(buffer->memory && buffer->handle);
  vkFreeMemory(vkdevice, buffer->memory, vk.allocator);
  vkDestroyBuffer(vkdevice, buffer->handle, vk.allocator);
  
  buffer->memory = 0;
  buffer->handle = 0;
  buffer->size = 0;
  buffer->usage = 0;
}

// b32 vk_buffer_resize(u64 new_size, VK_Buffer* buffer, VkQueue queue, VkCommandPool pool) {

//   // Create new buffer
//   VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
//   buffer_info.size = new_size;
//   buffer_info.usage = buffer->usage;
//   buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  
//   VkBuffer new_buffer;
//   VK_CHECK(vkCreateBuffer(vkdevice, &buffer_info, vk.allocator, &new_buffer));
  
//   // Gather memory requirements
//   VkMemoryRequirements requirements;
//   vkGetBufferMemoryRequirements(vkdevice, new_buffer, &requirements);
  
//   // Allocate memory info
//   VkMemoryAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
//   allocate_info.allocationSize = requirements.size;
//   allocate_info.memoryTypeIndex = (u32)buffer->memory_index;
  
//   // Allocate the memory
//   VkDeviceMemory new_memory;
//   VkResult result = vkAllocateMemory(vkdevice, &allocate_info, vk.allocator, &new_memory);
//   if (result != VK_SUCCESS) {
//     Error("Unable to resize vulkan buffer because the required memory allocation failed. Error: %i", result);
//     return false;
//   }
  
//   // Bind the new buffer's memory
//   VK_CHECK(vkBindBufferMemory(vkdevice, new_buffer, new_memory, 0));
  
//   // Copy over the data
//   // vk_buffer_copy_to(buffer, 0, &new_buffer, 0, buffer->size);
  
//   // Make sure anything potentially using these if finished
//   vkDeviceWaitIdle(vkdevice);
  
//   // Destroy the old
//   vkFreeMemory(vkdevice, buffer->memory, vk.allocator);
//   buffer->memory = 0;
//   vkDestroyBuffer(vkdevice, buffer->handle, vk.allocator);
//   buffer->handle = 0;
  
//   // Set new properties
//   buffer->size = new_size;
//   buffer->memory = new_memory;
//   buffer->handle = new_buffer;
//   return true;
// }

void vk_buffer_map_memory(VK_Buffer* buffer, u64 offset, u64 size) {
  VK_CHECK(vkMapMemory(vkdevice, buffer->memory, offset, size, 0, (void**)&buffer->maped_memory));
}

void vk_buffer_unmap_memory(VK_Buffer* buffer) {
  vkUnmapMemory(vkdevice, buffer->memory);
}

void upload_to_gpu(VK_Buffer* buffer, MemRange range, void* data) {
  MemCopy(vk.stage_buffer.maped_memory, data, range.size);

  VK_CommandBuffer temp_cmd = vk_cmd_alloc_and_begin_single_use();
  
  VkBufferCopy copy_region;
  copy_region.srcOffset = 0;
  copy_region.dstOffset = range.offset;
  copy_region.size = range.size;
  
  vkCmdCopyBuffer(temp_cmd.handle, vk.stage_buffer.handle, buffer->handle, 1, &copy_region);
  
  // Submit the buffer for execution and wait for it to complete
  vk_cmd_end_single_use(&temp_cmd);
}
