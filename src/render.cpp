#include "com.h"
#include "stb_image.h"
#include "stb_truetype.h"

static R_Vertex dummy_vertices[] = {
 // Body - front
 {{-1, -1, +1}, { 0,  0, +1}, {0, 0}, {0.8, 0.8, 0.8, 1}},
 {{+1, -1, +1}, { 0,  0, +1}, {1, 0}, {0.8, 0.8, 0.8, 1}},
 {{+1, +1, +1}, { 0,  0, +1}, {1, 1}, {0.8, 0.8, 0.8, 1}},
 {{-1, +1, +1}, { 0,  0, +1}, {0, 1}, {0.8, 0.8, 0.8, 1}},

 // Body - back
 {{+1, -1, -1}, { 0,  0, -1}, {0, 0}, {0.8, 0.8, 0.8, 1}},
 {{-1, -1, -1}, { 0,  0, -1}, {1, 0}, {0.8, 0.8, 0.8, 1}},
 {{-1, +1, -1}, { 0,  0, -1}, {1, 1}, {0.8, 0.8, 0.8, 1}},
 {{+1, +1, -1}, { 0,  0, -1}, {0, 1}, {0.8, 0.8, 0.8, 1}},

 // Body - right
 {{+1, -1, +1}, {+1,  0,  0}, {0, 0}, {0.8, 0.8, 0.8, 1}},
 {{+1, -1, -1}, {+1,  0,  0}, {1, 0}, {0.8, 0.8, 0.8, 1}},
 {{+1, +1, -1}, {+1,  0,  0}, {1, 1}, {0.8, 0.8, 0.8, 1}},
 {{+1, +1, +1}, {+1,  0,  0}, {0, 1}, {0.8, 0.8, 0.8, 1}},

 // Body - left
 {{-1, -1, -1}, {-1,  0,  0}, {0, 0}, {0.8, 0.8, 0.8, 1}},
 {{-1, -1, +1}, {-1,  0,  0}, {1, 0}, {0.8, 0.8, 0.8, 1}},
 {{-1, +1, +1}, {-1,  0,  0}, {1, 1}, {0.8, 0.8, 0.8, 1}},
 {{-1, +1, -1}, {-1,  0,  0}, {0, 1}, {0.8, 0.8, 0.8, 1}},

 // Roof - front
 {{-1, +1, +1}, {0, 0.707, 0.707}, {0, 0}, {1, 0.2, 0.1, 1}},
 {{+1, +1, +1}, {0, 0.707, 0.707}, {1, 0}, {1, 0.2, 0.1, 1}},
 {{ 0, +2, +1}, {0, 0.707, 0.707}, {0.5, 1}, {1, 0.2, 0.1, 1}},

 // Roof - back
 {{+1, +1, -1}, {0, 0.707, -0.707}, {0, 0}, {0.8, 0.1, 0.1, 1}},
 {{-1, +1, -1}, {0, 0.707, -0.707}, {1, 0}, {0.8, 0.1, 0.1, 1}},
 {{ 0, +2, -1}, {0, 0.707, -0.707}, {0.5, 1}, {0.8, 0.1, 0.1, 1}},

 // Roof - right
 {{+1, +1, +1}, {0.707, 0.707, 0}, {0, 0}, {1, 0.3, 0.1, 1}},
 {{+1, +1, -1}, {0.707, 0.707, 0}, {1, 0}, {1, 0.3, 0.1, 1}},
 {{ 0, +2,  0}, {0.707, 0.707, 0}, {0.5, 1}, {1, 0.3, 0.1, 1}},

 // Roof - left
 {{-1, +1, -1}, {-0.707, 0.707, 0}, {0, 0}, {0.9, 0.2, 0.1, 1}},
 {{-1, +1, +1}, {-0.707, 0.707, 0}, {1, 0}, {0.9, 0.2, 0.1, 1}},
 {{ 0, +2,  0}, {-0.707, 0.707, 0}, {0.5, 1}, {0.9, 0.2, 0.1, 1}},
};

static u32 dummy_indices[] = {
  // Body
  0, 1, 2,  0, 2, 3,
  4, 5, 6,  4, 6, 7,
  8, 9,10,  8,10,11,
  12,13,14, 12,14,15,

  // Roof
  16,17,18,
  19,20,21,
  22,23,24,
  25,26,27,
};

R_DrawBatch r_make_draw_batch(Allocator alloc, Gfx_Pipeline pip) {
  R_DrawBatch res = {
    .draws = array_make(R_DrawCall, alloc),
    .unindexed_draws = array_make(R_DrawCall, alloc),
  };
  return res;
}

R_Attachment r_make_attachment(R_AttachmentDesc desc) {
  R_Attachment res = {.type = desc.type};
  Gfx_ImageUsage usage = 0;
  switch (desc.type) {
    InvalidDefaultCase;
    case R_AttachmentType_Color: usage = Gfx_ImageUsage_ColorAttachment; break;
    case R_AttachmentType_Resolve: usage = Gfx_ImageUsage_ResolveAttachment; break;
    case R_AttachmentType_Depth: usage = Gfx_ImageUsage_DepthStencilAttachment; break;
  }
  Loop (i, st->gfx.images_in_flight) {
    res.images[i] = gfx_make_image({
      .usage = usage,
      .width = desc.size.x,
      .height = desc.size.y
    });
    switch (desc.type) {
      InvalidDefaultCase;
      case R_AttachmentType_Color: res.views[i] = gfx_make_view({Gfx_ViewType_ColorAttachment, res.images[i]}); break;
      case R_AttachmentType_Resolve: res.views[i] = gfx_make_view({Gfx_ViewType_ResolveAttachment, res.images[i]}); break;
      case R_AttachmentType_Depth: res.views[i] = gfx_make_view({Gfx_ViewType_DepthStencilAttachment, res.images[i]}); break;
    }
  }
  return res;
}
void r_destroy_attachment(R_Attachment attachment) {
  Loop (i, st->gfx.images_in_flight) {
    gfx_destroy_image(attachment.images[i]);
    gfx_destroy_view(attachment.views[i]);
  }
}
void r_recreate_attachment(R_Attachment* attachment, v2u size) {
  r_destroy_attachment(*attachment);
  *attachment = r_make_attachment({.type = attachment->type, size});
}

R_RenderTarget r_make_render_target(R_RenderTargetUsage usage, v2u size) {
  R_RenderTarget res = {.attachments = usage};
  if (FlagHas(usage, R_RenderTargetUsage_Color)) {
    res.color = r_make_attachment({.type = R_AttachmentType_Color, .size = size});
  }
  if (FlagHas(usage, R_RenderTargetUsage_Resolve)) {
    res.resolve = r_make_attachment({.type = R_AttachmentType_Resolve, .size = size});
  }
  if (FlagHas(usage, R_RenderTargetUsage_Depth)) {
    res.depth = r_make_attachment({.type = R_AttachmentType_Depth, .size = size});
  }
  return res;
}
void r_destroy_render_target(R_RenderTarget rt) {
  if (FlagHas(rt.attachments, R_RenderTargetUsage_Color)) {
    r_destroy_attachment(rt.color);
  }
  if (FlagHas(rt.attachments, R_RenderTargetUsage_Resolve)) {
    r_destroy_attachment(rt.resolve);
  }
  if (FlagHas(rt.attachments, R_RenderTargetUsage_Depth)) {
    r_destroy_attachment(rt.depth);
  }
}
void r_recreate_render_target(R_RenderTarget* rt, v2u size) {
  r_destroy_render_target(*rt);
  *rt = r_make_render_target(rt->attachments, size);
}
Gfx_Attachments r_render_target_to_attachments(R_RenderTarget rt) {
  Gfx_Attachments att = {};
  if (FlagHas(rt.attachments, R_RenderTargetUsage_Color)) {
    att.color = rt.color.views[st->gfx.current_image_idx];
  }
  if (FlagHas(rt.attachments, R_RenderTargetUsage_Resolve)) {
    att.resolve = rt.resolve.views[st->gfx.current_image_idx];
  }
  if (FlagHas(rt.attachments, R_RenderTargetUsage_Depth)) {
    att.depth_stencil = rt.depth.views[st->gfx.current_image_idx];
  }
  return att;
}

