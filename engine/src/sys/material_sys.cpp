#include "material_sys.h"

#include "render/r_frontend.h"
#include "sys/texture_sys.h"

struct MaterialSystemState {
  MaterialSystemConfig config;
  Material default_material;
  
  Material* registered_materials;
  
  Hashtable registered_material_table;
};

struct MaterialRef {
  u64 reference_count;
  u32 handle;
  b8 auto_release;
};

global MaterialSystemState* state;

internal void create_default_material();
internal void load_material(MaterialConfig config, Material* m);
internal void destroy_material(Material* m);
internal MaterialConfig load_configuration_file(String path);

void material_system_init(Arena* arena, MaterialSystemConfig config) {
  Assert(config.max_material_count != 0 && "material_system_initialize - config.max_material_count must be > 0");

  state = push_struct(arena, MaterialSystemState);
  state->registered_materials = push_array(arena, Material, config.max_material_count);
  state->registered_material_table = hashtable_create(arena, sizeof(MaterialRef), config.max_material_count, false);
  state->config = config;

  // Fill the hashtable with invalid references to use as a default.
  MaterialRef invalid_ref;
  invalid_ref.auto_release = false;
  invalid_ref.handle = INVALID_ID; // Primary reason for needing default values.
  invalid_ref.reference_count = 0;
  hashtable_fill(&state->registered_material_table, &invalid_ref);

  // Invalidate all materials in the array.
  Loop (i, state->config.max_material_count) {
    state->registered_materials[i].id = INVALID_ID;
    state->registered_materials[i].generation = INVALID_ID;
    state->registered_materials[i].internal_id = INVALID_ID;
  }

  create_default_material();
}

void material_system_shutdown() {
  // Invalidate all materials in the array.
  u32 count = state->config.max_material_count;
  Loop (i, count) {
    if (state->registered_materials[i].id != INVALID_ID) {
      destroy_material(&state->registered_materials[i]);
    }
  }

  // Destroy the default material.
  destroy_material(&state->default_material);
}

Material* material_system_acquire(String name) {
  Scratch scratch;
  // Load the given material configuration from disk.

  // Load file from disk
  // TODO: Should be able to be located anywhere.
  char* format_str = "assets/materials/%s.%s";

  // TODO: try different extensions
  String64 string = {};
  // str_copy(string, name);
  String file_path = push_strf(scratch.arena, format_str, name, "kmt"_);
  // String file_path = push_strf(scratch.arena, format_str, string, "kmt");
  MaterialConfig config = load_configuration_file(file_path);

  // Now acquire from loaded config.
  return material_system_acquire_from_config(config);
}

Material* material_system_acquire_from_config(MaterialConfig config) {
  // Return default material.
  if (str_matchi(config.name, DEFAULT_MATERIAL_NAME)) {
    return &state->default_material;
  }

  MaterialRef ref;
  hashtable_get(&state->registered_material_table, config.name, &ref);
  // This can only be changed the first time a material is loaded.
  if (ref.reference_count == 0) {
    ref.auto_release = config.auto_release;
  }
  ++ref.reference_count;
  if (ref.handle == INVALID_ID) {
    // This means no material exists here. Find a free index first.
    u32 count = state->config.max_material_count;
    Material* m = 0;
    Loop (i, count) {
      if (state->registered_materials[i].id == INVALID_ID) {
        // A free slot has been found. Use its index as the handle.
        ref.handle = i;
        m = &state->registered_materials[i];
        break;
      }
    }

    // Make sure an empty slot was actually found.
    if (!m || ref.handle == INVALID_ID) {
      Error("material_system_acquire - Material system cannot hold anymore materials. Adjust configuration to allow more"_);
      return 0;
    }

    // Create new material.
    load_material(config, m);

    if (m->generation == INVALID_ID) {
      m->generation = 0;
    } else {
      ++m->generation;
    }

    // Also use the handle as the material id.
    m->id = ref.handle;
    Trace("Material '%s' does not yet exist. Created, and ref_count is now %i", (String)config.name, ref.reference_count);
  } else {
    Trace("Material '%s' already exists, ref_count increased to %i", config.name, ref.reference_count);
  }

  // Update the entry.
  hashtable_set(&state->registered_material_table, config.name, &ref);
  return &state->registered_materials[ref.handle];

  // NOTE: This would only happen in the event something went wrong with the state.
  Error("material_system_acquire_from_config failed to acquire material '%s'. Null pointer will be returned", config.name);
  return 0;
}

void material_sys_release(String name) {
  // Ignore release requests for the default material.
  if (str_matchi(name, DEFAULT_MATERIAL_NAME)) {
    return;
  }
  MaterialRef ref;
  hashtable_get(&state->registered_material_table, name, &ref);
  if (ref.reference_count == 0) {
    Warn("Tried to release non-existent material: '%s'", name);
    return;
  }
  --ref.reference_count;
  if (ref.reference_count == 0 && ref.auto_release) {
    Material* m = &state->registered_materials[ref.handle];

    // Destroy/reset material.
    destroy_material(m);

    // Reset the reference.
    ref.handle = INVALID_ID;
    ref.auto_release = false;
    Trace("Released material '%s'., Material unloaded because reference count=0 and auto_release=true", name);
  } else {
    Trace("Released material '%s', now has a reference count of '%i' (auto_release=%s)", name, ref.reference_count, ref.auto_release ? "true"_ : "false"_);
  }

  // Update the entry.
  hashtable_set(&state->registered_material_table, name, &ref);
}

