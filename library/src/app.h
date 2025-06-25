#pragma once
#include "lib.h"

struct App {
  Arena* arena;
  void (*init)(u8** state);
  void (*update)(u8* state);

  b8 is_running;
  b8 is_suspended;

  Clock clock;
  f32 last_time;
  
  u8* state;
  
  OS_Handle lib;
  String lib_filepath;
  String lib_temp_filepath;
  DenseTime modified;
};
