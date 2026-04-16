#include "lib.h"
#include "common.h"

struct App {
  u8* state;
  void (*update)(u8** state);
  OS_Handle lib;
  String lib_filepath;
};

global App st;

void main_update(u8** state);

i32 main(i32 count, char* args[]) {
  u64 start = os_now_ns();
  global_allocator_init();
  tctx_init();
  test();
  os_init(args[0]);
  os_gfx_init();
  common_init();
  Scratch scratch;
#if HOTRELOAD_BUILD
  String current_dir = os_get_current_directory();
  st.lib_filepath = push_str_cat(scratch, current_dir, "/libgame.so");
  st.lib = os_lib_open(st.lib_filepath);
  Assign(st.update, os_lib_get_proc(st.lib, "main_update"));
  asset_watch_add(st.lib_filepath, []() {
    os_lib_close(st.lib);
    os_sleep_ms(10);
    st.lib = os_lib_open(st.lib_filepath);
    Assign(st.update, os_lib_get_proc(st.lib, "main_update"));
  });
#else
  st.update = main_update;
#endif
  u64 target_fps = Billion(1) / 60;
  u64 last_time = os_now_ns();
  Timer timer = timer_init(1);

  u64 end = os_now_ns();
  Info("main init took: %f64 sec", f64(end - start) / Billion(1));

  // Main loop
  while (!os_window_should_close()) {
    profile_begin();
    os_pump_messages();
    u64 start_time = os_now_ns();
    g_dt = f32(start_time - last_time) / Billion(1);
    g_time += g_dt;
    last_time = start_time;

    // Main logic
    {
    TimeBlock("start");
    vk_begin_draw_frame();
    ui_begin();
    st.update(&st.state);
    ui_end();
    vk_end_draw_frame();
    os_input_update();
    asset_watch_update();
    }

    u64 frame_duration = os_now_ns() - start_time;
    if (frame_duration < target_fps) {
      u64 sleep_time = target_fps - frame_duration;
      os_sleep_ms(sleep_time / Million(1));
    }
    if (timer_tick(timer)) {
      Info("Frame rate: %f, %f fps", g_dt, 1/g_dt);
    }
    profile_end();
  }
  // vk_shutdown();
  // os_gfx_shutdown();
  os_exit(0);
}




