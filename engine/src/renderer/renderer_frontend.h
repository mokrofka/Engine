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

void renderer_create_texture(
    const char* name, b8 auto_release, i32 width, i32 height, i32 channel_count,
    const u8* pixels, b8 has_transparency, struct Texture* texture);
void renderer_destroy_texture(struct Texture* texture);
