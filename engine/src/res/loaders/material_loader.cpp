#include "sys/res.h"

// MaterialConfig res_load_material_config(String filepath) {
//   Scratch scratch;
//   MaterialConfig material_cfg = {};

//   String file_path = push_strf(scratch, "%s/%s/%s%s", res_sys_base_path(), "materials", filepath, ".kmt");
  
//   OS_Handle f = os_file_open(file_path, OS_AccessFlag_Read);
//   if (!f) {
//     Error("material_loader_load - unable to open material file for reading: '%s'", file_path);
//     goto error;
//   }
  
//   material_cfg.auto_release = true;
//   material_cfg.diffuse_color = v4_one(); // white.
  
//   u64 file_size = os_file_size(f);
//   u8* buffer = push_buffer(scratch, file_size);
//   os_file_read(f, file_size, buffer);
//   StringCursor cursor = {buffer, buffer+file_size};
  
//   u32 line_length = 0;
//   u32 line_number = 1;
//   while (String line = str_read_line(&cursor)) {
//     // Trim the string.
//     String trimmed = str_trim(line);

//     // Get the trimmed length.
//     line_length = trimmed.size;

//     // Skip blank lines and comments.
//     if (trimmed.str[0] == '#') {
//       ++line_number;
//       continue;
//     }

//     // Split into var/value
//     u64 equal_index = str_index_of(trimmed, '=');
//     if (equal_index == -1) {
//       Warn("Potential formatting issue found in file '%s': '=' token not found. Skipping line %u.", file_path, line_number);
//       ++line_number;
//       continue;
//     }

//     String var_name = str_substr(trimmed, {0, equal_index});
//     String trimmed_var_name = str_trim(var_name);

//     // String value = str_mid(raw_value, trimmed, equal_index + 1, -1); // Read the rest of the line
//     String value = str_substr(trimmed, {equal_index+1, trimmed.size});
//     String trimmed_value = str_trim(value);

//     // Process the variable.
//     if (str_matchi(trimmed_var_name, "version")) {
//       // TODO: version
//     } else if (str_matchi(trimmed_var_name, "name")) {
//       str_copy(material_cfg.name64, trimmed_value);
//     } else if (str_matchi(trimmed_var_name, "diffuse_map_name")) {
//       str_copy(material_cfg.diffuse_map_name64, trimmed_value);
//     } else if (str_matchi(trimmed_var_name, "diffuse_color")) {
//       // Parse the colour
//       if (!str_to_v4(trimmed_value.str, &material_cfg.diffuse_color)) {
//         Warn("Error parsing diffuse_colour in file '%s'. Using default of white instead.", file_path);
//       }
//     } else if (str_matchi(trimmed_value, "type")) {
//       // TODO other material types
//       if (str_matchi(trimmed_value, "ui")) {
//         material_cfg.type = MaterialType_UI;
//       }
//     }

//     // TODO: more fields.

//     ++line_number;
//   }
//   os_file_close(f);

//   error:
//   return material_cfg;
// }
