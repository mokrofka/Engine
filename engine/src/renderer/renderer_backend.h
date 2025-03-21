#pragma once

#include "renderer_types.inl"

struct PlatformState;

b8 renderer_backend_create(RendererBackendType type, RendererBackend* our_renderer_backend);
void renderer_backend_destroy(RendererBackend* renderer_backend);
