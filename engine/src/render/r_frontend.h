#pragma once
#include "lib.h"
#include "vulkan/vk_backend.h"

#include "r_types.h"

struct R_Config {
  u64 mem_reserve;
};

void r_init(Arena* arena, R_Config config);
void r_shutdown();

void r_on_resized(u32 width, u32 height);

void r_begin_draw_frame();
void r_end_draw_frame();

KAPI v2 get_viewport_size();
