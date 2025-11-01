#include "base/defines.h"
#include "base/logger.h"
#include "renderer.h"

#include "asset_watch.h"
#include "test.h"

struct App {
  void (*update)(u8** state);

  b8 is_running;
  b8 is_suspended;
  
  u8* state;
  
  OS_Handle lib;
  String lib_filepath;
  String lib_temp_filepath;
  DenseTime modified;
};

global App st;

void app_update(u8** state);

i32 main(i32 count, char* arr[]) {
  global_allocator_init();
  tctx_init();
  os_init();
  os_gfx_init();
  asset_watch_init();
  res_sys_init("../assets");
  r_init();
  test();
  
  Scratch scratch;

#if HOTRELOAD_BUILD
  String current_dir = os_get_current_directory();
  st.lib_filepath = push_str_cat(scratch, current_dir, "/libgame.so");
  st.lib_temp_filepath = push_str_cat(scratch, current_dir, "/libgame_temp.so");
  
  os_copy_file_path(st.lib_temp_filepath, st.lib_filepath);
  st.lib = os_lib_open(st.lib_temp_filepath);
  Assign(st.update, os_lib_get_proc(st.lib, "app_update"));

  Assert(st.lib && st.update);
  asset_watch_add(st.lib_filepath, []() {
    os_lib_close(st.lib);
    os_copy_file_path(st.lib_temp_filepath, st.lib_filepath);
    st.lib = os_lib_open(st.lib_temp_filepath);
    Assign(st.update, os_lib_get_proc(st.lib, "app_update"));
  });
#else
  st.update = app_update;
#endif

  f64 target_fps = 1.0f / 60.0f;
  f64 last_time = os_now_seconds();
  f64 timer = 0;

  while (!os_window_should_close()) {
    os_pump_messages();
    f64 start_time = os_now_seconds();
    delta_time = start_time - last_time;
    last_time = start_time;
    
    if (!st.is_suspended) {

      r_begin_draw_frame();
      st.update(&st.state);
      r_end_draw_frame();

      os_input_update();
      asset_watch_update();
    }

    f64 frame_duration = os_now_seconds() - start_time;
    f64 sleep_time = target_fps - frame_duration;
    if (sleep_time > 0.0f) {
      os_sleep(SecToMs(sleep_time));
    }

    timer += delta_time;
    if (timer >= 1.0) {
      timer = 0;
      Info("Frame rate: %f ms, %f fps", delta_time, 1/delta_time);
    }
  }

  r_shutdown();
  os_gfx_shutdown();
  os_exit(0);
  
}


