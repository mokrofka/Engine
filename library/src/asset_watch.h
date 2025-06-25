#pragma once
#include "lib.h"

KAPI void asset_watch_init();

KAPI void asset_watch_add(void (*reload_callback)(String filepath, u32 id));

KAPI void asset_watch_update();
