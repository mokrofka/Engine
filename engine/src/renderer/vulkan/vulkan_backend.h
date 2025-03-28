#pragma once

#include "renderer/renderer_backend.h"
#include "resources/resources_types.h"

b8 vulkan_renderer_backend_initialize(RendererBackend* backend);

void vulkan_renderer_backend_shutdown(RendererBackend* backend);

void vulkan_renderer_backend_on_resize(RendererBackend* backend, u16 width, u16 height);

b8 vulkan_renderer_backend_begin_frame(RendererBackend* backend, f32 delta_time);
void vulkan_renderer_update_global_state(mat4 projeection, mat4 view, v3 view_position, v4 ambient_colour, i32 mode);
b8 vulkan_renderer_backend_end_frame(RendererBackend* backend, f32 delta_time);

void vulkan_renderer_update_object(GeometryRenderData data);

void vulkan_renderer_create_texture(
    const char* name, b8 auto_release, i32 width, i32 height, i32 channel_count,
    const u8* pixels, b8 has_transparency, Texture* texture);
void vulkan_renderer_destroy_texture(Texture* texture);
