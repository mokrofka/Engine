#pragma once
#include "lib.h"

void asset_watch_init(Arena* arena);

void asset_watch_add(void (*reload_callback)(String filepath, u32 id));

void asset_watch_update();
