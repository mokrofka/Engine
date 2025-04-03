#pragma once

#include "r_types.inl"

struct StaticMeshData;
struct PlatformState;

b8 r_init(Arena* arena);
void r_shutdown();

void r_on_resized(u16 width, u16 height);

b8 r_draw_frame(RenderPacket* packet);

// HACK this should not be exposed out the engine
KAPI void r_set_view(mat4 view);

void r_create_texture(const u8* pixels, struct Texture* texture);
void r_destroy_texture(struct Texture* texture);
