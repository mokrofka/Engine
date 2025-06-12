#include "texture.h"

#include "render/r_frontend.h"
#include "sys/res_sys.h"

#include "vendor/stb_image.h"

struct TextureSystemState {
  TextureSystemConfig config;
  Texture default_texture;
  
  // Array of registered textures
  Texture* registered_textures;
  
  // Hashtable for texture lookups
  HashMap registered_texture_table;
};

struct TextureRef {
  u32 reference_count;
  u32 handle;
};

global TextureSystemState st;

void texture_system_init(Arena* arena, TextureSystemConfig config) {
  st.registered_textures = push_array(arena, Texture, config.max_texture_count);
  st.registered_texture_table = hashmap_create(arena, sizeof(TextureRef), config.max_texture_count, false);
  
  st.config = config;
  
  TextureRef invalid_ref;
  invalid_ref.handle = INVALID_ID; // Primary reason for needing default values
  invalid_ref.reference_count = 0;
  hashmap_fill(st.registered_texture_table, &invalid_ref);
}

Texture res_texture_load(String filepath) {
  Scratch scratch;
  Texture texture = {};
  
  i32 required_channel_count = 4;
  stbi_set_flip_vertically_on_load(true);

  String file_path = push_strf(scratch, "%s/%s/%s", res_sys_base_path(), "textures"_, filepath);

  String file_path_c = push_str_copy(scratch, file_path);

  u8* data = stbi_load(
    (char*)file_path_c.str,
    (i32*)&texture.width,
    (i32*)&texture.height,
    (i32*)&texture.channel_count,
    required_channel_count);
  if (!data) {
    Error("Image resource loader failed to load file '%s'", file_path);
    goto error;
  }

  str_copy(texture.file_path64, file_path);

  texture.data = data;
  texture.channel_count = required_channel_count;
  
  error:
  return texture;
}

void res_texture_unload(void* data) {
  stbi_image_free(data);
}


internal void destroy_texture(Texture* t) {
  res_texture_unload(t->data);
}

void texture_load(String name) {
  Texture texture = res_texture_load(name);
  vk_texture_load(&texture);
}
