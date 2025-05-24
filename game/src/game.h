#pragma once
#include "lib.h"
#include "object.h"
#include "game_types.h"

C_LINKAGE_BEGIN

ExportAPI void app_init(struct App* game_inst);
ExportAPI void app_update(struct App* game_inst);
ExportAPI void app_on_resize(struct App* game_inst);

C_LINKAGE_END

extern GameState* st;