R_TextureDesc r_load_texture(String name) {
  Scratch scratch;
  u32 required_channel_count = 4;
  u32 channel_count;
  String path = push_strf(scratch, "%s/%s", st->textures_dir, name);
  Slice buf = os_file_path_read_all(scratch, path);
  R_TextureDesc res = {};
  res.data = stbi_load_from_memory(buf.data, buf.count, (i32*)&res.width, (i32*)&res.height, (i32*)&channel_count, required_channel_count);
  Assert(res.data);
  return res;
}

R_TextureDesc r_load_cubemap(String dir) {
  Scratch scratch;
  String sides[] = {
    "right", "left",
    "top", "bottom",
    "front", "back",
  };
  R_TextureDesc cube = {};
  LoopArray (i, sides) {
    String name = push_strf(scratch, "%s/%s%s", dir, sides[i], S(".png"));
    R_TextureDesc tex = r_load_texture(name);
    cube.cube[i] = tex.data;
    cube.width = tex.width;
    cube.height = tex.height;
  }
  return cube;
}

R_MeshDesc r_load_mesh(String name) {
  Scratch scratch;
  String filepath = push_strf(scratch, "%s/%s", st->models_dir, name);
  String format = str_skip_last_dot(name);
  R_MeshDesc res = {};
  if (str_match(format, "glb")) {
    res = load_gltf(scratch, filepath, true);
  } else if (str_match(format, "gltf")) {
    res = load_gltf(scratch, filepath, false);
  } else if (str_match(format, "obj")) {
    res = load_obj(scratch, filepath);
  } else {
    InvalidPath;
  }
  return res;
}

R_TextureId r_make_texture(R_TextureDesc desc) {
  Scratch scratch;
  var& g = st->r;
  _DefSet(desc.pixel_format, Gfx_PixelFormat_RGBA8);
  R_Texture dum = desc.is_cube ? pool_get(g.textures, g.dummy_cubemap) : pool_get(g.textures, g.dummy_texture);
  R_TextureId res = pool_push(g.textures, dum);
  struct Ctx {
    R_TextureDesc desc;
    R_TextureId texture_id;
  };
  var f = [](void* in){
    Scratch scratch;
    Ctx& ctx = *(Ctx*)in;
    var& g = st->r;
    var desc = ctx.desc;
    if (desc.name.size) {
      R_TextureDesc loaded = desc.is_cube ? r_load_cubemap(desc.name) : r_load_texture(desc.name);
      desc.width = loaded.width;
      desc.height = loaded.height;
      ArrayCopy(desc.cube, loaded.cube);
    }
    Slice<Slice<u8>> data = push_slice(scratch, Slice<u8>, 6);
    Loop (i, 6) {
      data[i] = Slice(desc.cube[i], desc.width * desc.height * gfx_pixelformat_bytesize(desc.pixel_format));
    }
    u32 stage_offset = desc.is_cube ? gfx_push_stage_buffers(data) : gfx_push_stage_buffer(data[0]);
    LockScope(g.push_to_gpu_queue_mutex);
    queue_push(g.push_to_gpu_queue, {
      .type = R_PushToGpuType_Texture,
      .texture_id = ctx.texture_id,
      .stage_off = stage_offset,
      .image_desc = {
        .type = desc.is_cube ? Gfx_ImageType_Cube : Gfx_ImageType_2D,
        .width = desc.width,
        .height = desc.height,
        .pixel_format = desc.pixel_format,
        .mipmaps = true,
      },
    });
  };
  if (desc.name.size) {
    var& ctx = thread_push_ctx(Ctx);
    ctx = {
      .desc = desc,
      .texture_id = res,
    };
    thread_push(&ctx, f);
  } else {
    Ctx ctx = {
      .desc = desc,
      .texture_id = res,
    };
    f(&ctx);
  }
  return res;
}

// R_TextureId r_make_texture(R_TextureDesc desc) {
//   var& g = st->r;
//   Scratch scratch;
//   _DefSet(desc.pixel_format, Gfx_PixelFormat_RGBA8);
//   R_Texture dum = desc.is_cube ? pool_get(g.textures, g.dummy_cubemap) : pool_get(g.textures, g.dummy_texture);
//   R_TextureId res = pool_push(g.textures, dum);
//   struct Ctx {
//     R_TextureId tex;
//     R_TextureDesc desc;
//   };
//   var f = [](void* in){
//     Scratch scratch;
//     Ctx& ctx = *(Ctx*)in;
//     var desc = ctx.desc;
//     var& g = st->r;
//     if (desc.name.size) {
//       R_TextureDesc loaded = ctx.desc.is_cube ? r_load_cubemap(ctx.desc.name) : r_load_texture(ctx.desc.name);
//       ctx.desc.width = loaded.width;
//       ctx.desc.height = loaded.height;
//       ArrayCopy(ctx.desc.cube, loaded.cube);
//     }
//     LockScope(g.async_mutex);
//     Gfx_Image image = gfx_make_image({
//       .type = ctx.desc.is_cube ? Gfx_ImageType_Cube : Gfx_ImageType_2D,
//       .width = ctx.desc.width,
//       .height = ctx.desc.height,
//       .pixel_format = ctx.desc.pixel_format,
//       .mipmaps = true,
//     });
//     Gfx_View view = gfx_make_view({Gfx_ViewType_Texture, image});
//     Slice<Slice<u8>> data = push_slice(scratch, Slice<u8>, 6);
//     Loop (i, 6) {
//       data[i] = Slice(ctx.desc.cube[i], ctx.desc.width * ctx.desc.height * gfx_pixelformat_bytesize(ctx.desc.pixel_format));
//     }
//     u32 counter = gfx_push_stage_buffer_cmd({
//       .type = Gfx_StageBufferCmdType_UploadTexture,
//       .img = image,
//       .stage_offset = ctx.desc.is_cube ? gfx_push_stage_buffers(data) : gfx_push_stage_buffer(data[0]),
//       .mipmaps = true,
//     });
//     queue_push(g.async_cmd_queue, {
//       .type = R_AsyncCmdType_UploadTexture,
//       .texture_id = ctx.tex,
//       .texture = {
//         .image = image,
//         .view = view,
//       },
//       .counter = counter,
//     });
//   };
//   if (desc.name.size) {
//     var& ctx = thread_push_ctx(Ctx);
//     ctx = {
//       .desc = desc,
//       .tex = res,
//     };
//     thread_push(&ctx, f);
//   } else {
//     Ctx ctx = {
//       .desc = desc,
//       .tex = res,
//     };
//     f(&ctx);
//   }
//   return res;
// }

R_TextureId r_make_dummy_texture(R_TextureDesc desc) {
  var& g = st->r;
  if (desc.name.size) {
    R_TextureDesc loaded = desc.is_cube ? r_load_cubemap(desc.name) : r_load_texture(desc.name);
    desc.width = loaded.width;
    desc.height = loaded.height;
    ArrayCopy(desc.cube, loaded.cube);
  }
  Gfx_ImageDesc img_desc = {
    .type = desc.is_cube ? Gfx_ImageType_Cube : Gfx_ImageType_2D,
    .width = desc.width,
    .height = desc.height,
    .pixel_format = desc.pixel_format,
    .mipmaps = true,
  };
  ArrayCopy(img_desc.cube, desc.cube);
  R_Texture tex = {
    .image = gfx_make_image(img_desc),
    .view = gfx_make_view({Gfx_ViewType_Texture, tex.image}),
  };
  R_TextureId res = pool_push(g.textures, tex);
  return res;
}

