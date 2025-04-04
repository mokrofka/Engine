#pragma once

#include "r_types.h"

b8 r_backend_create(R_BackendType type, R_Backend* our_renderer_backend);
void r_backend_destroy(R_Backend* renderer_backend);
