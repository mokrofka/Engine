#include "texture_system.h"
#include "render/r_frontend.h"

#include <logger.h>
#include <str.h>
#include <memory.h>
#include <containers/hashtable.h>

// TODO resource loader
#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"

struct TextureSystemState {
  TextureSystemConfig config;
  Texture default_texture;
  
  // Array of registered textures
  Texture* registered_textures;
  
  // Hashtable for texture lookups
  Hashtable registered_texture_table;
};

struct TextureReference {
  u64 reference_count;
  u32 handle;
  b8 auto_release;
};

global TextureSystemState* state;

internal void create_default_textures();
internal void destroy_default_textures(TextureSystemState* state);
internal b8 load_texture(char* texture_name, Texture* t);
internal void destroy_texture(Texture* t);

void texture_system_init(Arena* arena, TextureSystemConfig config) {
  if (config.max_texture_count == 0) {
    Error("texture_system_init - configl.max_texture_count must be > 0.");
  }
  
  // Block of memory will contain state struct, thenb block for array, then block for hashtable
  u64 struct_requirement = sizeof(TextureSystemState);
  u64 array_requirement = sizeof(Texture) * config.max_texture_count;
  u64 hashtable_requirement = sizeof(TextureReference) * config.max_texture_count;
  u64 memory_requirement = struct_requirement + array_requirement + hashtable_requirement;
  
  state = push_buffer(arena, TextureSystemState, memory_requirement);
  state->config = config;
  
  // The array block is after the state. Already allocated, so just set the pointer
  void* array_block = state + struct_requirement;
  state->registered_textures = (Texture*)array_block;
  
  // Hashtable block is after array
  void* hashtable_block = (u8*)array_block + array_requirement;
  
  // Create a hashtable for texture lookups
  hashtable_create(sizeof(TextureReference), config.max_texture_count, hashtable_block, false, &state->registered_texture_table);
  
  // Fill the hashtable with invalid references to use as a default
  TextureReference invalid_ref;
  invalid_ref.auto_release = false;
  invalid_ref.handle = INVALID_ID; // Primary reason for needing default values
  invalid_ref.reference_count = 0;
  hashtable_fill(&state->registered_texture_table, &invalid_ref);
  
  // Invalidate all textures in the array
  u32 count = state->config.max_texture_count;
  for (u32 i = 0; i < count; ++i) {
    state->registered_textures[i].id = INVALID_ID;
    state->registered_textures[i].generation = INVALID_ID;
  }
  
  // Create default textures for use in the system
  create_default_textures();
}

void texture_system_shutdown() {
  if (state) {
    // Destroy all loaded textures
    for (u32 i = 0; i < state->config.max_texture_count; ++i) {
      Texture* t = &state->registered_textures[i];
      if (t->generation != INVALID_ID) {
        r_destroy_texture(t);
      }
    }
  }
  
  destroy_default_textures(state);
  
  state = 0;
}

