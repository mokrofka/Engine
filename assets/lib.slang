import common;
#include "bindings.slang"

#define Loop(it, c) for (u32 it = 0; it < c; ++it)
#define VS_Entry [shader("vertex")]
#define FS_Entry [shader("fragment")]
#define CS_Entry [shader("compute")]

#define VkBind(idx) [[vk::binding(0, idx)]]
