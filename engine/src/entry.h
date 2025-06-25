#pragma once
#include "lib.h"

#include "engine.h"
#include "game.h"

int main() {
#ifdef MONOLITHIC_BUILD
  App app_inst = {
    .init = app_init,
    .update = app_update,
    .on_resize = app_on_resize
  };
#else
  App app_inst = {};
#endif

  engine_init(&app_inst);
  engine_run(&app_inst);
}