Texture* texture_system_acquire(char* name, b8 auto_release) {
  // Return default texture, but warn about it since this should be returned via get_default_texture(); 
  if (cstr_equali(name, DEFAULT_TEXTURE_NAME)) {
    Warn("texture_system_acquire called for default texture. Use texture_system_get_default_texture for texture 'default'.");
    return &state->default_texture;
  }
  
  TextureReference ref;
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
    for (u32 i = 0; i < count; ++i) {
      if (state->registered_textures[i].id == INVALID_ID) {
        // A free slot has been found. Use its index as the handle
        ref.handle = i;
        t = &state->registered_textures[i];
        break;
      };
    }

    // Make sure an empty slot was acturally found
    if (!t || ref.handle == INVALID_ID) {
      Fatal("texture_system_acquire - Texture system cannot hold anymore textures. Adjust configuration to allow more.");
      return 0;
    }

    // Create new texture
    if (!load_texture(name, t)) {
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
  Error("texture_system_acquire failed to acquire texture '%s'. Null pointer will be returned", name);
  return 0;
}

void texture_system_release(char* name) {
  // Ignore release requests for the default texture.
  if (cstr_equali(name, DEFAULT_TEXTURE_NAME)) {
    return;
  }
  TextureReference ref;
  hashtable_get(&state->registered_texture_table, name, &ref);
  if (ref.reference_count == 0) {
    Warn("Tried to release non-existent texture: '%s'", name);
    return;
  }

  // Take a copy of the name since it will be wiped out by destroy,
  // (as passed in name is generally a pointer to the actual texture's name)
  char name_copy[TEXTURE_NAME_MAX_LENGTH];
  MemCopy(name_copy, name, TEXTURE_NAME_MAX_LENGTH);

  --ref.reference_count;
  if (ref.reference_count == 0 && ref.auto_release) {
    Texture* t = &state->registered_textures[ref.handle];

    // Destroy/reset texture
    destroy_texture(t);

    // Reset the reference.
    ref.handle = INVALID_ID;
    ref.auto_release = false;
    Trace("Released texture '%s'., Texture unloaded because reference count=0 and auto_release=true.", name_copy);
  } else {
    Trace("Released texture '%s', now has a reference count of '%i' (auto_release=%s).", name_copy, ref.reference_count, ref.auto_release ? "true" : "false");
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
  Trace("Creating default texture...");
  const u32 tex_dimension = 256;
  const u32 channels = 4;
  const u32 pixel_count = tex_dimension * tex_dimension;
  u8 pixels[pixel_count * channels];
  MemSet(pixels, 255, sizeof(u8) * pixel_count * channels);

  // Each pixel.
  for (u64 row = 0; row < tex_dimension; ++row) {
    for (u64 col = 0; col < tex_dimension; ++col) {
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

  MemCopy(state->default_texture.name, DEFAULT_TEXTURE_NAME, TEXTURE_NAME_MAX_LENGTH);
  state->default_texture.width = tex_dimension;
  state->default_texture.height = tex_dimension;
  state->default_texture.channel_count = 4;
  state->default_texture.generation = INVALID_ID;
  state->default_texture.has_transparency = false;
  r_create_texture(pixels, &state->default_texture);
  // Manually set the texture generation to invalid since this is a default texture
  state->default_texture.generation = INVALID_ID;
}

internal void destroy_default_textures(TextureSystemState* state) {
  destroy_texture(&state->default_texture);
}

internal b8 load_texture(char* texture_name, Texture* t) {
  // TODO Should e able to be loaced anywhere 
  const char* format_str = "assets/textures/%s.%s";
  const i32 required_channel_count = 4;
  stbi_set_flip_vertically_on_load(true);
  u8 full_file_path[512];
  
  // TODO try different extensions
  str_format(full_file_path, format_str, texture_name, "png");
  
  // Use a temporary texture to load into
  Texture temp_texture;

  u8* data = stbi_load(
      (char*)full_file_path,
      (i32*)&temp_texture.width,
      (i32*)&temp_texture.height,
      (i32*)&temp_texture.channel_count,
      required_channel_count);

  temp_texture.channel_count = required_channel_count; 
  
  if (data) {
    u32 current_generation = t->generation;
    t->generation = INVALID_ID;
    
    u64 total_size = temp_texture.width * temp_texture.height * required_channel_count;
    // Check for transparency
    b32 has_transparency = false;
    for (u64 i = 0; i < total_size; ++i) {
      u8 a = data[i + 3];
      if (a < 255) {
        has_transparency = true;
        break;
      }
    }
    
    if (stbi_failure_reason()) {
      Warn("load_texture() failed to load file '%s': %s", full_file_path, stbi_failure_reason());
      // Clear the error so the next load doesn't fail
      stbi__err(0, 0);
      return false;
    }
    
    // Take a copy of the name
    MemCopy(temp_texture.name, texture_name, TEXTURE_NAME_MAX_LENGTH);
    temp_texture.generation = INVALID_ID;
    temp_texture.has_transparency = has_transparency;
    
    // Acquire internal texture resources and upload to GPU
    r_create_texture(data, &temp_texture);
  
    // Take a copy of the old texture
    *t = temp_texture;
    
    if (current_generation == INVALID_ID) {
      t->generation = 0;
    } else {
      t->generation = current_generation + 1;
    }
    
    // Clean up data
    stbi_image_free(data);
    return true;
  } else {
    if (stbi_failure_reason()) {
      Warn("load_texture() failed to load file '%s': %s", full_file_path, stbi_failure_reason());
      // Clear the error so the next load doesn't fail
      stbi__err(0, 0);
    }
    return false;
  }
}

internal void destroy_texture(Texture* t) {
  // Clean up back resources  
  r_destroy_texture(t);
  
  MemZeroStruct(t);
  t->id = INVALID_ID;
  t->generation = INVALID_ID;
}
