#pragma once
#include "lib.h"

#include "r_types.h"

struct StaticMeshData;
struct PlatformState;

void r_init(Arena* arena);
void r_shutdown();

void r_on_resized(u32 width, u32 height);

void r_draw_frame(R_Packet* packet);
void r_begin_draw_frame(R_Packet* packet);
void r_end_draw_frame(R_Packet* packet);

// HACK this should not be exposed out the engine
KAPI void r_set_view(mat4 view);

void* r_create_texture(u8* pixels, u32 width, u32 height, u32 channel_count);
void r_destroy_texture(Texture* texture);

void r_create_material(Material* material);
void r_destroy_material(Material* material);

void r_create_geometry(Geometry* geometry, u32 vertex_size, u32 vertex_count, void* vertices, u32 index_size, u32 index_count, void* indices);
void r_destroy_geometry(Geometry* geometry);
