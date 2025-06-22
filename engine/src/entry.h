#pragma once
#include "lib.h"

#include <engine.h>

int main() {
  App app_inst = {};
  engine_init(&app_inst);
  engine_run(&app_inst);
}
