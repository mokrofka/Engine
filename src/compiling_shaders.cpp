#include "lib.h"
#undef Debug
#define Debug(...)

i32 main(i32 args_count, char* args[]) {
  mem_track_init();
  os_init(args[0]);
  tctx_init();
  u64 start = os_now_ns();

  Scratch scratch;
  String cur_dir = os_cur_directory();
  Debug("current directory: %s", cur_dir);
  String shader_dir = push_strf(scratch, "%s/%s", cur_dir, String(args[1]));
  Debug("shader directory: %s", shader_dir);
  String compiled_shader_dir = push_strf(scratch, "%s/%s", cur_dir, String(args[2]));
  Debug("compiled directory: %s", compiled_shader_dir);
  String time_stamps_file_path = push_strf(scratch, "%s/%s", cur_dir, "saved_time_stamps_for_shad");

  if (!os_directory_path_exist(shader_dir)) {
    os_directory_make(shader_dir);
  }
  if (!os_directory_path_exist(compiled_shader_dir)) {
    os_directory_make(compiled_shader_dir);
  }

  String com_slang_file_path = push_strf(scratch, "%s/%s", shader_dir, String("com.slang"));
  String lib_slang_file_path = push_strf(scratch, "%s/%s", shader_dir, String("lib.slang"));
  DenseTime com_slang_modified = os_file_path_properties(com_slang_file_path).modified;
  DenseTime lib_slang_modified = os_file_path_properties(lib_slang_file_path).modified;
  FileProperties time_stamps_props = os_file_path_properties(time_stamps_file_path);

  struct {
    DenseTime com_slang_modified;
    DenseTime lib_slang_modified;
  } stamps;
  b32 is_change = false;
  if (!time_stamps_props.size) {
    is_change = true;
    OS_Handle file = os_file_open(time_stamps_file_path, OS_AccessFlag_Write);
    stamps = {
      com_slang_modified,
      lib_slang_modified,
    };
    os_file_write(file, slice_struct_to_bytes(&stamps));
  } else {
    OS_Handle file = os_file_open(time_stamps_file_path, OS_AccessFlag_Read | OS_AccessFlag_Write);
    os_file_read(file, slice_struct_to_bytes(&stamps));
    if (stamps.com_slang_modified != com_slang_modified || stamps.lib_slang_modified != lib_slang_modified) {
      is_change = true;
      stamps = {
        com_slang_modified,
        lib_slang_modified,
      };
      os_file_write(file, slice_struct_to_bytes(&stamps));
    }
  }

  struct Shader {
    String shader_path;
    String compiled_shader_path;
    b32 vert;
    b32 frag;
    b32 comp;
    OS_Handle pid;
  };
  var compiled_shaders = array_make(Shader, scratch);
  {
    OS_FileIter* it = os_file_iter_begin(scratch, shader_dir, OS_FileIterFlag_SkipFolders);
    for (OS_FileInfo info = {}; os_file_iter_next(scratch, it, &info);) {
      String shader_path = push_strf(scratch, "%s/%s", shader_dir, info.name);
      String shader_name = str_chop_last_dot(info.name);
      String compiled_shader_path = push_strf(scratch, "%s/%s.spv", compiled_shader_dir, shader_name);
      if ((os_file_path_mtime(shader_path) != os_file_path_mtime(compiled_shader_path) || is_change) && (!str_match(info.name, "com.slang") && !str_match(info.name, "lib.slang"))) {
        Shader f = {
          .shader_path = shader_path,
          .compiled_shader_path = compiled_shader_path,
        };
        array_push(compiled_shaders, f);
      }
    }
    os_file_iter_end(it);
  }

  Loop (i, compiled_shaders.count) {
    Shader& x = compiled_shaders[i];
    Debug("%s", x.shader_path);
    StringList list = {};
    str_list_push(scratch, &list, "slangc");
    str_list_push(scratch, &list, x.shader_path);
    str_list_push(scratch, &list, "-target");
    str_list_push(scratch, &list, "spirv");
    str_list_push(scratch, &list, "-g");
    str_list_push(scratch, &list, "-o");
    str_list_push(scratch, &list, x.compiled_shader_path);
    x.pid = os_process_launch(list);
  }

  Loop (i, compiled_shaders.count) {
    Shader& x = compiled_shaders[i];
    os_process_join(x.pid);
  }
  Loop (i, compiled_shaders.count) {
    Shader& x = compiled_shaders[i];
    os_file_path_copy_mtime(x.shader_path, x.compiled_shader_path);
  }

  u64 end = os_now_ns();
  Info("took seconds: %f", f64(end - start) / Billion(1));
  os_exit(0);
}

