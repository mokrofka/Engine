#pragma once
#include "lib.h"

#include "res/res_types.h"

KAPI Shader& shader_get(String name);

KAPI void shader_init();

KAPI void shader_create(Shader shader);
