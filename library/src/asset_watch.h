#pragma once
#include "lib.h"

KAPI void asset_watch_init();

// Full filepath
KAPI void asset_watch_add(String name, void (*reload_callback)(String filepath));
// Relative to assets directory
KAPI void asset_watch_directory_add(String name, void (*reload_callback)(String filepath));

KAPI void asset_watch_update();
