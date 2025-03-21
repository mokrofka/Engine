#pragma once

#include "renderer/renderer_backend.h"

b8 vulkan_renderer_backend_initialize(RendererBackend* backend);

void vulkan_renderer_backend_shutdown(RendererBackend* backend);

void vulkan_renderer_backend_on_resize(RendererBackend* backend, u16 width, u16 height);

b8 vulkan_renderer_backend_begin_frame(RendererBackend* backend, f32 delta_time);
b8 vulkan_renderer_backend_end_frame(RendererBackend* backend, f32 delta_time);
