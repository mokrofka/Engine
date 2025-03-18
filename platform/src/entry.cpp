#include "entry.h"
#include "application.h"

#include <core/logger.h>

#include <stdio.h>

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
    Info("Application did not shutdown gracefully.");
    return 1;
  }
  
  getchar();
}
