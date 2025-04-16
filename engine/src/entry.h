#pragma once
#include "lib.h"

#include <app_types.h>
#include <engine.h>

void application_create(Application* out_app);

int main() {
  Application app_inst = {};
  application_create(&app_inst);
  
  engine_create(&app_inst);
  
  engine_run(&app_inst);
}
