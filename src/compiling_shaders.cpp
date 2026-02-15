#include "lib.h"
#undef Info
#define Info(...)

i32 main(i32 args_count, char* args[]) {
  os_init(args[0]);
  tctx_init();
  u64 start = os_now_ns();

  Scratch scratch;
  String curr_dir = os_get_current_directory();
  Info("current directory: %s", curr_dir);
  String shader_dir = push_strf(scratch, "%s/%s", curr_dir, String(args[1]));
  Info("shader directory: %s", shader_dir);
  String compiled_shader_dir = push_strf(scratch, "%s/%s", curr_dir, String(args[2]));
  Info("compiled directory: %s", compiled_shader_dir);
  String file_path = push_strf(scratch, "%s/%s", curr_dir, "compiling_shaders_data");

  if (!os_directory_path_exist(shader_dir)) {
    os_directory_create_p(shader_dir);
  }
  if (!os_directory_path_exist(compiled_shader_dir)) {
    os_directory_create_p(compiled_shader_dir);
  }

  String global_glsl_path = push_strf(scratch, "%s/%s", shader_dir, String("defines/global.glsl"));
  FileProperties global_props = os_file_path_properties(global_glsl_path);
  OS_Handle file = os_file_open(file_path, OS_AccessFlag_Read|OS_AccessFlag_Write);
  FileProperties file_props = os_file_properties(file);
  b32 is_change = false;
  if (file_props.size != sizeof(DenseTime)) {
    is_change = true;
  }
  else {
    Buffer buf = os_file_path_read_all(scratch, file_path);
    DenseTime* modified = (DenseTime*)buf.data;
    if (global_props.modified != *modified) {
      is_change = true;
    }
  }
  if (is_change) {
    os_file_write(file, sizeof(DenseTime), &global_props.modified);
  }

  struct CompiledFile {
    OS_Handle pid;
    String shader_full_path;
    String compiled_shader_full_path;
  };
  Darray<CompiledFile> compiled_shaders(scratch);
  {
    OS_FileIter* it = os_file_iter_begin(scratch, shader_dir, OS_FileIterFlag_SkipFolders);
    Info("files:");
    for (OS_FileInfo info = {}; os_file_iter_next(scratch, it, &info);) {
      Info("%s", info.name);
      String shader_full_path = push_strf(scratch, "%s/%s", shader_dir, info.name);
      String compiled_shader_full_path = push_strf(scratch, "%s/%s.spv", compiled_shader_dir, info.name);
      if (!os_file_path_equal_mtime(shader_full_path, compiled_shader_full_path) || is_change) {
        // Info("it's compiling");
        StringList list = {};
        str_list_pushf(scratch, &list, "glslangValidator");
        str_list_pushf(scratch, &list, "-V");
        str_list_pushf(scratch, &list, "%s", shader_full_path);
        str_list_pushf(scratch, &list, "-o");
        str_list_pushf(scratch, &list, "%s", compiled_shader_full_path);
        OS_Handle pid = os_process_launch(list);
        compiled_shaders.add({pid, shader_full_path, compiled_shader_full_path});
      }
    }
    os_file_iter_end(it);
  }

  Darray<CompiledFile> tmp(scratch);
  for (i32 i = 0; i < compiled_shaders.count; ++i) {
    Info(compiled_shaders[i].shader_full_path);
    i32 result = os_process_join(compiled_shaders[i].pid);
    if (result == 0) {
      tmp.add(compiled_shaders[i]);
    }
  }
  for (CompiledFile& x : tmp) {
    os_file_path_copy_mtime(x.shader_full_path, x.compiled_shader_full_path);
  }

  u64 end = os_now_ns();
  Info("took seconds: %f", f64(end - start) / Billion(1));
  os_exit(0);
}