R_MeshId r_make_mesh(R_MeshDesc desc) {
  Scratch scratch;
  var& g = st->r;
  R_MeshId res = pool_push(g.meshes, pool_get(g.meshes, g.dummy_mesh));
  struct Ctx {
    R_MeshDesc desc;
    R_MeshId mesh_id;
  };
  var f = [](void* in){
    Scratch scratch;
    Ctx& ctx = *(Ctx*)in;
    var& g = st->r;
    var desc = ctx.desc;
    if (desc.name.size) {
      desc = r_load_mesh(desc.name);
    }
    os_mutex_lock(g.vert_index_buffer_mutex);
    u64 vert_buf_off = gfx_push_buffer(g.vert_reg, slice_size(desc.vertices));
    u64 index_buf_off = gfx_push_buffer(g.index_reg, slice_size(desc.indices));
    os_mutex_unlock(g.vert_index_buffer_mutex);

    u32 base_vert = vert_buf_off / sizeof(R_Vertex);
    u32 stage_offset = gfx_push_stage_buffer(slice_to_bytes(desc.vertices));
    u32 base_index = 0;
    if (desc.indices.count) {
      base_index = index_buf_off / sizeof(u32);
      gfx_push_stage_buffer(slice_to_bytes(desc.indices));
    }
    LockScope(g.push_to_gpu_queue_mutex);
    queue_push(g.push_to_gpu_queue, {
      .type = R_PushToGpuType_Mesh,
      .mesh_id = ctx.mesh_id,
      .mesh = {
        .vert_count = (u32)desc.vertices.count,
        .base_vert = base_vert,
        .index_count = (u32)desc.indices.count,
        .base_index = base_index,
      },
      .stage_off = stage_offset,
    });
  };
  if (desc.name.size) {
    var& ctx = thread_push_ctx(Ctx);
    ctx = {
      .desc = desc,
      .mesh_id = res,
    };
    thread_push(&ctx, f);
  } else {
    Ctx ctx = {
      .desc = desc,
      .mesh_id = res,
    };
    f(&ctx);
  }
  return res;
}

R_MeshId r_make_dummy_mesh(R_MeshDesc desc) {
  var& g = st->r;
  if (desc.name.size) {
    desc = r_load_mesh(desc.name);
  }
  u64 buf_offset = gfx_push_buffer(g.vert_reg, slice_size(desc.vertices));
  u32 base_vert = buf_offset / sizeof(R_Vertex);
  gfx_update_buffer(g.vert_reg, buf_offset, slice_to_bytes(desc.vertices));
  u32 base_index = 0;
  if (desc.indices.count) {
    u64 buf_offset = gfx_push_buffer(g.index_reg, slice_size(desc.indices));
    base_index = buf_offset / sizeof(u32);
    gfx_update_buffer(g.index_reg, buf_offset, slice_to_bytes(desc.indices));
  }
  Gfx_Mesh mesh = {
    .base_vert = base_vert,
    .vert_count = (u32)desc.vertices.count,
    .base_index = base_index,
    .index_count = (u32)desc.indices.count,
  };
  R_MeshId res = pool_push(g.meshes, mesh);
  return res;
}

u32 r_make_pipeline_state(Gfx_PipelineState s) {
  var& g = st->r;
  u32 res = array_push(g.batches, r_make_draw_batch(g.gpa, {}));
  g.batches[res].state = s;
  return res;
}

R_MaterialId r_make_material(R_Material mat) {
  var& g = st->r;
  if (!mat.base_color.idx) {
    mat.base_color = g.dummy_texture;
  }
  R_MaterialId res = pool_push(g.materials, mat);
  return res;
}

Gfx_Pipeline r_make_pipeline(String name, Gfx_PipelineDesc desc) {
  Scratch scratch;
  var& g = st->r;
  u32 module_idx = or_else(map_get(g.shader_to_module_idx, name),
    Slice code = os_file_path_read_all(scratch, push_strf(scratch, "%s/%s.spv", st->shader_compiled_dir, name));
    R_ShaderModuleWithPipelines entry = {.shd = gfx_make_shader(code)};
    u32 idx = array_push(g.shader_modules, entry);
    map_set(g.shader_to_module_idx, name, idx);
    idx;
  );
  desc.shader = g.shader_modules[module_idx].shd;
  Gfx_Pipeline pip = gfx_make_pipeline(desc);
  R_ShaderModuleWithPipelines& entry = g.shader_modules[module_idx];
  array_push(entry.pipelines, pip);
  return pip;
}

Gfx_Pipeline r_make_pipeline2(String name, Gfx_PipelineDesc2 desc) {
  Scratch scratch;
  var& g = st->r;
  u32 module_idx = or_else(map_get(g.shader_to_module_idx, name),
    Slice code = os_file_path_read_all(scratch, push_strf(scratch, "%s/%s.spv", st->shader_compiled_dir, name));
    R_ShaderModuleWithPipelines entry = {.shd = gfx_make_shader(code)};
    u32 idx = array_push(g.shader_modules, entry);
    map_set(g.shader_to_module_idx, name, idx);
    idx;
  );
  desc.shader = g.shader_modules[module_idx].shd;
  Gfx_Pipeline pip = gfx_make_pipeline2(desc);
  array_push(g.shader_modules[module_idx].pipelines, pip);
  return pip;
}

R_FontId r_make_font(R_FontDesc desc) {
  // return r_make_dummy_font(desc);
  Scratch scratch;
  var& g = st->r;
  R_FontId res = pool_push(g.fonts, pool_get(g.fonts, g.dummy_font));
  struct Ctx {
    R_FontDesc desc;
    R_FontId font_id;
  };
  var& ctx = thread_push_ctx(Ctx);
  ctx = {
    .desc = desc,
    .font_id = res,
  };
  thread_push(&ctx, [](void* in) {
    Scratch scratch;
    Ctx& ctx = *(Ctx*)in;
    var& g = st->r;
    String path = push_strf(scratch, "%s/%s/%s", st->asset_dir, S("fonts"), ctx.desc.name);
    Slice data = os_file_path_read_all(scratch, path);
    stbtt_bakedchar characters_info[96];
    u32 font_height = ctx.desc.font_height;
    u32 pixels_size = 96 * Square(font_height);
    u32 dim = Sqrt(pixels_size);
    u32 width = RoundUp(dim, font_height);
    u32 height = RoundUp(dim, font_height);
    u8* pixels = push_buffer(g.arena, width*height);
    stbtt_BakeFontBitmap(data.data, 0, font_height, pixels, width, height, 32, 96, characters_info);
    R_TextureId texture_id = r_make_texture({.width = width, .height = height, .data = pixels, .pixel_format = Gfx_PixelFormat_R8});
    R_Font font = {.font_height = font_height, .texture = texture_id};
    LoopArray (i, font.glyphs) {
      stbtt_bakedchar bakedchar = characters_info[i];
      font.glyphs[i] = {
        .rect.x0 = bakedchar.x0,
        .rect.y0 = bakedchar.y0,
        .rect.x1 = bakedchar.x1,
        .rect.y1 = bakedchar.y1,
        .xoff = bakedchar.xoff,
        .yoff = bakedchar.yoff,
        .xadvance = bakedchar.xadvance,
      };
    }
    LockScope(g.waiting_fonts_mutex);
    array_push(g.waiting_fonts, {
      .id = ctx.font_id,
      .font = font,
    });
  });
  return res;
}

R_FontId r_make_dummy_font(R_FontDesc desc) {
  Scratch scratch;
  var& g = st->r;
  String path = push_strf(scratch, "%s/%s/%s", st->asset_dir, S("fonts"), desc.name);
  Slice data = os_file_path_read_all(scratch, path);
  stbtt_bakedchar characters_info[96];
  u32 font_height = desc.font_height;
  u32 pixels_size = 96 * Square(font_height);
  u32 dim = Sqrt(pixels_size);
  u32 width = RoundUp(dim, font_height);
  u32 height = RoundUp(dim, font_height);
  u8* pixels = push_buffer(g.arena, width*height);
  stbtt_BakeFontBitmap(data.data, 0, font_height, pixels, width, height, 32, 96, characters_info);
  R_TextureId texture_id = r_make_texture({.width = width, .height = height, .data = pixels, .pixel_format = Gfx_PixelFormat_R8, .async = desc.async});
  R_Font font = {.font_height = font_height, .texture = texture_id};
  LoopArray (i, font.glyphs) {
    stbtt_bakedchar bakedchar = characters_info[i];
    font.glyphs[i] = {
      .rect.x0 = bakedchar.x0,
      .rect.y0 = bakedchar.y0,
      .rect.x1 = bakedchar.x1,
      .rect.y1 = bakedchar.y1,
      .xoff = bakedchar.xoff,
      .yoff = bakedchar.yoff,
      .xadvance = bakedchar.xadvance,
    };
  }
  R_FontId res = pool_push(g.fonts, font);
  return res;
}

b32 r_texture_is_ready(R_TextureId id) {
  var& g = st->r;
  return pool_get(g.textures, id).is_ready;
}

void r_readback_texture(R_TextureId t, u8* dst) {
  var& g = st->r;
  R_Texture texture = pool_get(g.textures, t);
  gfx_readback_image(texture.image, dst);
}

