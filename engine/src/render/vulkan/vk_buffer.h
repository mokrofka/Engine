#pragma once
#include "vk_types.h"

VK_Buffer vk_buffer_create(u64 size, u32 usage, u32 memory_property_flags, b32 bind_on_create);

void vk_buffer_destroy(VK_Buffer* buffer);

b32 vk_buffer_resize(u64 new_size, VK_Buffer* buffer, VkQueue queue, VkCommandPool pool);

void vk_buffer_bind(VK_Buffer* buffer, u64 offset);

void* vk_buffer_map_memory(VK_Buffer* buffer, u64 offset, u64 size, u32 flags);
void vk_buffer_unmap_memory(VK_Buffer* buffer);

void vk_buffer_load_data(VK_Buffer* buffer, u64 offset, u64 size, u32 flags, void* data);
void vk_buffer_load_image_data(VK_Buffer* buffer, u64 offset, u64 size, u32 flags, void* data);

void vk_buffer_copy_to(VK_Buffer* source, u64 source_offset, VK_Buffer* dest, u64 dest_offset, u64 size);

u64 vk_buffer_alloc(VK_Buffer* buffer, u64 size, u64 alignment);

void upload_data_range(VK_Buffer* buffer, MemRange range, void* data);
