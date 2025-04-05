#pragma once

#include "defines.h"

#include <vulkan/vulkan.h>

void vk_os_get_required_extension_names(struct Arena* arena);

VkSurfaceKHR vk_os_create_surface();

