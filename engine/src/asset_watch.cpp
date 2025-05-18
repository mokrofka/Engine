#include "asset_watch.h"
#include "sys/shader_sys.h"

struct FileWatch {
  String path;
  DenseTime modified;
  u32 id;
  void (*reload_callback)(String filepath, u32);
};

struct AssetWatcherState {
  Arena* arena;
  FileWatch watch;
  u32 watches_count;
  OS_Handle shader_dir_handle;
  OS_Handle compiled_shader_dir_handle;
  b8 pending;
};

AssetWatcherState* st;

void asset_watch_init(Arena* arena) {
  st = push_struct(arena, AssetWatcherState);
  st->arena = arena_alloc(arena, KB(1));
  st->shader_dir_handle = os_directory_open("D:\\VS_Code\\Engine\\assets\\shaders"_);
  st->compiled_shader_dir_handle = os_directory_open("D:\\VS_Code\\Engine\\assets\\shaders\\compiled"_);
  st->pending = true;
  os_directory_watch(st->shader_dir_handle, 0);
  os_directory_watch(st->compiled_shader_dir_handle, 1);
}

void asset_watch_add(void (*reload_callback)(String filepath, u32 id)) {
  st->watch.reload_callback = reload_callback;
  // u32 index = st->watches_count;
  // st->watches[index].path = push_str_copy(st->arena, filepath);
  // st->watches[index].id = id;
  // st->watches[index].reload_callback = reload_callback;
  
  // FileProperties prop = os_properties_from_file_path(filepath);
  // st->watches[index].modified = prop.modified;
  // ++st->watches_count;
}

// void asset_watch_update() {
//   Loop (i, st->watches_count) {
//     FileWatch* watch = &st->watches[i];
//     FileProperties prop = os_properties_from_file_path(watch->path);
    
//     if (os_file_compare_time(prop.modified, watch->modified)) {
//       watch->reload_callback(watch->path, watch->id);
//       watch->modified = prop.modified;
//     }
//   }
// }

void asset_watch_update() {
  Scratch scratch;
  if (!st->pending) 
    return;

  b32 ready = os_directory_check_change(st->shader_dir_handle, 0);
  if (ready) {
    String name = os_directory_name_change(scratch, 0);

    // twice because of windows
    os_directory_watch(st->shader_dir_handle, 0);
    os_directory_watch(st->shader_dir_handle, 0);
    
    String filepath = push_str_cat(scratch, "D:\\VS_Code\\Engine\\assets\\shaders\\"_, name);
    String filepath_output = push_str_cat(scratch, "D:\\VS_Code\\Engine\\assets\\shaders\\compiled\\"_, name);
    filepath_output = push_str_cat(scratch, filepath_output, ".spv"_);
    String cmd = push_strf(scratch, "glslangValidator.exe -V \"%s\" -o \"%s\"", filepath, filepath_output);
    
    os_process_create(cmd);
  }
  
  b32 ready_compile = os_directory_check_change(st->compiled_shader_dir_handle, 1);
  if (ready_compile) {
    String name = os_directory_name_change(scratch, 1);

    // twice because of windows
    os_directory_watch(st->compiled_shader_dir_handle, 1);
    os_directory_watch(st->compiled_shader_dir_handle, 1);
    Info("%s", name);
    String shader_name = str_chop_last_dot(name);
    shader_name = str_chop_last_dot(shader_name);
    
    // String filepath = push_str_cat(scratch, "D:\\VS_Code\\Engine\\assets\\shaders\\compiled\\"_, name);
    st->watch.reload_callback(shader_name, shader_get(shader_name));
    
    // String filepath = push_str_cat(scratch, "D:\\VS_Code\\Engine\\assets\\shaders\\"_, name);
    // String filepath_output = push_str_cat(scratch, "D:\\VS_Code\\Engine\\assets\\shaders\\compiled\\"_, name);
    // filepath_output = push_str_cat(scratch, filepath_output, ".spv"_);
    // String cmd = push_strf(scratch, "glslangValidator.exe -V \"%s\" -o \"%s\"", filepath, filepath_output);
    
    // os_process_create(cmd);
  }
}
