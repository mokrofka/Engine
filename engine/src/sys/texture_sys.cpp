#include "texture_sys.h"

#include "render/r_frontend.h"

#include "sys/res_sys.h"

struct TextureSystemState {
  TextureSystemConfig config;
  Texture default_texture;
  
  // Array of registered textures
  Texture* registered_textures;
  
  // Hashtable for texture lookups
  Hashtable registered_texture_table;
};

struct TextureRef {
  u64 reference_count;
  u32 handle;
  b8 auto_release;
};

global TextureSystemState* state;

internal void create_default_textures();
internal void destroy_default_textures(TextureSystemState* state);
internal Texture load_texture(String texture_name);
internal void destroy_texture(Texture* t);

void texture_system_init(Arena* arena, TextureSystemConfig config) {
  Assert(config.max_texture_count > 0);
  
  state = push_struct(arena, TextureSystemState);
  state->registered_textures = push_array(arena, Texture, config.max_texture_count);
  state->registered_texture_table = hashtable_create(arena, sizeof(TextureRef), config.max_texture_count, false);
  
  state->config = config;
  
  // Fill the hashtable with invalid references to use as a default
  TextureRef invalid_ref;
  invalid_ref.auto_release = false;
  invalid_ref.handle = INVALID_ID; // Primary reason for needing default values
  invalid_ref.reference_count = 0;
  hashtable_fill(&state->registered_texture_table, &invalid_ref);
  
  // Invalidate all textures in the array
  u32 count = state->config.max_texture_count;
  Loop (i, count) {
    state->registered_textures[i].id = INVALID_ID;
    state->registered_textures[i].generation = INVALID_ID;
  }

  // Create default textures for use in the system
  create_default_textures();
}

void texture_system_shutdown() {
  if (state) {
    // Destroy all loaded textures
    Loop (i, state->config.max_texture_count) {
      Texture* t = &state->registered_textures[i];
      if (t->generation != INVALID_ID) {
        r_destroy_texture(t);
      }
    }
  }
  
  destroy_default_textures(state);
  
  state = 0;
}

Texture* texture_system_acquire(String name, b32 auto_release) {
  // Return default texture, but warn about it since this should be returned via get_default_texture(); 
  if (str_matchi(name, DefaultTextureName)) {
    Warn("texture_system_acquire called for default texture. Use texture_system_get_default_texture for texture 'default'.");
    return &state->default_texture;
  }
  
  TextureRef ref;
  hashtable_get(&state->registered_texture_table, name, &ref);
  // This can only be changed the firts time a texture is loaded
  if (ref.reference_count == 0) {
    ref.auto_release = auto_release;
  }
  ++ref.reference_count;
  if (ref.handle == INVALID_ID) {
    // This means no texture exists here.Find a free index first
    u32 count = state->config.max_texture_count;
    Texture* t = 0;
    Loop (i, count) {
      if (state->registered_textures[i].id == INVALID_ID) {
        // A free slot has been found. Use its index as the handle
        ref.handle = i;
        t = &state->registered_textures[i];
        break;
      };
    }

    // Check overflow
    if (!t || ref.handle == INVALID_ID) {
      Fatal("texture_system_acquire - Texture system cannot hold anymore textures. Adjust configuration to allow more.");
      return 0;
    }

    // Create new texture
    *t = load_texture(name);
    if (!t->internal_data) {
      Error("Failed to load texture '%s'.", name);
      return 0;
    }

    // Also use the handle as the texture id
    t->id = ref.handle;
    Trace("Texture '%s' does not yet exist. Created, and ref_count is now %i.", name, ref.reference_count);
  } else {
    Trace("Texture '%s' already exists, ref_count increased to %i.", name, ref.reference_count);
  }

  // Update the entry
  hashtable_set(&state->registered_texture_table, name, &ref);
  return &state->registered_textures[ref.handle];

  // NOTE: This would only happen in the event something went wrong with the state
  Error("texture_system_acquire failed to acquire texture '%s'. Null pointer will be returned", name.str);
  return 0;
}

