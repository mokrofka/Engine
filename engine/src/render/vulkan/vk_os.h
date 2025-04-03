#pragma once

#include "defines.h"

void vk_os_get_required_extension_names(struct Arena* arena);

b8 vk_os_create_surface(struct VK_Context* context);

