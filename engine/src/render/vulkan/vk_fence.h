#pragma once
#include "vk_types.h"

VK_Fence vk_fence_create(b8 create_signaled);

void vk_fence_destroy(VK_Fence* fence);

b8 vk_fence_wait(VK_Fence* fence, u64 timeout_ns);

void vk_fence_reset(VK_Fence* fence);