u32 r_texture_descriptor_idx(R_TextureId id) {
  var& g = st->r;
  u32 res = pool_get(g.textures, id).view.idx;
  return res;
}

void r_set_cubemap(R_TextureId cubemap) {
  var& g = st->r;
  g.cur_cubemap = cubemap;
}

void r_update_texture(R_TextureId t, u8* data) {
  var& g = st->r;
  R_Texture tex = pool_get(g.textures, t);
  gfx_update_image(tex.image, data);
}

void r_update_mesh(R_MeshId mesh, R_MeshDesc desc) {
}

void r_destroy_texture(R_TextureId t) {
  var& g = st->r;
  R_Texture texture = pool_get(g.textures, t);
  gfx_destroy_image(texture.image);
  gfx_destroy_view(texture.view);
  pool_remove(g.textures, t);
}

void r_destroy_mesh(R_MeshId mesh) {
}

void r_material_destroy(R_MaterialId mat) {
  var& g = st->r;
  pool_remove(g.materials, mat);
}

void r_reload_shader(String name) {
  Scratch scratch;
  R_State& g = st->r;
  Info("reload '%s' shader", name);
  var module_idx = map_get(g.shader_to_module_idx, name);
  Assert(module_idx.ok);
  R_ShaderModuleWithPipelines entry = g.shader_modules[module_idx.value];
  Slice code = os_file_path_read_all(scratch, push_strf(scratch, "%s/%s.spv", st->shader_compiled_dir, name));
  gfx_update_shader(entry.shd, code);
  Loop (i, entry.pipelines.count) {
    Gfx_Pipeline pip = entry.pipelines[i];
    // gfx_update_pipeline(pip, gfx_query_pipeline_desc(pip));
    gfx_update_pipeline2(pip, gfx_query_pipeline_desc2(pip));
  }
}

void r_shaders_compile(Allocator arena) {
  Scratch scratch(arena);
  String cur_dir = os_cur_directory();
  String shader_dir = st->shader_dir;
  String compiled_shader_dir = st->shader_compiled_dir;
  String saved_time_stamps = push_strf(scratch, "%s/%s", cur_dir, String("saved_time_stamps_for_shad"));

  if (!os_directory_path_exist(shader_dir)) {
    os_directory_make(shader_dir);
  }
  if (!os_directory_path_exist(compiled_shader_dir)) {
    os_directory_make(compiled_shader_dir);
  }

  String com_path = push_strf(scratch, "%s/%s", shader_dir, String("com.slang"));
  String lib_path = push_strf(scratch, "%s/%s", shader_dir, String("lib.slang"));
  String header_path = push_strf(scratch, "%s/../src/shader_header.h", os_cur_directory());
  DenseTime com_modified = os_file_path_mtime(com_path);
  DenseTime lib_modified = os_file_path_mtime(lib_path);
  DenseTime header_modified = os_file_path_mtime(header_path);
  FileProperties time_stamp_file_props = os_file_path_properties(saved_time_stamps);

  ///////////////////////////////////
  // Recompile?
  struct FileData {
    u64 com_modified;
    u64 lib_modified;
    u64 header_modified;
  };
  b32 recompile = false;
  if (time_stamp_file_props.size == 0) {
    recompile = true;
  } else {
    Slice buf = os_file_path_read_all(scratch, saved_time_stamps);
    FileData* data = (FileData*)buf.data;
    if (com_modified != data->com_modified || lib_modified != data->lib_modified || header_modified != data->header_modified) {
      recompile = true;
    }
  }
  if (recompile) {
    FileData data = {
      .com_modified = com_modified,
      .lib_modified = lib_modified,
      .header_modified = header_modified,
    };
    os_file_path_write_all(saved_time_stamps, slice_struct_to_bytes(&data));
  }

  ///////////////////////////////////
  // Which files recompile?
  struct File {
    String file_path;
    String compiled_file_path;
    String shader_name;
    OS_Handle pid;
  };
  var files = array_make(File, scratch);
  Slice infos = os_file_iter_directory(scratch, shader_dir, OS_FileIterFlag_SkipFolders);
  Loop (i, infos.count) {
    String name = infos[i].name;
    String file_path = push_strf(scratch, "%s/%s", shader_dir, name);
    String shader_name = str_chop_last_dot(name);
    String compiled_file_path = push_strf(scratch, "%s/%s.spv", compiled_shader_dir, shader_name);
    if (str_match(name, "com.slang") || str_match(name, "lib.slang")) {
      continue;
    }
    FileProperties compiled_props = os_file_path_properties(compiled_file_path);
    if (infos[i].props.modified != compiled_props.modified || recompile) {
      File f = {
        .file_path = file_path,
        .compiled_file_path = compiled_file_path,
        .shader_name = shader_name,
      };
      array_push(files, f);
    }
  }

  ///////////////////////////////////
  // Compilation
  R_State& g = st->r;
  var file_names = array_make(String, arena);
  g.shader_module_compilation_pids = push_slice(st->arena, OS_Handle, files.count);
  g.shaders_to_compile = push_slice(st->arena, String, files.count);
  Loop (i, files.count) {
    File& f = files[i];
    var arr = array_make(String, scratch);
    array_push(arr, S("slangc"), f.file_path, S("-target"), S("spirv"), S("-O0"), S("-g3"), S("-o"), f.compiled_file_path);
    g.shader_module_compilation_pids[i] = os_process_make(slice(arr));
    Debug("%s", f.file_path);
    array_push(file_names, f.shader_name);
  }
  g.shaders_to_compile = slice(file_names);
}

void r_shaders_compile_join() {
  Scratch scratch;
  R_State& g = st->r;
  Loop (i, g.shaders_to_compile.count) {
    os_process_join(g.shader_module_compilation_pids[i]);
    String shader_file_path = push_strf(scratch, "%s/%s.slang", st->shader_dir, g.shaders_to_compile[i]);
    String compiled_file_path = push_strf(scratch, "%s/%s.spv", st->shader_compiled_dir, g.shaders_to_compile[i]);
    os_file_path_copy_mtime(shader_file_path, compiled_file_path);
  }
}

void r_bind_pipeline(Gfx_Pipeline pip) {
  var& g = st->r;
  g.cur_pip = pip;
  gfx_bind_pipeline(pip);
}

void r_default_pipeline_state(Gfx_PipelineState* s) {
  _DefSet(s->primitive_type, Gfx_PrimitiveType_Triangle);
  _DefSet(s->cull_mode, Gfx_CullMode_None);
  _DefSet(s->face_winding, Gfx_FaceWinding_CCW);
  _DefSet(s->depth.compare, Gfx_CompareOp_Always);
  _DefSet(s->blend.write_mask, Gfx_ColorMask_RGBA);
  _DefSet(s->blend.op_rgb, Gfx_BlendOp_Add);
  _DefSet(s->blend.op_alpha, Gfx_BlendOp_Add);
}

void r_apply_state_raw(Gfx_PipelineState s = {}) {
  gfx_apply_primitive_type(s.primitive_type);
  gfx_apply_cull_mode(s.cull_mode);
  gfx_apply_face_winding(s.face_winding);
  gfx_apply_depth_test(s.depth.test_disable);
  gfx_apply_depth_write(s.depth.write_enabled);
  gfx_apply_depth_compare(s.depth.compare);
  gfx_apply_color_blend_enable(s.blend.enabled);
  if (s.blend.enabled) {
    gfx_apply_color_blend_equation(s.blend);
  }
  gfx_apply_color_blend_mask(s.blend);
}

