#pragma once
#include "lib.h"

#include "res/res_types.h"

KAPI u32 shader_get(String name);

void shader_init(Arena* arena);

KAPI u32 shader_create(Shader shader);
