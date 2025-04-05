#pragma once
#include "vk_types.h"

void vk_fence_create(VK_Context* context, b8 create_signaled, VK_Fence* out_fence);

void vk_fence_destroy(VK_Context* context, VK_Fence* fence);

b8 vk_fence_wait(VK_Context* context, VK_Fence* fence, u64 timeout_ns);

void vk_fence_reset(VK_Context* context, VK_Fence* fence);
