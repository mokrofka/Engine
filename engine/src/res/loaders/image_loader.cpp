#include "image_loader.h"

#include "loader_utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"

b32 image_loader_load(ResLoader* self, String name, Res* out_res) {
  Scratch scratch;
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
    file_path_c,
    &width,
    &height,
    &channel_count,
    required_channel_count);

  // Check for a failure reason. If there is one, abort, clear memory if allocated, return false.
  const char* fail_reason = stbi_failure_reason();
  if (fail_reason) {
    Error("Image resource loader failed to load file '%s': %s", file_path, fail_reason);
    // Clear the error so the next load doesn't fail.
    stbi__err(0, 0);

    if (data) {
      stbi_image_free(data);
    }
    return false;
  }

  if (!data) {
    Error("Image resource loader failed to load file '%s'", file_path);
    return false;
  }

  str_copy(out_res->file_path64, file_path);

  // TODO: Should be using an allocator here.
  ImageResData* resource_data = mem_alloc_struct(ImageResData);
  resource_data->pixels = data;
  resource_data->width = width;
  resource_data->height = height;
  resource_data->channel_count = required_channel_count;

  Assign(out_res->data, resource_data);
  out_res->data_size = sizeof(ImageResData);
  str_copy(out_res->name64, name);

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
