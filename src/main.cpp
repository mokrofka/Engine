#include "lib.h"

shared_function void common_main(HotReloadData* data);

i32 main(i32 count, char* args[]) {
  tctx_init();
  os_init(args[0]);

  struct App {
    HotReloadData state;
    void (*update)(HotReloadData* data);
    OS_Handle lib;
    String lib_filepath;
  } st = {};

#if HOTRELOAD_BUILD
  Scratch scratch;
  String current_dir = os_get_current_directory();
  st.lib_filepath = push_str_cat(scratch, current_dir, "/libgame.so");
  st.state.lib = st.lib_filepath;
  st.lib = os_lib_open(st.lib_filepath);
  Assign(st.update, os_lib_get_proc(st.lib, "common_main"));
#else
  st.update = common_main;
#endif

  while (true) {
    st.update(&st.state);
    os_lib_close(st.lib);
    os_sleep_ms(10);
    st.lib = os_lib_open(st.lib_filepath);
    Assign(st.update, os_lib_get_proc(st.lib, "common_main"));
  }
}



