#pragma once
#include "lib.h"

#include "render/r_types.h"

#define DefaultTextureName "default"

KAPI void texture_init();

KAPI void texture_load(String name);
KAPI Texture& texture_get(String name);
