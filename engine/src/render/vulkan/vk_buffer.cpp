#include "vk_buffer.h"

#include "vk_device.h"
#include "vk_command_buffer.h"
#include "vk_utils.h"

#include <logger.h>
#include <memory.h>

b8 vk_buffer_create(
  VK_Context* context,
  u64 size,
  VkBufferUsageFlagBits usage,
  u32 memory_property_flags,
  b8 bind_on_create,
  VK_Buffer* buffer) {
    
  MemZeroStruct(buffer);
  buffer->size = size;
  buffer->usage = usage;
  buffer->memory_property_flags = memory_property_flags;
  
  VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  buffer_info.size = size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // NOTE only used in one queue
  
  VK_CHECK(vkCreateBuffer(context->device.logical_device, &buffer_info, context->allocator, &buffer->handle));

  // Gather memory requirements
  VkMemoryRequirements requirements;
  vkGetBufferMemoryRequirements(context->device.logical_device, buffer->handle, &requirements);
  buffer->memory_index = context->find_memory_index(requirements.memoryTypeBits, buffer->memory_property_flags);
  if (buffer->memory_index == -1) {
    Error("Unable to create vulkan buffer because the required memory type index was not found.");
    return false;
  }
  
  // Allocate memory info
  VkMemoryAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  allocate_info.allocationSize = requirements.size;
  allocate_info.memoryTypeIndex = (u32)buffer->memory_index;
  
  // Allocate the memory.
  VkResult result = vkAllocateMemory(
    context->device.logical_device, 
    &allocate_info, 
    context->allocator, 
    &buffer->memory);
    
  if (result != VK_SUCCESS) {
    Error("Unable to create vulkan buffer because the required memory allocation failed. Error: %i", result);
    return false;
  }
  
  if (bind_on_create) {
    vk_buffer_bind(context, buffer, 0);
  }
  
  return true;
}

void vk_buffer_destroy(VK_Context* context, VK_Buffer* buffer) {
  if (buffer->memory) {
    vkFreeMemory(context->device.logical_device, buffer->memory, context->allocator);
    buffer->memory = 0;
  }
  if (buffer->handle) {
    vkDestroyBuffer(context->device.logical_device, buffer->handle, context->allocator);
    buffer->handle = 0;
  }
  buffer->size = 0;
  buffer->usage = (VkBufferUsageFlagBits)0;
  buffer->is_locked = false;
}

b8 vk_buffer_resize(
  VK_Context* context,
  u64 new_size,
  VK_Buffer* buffer,
  VkQueue queue,
  VkCommandPool pool) {
  
  // Create new buffer
  VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  buffer_info.size = new_size;
  buffer_info.usage = buffer->usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  
  VkBuffer new_buffer;
  VK_CHECK(vkCreateBuffer(context->device.logical_device, &buffer_info, context->allocator, &new_buffer));
  
  // Gather memory requirements
  VkMemoryRequirements requirements;
  vkGetBufferMemoryRequirements(context->device.logical_device, new_buffer, &requirements);
  
  // Allocate memory info
  VkMemoryAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  allocate_info.allocationSize = requirements.size;
  allocate_info.memoryTypeIndex = (u32)buffer->memory_index;
  
  // Allocate the memory
  VkDeviceMemory new_memory;
  VkResult result = vkAllocateMemory(context->device.logical_device, &allocate_info, context->allocator, &new_memory);
  if (result != VK_SUCCESS) {
    Error("Unable to resize vulkan buffer because the required memory allocation failed. Error: %i", result);
    return false;
  }
  
  // Bind the new buffer's memory
  VK_CHECK(vkBindBufferMemory(context->device.logical_device, new_buffer, new_memory, 0));
  
  // Copy over the data
  vk_buffer_copy_to(context, pool, 0, queue, buffer->handle, 0, new_buffer, 0, buffer->size);
  
  // Make sure anything potentially using these if finished
  vkDeviceWaitIdle(context->device.logical_device);
  
  // Destroy the old
  if (buffer->memory) {
    vkFreeMemory(context->device.logical_device, buffer->memory, context->allocator);
    buffer->memory = 0;
  }
  if (buffer->memory) {
    vkDestroyBuffer(context->device.logical_device, buffer->handle, context->allocator);
    buffer->handle = 0;
  }
  
  // Set new properties
  buffer->size = new_size;
  buffer->memory = new_memory;
  buffer->handle = new_buffer;
  return true;
}

void vk_buffer_bind(VK_Context* context, VK_Buffer* buffer, u64 offset) {
  VK_CHECK(vkBindBufferMemory(context->device.logical_device, buffer->handle, buffer->memory, offset));
}

void* vk_buffer_lock_memory(VK_Context* context, VK_Buffer* buffer, u64 offset, u64 size, u32 flags) {
  void* data;
  VK_CHECK(vkMapMemory(context->device.logical_device, buffer->memory, offset, size, flags, &data));
  return data;
}

void vk_buffer_unlock_memory(VK_Context* context, VK_Buffer* buffer) {
  vkUnmapMemory(context->device.logical_device, buffer->memory);
}

void vk_buffer_load_data(VK_Context* context, VK_Buffer* buffer, u64 offset, u64 size, u32 flags, const void* data) {
  void* data_ptr;
  VK_CHECK(vkMapMemory(context->device.logical_device, buffer->memory, offset, size, flags, &data_ptr));
  MemCopy(data_ptr, data, size);
  vkUnmapMemory(context->device.logical_device, buffer->memory);
}

void vk_buffer_copy_to(
    VK_Context* context,
    VkCommandPool pool,
    VkFence fence,
    VkQueue queue,
    VkBuffer source,
    u64 source_offset,
    VkBuffer dest,
    u64 dest_offset,
    u64 size) {

  vkQueueWaitIdle(queue);
  
  // Create a one-time-use command buffer
  VK_CommandBuffer temp_command_buffer;
  vk_command_buffer_alloc_and_begin_single_use(context, pool, &temp_command_buffer);
  
  // Prepare the copy command and add it to the command buffer
  VkBufferCopy copy_region;
  copy_region.srcOffset = source_offset;
  copy_region.dstOffset = dest_offset;
  copy_region.size = size;
  
  vkCmdCopyBuffer(temp_command_buffer.handle, source, dest, 1, &copy_region);
  
  // Submit the buffer for execution and wait for it to complete
  vk_command_buffer_end_single_use(context, pool, &temp_command_buffer, queue);
}
