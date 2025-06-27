#include "asset_watch.h"
#include "sys/res.h"
#include "sys/shader.h"

struct FileWatch {
  String path;
  DenseTime modified;
  void (*callback)();
};

struct DirectoryWatch {
  String path;
  OS_Handle dir_handle;
  void (*callback)(String name);
};

struct AssetWatcherState {
  Arena* arena;
  u32 watches_count;
  FileWatch watches[128];
  u32 directories_count;
  DirectoryWatch directories[128];

  // OS_Handle shader_dir_handle;
  // OS_Handle compiled_shader_dir_handle;
};

AssetWatcherState st;

void asset_watch_init() {
  Scratch scracth;
  st = {
    .arena = mem_arena_alloc(KB(1)),
  };
}

void asset_watch_add(String watch_name, void (*callback)()) {
  FileProperties props = os_properties_from_file_path(watch_name);
  st.watches[st.watches_count] = {
    .path = watch_name,
    .modified = props.modified,
    .callback = callback,
  };
  ++st.watches_count;
}

void asset_watch_directory_add(String watch_name, void (*reload_callback)(String name)) {
  String dir_path = push_strf(st.arena, "%s/%s", res_sys_base_path(), watch_name);
  st.directories[st.directories_count] = {
    .path = dir_path,
    .dir_handle = os_directory_open(dir_path),
    .callback = reload_callback,
  };
  os_directory_watch(st.directories[st.directories_count].dir_handle, st.directories_count);
  ++st.directories_count;
}

void asset_watch_update() {
  Scratch scratch;

  Loop (i, st.watches_count) {
    FileProperties props = os_properties_from_file_path(st.watches[i].path);
    u64 new_write_time = props.modified;
    b32 game_modified = os_file_compare_time(new_write_time, st.watches[i].modified);
    if (game_modified) {
      st.watches[i].callback();
      st.watches[i].modified = props.modified;
    }
  }

  Loop (i, st.directories_count) {
    b32 yea = os_directory_check_change(st.directories[i].dir_handle, i);
    if (yea) {
      String name = os_directory_name_change(scratch, i);
      os_directory_watch(st.directories[i].dir_handle, i);
      st.directories[i].callback(name);
    }
  }
}
