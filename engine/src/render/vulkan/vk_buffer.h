#pragma once
#include "vk_types.h"

b8 vk_buffer_create(
  u64 size,
  VkBufferUsageFlagBits usage,
  u32 memory_property_flags,
  b8 bind_on_create,
  VK_Buffer* buffer);
  
void vk_buffer_destroy(VK_Buffer* buffer);

b8 vk_buffer_resize(
  u64 new_size,
  VK_Buffer* buffer,
  VkQueue queue,
  VkCommandPool pool);

void vk_buffer_bind(VK_Buffer* buffer, u64 offset);

void* vk_buffer_lock_memory(VK_Buffer* buffer, u64 offset, u64 size, u32 flags);
void vk_buffer_unlock_memory(VK_Buffer* buffer);

void vk_buffer_load_data(VK_Buffer* buffer, u64 offset, u64 size, u32 flags, const void* data);

void vk_buffer_copy_to(
  VkCommandPool pool,
  VkFence fence,
  VkQueue queue,
  VkBuffer source,
  u64 source_offset,
  VkBuffer dest,
  u64 dest_offset,
  u64 size);


