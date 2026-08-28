#include "base/base_impl.cpp"

shared_function void update_main(HotReloadData* data);

i32 main(i32 count, char* args[]) {
  mem_track_init();
  tctx_init();
  os_init(args[0]);

  Scratch scratch;
  HotReloadData state = {};
  void (*update)(HotReloadData* data) = {};
  OS_Handle lib = {};

#if HOTRELOAD_BUILD
  state.lib_path = push_str_cat(scratch, os_cur_directory(), "/libgame.so");
  lib = os_lib_open(state.lib_path);
  Assign(update, os_lib_get_proc(lib, "update_main"));
#else
  update = update_main;
#endif

  while (true) {
    update(&state);
    os_lib_close(lib);
    os_sleep_ms(10);
    lib = os_lib_open(state.lib_path);
    Assign(update, os_lib_get_proc(lib, "update_main"));
  }
}
