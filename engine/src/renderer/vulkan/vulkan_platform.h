#pragma once

#include "defines.h"

struct PlatformState;
struct VulkanContext;

void platform_get_required_extension_names(struct Arena* arena);

b8 platform_create_vulkan_surface(VulkanContext* context);

