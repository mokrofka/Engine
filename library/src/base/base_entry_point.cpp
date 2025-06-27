#include "render/r_frontend.h"

#include "sys/texture.h"
#include "sys/geometry.h"
#include "sys/res.h"
#include "sys/shader.h"

#include "asset_watch.h"
#include "ui.h"

void base_main_thread_entry(void (*entry)()) {
  global_allocator_init();
  tctx_init();
  os_init();
  Scratch scratch;

  ResSysConfig config = {
    .asset_base_path = "../assets"
  };
  res_sys_init(config);

  event_init();
  
  {
    WindowConfig config = {
      .position_x = 100,
      .position_y = 100,
      .width = 1000,
      .height = 600,
    };
    os_window_create(config);
  }

  asset_watch_init();
  
  shader_init();
  
  texture_init();

  r_init();
  
  ui_init();

  geometry_init(scratch);

  entry();
}