void r_apply_state(Gfx_PipelineState s) {
  var& g = st->r;
  r_default_pipeline_state(&s);
  if (g.cur_pip.idx != g.prev_pip.idx) {
    g.prev_pip = g.cur_pip;
    r_apply_state_raw(s);
  } else {
    var& prev = g.prev_pip_state;
    if (s.primitive_type != prev.primitive_type) {
      gfx_apply_primitive_type(s.primitive_type);
    }
    if (s.cull_mode != prev.cull_mode) {
      gfx_apply_cull_mode(s.cull_mode);
    }
    if (s.face_winding != prev.face_winding) {
      gfx_apply_face_winding(s.face_winding);
    }
    if (s.depth.test_disable != prev.depth.test_disable) {
      gfx_apply_depth_test(s.depth.test_disable);
    }
    if (s.depth.write_enabled != prev.depth.write_enabled) {
      gfx_apply_depth_write(s.depth.write_enabled);
    }
    if (s.depth.compare != prev.depth.compare) {
      gfx_apply_depth_compare(s.depth.compare);
    }
    if (s.depth.write_enabled != prev.depth.write_enabled) {
      gfx_apply_color_blend_enable(s.blend.enabled);
    }
    if (s.blend.enabled) {
      Gfx_BlendState b = s.blend;
      Gfx_BlendState pb = prev.blend;
      if (b.src_factor_rgb != pb.src_factor_rgb || b.dst_factor_rgb != pb.dst_factor_rgb || b.op_rgb != pb.op_rgb ||
          b.src_factor_alpha != pb.src_factor_alpha || b.dst_factor_alpha != pb.dst_factor_alpha || b.op_alpha != pb.op_alpha)
      {
        gfx_apply_color_blend_equation(s.blend);
      }
    }
    if (s.blend.write_mask != prev.blend.write_mask) {
      gfx_apply_color_blend_mask(s.blend);
    }
  }
  g.prev_pip_state = s;
}

void r_init() {
  ProfFunc;
  Scratch scratch;
  Arena arena = arena_make(.name = "render arena");
  R_State& g = st->r;
  g.arena = arena;
  g.gpa = alloc_make(g.arena);
  g.scale = 1;
  g.push_to_gpu_queue_mutex = os_mutex_make();
  g.vert_index_buffer_mutex = os_mutex_make();
  g.waiting_fonts_mutex = os_mutex_make();

  gfx_init({.cpu_mem_size = MB(100), .gpu_mem_size = MB(10), .image_mem_size = MB(10)});

  ///////////////////////////////////
  // Buffers
  {
    g.cpu_vert_reg = gfx_make_buffer(MB(1), Gfx_MemType_Cpu);
    g.cpu_vertices = (R_Vertex*)gfx_buffer_base_ptr(g.cpu_vert_reg);
    g.vert_reg = gfx_make_buffer(MB(1), Gfx_MemType_Gpu);
    g.index_reg = gfx_make_buffer(MB(1), Gfx_MemType_Gpu);
  }

  ///////////////////////////////////
  // Descriptors
  {
    gfx_make_bind({.type = Gfx_BindType_Image, .binding = Bindings::Textures, .count = Gfx_MaxImages});
    gfx_make_bind({.type = Gfx_BindType_Sampler, .binding = Bindings::Samplers, .count = Gfx_MaxSamplers});
    gfx_make_bind({.type = Gfx_BindType_Image, .binding = Bindings::CubeTextures, .count = Gfx_MaxCubeTextures});
    Gfx_BufferDesc buffers[] = {
      {
        .binding = Bindings::State,
        .size = sizeof(GpuState),
        .out_cpu_ptr = (void**)&g.gpu_state,
      },
      {
        .binding = Bindings::Entities,
        .size = sizeof(GpuEntity) * MaxEntities,
        .out_cpu_ptr = (void**)&g.gpu_entities,
      },
      {
        .binding = Bindings::Materials,
        .size = sizeof(GpuMaterial) * R_MaxMaterials,
        .out_cpu_ptr = (void**)&g.gpu_materials,
      },
      {
        .binding = Bindings::DrawCtx,
        .size = sizeof(GpuDrawCall) * MaxEntities,
        .out_cpu_ptr = (void**)&g.gpu_drawcalls,
      },
      {
        .binding = Bindings::SoftwareRender,
        .size = (Scope(v2u size = os_screen_size(); size.x * size.y * sizeof(u32);)),
        .out_cpu_ptr = (void**)&g.gpu_software_render,
      },
      {
        .binding = Bindings::UI_RectBinding,
        .size = sizeof(R_UI_Rect) * MaxEntities,
        .out_cpu_ptr = (void**)&g.gpu_ui_rects,
      }
    };
    gfx_make_binding_buffers(slice(buffers));
    gfx_flush();
    g.gpu_state->p = st->gfx.cpu_buf_address;
  }

  g.world_rt = r_make_render_target(R_RenderTargetUsage_Color | R_RenderTargetUsage_Resolve | R_RenderTargetUsage_Depth, os_window_size());
  g.com_sampler = gfx_make_sampler({});

  {
    ProfBlock("Waiting for compiling shaders");
    r_shaders_compile_join();
  }

  ///////////////////////////////////
  // Dummy
  {
    R_TextureDesc dum = r_load_texture("dummy.png");
    g.dummy_texture = r_make_dummy_texture({
      .data = dum.data,
      .width = dum.width,
      .height = dum.height,
    });
    R_TextureDesc cube = {
      .width = dum.width,
      .height = dum.width,
      .is_cube = true,
    };
    Loop (i, 6) {
      cube.cube[i] = (u8*)dum.data;
    }
    g.dummy_cubemap = r_make_dummy_texture(cube);
    g.gpu_state->cubemap = r_texture_descriptor_idx(g.dummy_cubemap);
    g.dummy_mesh = r_make_dummy_mesh({.name = "dummy.glb"});
    g.dummy_font = r_make_dummy_font({"Roboto-Bold.ttf", 32});
  }

  ///////////////////////////////////
  // Load basic shaders
  {
    g.uber_pip = r_make_pipeline2("uber", {});
    g.uber_pip_screen = r_make_pipeline2("uber", {
      .sample_count = 1,
    });
  }
  {
    g.ui_rect_pip = r_make_pipeline2("ui_rect", {});
  }

  Info("Renderer initialized");

}

void r_shutdown() {
  gfx_shutdown();
}

void r_begin() {
  imgui_begin_frame();
}

