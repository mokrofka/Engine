#pragma once

#include "defines.h"

void platform_get_required_extension_names(struct Arena* arena);

b8 platform_create_vulkan_surface(struct VulkanContext* context);

