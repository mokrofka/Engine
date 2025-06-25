#pragma once
#include "lib.h"

#ifdef MONOLITHIC_BUILD

ExportAPI void app_init(u8** state);
ExportAPI void app_update(u8* state);

#else
C_LINKAGE_BEGIN
ExportAPI void app_init(u8** state);
ExportAPI void app_update(u8* state);
C_LINKAGE_END

#endif

