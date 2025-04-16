#pragma once
#include "lib.h"

C_LINKAGE_BEGIN

ExportAPI void application_update(struct Application* game_inst);
ExportAPI void application_init(struct Application* game_inst);
ExportAPI void application_render(struct Application* game_inst);
ExportAPI void application_on_resize(struct Application* game_inst);

C_LINKAGE_END
