#pragma once

#include "renderer/renderer_backend.h"

b8 vulkan_renderer_backend_initialize(RendererBackend* backend);

void vulkan_renderer_backend_shutdown(RendererBackend* backend);

void vulkan_renderer_backend_on_resize(RendererBackend* backend, u16 width, u16 height);

b8 vulkan_renderer_backend_begin_frame(RendererBackend* backend, f32 delta_time);
void vulkan_renderer_update_global_state(mat4 projeection, mat4 view, v3 view_position, v4 ambient_colour, i32 mode);
b8 vulkan_renderer_backend_end_frame(RendererBackend* backend, f32 delta_time);

void vulkan_backend_update_object(mat4 model);
