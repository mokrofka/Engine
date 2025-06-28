#pragma once
#include "vk_types.h"

VK_Buffer vk_buffer_create(u64 size, u32 usage, u32 memory_property_flags);

void vk_buffer_destroy(VK_Buffer& buffer);

void vk_buffer_map_memory(VK_Buffer& buffer, u64 offset, u64 size);
void vk_buffer_unmap_memory(VK_Buffer& buffer);

void vk_upload_to_gpu(VK_Buffer& buffer, Range range, void* data);