void r_end() {
  ProfFunc;
  Scratch scratch;
  var& g = st->r;
  gfx_begin();
  g.gpu_state->res.x = os_window_size().x;
  g.gpu_state->res.y = os_window_size().y;
  g.prev_pip = {};

  // Resize?
  if (st->gfx.swapchain_resized || g.old_scale != g.scale) {
    g.old_scale = g.scale;
    gfx_idle();
    v2u win_size = os_window_size();
    r_recreate_render_target(&g.world_rt, win_size);
  }

  // Push to gpu
  {
    LockScope(g.push_to_gpu_queue_mutex);
    Loop (i, g.push_to_gpu_queue.count) {
      var push = queue_pop(g.push_to_gpu_queue);
      switch (push.type) {
        InvalidDefaultCase;
        case R_PushToGpuType_Texture: {
          Gfx_Image image = gfx_make_image(push.image_desc);
          Gfx_View view = gfx_make_view({Gfx_ViewType_Texture, image});
          u32 counter = gfx_push_stage_buffer_cmd({
            .type = Gfx_CmdType_Texture,
            .img = image,
            .stage_offset = push.stage_off,
          });
          push.texture = {
            .image = image,
            .view = view,
          };
          push.counter = counter;
          queue_push(g.finished_gpu_queue, push);
        } break;
        case R_PushToGpuType_Mesh: {
          var mesh = push.mesh;
          u32 vert_size = mesh.vert_count*sizeof(R_Vertex);
          u32 counter = gfx_push_stage_buffer_cmd({
            .type = Gfx_CmdType_Mesh,
            .stage_offset = push.stage_off,
            .buf = g.vert_reg,
            .size = vert_size,
            .buf_offset = push.mesh.base_vert*sizeof(R_Vertex),
          });
          if (mesh.index_count) {
            gfx_push_stage_buffer_cmd({
              .type = Gfx_CmdType_Mesh,
              .stage_offset = push.stage_off + vert_size,
              .buf = g.index_reg,
              .size = mesh.index_count*sizeof(u32),
              .buf_offset = push.mesh.base_index*sizeof(u32),
            });
          }
          push.counter = counter;
          queue_push(g.finished_gpu_queue, push);
        } break;
      }
    }
  }

  // Update dummies
  {
    Loop (i, g.finished_gpu_queue.count) {
      if (queue_front(g.finished_gpu_queue).counter <= gfx_ready_counter()) {
        var slot = queue_pop(g.finished_gpu_queue);
        switch (slot.type) {
          InvalidDefaultCase;
          case R_PushToGpuType_Texture: {
            var& t = pool_get(g.textures, slot.texture_id);
            t = slot.texture;
            t.is_ready = true;
          } break;
          case R_PushToGpuType_Mesh: {
            var& t = pool_get(g.meshes, slot.mesh_id);
            t = slot.mesh;
          } break;
        }
      } else break;
    } 

    LockScope(g.waiting_fonts_mutex);
    LoopNoInc (i, g.waiting_fonts.count) {
      var slot = g.waiting_fonts[i];
      if (r_texture_is_ready(slot.font.texture)) {
        pool_get(g.fonts, slot.id) = slot.font;
        array_swap_remove(g.waiting_fonts, i);
      } else {
        ++i;
      }
    }
  }

  {
    var& gpu_st = *g.gpu_state;
    gpu_st.projection_view = st->projection * st->view;
    gpu_st.projection = st->projection;
    gpu_st.view = st->view;
    gpu_st.ambient_color = st->ambient_color;
    u32 idx = 0;
    LoopIter (it, pool_begin(g.materials)) {
      var& mat = *it;
      mat.idx = idx;
      R_MaterialProps props = mat.props;
      g.gpu_materials[idx++] = {
        .ambient = props.ambient,
        .diffuse = props.diffuse,
        .specular = props.specular,
        .shininess = props.shininess,
        .tex = r_texture_descriptor_idx(mat.base_color),
      };
    }
    gpu_st.cubemap = r_texture_descriptor_idx(g.cur_cubemap);
  }

  u32 drawcall_count = 0;

  ///////////////////////////////////
  // World
  {
    gfx_begin_pass({.attachments = r_render_target_to_attachments(g.world_rt)});
    {
      gfx_apply_viewport(rng2_make(v2(), os_window_size()), true);
      gfx_apply_scissor(rng2_make(v2(), os_window_size()));
      gfx_bind_vert(g.vert_reg);
      gfx_bind_index(g.index_reg);
      r_bind_pipeline(g.uber_pip);
      for (var& batch : g.batches) {
        r_apply_state(batch.state);
        var emit_batch = [&](Slice<R_DrawCall> pushes, b32 indexed) {
          u32 base = gfx_begin_indirect();
          Loop (i, pushes.count) {
            R_DrawCall draw = pushes[i];
            m4x4 model = m4x4_transform(draw.scale, draw.pos, draw.rot);
            // m4x4 model = m4x4_from_quat(draw.rot) * m4x4_translate(draw.pos) * m4x4_scale(draw.scale);
            // m4x4 model =  m4x4_translate(draw.pos) * m4x4_scale(draw.scale) * m4x4_from_quat(draw.rot);
            var mat = pool_get(g.materials, draw.mat);
            g.gpu_drawcalls[drawcall_count] = {
              .model = model,
              .color = draw.color,
              .mat = mat.idx,
              .type = draw.type,
            };
            Gfx_Mesh mesh = pool_get(g.meshes, draw.mesh);
            gfx_push_indirect_mesh(mesh, drawcall_count++);
          }
          Gfx_IndirectDrawCall draw = gfx_end_indirect(base);
          if (draw.count) {
            vk_push_constants({draw.base});
            if (indexed) {
              gfx_draw_indexed_indirect(draw);
            } else {
              gfx_draw_indirect(draw);
            }
          }
        };
        emit_batch(slice(batch.draws), true);
        emit_batch(slice(batch.unindexed_draws), false);
        array_clear(batch.draws);
        array_clear(batch.unindexed_draws);
      }

      // Debug drawing
      gfx_bind_vert(g.cpu_vert_reg);
      if (g.draw_lines.count) {
        MemCopyArray(g.cpu_vertices, g.draw_lines.data, g.draw_lines.count);
        r_apply_state({
          .primitive_type = Gfx_PrimitiveType_Line,
          .depth = {
            .compare = Gfx_CompareOp_Less,
            .write_enabled = true,
          },
          .blend = {
            .enabled = true,
            .src_factor_rgb = Gfx_BlendFactor_SrcAlpha,
            .dst_factor_rgb = Gfx_BlendFactor_OneMinusSrcAlpha,
            .src_factor_alpha = Gfx_BlendFactor_SrcAlpha,
            .dst_factor_alpha = Gfx_BlendFactor_OneMinusSrcAlpha,
          }
        });
        g.gpu_drawcalls[drawcall_count].type = ShaderType::Line;
        vk_push_constants({drawcall_count++});
        gfx_draw(0, g.draw_lines.count);
        array_clear(g.draw_lines);
      }
      // if (g.draw_lines_persistent.count) {
      //   gfx_bind_pipeline(g.uber_pip_debug_line);
      //   gfx_draw(g.draw_base_persistent_lines, g.draw_lines_persistent.count);
      // }

      // Cube map
      r_apply_state({.depth = {.compare = Gfx_CompareOp_LessEqual}});
      gfx_bind_vert(g.vert_reg);
      g.gpu_drawcalls[drawcall_count].type = ShaderType::Cube;
      vk_push_constants({drawcall_count++});
      gfx_draw_mesh(pool_get(g.meshes, get_mesh(Mesh_Cube)));

      // UI drawing
      gfx_apply_viewport(rng2_make(v2(), os_window_size()));
      if (g.draw_rects.count) {
        LoopArr (i, g.draw_rects) {
          var rect = g.draw_rects[i];
          g.gpu_ui_rects[i] = {
            .dst_p0 = rect.dst_p0,
            .dst_p1 = rect.dst_p1,
            .src_p0 = rect.src_p0,
            .src_p1 = rect.src_p1,
            .tex_size = rect.tex_size,
            .corner_radius = rect.corner_radius,
            .edge_softness = rect.edge_softness,
            .texture = rect.texture,
            .flags = rect.flags,
          };
          ArrayCopy(g.gpu_ui_rects[i].colors, rect.colors);
        }
        r_bind_pipeline(g.ui_rect_pip);
        r_apply_state({
          .primitive_type = Gfx_PrimitiveType_TriangleStrip,
          .blend = {
            .enabled = true,
            .src_factor_rgb = Gfx_BlendFactor_SrcAlpha,
            .dst_factor_rgb = Gfx_BlendFactor_OneMinusSrcAlpha,
            .src_factor_alpha = Gfx_BlendFactor_SrcAlpha,
            .dst_factor_alpha = Gfx_BlendFactor_OneMinusSrcAlpha,
          }
        });
        gfx_draw(0, 4, g.draw_rects.count);
        array_clear(g.draw_rects);
      }
    }
    gfx_end_pass();
  }
  
  ///////////////////////////////////
  // Swapchain
  gfx_begin_pass({});
  {
    gfx_bind_pipeline(g.uber_pip_screen);
    r_apply_state({});
    g.gpu_drawcalls[drawcall_count] = {
      .type = ShaderType::Screen,
      .cur_resolve_idx = g.world_rt.resolve.views[st->gfx.current_image_idx].idx,
    };
    vk_push_constants({drawcall_count++});
    gfx_draw(0, 3);
    // gfx_bind_pipeline(g.software_render_pip);
    // gfx_draw(0, 3);
    imgui_end_frame();
  }
  gfx_end_pass();
  gfx_end();
}

void r_draw_mesh(R_MeshId mesh, R_MaterialId mat, v3 pos) {
  var& g = st->r;
  var material = pool_get(g.materials, mat);
  u32 batch_idx = material.batch;
  R_DrawCall cmd = {
    .pos = pos,
    .scale = v3(1),
    .rot = quat_identity(),
    .mesh = mesh,
    .mat = mat,
    .type = material.type,
  };
  if (pool_get(g.meshes, mesh).index_count) {
    array_push(g.batches[batch_idx].draws, cmd);
  } else {
    array_push(g.batches[batch_idx].unindexed_draws, cmd);
  }
}

void r_draw_mesh_trs(R_MeshId mesh, R_MaterialId mat, v3 pos, v4 rot, v3 scale) {
  var& g = st->r;
  var material = pool_get(g.materials, mat);
  u32 batch_idx = material.batch;
  R_DrawCall cmd = {
    .pos = pos,
    .scale = scale,
    .rot = rot,
    .mesh = mesh,
    .mat = mat,
    .type = material.type,
  };
  if (pool_get(g.meshes, mesh).index_count) {
    array_push(g.batches[batch_idx].draws, cmd);
  } else {
    array_push(g.batches[batch_idx].unindexed_draws, cmd);
  }
}

