#pragma once
#include "lib.h"
#include "object.h"
#include "game_types.h"

C_LINKAGE_BEGIN

ExportAPI void application_update(struct App* game_inst);
ExportAPI void application_init(struct App* game_inst);
ExportAPI void application_render(struct App* game_inst);
ExportAPI void application_on_resize(struct App* game_inst);

C_LINKAGE_END

extern GameState* st;
