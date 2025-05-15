#include "material_sys.h"

#include "render/r_frontend.h"
#include "sys/texture.h"
#include "sys/res_sys.h"

struct MaterialSystemState {
  MaterialSystemConfig config;
  Material default_material;
  
  Material* registered_materials;
  
  HashMap registered_material_table;
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

void material_system_init(Arena* arena, MaterialSystemConfig config) {
  Assert(config.max_material_count != 0 && "material_system_initialize - config.max_material_count must be > 0");

  state = push_struct(arena, MaterialSystemState);
  state->registered_materials = push_array(arena, Material, config.max_material_count);
  state->registered_material_table = hashmap_create(arena, sizeof(MaterialRef), config.max_material_count, false);
  state->config = config;

  // Fill the hashtable with invalid references to use as a default.
  MaterialRef invalid_ref;
  invalid_ref.auto_release = false;
  invalid_ref.handle = INVALID_ID; // Primary reason for needing default values.
  invalid_ref.reference_count = 0;
  hashmap_fill(&state->registered_material_table, &invalid_ref);

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
  // Res material_resource;
  MaterialConfig material_cfg = res_load_material_config(name);
  if (!material_cfg.name64) {
    Error("Failed to load material resource, returning null");
    return 0;
  }
  
  Material* m = material_system_acquire_from_config(material_cfg);
  
  // Clean up
  if (!m) {
    Error("Failed to load material resource, returning null");
  }
  
  return m;
}

Material* material_system_acquire_from_config(MaterialConfig config) {
  // Return default material.
  if (str_matchi(config.name64, DefaultMaterialName)) {
    return &state->default_material;
  }

  MaterialRef ref;
  hashmap_get(&state->registered_material_table, config.name64, &ref);
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
    Trace("Material '%s' does not yet exist. Created, and ref_count is now %i", (String)config.name64, ref.reference_count);
  } else {
    Trace("Material '%s' already exists, ref_count increased to %i", config.name64, ref.reference_count);
  }

  // Update the entry.
  hashmap_set(&state->registered_material_table, config.name64, &ref);
  return &state->registered_materials[ref.handle];

  // NOTE: This would only happen in the event something went wrong with the state.
  Error("material_system_acquire_from_config failed to acquire material '%s'. Null pointer will be returned", config.name64);
  return 0;
}

void material_sys_release(String name) {
  // Ignore release requests for the default material.
  if (str_matchi(name, DefaultMaterialName)) {
    return;
  }
  MaterialRef ref;
  hashmap_get(&state->registered_material_table, name, &ref);
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
  hashmap_set(&state->registered_material_table, name, &ref);
}

Material* material_sys_get_default() {
  return &state->default_material;
}

internal void load_material(MaterialConfig config, Material* m) {
  // name
  str_copy(m->name64, config.name64);
  
  // Type
  m->type = config.type;

  // Diffuse colour
  m->diffuse_color = config.diffuse_color;

  // Diffuse map
  if (config.diffuse_map_name64.size > 0) {
    m->diffuse_map.use = TextureUse_MapDiffuse ;
    m->diffuse_map.texture = texture_system_acquire(config.diffuse_map_name64, true);
    if (!m->diffuse_map.texture) {
      Warn("Unable to load texture '%s' for material '%s', using default", (String)config.diffuse_map_name64, (String)m->name64);
      m->diffuse_map.texture = texture_system_get_default_texture();
    }
  } else {
    // NOTE: Only set for clarity, as call to kzero_memory above does this already.
    m->diffuse_map.use = TextureUse_Unknown;
    m->diffuse_map.texture = 0;
  }

  // TODO: other maps

  // Send it off to the renderer to acquire resources.
  r_create_material(m);
}

internal void destroy_material(Material* m) {
  Trace("Destroying material '%s'...", (String)m->name64);

  // Release texture references.
  if (m->diffuse_map.texture) {
    texture_system_release(m->diffuse_map.texture->file_path64);
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
  str_copy(state->default_material.name64, DefaultMaterialName);
  state->default_material.diffuse_color = v4_one(); // white
  state->default_material.diffuse_map.use = TextureUse_MapDiffuse ;
  state->default_material.diffuse_map.texture = texture_system_get_default_texture();

  r_create_material(&state->default_material);
}
