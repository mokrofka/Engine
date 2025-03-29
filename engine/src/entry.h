#include "application_type.h"
#include "logger.h"
#include "engine.h"

extern b8 application_create(Application* out_app);

extern b8 application_initialize(Application* app);

int main() {
  
  Application app_inst = {};
  
  if (!application_create(&app_inst)) {
    Fatal("Failed to create application");
    return 1;
  }
  
  if (!engine_create(&app_inst)) {
    Fatal("Failed to create engine");
    return 1;
  }
  
  if (!engine_run(&app_inst)) {
    Fatal("Application did not shutdown gracefully.");
    return 1;
  }
}
