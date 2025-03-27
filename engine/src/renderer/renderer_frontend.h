#pragma once

#include "renderer_types.inl"

struct StaticMeshData;
struct PlatformState;

b8 renderer_initialize(u64* memory_requirement, void* out_state);
void renderer_shutdown();

void renderer_on_resized(u16 width, u16 height);

b8 renderer_draw_frame(RenderPacket* packet);

// HACK this should not be exposed out the engine
KAPI void renderer_set_view(mat4 view);
