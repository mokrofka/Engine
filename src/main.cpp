#include "base/base_impl.cpp"

shared_function void update(HotReloadData* data);

i32 main(i32 count, char* args[]) {
	mem_track_init();
	tctx_init();
	os_init(args[0]);

	Scratch scratch;
	HotReloadData state = {};
	void (*com)(HotReloadData* data) = {};
	OS_Handle lib = {};

#if HOTRELOAD_BUILD
	state.lib_path = push_str_cat(scratch, os_cur_directory(), "/libgame.so");
	lib = os_lib_open(state.lib_path);
	Assign(com, os_lib_get_proc(lib, "update"));
#else
	com = update;
#endif

	while (true) {
		com(&state);
		os_lib_close(lib);

		os_sleep_ms(10);
		lib = os_lib_open(state.lib_path);
		Assign(com, os_lib_get_proc(lib, "update"));
	}
}
