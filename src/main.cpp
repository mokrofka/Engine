#include "lib.h"
#include "common.h"

struct App {
  void (*update)(u8** state);

  b8 is_running;
  b8 is_suspended;
  
  u8* state;
  
  OS_Handle lib;
  String lib_filepath;
  String lib_temp_filepath;
};

global App st;

void app_update(u8** state);

i32 main(i32 count, char* args[]) {
  global_alloc_init();
  tctx_init();
  Info("%i", ModPow2(-31, -4));
  // test();
  os_init(args[0]);
  os_gfx_init();
  asset_watch_init();
  common_init();

  Scratch scratch;

#if HOTRELOAD_BUILD
  String current_dir = os_get_current_directory();
  st.lib_filepath = push_str_cat(scratch, current_dir, "/libgame.so");
  st.lib_temp_filepath = push_str_cat(scratch, current_dir, "/libgame_temp.so");
  
  os_file_path_copy(st.lib_temp_filepath, st.lib_filepath);
  st.lib = os_lib_open(st.lib_temp_filepath);
  Assign(st.update, os_lib_get_proc(st.lib, "app_update"));

  Assert(st.lib && st.update);
  asset_watch_add(st.lib_filepath, []() {
    os_lib_close(st.lib);
    os_file_path_copy(st.lib_temp_filepath, st.lib_filepath);
    os_sleep_ms(10);
    st.lib = os_lib_open(st.lib_temp_filepath);
    Assign(st.update, os_lib_get_proc(st.lib, "app_update"));
  });
#else
  st.update = app_update;
#endif

  u64 target_fps = Billion(1) / 60;
  u64 last_time = os_now_ns();
  f32 timer = 0;

  while (!os_window_should_close()) {
    os_pump_messages();
    u64 start_time = os_now_ns();
    delta_time = f32(start_time - last_time) / Billion(1);
    last_time = start_time;
    
    if (!st.is_suspended) {

      r_begin_draw_frame();
      st.update(&st.state);
      r_end_draw_frame();

      os_input_update();
      asset_watch_update();
    }

    u64 frame_duration = os_now_ns() - start_time;
    if (frame_duration < target_fps) {
      u64 sleep_time = target_fps - frame_duration;
      os_sleep_ms(sleep_time / Million(1));
    }

    timer += delta_time;
    if (timer >= 1.0) {
      timer = 0;
      Info("Frame rate: %f, %f fps", delta_time, 1/delta_time);
    }
  }

  // r_shutdown();
  os_gfx_shutdown();
  os_exit(0);
}


