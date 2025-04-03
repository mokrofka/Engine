#pragma once

#include "r_types.inl"

b8 renderer_backend_create(R_BackendType type, R_Backend* our_renderer_backend);
void renderer_backend_destroy(R_Backend* renderer_backend);
