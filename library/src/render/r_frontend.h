#pragma once
#include "lib.h"
#include "vulkan/vk_backend.h"

#include "r_types.h"

KAPI void r_init();
KAPI void r_shutdown();

KAPI void r_on_resized(u32 width, u32 height);

KAPI void r_begin_draw_frame();
KAPI void r_end_draw_frame();