void texture_system_release(String name) {
  Scratch scratch;
  // Ignore release requests for the default texture.
  if (str_matchi(name, DefaultTextureName)) {
    return;
  }
  TextureRef ref;
  hashtable_get(&state->registered_texture_table, name, &ref);
  if (ref.reference_count == 0) {
    Warn("Tried to release non-existent texture: '%s'", name);
    return;
  }

  // Take a copy of the name since it will be wiped out by destroy,
  // (as passed in name is generally a pointer to the actual texture's name)
  String name_copy = push_str_copy(scratch, name);

  --ref.reference_count;
  if (ref.reference_count == 0 && ref.auto_release) {
    Texture* t = &state->registered_textures[ref.handle];

    // Destroy/reset texture
    destroy_texture(t);

    // Reset the reference.
    ref.handle = INVALID_ID;
    ref.auto_release = false;
    Trace("Released texture '%s'., Texture unloaded because reference count=0 and auto_release=true", name_copy);
  } else {
    Trace("Released texture '%s', now has a reference count of '%i' (auto_release=%s)", name_copy, ref.reference_count, ref.auto_release ? "true"_ : "false"_);
  }

  // Update the entry.
  hashtable_set(&state->registered_texture_table, name_copy, &ref);
}

Texture* texture_system_get_default_texture() {
  return &state->default_texture;

  Error("texture_system_get_default_texture called before texture system initialization! Null pointer returned.");
  return 0;
}

internal void create_default_textures() {
  // NOTE: Create default texture, a 256x256 blue/white checkerboard pattern.
  // This is done in code to eliminate asset dependencies.
  Trace("Creating default texture..."_);
  const u32 tex_dimension = 256;
  const u32 channels = 4;
  const u32 pixel_count = tex_dimension * tex_dimension;
  u8 pixels[pixel_count * channels];
  MemSet(pixels, 255, sizeof(u8) * pixel_count * channels);

  // Each pixel.
  Loop (row, tex_dimension) {
    Loop (col, tex_dimension) {
      u64 index = (row * tex_dimension) + col;
      u64 index_bpp = index * channels;
      if (row % 2) {
        if (col % 2) {
          pixels[index_bpp + 0] = 0;
          pixels[index_bpp + 1] = 0;
        }
      } else {
        if (!(col % 2)) {
          pixels[index_bpp + 0] = 0;
          pixels[index_bpp + 1] = 0;
        }
      }
    }
  }

  str_copy(state->default_texture.file_path64, DefaultTextureName);
  state->default_texture.width = tex_dimension;
  state->default_texture.height = tex_dimension;
  state->default_texture.channel_count = 4;
  state->default_texture.generation = INVALID_ID;
  state->default_texture.has_transparency = false;
  state->default_texture.internal_data = r_create_texture(pixels, state->default_texture.width, state->default_texture.height, state->default_texture.channel_count);
  // Manually set the texture generation to invalid since this is a default texture
  state->default_texture.generation = INVALID_ID;
}

internal void destroy_default_textures(TextureSystemState* state) {
  destroy_texture(&state->default_texture);
}

internal Texture load_texture(String texture_name) {
  Texture texture = res_load_texture(texture_name);
  if (!texture.data) {
    goto error;
  }

  u64 total_size = texture.width * texture.height * texture.channel_count;
  // Check for transparency
  b32 has_transparency = false;
  Loop (i, total_size) {
    u8 a = texture.data[i + 3];
    if (a < 255) {
      has_transparency = true;
      break;
    }
  }

  texture.generation = INVALID_ID;
  texture.has_transparency = has_transparency;

  texture.internal_data = r_create_texture(texture.data, texture.width, texture.height, texture.channel_count);
  
  error:
  return texture;
}

internal void destroy_texture(Texture* t) {
  // Clean up back resources  
  r_destroy_texture(t);
  res_unload_texture(t->data);
  
  MemZeroStruct(t);
  t->id = INVALID_ID;
  t->generation = INVALID_ID;
}
