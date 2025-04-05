#include <app_types.h>

#include <engine.h>

#include <logger.h>
#include <memory.h>
#include <str.h>

 void application_create(Application* out_app);

int main() {
  Application app_inst = {};
  application_create(&app_inst);
  
  engine_create(&app_inst);
  
  engine_run(&app_inst);
}
