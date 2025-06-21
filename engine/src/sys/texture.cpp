#include "texture.h"

#include "render/r_frontend.h"
#include "sys/res.h"

#include "vendor/stb_image.h"

struct TextureSystemState {
  Arena* arena;
  HashMap hashmap;
  u32 texture_count;
};

global TextureSystemState st;

#define MaxTextureCount KB(1)
void texture_init(Arena* arena) {
  st.arena = arena_alloc(arena, KB(1));
  st.hashmap = hashmap_create(arena, sizeof(u32), MaxTextureCount);
  u32 invalid_id = INVALID_ID;
  hashmap_fill(st.hashmap, &invalid_id);
}

Texture res_texture_load(String name) {
  Scratch scratch;
  Texture texture = {};
  
  u32 required_channel_count = 4;
  stbi_set_flip_vertically_on_load(true);

  String filepath = push_strf(scratch, "%s/%s/%s", res_sys_base_path(), str_lit("textures"), name);

  String filepath_c = push_str_copy(scratch, filepath);

  u8* data = stbi_load(
    (char*)filepath_c.str,
    (i32*)&texture.width,
    (i32*)&texture.height,
    (i32*)&texture.channel_count,
    required_channel_count);
  if (!data) {
    Error("Image resource loader failed to load file '%s'", filepath);
    return {};
  }

  texture.filepath = push_str_copy(st.arena, filepath);

  texture.data = data;
  texture.channel_count = required_channel_count;
  
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
  vk_texture_load(texture);
}

KAPI u32 texture_get(String name) {
  u32 id;
  hashmap_get(st.hashmap, name, &id);
  Assert(id != INVALID_ID);
  return id;
}