void r_draw_entity(ThingId id) {
  var& g = st->r;
  Thing& e = get_thing(id);
  var material = pool_get(g.materials, e.mat);
  u32 batch_idx = material.batch;
  R_DrawCall cmd = {
    .pos = e.pos,
    .scale = e.scale,
    .rot = e.rot,
    .color = e.color,
    .mesh = e.mesh,
    .mat = e.mat,
    .type = material.type,
  };
  if (pool_get(g.meshes, e.mesh).index_count) {
    array_push(g.batches[batch_idx].draws, cmd);
  } else {
    array_push(g.batches[batch_idx].unindexed_draws, cmd);
  }
}

void r_draw_line(v3 a, v3 b, v4 color) {
  R_Vertex vert[] = {
    {.pos = a, .color = color},
    {.pos = b, .color = color},
  };
  array_push_elems(st->r.draw_lines, slice(vert));
}

void r_draw_line_persistent(v3 a, v3 b, v4 color) {
  R_Vertex vert[] = {
    {.pos = a, .color = color},
    {.pos = b, .color = color},
  };
  array_push_elems(st->r.draw_lines_persistent, slice(vert));
}

void r_draw_grid(v3 center, u32 slices, f32 spacing, v4 color) {
  var& lines = st->r.draw_lines;

  f32 half = slices * spacing * 0.5f;

  for (u32 i = 0; i <= slices; ++i) {
    f32 t = i * spacing - half;

    // Horizontal line
    array_push(lines, {.pos = center + v3(-half, 0, t), .color = color});
    array_push(lines, {.pos = center + v3(+half, 0, t), .color = color});

    // Vertical line
    array_push(lines, {.pos = center + v3(t, 0, -half), .color = color});
    array_push(lines, {.pos = center + v3(t, 0, +half), .color = color});
  }
}

void r_draw_cuboid(Rng3 rng, v4 color) {
  v3 p000 = {rng.min.x, rng.min.y, rng.min.z};
  v3 p001 = {rng.min.x, rng.min.y, rng.max.z};
  v3 p010 = {rng.min.x, rng.max.y, rng.min.z};
  v3 p011 = {rng.min.x, rng.max.y, rng.max.z};

  v3 p100 = {rng.max.x, rng.min.y, rng.min.z};
  v3 p101 = {rng.max.x, rng.min.y, rng.max.z};
  v3 p110 = {rng.max.x, rng.max.y, rng.min.z};
  v3 p111 = {rng.max.x, rng.max.y, rng.max.z};

  r_draw_line(p000, p001, color);
  r_draw_line(p000, p010, color);
  r_draw_line(p000, p100, color);

  r_draw_line(p111, p110, color);
  r_draw_line(p111, p101, color);
  r_draw_line(p111, p011, color);

  r_draw_line(p001, p011, color);
  r_draw_line(p001, p101, color);

  r_draw_line(p010, p011, color);
  r_draw_line(p010, p110, color);

  r_draw_line(p100, p101, color);
  r_draw_line(p100, p110, color);
}

void r_draw_rect(Rng2 rect, v4 color) {
  var& g = st->r;
  R_UI_Rect vert = {
    .dst_p0 = rect.min,
    .dst_p1 = rect.max,
  };
  for (v4& x : vert.colors) x = color;
  array_push(g.draw_rects, vert);
}

void r_draw_rect_rounded(Rng2 rect, v4 color, f32 corner_radius, f32 edge_softness) {
  var& g = st->r;
  R_UI_Rect vert = {
    .dst_p0 = rect.min,
    .dst_p1 = rect.max,
    .corner_radius = corner_radius,
    .edge_softness = edge_softness,
  };
  for (v4& x : vert.colors) x = color;
  array_push(g.draw_rects, vert);
}

void r_draw_rect_gradient(Rng2 rect, R_Gradient grad) {
  var& g = st->r;
  R_UI_Rect vert = {
    .dst_p0 = rect.min,
    .dst_p1 = rect.max,
  };
  vert.colors[0] = grad.color0;
  vert.colors[1] = grad.color1;
  vert.colors[2] = grad.color2;
  vert.colors[3] = grad.color3;
  array_push(g.draw_rects, vert);
}

void r_draw_texture(Rng2 rect, R_TextureId tex) {
  var& g = st->r;
  var t = pool_get(g.textures, tex);
  var desc = gfx_query_image_desc(t.image);
  v2 tex_size = v2(desc.width, desc.height);
  R_UI_Rect vert = {
    .dst_p0 = rect.min,
    .dst_p1 = rect.max,
    .src_p0 = v2(),
    .src_p1 = tex_size,
    .tex_size = tex_size,
    .texture = r_texture_descriptor_idx(tex),
  };
  for (v4& x : vert.colors) x = ColorWhite;
  array_push(g.draw_rects, vert);
}

void r_draw_rect_outline(Rng2 rect, u32 thickness, v4 color) {
  // Top
  r_draw_rect(rng2_make(rect.min, v2(rng2_dim(rect).x, thickness)), color);

  // Bottom
  r_draw_rect(rng2_make(v2(rect.min.x, rect.max.y - thickness), v2(rng2_dim(rect).x, thickness)), color);

  // Left
  r_draw_rect(rng2_make(rect.min, v2(thickness, rng2_dim(rect).y)), color);

  // Right
  r_draw_rect(rng2_make(v2(rect.max.x - thickness, rect.min.y), v2(thickness, rng2_dim(rect).y)), color);
}

void r_draw_text_ext(R_FontId font, v2 pos, String str, v4 color, u32 font_height) {
  var& g = st->r;
  var fo = pool_get(g.fonts, font);
  var desc = gfx_query_image_desc(pool_get(g.textures, fo.texture).image);
  v2 tex_size = v2(desc.width, desc.height);
  v2 cursor = pos;
  f32 scale = (f32)font_height / fo.font_height;
  Loop (i, str.size) {
    u8 c = str.str[i];
    var glyph = fo.glyphs[c - 32];

    // screen pos
    f32 y0 = cursor.y + glyph.yoff * scale;
    f32 x0 = cursor.x + glyph.xoff * scale;
    f32 x1 = x0 + rng2u_width(glyph.rect) * scale;
    f32 y1 = y0 + rng2u_height(glyph.rect) * scale;

    // advance cursor
    cursor.x += glyph.xadvance * scale;

    R_UI_Rect rect = {
      .dst_p0 = v2(x0,y0),
      .dst_p1 = v2(x1,y1),
      .src_p0 = glyph.rect.min,
      .src_p1 = glyph.rect.max,
      .tex_size = tex_size,
      .texture = r_texture_descriptor_idx(fo.texture),
      .flags = GpuUI_RectFlag_IsFont,
    };
    for (v4& col : rect.colors) col = color;
    array_push(g.draw_rects, rect);
  }
}

////////////////////////////////////////////////////////////////////////
// @Dear imgui

#if DEAR_IMGUI
#include "imgui/imgui_impl_vulkan.h"

