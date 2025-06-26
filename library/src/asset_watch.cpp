#include "asset_watch.h"
#include "sys/res.h"
#include "sys/shader.h"

struct FileWatch {
  String path;
  DenseTime modified;
  void (*callback)(String filepath);
};

struct DirectoryWatch {
  String path;
  void (*callback)(String filepath);
};

struct AssetWatcherState {
  Arena* arena;
  u32 watches_count;
  FileWatch watches[128];
  u32 directories_count;
  OS_Handle directories[128];

  // OS_Handle shader_dir_handle;
  // OS_Handle compiled_shader_dir_handle;
};

AssetWatcherState st;

void asset_watch_init() {
  Scratch scracth;
  st = {
    .arena = mem_arena_alloc(KB(1)),
  };
  // st.shader_dir_handle = os_directory_open("D:\\VS_Code\\Engine\\assets\\shaders");
  // st.compiled_shader_dir_handle = os_directory_open("D:\\VS_Code\\Engine\\assets\\shaders\\compiled");
  // os_directory_watch(st.shader_dir_handle, 0);
  // os_directory_watch(st.compiled_shader_dir_handle, 1);
}

void asset_watch_add(String name, void (*callback)(String filepath)) {
  st.watches[st.watches_count].callback = callback;
  ++st.watches_count;
}

void asset_watch_directory_add(String name, void (*reload_callback)(String filepath)) {

}

void asset_watch_update() {
  // Scratch scratch;

  // b32 ready = os_directory_check_change(st.shader_dir_handle, 0);
  // if (ready) {
  //   String name = os_directory_name_change(scratch, 0);

  //   // twice because of windows
  //   os_directory_watch(st.shader_dir_handle, 0);
  //   os_directory_watch(st.shader_dir_handle, 0);
    
  //   String filepath = push_str_cat(scratch, "D:\\VS_Code\\Engine\\assets\\shaders\\", name);
  //   String filepath_output = push_str_cat(scratch, "D:\\VS_Code\\Engine\\assets\\shaders\\compiled\\", name);
  //   filepath_output = push_str_cat(scratch, filepath_output, ".spv");
  //   String cmd = push_strf(scratch, "glslangValidator.exe -V \"%s\" -o \"%s\"", filepath, filepath_output);
    
  //   os_process_create(cmd);
  // }
  
  // b32 ready_compile = os_directory_check_change(st.compiled_shader_dir_handle, 1);
  // if (ready_compile) {
  //   String name = os_directory_name_change(scratch, 1);

  //   // twice because of windows
  //   os_directory_watch(st.compiled_shader_dir_handle, 1);
  //   os_directory_watch(st.compiled_shader_dir_handle, 1);
  //   Info("%s", name);
  //   String shader_name = str_chop_last_dot(name);
  //   shader_name = str_chop_last_dot(shader_name);
    
  //   // String filepath = push_str_cat(scratch, "D:\\VS_Code\\Engine\\assets\\shaders\\compiled\\", name);
  //   st.watch.callback(shader_name, shader_get(shader_name));
  // }
}
