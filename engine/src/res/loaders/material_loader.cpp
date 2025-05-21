#include "material_loader.h"

#include "loader_utils.h"

b32 material_loader_load(Arena* arena, ResLoader* self, String name, Res* out_res) {
  Scratch scratch(&arena, 1);
  Assert(self && name && out_res);

  String file_path = push_strf(scratch, "%s/%s/%s%s", res_sys_base_path(), (String)self->type_path64, name, ".kmt"_);

  str_copy(out_res->file_path64, file_path);

  MaterialConfig* res_data;
  // Set some defaults.
  // res_data->type = MaterialType_World;
  // res_data->auto_release = true;
  // res_data->diffuse_color = v4_one(); // white.
  // res_data->diffuse_map_name64.size = 0;
  // str_copy(res_data->name64, name);
  
  OS_Handle f = os_file_open(file_path, OS_AccessFlag_Read);
  if (!f) {
    Error("material_loader_load - unable to open material file for reading: '%s'", file_path);
    return false;
  }
  u64 file_size = os_file_size(f);
  u8* buffer = push_buffer(scratch, file_size);
  os_file_read(f, file_size, buffer);
  StringCursor cursor = {buffer, buffer+file_size};
  
  u32 line_length = 0;
  u32 line_number = 1;
  while (String line = str_read_line(&cursor)) {
    // Trim the string.
    String trimmed = str_trim(line);

    // Get the trimmed length.
    line_length = trimmed.size;

    // Skip blank lines and comments.
    if (trimmed.str[0] == '#') {
      ++line_number;
      continue;
    }

    // Split into var/value
    i32 equal_index = str_index_of(trimmed, '=');
    if (equal_index == -1) {
      Warn("Potential formatting issue found in file '%s': '=' token not found. Skipping line %u.", file_path, line_number);
      ++line_number;
      continue;
    }

    String var_name = str_substr(trimmed, {0, equal_index});
    String trimmed_var_name = str_trim(var_name);

    // String value = str_mid(raw_value, trimmed, equal_index + 1, -1); // Read the rest of the line
    String value = str_substr(trimmed, {equal_index+1, (i32)trimmed.size});
    String trimmed_value = str_trim(value);

    // Process the variable.
    if (str_matchi(trimmed_var_name, "version"_)) {
      // TODO: version
    } else if (str_matchi(trimmed_var_name, "name"_)) {
      str_copy(res_data->name64, trimmed_value);
    } else if (str_matchi(trimmed_var_name, "diffuse_map_name"_)) {
      str_copy(res_data->diffuse_map_name64, trimmed_value);
    } else if (str_matchi(trimmed_var_name, "diffuse_color"_)) {
      // Parse the colour
      if (!str_to_v4(trimmed_value.str, &res_data->diffuse_color)) {
        Warn("Error parsing diffuse_colour in file '%s'. Using default of white instead.", file_path);
      }
    } else if (str_matchi(trimmed_value, "type"_)) {
      // TODO other material types
      if (str_matchi(trimmed_value, "ui"_)) {
        res_data->type = MaterialType_UI;
      }
    }

    // TODO: more fields.

    ++line_number;
  }
  os_file_close(f);

  // Assign(out_res->data, res_data);
  out_res->data_size = sizeof(MaterialConfig);
  str_copy(out_res->name64, name);

  return true;
}

void material_loader_unload(ResLoader* self, Res* res) {
  res_unload(self, res);
}

ResLoader material_res_loader_create() {
  ResLoader loader;
  loader.type = ResType_Material;
  loader.custom_type64.size = 0;
  loader.load = material_loader_load;
  loader.unload = material_loader_unload;
  str_copy(loader.type_path64, "materials"_);

  return loader;
}

MaterialConfig res_load_material_config(String filepath) {
  Scratch scratch;
  MaterialConfig material_cfg = {};

  String file_path = push_strf(scratch, "%s/%s/%s%s", res_sys_base_path(), "materials"_, filepath, ".kmt"_);
  
  OS_Handle f = os_file_open(file_path, OS_AccessFlag_Read);
  if (!f) {
    Error("material_loader_load - unable to open material file for reading: '%s'", file_path);
    goto error;
  }
  
  material_cfg.auto_release = true;
  material_cfg.diffuse_color = v4_one(); // white.
  
  u64 file_size = os_file_size(f);
  u8* buffer = push_buffer(scratch, file_size);
  os_file_read(f, file_size, buffer);
  StringCursor cursor = {buffer, buffer+file_size};
  
  u32 line_length = 0;
  u32 line_number = 1;
  while (String line = str_read_line(&cursor)) {
    // Trim the string.
    String trimmed = str_trim(line);

    // Get the trimmed length.
    line_length = trimmed.size;

    // Skip blank lines and comments.
    if (trimmed.str[0] == '#') {
      ++line_number;
      continue;
    }

    // Split into var/value
    i32 equal_index = str_index_of(trimmed, '=');
    if (equal_index == -1) {
      Warn("Potential formatting issue found in file '%s': '=' token not found. Skipping line %u.", file_path, line_number);
      ++line_number;
      continue;
    }

    String var_name = str_substr(trimmed, {0, equal_index});
    String trimmed_var_name = str_trim(var_name);

    // String value = str_mid(raw_value, trimmed, equal_index + 1, -1); // Read the rest of the line
    String value = str_substr(trimmed, {equal_index+1, (i32)trimmed.size});
    String trimmed_value = str_trim(value);

    // Process the variable.
    if (str_matchi(trimmed_var_name, "version"_)) {
      // TODO: version
    } else if (str_matchi(trimmed_var_name, "name"_)) {
      str_copy(material_cfg.name64, trimmed_value);
    } else if (str_matchi(trimmed_var_name, "diffuse_map_name"_)) {
      str_copy(material_cfg.diffuse_map_name64, trimmed_value);
    } else if (str_matchi(trimmed_var_name, "diffuse_color"_)) {
      // Parse the colour
      if (!str_to_v4(trimmed_value.str, &material_cfg.diffuse_color)) {
        Warn("Error parsing diffuse_colour in file '%s'. Using default of white instead.", file_path);
      }
    } else if (str_matchi(trimmed_value, "type"_)) {
      // TODO other material types
      if (str_matchi(trimmed_value, "ui"_)) {
        material_cfg.type = MaterialType_UI;
      }
    }

    // TODO: more fields.

    ++line_number;
  }
  os_file_close(f);

  error:
  return material_cfg;
}