ImGuiKey imgui_keycode_translate(Key key) {
  switch (key) {
    case Key_Tab:         return ImGuiKey_Tab;
    case Key_Left:        return ImGuiKey_LeftArrow;
    case Key_Right:       return ImGuiKey_RightArrow;
    case Key_Up:          return ImGuiKey_UpArrow;
    case Key_Down:        return ImGuiKey_DownArrow;

    case Key_Pageup:      return ImGuiKey_PageUp;
    case Key_Pagedown:    return ImGuiKey_PageDown;
    case Key_Home:        return ImGuiKey_Home;
    case Key_End:         return ImGuiKey_End;
    case Key_Delete:      return ImGuiKey_Delete;

    case Key_Backspace:   return ImGuiKey_Backspace;
    case Key_Space:       return ImGuiKey_Space;
    case Key_Enter:       return ImGuiKey_Enter;
    case Key_Escape:      return ImGuiKey_Escape;

    case Key_Capslock:    return ImGuiKey_CapsLock;
    case Key_Numlock:     return ImGuiKey_NumLock;
    case Key_Printscreen: return ImGuiKey_PrintScreen;
    case Key_Pause:       return ImGuiKey_Pause;

    case Key_LShift:      return ImGuiKey_LeftShift;
    case Key_RShift:      return ImGuiKey_RightShift;
    case Key_LControl:    return ImGuiKey_LeftCtrl;
    case Key_RControl:    return ImGuiKey_RightCtrl;
    case Key_LAlt:        return ImGuiKey_LeftAlt;
    case Key_RAlt:        return ImGuiKey_RightAlt;
    case Key_Lsuper:      return ImGuiKey_LeftSuper;
    case Key_Rsuper:      return ImGuiKey_RightSuper;

    // Digits
    case Key_0: return ImGuiKey_0;
    case Key_1: return ImGuiKey_1;
    case Key_2: return ImGuiKey_2;
    case Key_3: return ImGuiKey_3;
    case Key_4: return ImGuiKey_4;
    case Key_5: return ImGuiKey_5;
    case Key_6: return ImGuiKey_6;
    case Key_7: return ImGuiKey_7;
    case Key_8: return ImGuiKey_8;
    case Key_9: return ImGuiKey_9;

    // Letters
    case Key_A: return ImGuiKey_A;
    case Key_B: return ImGuiKey_B;
    case Key_C: return ImGuiKey_C;
    case Key_D: return ImGuiKey_D;
    case Key_E: return ImGuiKey_E;
    case Key_F: return ImGuiKey_F;
    case Key_G: return ImGuiKey_G;
    case Key_H: return ImGuiKey_H;
    case Key_I: return ImGuiKey_I;
    case Key_J: return ImGuiKey_J;
    case Key_K: return ImGuiKey_K;
    case Key_L: return ImGuiKey_L;
    case Key_M: return ImGuiKey_M;
    case Key_N: return ImGuiKey_N;
    case Key_O: return ImGuiKey_O;
    case Key_P: return ImGuiKey_P;
    case Key_Q: return ImGuiKey_Q;
    case Key_R: return ImGuiKey_R;
    case Key_S: return ImGuiKey_S;
    case Key_T: return ImGuiKey_T;
    case Key_U: return ImGuiKey_U;
    case Key_V: return ImGuiKey_V;
    case Key_W: return ImGuiKey_W;
    case Key_X: return ImGuiKey_X;
    case Key_Y: return ImGuiKey_Y;
    case Key_Z: return ImGuiKey_Z;

    // Function keys
    case Key_F1:  return ImGuiKey_F1;
    case Key_F2:  return ImGuiKey_F2;
    case Key_F3:  return ImGuiKey_F3;
    case Key_F4:  return ImGuiKey_F4;
    case Key_F5:  return ImGuiKey_F5;
    case Key_F6:  return ImGuiKey_F6;
    case Key_F7:  return ImGuiKey_F7;
    case Key_F8:  return ImGuiKey_F8;
    case Key_F9:  return ImGuiKey_F9;
    case Key_F10: return ImGuiKey_F10;
    case Key_F11: return ImGuiKey_F11;
    case Key_F12: return ImGuiKey_F12;

    // Symbols
    case Key_Semicolon:   return ImGuiKey_Semicolon;
    case Key_Equal:       return ImGuiKey_Equal;
    case Key_Comma:       return ImGuiKey_Comma;
    case Key_Minus:       return ImGuiKey_Minus;
    case Key_Dot:         return ImGuiKey_Period;
    case Key_Slash:       return ImGuiKey_Slash;
    case Key_Grave:       return ImGuiKey_GraveAccent;
    case Key_LBracket:    return ImGuiKey_LeftBracket;
    case Key_Backslash:   return ImGuiKey_Backslash;
    case Key_RBracket:    return ImGuiKey_RightBracket;
    case Key_Apostrophe:  return ImGuiKey_Apostrophe;

    default: return ImGuiKey_None;
  }
}

u32 imgui_mouse_button_translate(MouseButton button) {
  switch (button) {
    InvalidDefaultCase;
    case MouseButton_Left:   return ImGuiMouseButton_Left;
    case MouseButton_Right:  return ImGuiMouseButton_Right;
    case MouseButton_Middle: return ImGuiMouseButton_Middle;
  }
}

OS_InputEvent last_key_event;
Timer _timer_type_repeat_delay = {.interval = 1.0f/6};

void imgui_impl_new_frame() {
  ImGuiIO& io = ImGui::GetIO();
  io.DeltaTime = time_dt;
  v2u win_size = os_window_size();
  io.DisplaySize = ImVec2(win_size.x, win_size.y);
  Slice<OS_InputEvent> events = os_get_input_events();
  if (last_key_event.is_pressed) {
    _timer_type_repeat_delay.acc += time_dt;
    if (_timer_type_repeat_delay.acc >= _timer_type_repeat_delay.interval) {
      if (time_on_interval(1.0/20)) {
        io.AddInputCharacter(os_key_to_character(last_key_event.key, last_key_event.modifier));
      }
    }
  }
  Loop (i, events.count) {
    OS_InputEvent event = events[i];
    switch (event.type) {
      case OS_EventType_Key: {
        if (event.key < Key_COUNT && event.key != Key_Super) {
          last_key_event = event;
          _timer_type_repeat_delay.acc = 0;
        }
        ImGuiKey key = imgui_keycode_translate(event.key);
        io.AddKeyEvent(key, event.is_pressed);
        // Info("%u %u", event.key, event.is_pressed);
        if (event.is_pressed) {
          io.AddInputCharacter(os_key_to_character(event.key, event.modifier));
        }
      } break;
      case OS_EventType_MouseButton: {
        io.AddMouseButtonEvent(imgui_mouse_button_translate(event.mouse_button), event.is_pressed);
      } break;
      case OS_EventType_MouseMove: {
        io.AddMousePosEvent(event.x, event.y);
      } break;
      case OS_EventType_Scroll: {
        io.AddMouseWheelEvent(0, event.scroll);
      } break;
      case OS_EventType_Modifier:
        if (FlagHas(event.modifier, OS_Modifier_Shift)) {
          io.AddKeyEvent(ImGuiMod_Shift, true);
          // Info("mod shift true");
        } else {
          io.AddKeyEvent(ImGuiMod_Shift, false);
          // Info("mod shift false");
        }
        if (FlagHas(event.modifier, OS_Modifier_Alt)) {
          io.AddKeyEvent(ImGuiMod_Alt, true);
        } else {
          io.AddKeyEvent(ImGuiMod_Alt, false);
        }
        if (FlagHas(event.modifier, OS_Modifier_Ctrl)) {
          io.AddKeyEvent(ImGuiMod_Ctrl, true);
        } else {
          io.AddKeyEvent(ImGuiMod_Ctrl, false);
        } break;
      }
  }
}

void imgui_init() {
  ProfFunc;
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  ImGuiStyle& style = ImGui::GetStyle();
  style.FontScaleDpi = 1.3;

  ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
  platform_io.Platform_GetClipboardTextFn = imgui_platform_get_clipboard_text;
  platform_io.Platform_SetClipboardTextFn = imgui_platform_set_clipboard_text;

  VkDescriptorPoolSize pool_sizes[] = {
    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
  };
  VkDescriptorPoolCreateInfo pool_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .poolSizeCount = ArrayCount(pool_sizes),
    .pPoolSizes = pool_sizes,
    .maxSets = 1000,
    .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
  };
  VkDescriptorPool descriptor_pool;
  VK_CHECK(st->gfx.CreateDescriptorPool(vkdevice, &pool_info, null, &descriptor_pool));
  VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
  ImGui_ImplVulkan_InitInfo init_info = {
    .Instance = st->gfx.instance,
    .PhysicalDevice = st->gfx.device.physical_device,
    .Device = st->gfx.device.logical_device,
    .QueueFamily = st->gfx.device.graphics_queue_family_idx,
    .Queue = st->gfx.device.graphics_queue,
    .DescriptorPool = descriptor_pool,
    .MinImageCount = st->gfx.frames_in_flight,
    .ImageCount = st->gfx.images_in_flight,
    .PipelineInfoMain = {
      .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
      .PipelineRenderingCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &format,
      },
    },
    .UseDynamicRendering = true,
  };
  ImGui_ImplVulkan_LoadFunctions(VK_API_VERSION_1_4, [](const char* function_name, void* user_data) {
    return (PFN_vkVoidFunction)os_lib_get_proc(st->gfx.lib, function_name);
  }, null);
  ImGui_ImplVulkan_Init(&init_info);
}

void imgui_begin_frame() {
  imgui_impl_new_frame();
  ImGui_ImplVulkan_NewFrame();
  ImGui::NewFrame();
}

void imgui_end_frame() {
  ProfFunc;
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), st->gfx.render_cmds[st->gfx.current_frame_idx]);
  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();
}

#endif

