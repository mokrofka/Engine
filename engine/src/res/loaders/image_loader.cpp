#include "image_loader.h"

#include "loader_utils.h"

#include "vendor/stb_image.h"

b32 image_loader_load(Arena* arena, ResLoader* self, String name, Res* out_res) {
  Scratch scratch(&arena, 1);
  Assert(self && name && out_res);
  
  i32 required_channel_count = 4;
  stbi_set_flip_vertically_on_load(true);

  // TODO: try different extensions
  String file_path = push_strf(scratch, "%s/%s/%s%s", res_sys_base_path(), (String)self->type_path64, name, ".png"_);

  i32 width;
  i32 height;
  i32 channel_count;

  // For now, assume 8 bits per channel, 4 channels.
  // TODO: extend this to make it configurable.
  String file_path_c = push_str_copy(scratch, file_path);
  u8* data = stbi_load(
    (char*)file_path_c.str,
    &width,
    &height,
    &channel_count,
    required_channel_count);

  if (!data) {
    Error("Image resource loader failed to load file '%s'", file_path);
    return false;
  }

  str_copy(out_res->file_path64, file_path);

  // TODO: Should be using an allocator here.
  // TextureRes* res_data = mem_alloc_struct(TextureRes);
  // res_data->pixels = data;
  // res_data->width = width;
  // res_data->height = height;
  // res_data->channel_count = required_channel_count;

  // Assign(out_res->data, res_data);
  // out_res->data_size = sizeof(TextureRes);
  // str_copy(out_res->name64, name);

  return true;
}

void image_loader_unload(ResLoader* self, Res* res) {
  res_unload(self, res);
}

ResLoader image_resource_loader_create() {
  ResLoader loader;
  loader.type = ResType_Image;
  loader.custom_type64.size = 0;
  loader.load = image_loader_load;
  loader.unload = image_loader_unload;
  str_copy(loader.type_path64, "textures"_);

  return loader;
}

Texture res_load_texture(String filepath) {
  Scratch scratch;
  Texture texture = {};
  
  i32 required_channel_count = 4;
  stbi_set_flip_vertically_on_load(true);

  // TODO: try different extensions
  String file_path = push_strf(scratch, "%s/%s/%s%s", res_sys_base_path(), "textures"_,filepath, ".png"_);

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

void res_unload_texture(void* data) {
  stbi_image_free(data);
}
