#pragma once
#include "lib.h"

KAPI void asset_watch_init();

// Full filepath
KAPI void asset_watch_add(String watch_name, void (*callback)());
// Relative to assets directory
KAPI void asset_watch_directory_add(String watch_name, void (*reload_callback)(String filename), OS_WatchFlags flags);

KAPI void asset_watch_update();
