#pragma once
#include "vk_types.h"

enum RenderPassClearFlag {
  RenderpassClearFlag_None = 0x0,
  RenderpassClearFlag_ColorBuffer = 0x1,
  RenderpassClearFlag_DepthBuffer = 0x2,
  RenderpassClearFlag_StencilBuffer = 0x4,
};

u32 vk_renderpass_create(Rect rect, v4 color, f32 depth, u32 stencil, u32 clear_flags, b32 has_prev_pass, b32 has_next_pass);

void vk_renderpass_destroy(u32 id);

void vk_renderpass_begin(VK_CommandBuffer* command_buffer, VK_Renderpass* renderpass, VkFramebuffer frame_buffer);

void vk_renderpass_end(VK_CommandBuffer* command_buffer);

