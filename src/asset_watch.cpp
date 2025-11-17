#include "asset_watch.h"
#include "renderer.h"

struct FileWatch {
  String path;
  DenseTime modified;
  void (*callback)();
};

struct DirectoryWatch {
  String path;
  OS_Watch watch;
  void (*callback)(String name);
};

struct AssetWatchState {
  Arena* arena;
  Array<FileWatch, 128> watches;
  Array<DirectoryWatch, 128> directories;
};

global AssetWatchState st;

void asset_watch_init() {
  st.arena = arena_alloc();
}

void asset_watch_add(String watch_name, void (*callback)()) {
  FileProperties props = os_file_path_properties(watch_name);
  append(st.watches, {
    .path = watch_name,
    .modified = props.modified,
    .callback = callback,
  });
}

void asset_watch_directory_add(String watch_name, void (*reload_callback)(String name), OS_WatchFlags flags) {
  String dir_path = push_strf(st.arena, "%s/%s", asset_base_path(), watch_name);
  OS_Watch watch = os_watch_open(flags);
  os_watch_attach(watch, dir_path);
  append(st.directories, {
    .path = dir_path,
    .watch = watch,
    .callback = reload_callback,
  });
}

void asset_watch_update() {
  Scratch scratch;
  for (FileWatch x : st.watches) {
    FileProperties props = os_file_path_properties(x.path);
    if (props.modified > x.modified) {
      x.callback();
      x.modified = props.modified;
    }
  }
  for (DirectoryWatch x : st.directories) {
    StringList list = os_watch_check(scratch, x.watch);
    for (StringNode* node = list.first; node != null; node = node->next) {
      x.callback(node->string);
    }
  }
}
