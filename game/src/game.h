#include "defines.h"

C_LINKAGE_BEGIN

__declspec(dllexport) b8 application_update(struct Application* game_inst);
__declspec(dllexport) b8 application_init(struct Application* game_inst);
__declspec(dllexport) b8 application_render(struct Application* game_inst);
__declspec(dllexport) void application_on_resize(struct Application* game_inst);

C_LINKAGE_END