Material* material_sys_get_default() {
  return &state->default_material;
}

internal void load_material(MaterialConfig config, Material* m) {
  // name
  str_copy(m->name, config.name);

  // Diffuse colour
  m->diffuse_color = config.diffuse_color;

  // Diffuse map
  if (config.diffuse_map_name.size > 0) {
    m->diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;
    m->diffuse_map.texture = texture_system_acquire(config.diffuse_map_name, true);
    if (!m->diffuse_map.texture) {
      Warn("Unable to load texture '%s' for material '%s', using default", (String)config.diffuse_map_name, (String)m->name);
      m->diffuse_map.texture = texture_system_get_default_texture();
    }
  } else {
    // NOTE: Only set for clarity, as call to kzero_memory above does this already.
    m->diffuse_map.use = TEXTURE_USE_UNKNOWN;
    m->diffuse_map.texture = 0;
  }

  // TODO: other maps

  // Send it off to the renderer to acquire resources.
  r_create_material(m);
}

internal void destroy_material(Material* m) {
  Trace("Destroying material '%s'...", (String)m->name);

  // Release texture references.
  if (m->diffuse_map.texture) {
    texture_system_release(m->diffuse_map.texture->name);
  }

  // Release renderer resources.
  r_destroy_material(m);

  // Zero it out, invalidate IDs.
  MemZeroStruct(m);
  m->id = INVALID_ID;
  m->generation = INVALID_ID;
  m->internal_id = INVALID_ID;
}

internal void create_default_material() {
  state->default_material.id = INVALID_ID;
  state->default_material.generation = INVALID_ID;
  str_copy(state->default_material.name, DEFAULT_MATERIAL_NAME);
  state->default_material.diffuse_color = v4_one(); // white
  state->default_material.diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;
  state->default_material.diffuse_map.texture = texture_system_get_default_texture();

  r_create_material(&state->default_material);
}

internal MaterialConfig load_configuration_file(String path) {
  Scratch scratch;
  
  MaterialConfig out_config = {};
  OS_Handle f = os_file_open(path, OS_AccessFlag_Read);
  if (!f.u64) {
    Error("load_configuration_file - unable to open material file for reading: '%s'.", path);
    str_copy(out_config.name, DEFAULT_MATERIAL_NAME);
    return out_config;
  }
  u64 file_size = os_file_size(f);
  u8* buffer = push_buffer(scratch.arena, u8, file_size);
  os_file_read(f, file_size, buffer);

  // Read each line of the file.
  u64 line_length = 0;
  u32 line_number = 1;
  // while (filesystem_read_line(&f, 511, &p, &line_length)) {
  StringCursor cur = {buffer, buffer+file_size};
  while (cur.at < cur.end) {
    String line = str_read_line(&cur);
    
    // Trim the string.
    String trimmed = str_trim(line);

    // Get the trimmed length.
    line_length = trimmed.size;

    // Skip blank lines and comments.
    if (line_length < 1 || trimmed.str[0] == '#') {
      line_number++;
      continue;
    }

    // Split into var/value
    i32 equal_index = str_index_of(trimmed, '=');
    if (equal_index == -1) {
      Warn("Potential formatting issue found in file '%s': '=' token not found. Skipping line %ui.", path, line_number);
      ++line_number;
      continue;
    }

    String var_name = str_substr(trimmed, {0, equal_index});
    String trimmed_var_name = str_trim(var_name);

    // String value = str_mid(raw_value, trimmed, equal_index + 1, -1); // Read the rest of the line
    String value = str_substr(trimmed, {equal_index+1, (i32)trimmed.size});
    String trimmed_value = str_trim(value);

    // Process the variable.
    if (str_match(trimmed_var_name, "version"_)) {
      // TODO: version
    // } else if (str_equal(trimmed_var_name, "name"_)) {
    } else if (str_match(trimmed_var_name, "name"_)) {
      str_copy(out_config.name, trimmed_value);
    } else if (str_match(trimmed_var_name, "diffuse_map_name"_)) {
      str_copy(out_config.diffuse_map_name, trimmed_value);
    } else if (str_match(trimmed_var_name, "diffuse_color"_)) {
      // Parse the colour
      if (!str_to_v4(trimmed_value.str, &out_config.diffuse_color)) {
        Warn("Error parsing diffuse_colour in file '%s'. Using default of white instead.", path.str);
        out_config.diffuse_color = v4_one(); // white
      }
    }

    // TODO: more fields.

    ++line_number;
  }
  if (out_config.name.size == 0) {
    Error("material doesn't have name");
  }
  if (out_config.diffuse_map_name.size == 0) {
    Error("material doesn't have diffuse_map_name");
  }
  os_file_close(f);
  return out_config;
}
