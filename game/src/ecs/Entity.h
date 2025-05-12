#pragma once
#include "defines.h"

#include <bitset>

typedef u32 Entity;
typedef u8  ComponentType;

const Entity        MAX_ENTITIES   = 2;
const ComponentType MAX_COMPONENTS = 32;

typedef std::bitset<MAX_COMPONENTS> Signature;
