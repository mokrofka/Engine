#include "com.h"
#include "stb_image.h"
#include "stb_truetype.h"

u64 hash(R_ShaderDesc x) { return hash(x.name) + hash_memory(&x.pipeline_desc, sizeof(Gfx_PipelineDesc)); }
b32 equal(R_ShaderDesc a, R_ShaderDesc b) { return equal(a.name, b.name) && MemMatchStruct(&a.pipeline_desc, &b.pipeline_desc); }

b32 r_texture_is_null(R_Texture tex) { return tex.idx == 0; }
b32 r_mesh_is_null(R_Mesh mesh)      { return mesh.idx == 0; }
b32 r_material_is_null(R_Mesh m)     { return m.idx == 0; }
b32 r_font_is_null(R_Mesh f)         { return f.idx == 0; }
b32 r_shader_is_null(R_Mesh shd)     { return shd.idx == 0; }

R_DrawBatch r_make_draw_batch(Allocator alloc, Gfx_Pipeline pip) {
  R_DrawBatch res = {
    .pip = pip,
    .draws = array_make(R_DrawCall, alloc),
    .unindexed_draws = array_make(R_DrawCall, alloc),
  };
  return res;
}

R_ShaderModuleWithPipelines r_make_shader_module_with_pipelines(Allocator alloc) {
  R_ShaderModuleWithPipelines res = {
    .pipelines = array_make(Gfx_Pipeline, alloc),
  };
  return res;
}

R_ArenaBuffer r_make_arena_buffer(u64 size, Gfx_MemType type) {
  R_ArenaBuffer res = {
    .buf = gfx_make_buffer(size, type),
    .cap = size,
  };
  return res;
}

R_ArenaBuffer r_make_round_arena_buffer(u64 size, u64 round, Gfx_MemType type) {
  R_ArenaBuffer res = {
    .buf = gfx_make_buffer_round_base(size, round, type),
    .cap = size,
  };
  return res;
}

u64 r_push_arena_buffer(R_ArenaBuffer* buf, u64 size) {
  u64 res = offset_push(buf->pos, size);
  Assert(buf->pos <= buf->cap);
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
      case R_AttachmentType_Color: res.views[i] = gfx_make_view({.color_attachment = {.image = res.images[i]}}); break;
      case R_AttachmentType_Resolve: res.views[i] = gfx_make_view({.resolve_attachment = {.image = res.images[i]}}); break;
      case R_AttachmentType_Depth: res.views[i] = gfx_make_view({.depth_stencil_attachment = {.image = res.images[i]}}); break;
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

u32 r_texture_get_descriptor_idx(R_Texture id) {
  R_State& g = st->r;
  u32 res = pool_get(g.textures, id).view.idx;
  return res;
}

R_TextureDesc r_load_image(String name) {
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

R_Texture r_load_texture(String name) {
  R_State& g = st->r;
  R_TextureDesc texture = r_load_image(name);
  R_TextureData tex = {};
  tex.image = gfx_make_image({
    .width = texture.width,
    .height = texture.height,
    .data = texture.data,
    .mipmaps = true,
  }),
  tex.view = gfx_make_view({.texture = {.image = tex.image}});
  R_Texture res = pool_push(g.textures, tex);
  return res;
}

R_Texture r_load_async_texture(String name) {
  var& g = st->r;

  R_TextureData tex = pool_get(g.textures, g.dummy_texture);
  R_Texture res = pool_push(g.textures, tex);

  struct Ctx {
    String name;
    R_Texture tex;
  };
  var& ctx = thread_push_ctx(Ctx);
  ctx = {
    .name = name,
    .tex = res,
  };
  thread_push({.ctx = &ctx, .fn = [](void* ctx) {
    var& g = st->r;
    Ctx* data = (Ctx*)ctx;
    R_TextureDesc texture = r_load_image(data->name);
    LockScope(g.async_stage_mutex);
    R_TextureData& img_data = pool_get(g.textures, data->tex);
    img_data.width = texture.width;
    img_data.height = texture.height;
    img_data.image = gfx_make_image({
      .width = texture.width,
      .height = texture.height,
      .data = texture.data,
      .mipmaps = true,
    }),
    img_data.view = gfx_make_view({.texture = {.image = img_data.image}});
  }});
  return res;
}

R_Texture r_make_texture(R_TextureDesc tex) {
  var& g = st->r;
  R_TextureData texture = {};
  texture.image = gfx_make_image({
    .width = tex.width,
    .height = tex.height,
    .data = tex.data,
  }),
  texture.view = gfx_make_view({.texture = {.image = texture.image}});
  R_Texture res = pool_push(g.textures, texture);
  return res;
}

R_Texture r_make_cubemap(R_CubeMapDesc desc) {
  var& g = st->r;
  Gfx_ImageDesc img_desc = {
    .type = Gfx_ImageType_Cube,
    .width = desc.width,
    .height = desc.height,
  };
  ArrayCopy(img_desc.cube, desc.cubes);
  Gfx_Image img = gfx_make_image(img_desc);
  Gfx_View view = gfx_make_view({.texture = {.image = img}});
  R_Texture res = pool_push(g.textures, {.image = img, .view = view});
  return res;
}

R_Texture r_load_cubemap(String dir) {
  Scratch scratch;
  String sides[] = {
    "right", "left",
    "top", "bottom",
    "front", "back",
  };
  R_CubeMapDesc cube = {};
  LoopElement (i, sides) {
    String name = push_strf(scratch, "%s/%s%s", dir, sides[i], S(".png"));
    R_TextureDesc tex = r_load_image(name);
    cube.cubes[i] = tex.data;
    cube.width = tex.width;
    cube.height = tex.height;
  }
  R_Texture res = r_make_cubemap(cube);
  return res;
}

R_Texture r_load_async_cubemap(String dir) {
  Scratch scratch;
  var& g = st->r;
  R_TextureData tex = pool_get(g.textures, g.dummy_cubemap);
  R_Texture res = pool_push(g.textures, tex);
  struct Ctx {
    String dir;
    R_Texture tex;
  };
  var& ctx = thread_push_ctx(Ctx);
  ctx = {
    .dir = dir,
    .tex = res,
  };
  thread_push({&ctx, [](void* data) {
    Ctx ctx = *(Ctx*)data;
    Scratch scratch;
    var& g = st->r;
    String sides[] = {
      "right", "left",
      "top", "bottom",
      "front", "back",
    };
    R_CubeMapDesc cube = {};
    LoopElement (i, sides) {
      String name = push_strf(scratch, "%s/%s%s", ctx.dir, sides[i], S(".png"));
      R_TextureDesc tex = r_load_image(name);
      cube.cubes[i] = tex.data;
      cube.width = tex.width;
      cube.height = tex.height;
    }
    LockScope(g.async_stage_mutex);
    Gfx_ImageDesc desc = {
      .type = Gfx_ImageType_Cube,
      .width = cube.width,
      .height = cube.height,
    };
    ArrayCopy(desc.cube, cube.cubes);
    R_TextureData& img_data = pool_get(g.textures, ctx.tex);
    img_data.image = gfx_make_image(desc);
    img_data.view = gfx_make_view({.texture = {.image = img_data.image}});
  }});
  return res;
}

void r_set_cubemap(R_Texture cubemap) {
  var& g = st->r;
  g.cur_cubemap = cubemap;
}

void r_texture_update(R_Texture t, u8* data) {
  var& g = st->r;
  R_TextureData tex = pool_get(g.textures, t);
  gfx_update_image(tex.image, data);
}

void r_texture_readback(R_Texture t, u8* dst) {
  var& g = st->r;
  R_TextureData texture = pool_get(g.textures, t);
  gfx_readback_image(texture.image, dst);
}

void r_texture_destroy(R_Texture t) {
  var& g = st->r;
  R_TextureData texture = pool_get(g.textures, t);
  gfx_destroy_image(texture.image);
  gfx_destroy_view(texture.view);
  pool_remove(g.textures, t);
}

R_Mesh r_mesh_load(String name) {
  Scratch scratch;
  String filepath = push_strf(scratch, "%s/%s", st->models_dir, name);
  String format = str_skip_last_dot(name);
  R_MeshDesc mesh = {};
  if (str_match(format, "glb")) {
    mesh = load_gltf(scratch, filepath, true);
  } else if (str_match(format, "gltf")) {
    mesh = load_gltf(scratch, filepath, false);
  } else if (str_match(format, "obj")) {
    mesh = load_obj(scratch, filepath);
  } else {
    InvalidPath;
  }
  R_Mesh res = r_mesh_make(mesh);
  return res;
}

R_Mesh r_mesh_load_async(String name) {
  Scratch scratch;
  var& g = st->r;
  R_Mesh res = pool_push(g.meshes, {});
  struct Ctx {
    String name;
    R_Mesh mesh;
  };
  var& ctx = thread_push_ctx(Ctx);
  ctx = {
    .name = name,
    .mesh = res,
  };
  thread_push({.ctx = &ctx, .fn = [](void* ctx) {
    Scratch scratch;
    var& g = st->r;
    Ctx* data = (Ctx*)ctx;
    String name = data->name;
    String filepath = push_strf(scratch, "%s/%s", st->models_dir, name);
    String format = str_skip_last_dot(name);
    R_MeshDesc desc = {};
    if (str_match(format, "glb")) {
      desc = load_gltf(scratch, filepath, true);
    } else if (str_match(format, "gltf")) {
      desc = load_gltf(scratch, filepath, false);
    } else if (str_match(format, "obj")) {
      desc = load_obj(scratch, filepath);
    } else {
      InvalidPath;
    }

    {
      LockScope(g.async_stage_mutex);
      u64 offset = r_push_arena_buffer(&g.vert_arena, slice_size(desc.vertices));
      u32 base_vert = (gfx_buffer_base(g.vert_arena.buf) + offset) / sizeof(R_Vertex);
      gfx_update_buffer(g.vert_arena.buf, offset, slice_to_bytes(desc.vertices));
      u32 base_index = 0;
      if (desc.indices.count) {
        u64 offset = r_push_arena_buffer(&g.vert_arena, slice_size(desc.vertices));
        base_index = (gfx_buffer_base(g.index_arena.buf) + offset) / sizeof(u32);
        gfx_update_buffer(g.index_arena.buf, offset, slice_to_bytes(desc.indices));
      }
      Gfx_Mesh mesh = {
        .vert_count = (u32)desc.vertices.count,
        .base_vert = base_vert,
        .index_count = (u32)desc.indices.count,
        .base_index = base_index,
      };
      pool_get(g.meshes, data->mesh) = mesh;
    }
  }});
  return res;
}

R_Mesh r_mesh_make(R_MeshDesc desc) {
  R_State& g = st->r;
  u64 offset = r_push_arena_buffer(&g.vert_arena, slice_size(desc.vertices));
  u32 base_vert = (gfx_buffer_base(g.vert_arena.buf) + offset) / sizeof(R_Vertex);
  gfx_update_buffer(g.vert_arena.buf, offset, slice_to_bytes(desc.vertices));
  u32 base_index = 0;
  if (desc.indices.count) {
    u64 offset = r_push_arena_buffer(&g.vert_arena, slice_size(desc.vertices));
    base_index = (gfx_buffer_base(g.index_arena.buf) + offset) / sizeof(u32);
    gfx_update_buffer(g.index_arena.buf, offset, slice_to_bytes(desc.indices));
  }
  Gfx_Mesh mesh = {
    .vert_count = (u32)desc.vertices.count,
    .base_vert = base_vert,
    .index_count = (u32)desc.indices.count,
    .base_index = base_index,
  };
  R_Mesh res = pool_push(g.meshes, mesh);
  return res;
}

void r_mesh_update(R_Mesh mesh, R_MeshDesc desc) {
}

void r_mesh_destroy(R_Mesh mesh) {
}

R_Material r_material_make(R_MaterialDesc desc) {
  R_State& g = st->r;
  R_ShaderDesc key = {desc.shader, desc.pipeline_desc};
  var pip_res = map_get(g.shader_desc_to_pipeline, key);
  if (pip_res.err) {
    pip_res = r_pipeline_make(desc.shader, desc.pipeline_desc);
  }
  Gfx_Pipeline pip = pip_res;
  var batch_idx_r = map_get(g.pip_idx_to_entity_batch_idx, pip.idx);
  if (batch_idx_r.err) {
    u32 batch_idx = array_push(g.entity_batches, r_make_draw_batch(g.gpa, pip));
    map_set(g.pip_idx_to_entity_batch_idx, pip.idx, batch_idx);
    batch_idx_r = batch_idx;
  }
  u32 batch_idx = batch_idx_r;
  R_Texture texture_id = map_get(st->str_to_texture_id, desc.base_color);
  if (!pool_is_valid_handle(g.textures, texture_id)) {
    texture_id = g.dummy_texture;
  }
  R_MaterialData mat = {
    .desc = desc,
    .entity_batch_idx = batch_idx,
    .tex = texture_id,
  };
  R_Material res = pool_push(g.materials, mat);
  g.gpu_materials[res.idx] = {
    .ambient = desc.props.ambient,
    .diffuse = desc.props.diffuse,
    .specular = desc.props.specular,
    .shininess = desc.props.shininess, 
    .tex = r_texture_get_descriptor_idx(texture_id),
  };
  return res;
}

void r_material_destroy(R_Material mat) {
  var& g = st->r;
  pool_remove(g.materials, mat);
}

void r_shader_reload(String name) {
  Scratch scratch;
  R_State& g = st->r;
  Info("reload '%s' shader", name);
  var module_idx = map_get(g.shader_to_module_idx, name);
  Assert(!module_idx.err);
  R_ShaderModuleWithPipelines entry = g.shader_modules[module_idx];
  String shader = os_file_path_read_all_str(scratch, push_strf(scratch, "%s/%s.spv", st->shader_compiled_dir, name));
  gfx_update_shader(entry.shd, {.shader = shader});
  Loop (i, entry.pipelines.count) {
    Gfx_Pipeline pip = entry.pipelines[i];
    gfx_update_pipeline(pip, gfx_query_pipeline_desc(pip));
  }
}

Gfx_Pipeline r_pipeline_make(String name, Gfx_PipelineDesc desc) {
  Scratch scratch;
  R_State& g = st->r;
  R_ShaderDesc key = {name, desc};
  var module_idx_r = map_get(g.shader_to_module_idx, name);
  if (module_idx_r.err) {
    R_ShaderModuleWithPipelines entry = r_make_shader_module_with_pipelines(g.gpa);
    String shader = os_file_path_read_all_str(scratch, push_strf(scratch, "%s/%s.spv", st->shader_compiled_dir, name));
    entry.shd = gfx_make_shader({.shader = shader});
    u32 module_idx = array_push(g.shader_modules, entry);
    map_set(g.shader_to_module_idx, name, module_idx);
    module_idx_r = module_idx;
  }
  u32 module_idx = module_idx_r;
  desc.shader = g.shader_modules[module_idx].shd;
  Gfx_Pipeline pip = gfx_make_pipeline(desc);
  map_set(g.shader_desc_to_pipeline, key, pip);
  R_ShaderModuleWithPipelines& entry = g.shader_modules[module_idx];
  array_push(entry.pipelines, pip);
  return pip;
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
  DenseTime com_modified = os_file_path_mtime(com_path);
  DenseTime lib_modified = os_file_path_mtime(lib_path);
  FileProperties time_stamp_file_props = os_file_path_properties(saved_time_stamps);

  ///////////////////////////////////
  // Recompile?
  struct FileData {
    u64 com_modified;
    u64 lib_modified;
  };
  b32 recompile = false;
  if (time_stamp_file_props.size == 0) {
    recompile = true;
  } else {
    Slice buf = os_file_path_read_all(scratch, saved_time_stamps);
    FileData* data = (FileData*)buf.data;
    if (com_modified != data->com_modified || lib_modified != data->lib_modified) {
      recompile = true;
    }
  }
  if (recompile) {
    FileData data = {
      .com_modified = com_modified,
      .lib_modified = lib_modified,
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
    array_push(arr, S("slangc"), f.file_path, S("-target"), S("spirv"), S("-O0"), S("-g"), S("-o"), f.compiled_file_path);
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

R_Font r_font_load(String name, u32 size) {
  Scratch scratch;
  var& g = st->r;
  String path = push_strf(scratch, "%s/%s/%s", st->asset_dir, String("fonts"), name);
  Slice data = os_file_path_read_all(scratch, path);

  u32 width = 512;
  u32 height = 512;
  u8* pixels = push_buffer(scratch, width*height);
  u32 first_char = 32;
  u32 characters_num = 96;
  stbtt_bakedchar* characters_info = push_array(scratch, stbtt_bakedchar, characters_num);
  stbtt_BakeFontBitmap(data.data, 0, size, pixels, width, height, first_char, characters_num, characters_info);

  R_TextureData texture = {
    .width = width,
    .height = height,
    .data = pixels,
  };
  texture.image = gfx_make_image({
    .width = texture.width,
    .height = texture.height,
    .data = texture.data,
    .pixel_format = Gfx_PixelFormat_R8,
  }),
  texture.view = gfx_make_view({.texture = {.image = texture.image}});
  R_Texture texture_id = pool_push(g.textures, texture);

  R_FontData font = {.texture = texture_id};
  LoopElement (i, font.glyphs) {
    stbtt_bakedchar bakedchar = characters_info[i];
    R_Glyph glyph = {
      .x0 = bakedchar.x0,
      .y0 = bakedchar.y0,
      .x1 = bakedchar.x1,
      .y1 = bakedchar.y1,
      .xoff = bakedchar.xoff,
      .yoff = bakedchar.yoff,
      .xadvance = bakedchar.xadvance,
    };
    font.glyphs[i] = glyph;
  }
  R_Font res = pool_push(g.fonts, font);
  return res;
}

void r_init() {
  ProfFunc;
  Scratch scratch;
  Arena arena = arena_make("render arena");
  R_State& g = st->r;
  g.arena = arena;
  // g.gpa = alloc_make(g.arena, "vk gpa");
  g.gpa = alloc_make(g.arena);
  g.scale = 1;
  g.async_stage_mutex = os_mutex_make();

  gfx_init({.cpu_mem_size = MB(100), .gpu_mem_size = MB(10), .image_mem_size = MB(10)});

  ///////////////////////////////////
  // Buffers
  {
    g.vert_buffer_each_frame = gfx_make_buffer_round_base(MB(1), sizeof(R_Vertex), Gfx_MemType_Cpu);
    g.vert_ring_buffer = ring_make(gfx_buffer_base_ptr(g.vert_buffer_each_frame), MB(1));
    g.vert_arena = r_make_round_arena_buffer(MB(2) + sizeof(R_Vertex), sizeof(R_Vertex));
    g.index_arena = r_make_round_arena_buffer(MB(2) + sizeof(u32), sizeof(u32));
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
        .cpu_ptr = (void**)&g.gpu_state,
      },
      {
        .binding = Bindings::Entities,
        .size = sizeof(R_EntityGPU) * MaxEntities,
        .cpu_ptr = (void**)&g.gpu_entities,
      },
      {
        .binding = Bindings::Materials,
        .size = sizeof(R_MaterialGPU) * R_MaxMaterials,
        .cpu_ptr = (void**)&g.gpu_materials,
      },
      {
        .binding = Bindings::DrawCtx,
        .size = sizeof(R_DrawCallDataGPU) * MaxEntities,
        .cpu_ptr = (void**)&g.gpu_drawcall,
      },
      {
        .binding = Bindings::SoftwareRender,
        .size = ({v2u size = os_screen_size(); size.x * size.y * sizeof(u32);}),
        .cpu_ptr = (void**)&g.gpu_software_render,
      },
      {
        .binding = Bindings::UI_RectBinding,
        .size = sizeof(R_UI_Rect) * MaxEntities,
        .cpu_ptr = (void**)&g.gpu_ui_rect,
      }
    };
    gfx_make_buffers(slice(buffers));
    gfx_flush();
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
    u32 width = 1;
    u32 height = 1;
    u32* pixels = push_array(scratch, u32, width*height);
    MemSet(pixels, 255, width*height*4);
    R_TextureDesc tex = {
      .data = (u8*)pixels,
      .width = width,
      .height = height,
    };
    g.dummy_texture = r_make_texture(tex);
  }
  {
    u32 width = 1;
    u32 height = 1;
    u32* pixels = push_array(scratch, u32, width*height);
    MemSet(pixels, 255, width*height*4);
    R_CubeMapDesc cube = {
      .width = width,
      .height = height,
    };
    Loop (i, 6) {
      cube.cubes[i] = (u8*)pixels;
    }
    g.dummy_cubemap = r_make_cubemap(cube);
    g.gpu_state->cubemap = pool_get(g.textures, g.dummy_cubemap).view.idx;
  }

  ///////////////////////////////////
  // Load basic shaders
  g.triangle_pip = r_pipeline_make("triangle", {
    .depth = {
      .compare = Gfx_CompareOp_Less,
      .write_enabled = true,
    },
  });
  g.debug_line_pip = r_pipeline_make("line", {
    .primitive_type = Gfx_PrimitiveType_Line,
    .depth = {
      .compare = Gfx_CompareOp_Less,
      .write_enabled = true,
    },
    .color = {
      .blend = {
        .enabled = true,
        .src_factor_rgb = Gfx_BlendFactor_SrcAlpha,
        .dst_factor_rgb = Gfx_BlendFactor_OneMinusSrcAlpha,
        .src_factor_alpha = Gfx_BlendFactor_SrcAlpha,
        .dst_factor_alpha = Gfx_BlendFactor_OneMinusSrcAlpha,
      },
    },
  });
  g.screen_pip = r_pipeline_make("screen", {
    .sample_count = 1,
  });
  g.software_render_pip = r_pipeline_make("software_render", {
    .sample_count = 1,
  });
  g.cubemap_pip = r_pipeline_make("cubemap", {
    .depth = {
      .compare = Gfx_CompareOp_LessEqual,
    },
  });
  g.ui_pip = r_pipeline_make("ui", {});
  g.ui_rect_pip = r_pipeline_make("ui_rect", {
    .primitive_type = Gfx_PrimitiveType_TriangleStrip,
  });
  g.font_pip = r_pipeline_make("font", {});

  g.my_font = r_font_load("arial.ttf", 32);
  g.point_dir = v2(Cos(rand_f32()), Sin(rand_f32()));
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
  R_State& g = st->r;
  gfx_begin();

  g.gpu_state->res.x = os_window_size().x;
  g.gpu_state->res.y = os_window_size().y;
  // g.gpu_state->win_width = os_get_window_size().x;
  g.point.x += 1;
 if (g.point.x < 0 || g.point.x >= os_window_size().x) {
    g.point_dir.x = -g.point_dir.x;
  }
  if (g.point_dir.y < 0 || g.point_dir.y >= os_window_size().y) {
    g.point_dir.y = -g.point_dir.y;
  }

  // g.point += g.point_dir * get_dt();

  MemZeroArray(g.gpu_software_render, ({v2u size = os_window_size(); size.x*size.y;}));
  u32 size = 16;
  Loop (y, size) {
    Loop (x, size) {
      g.gpu_software_render[(u32)((g.point.y + y) * os_window_size().x + (g.point.x + x))] = u32_from_rgba(v4(1,1,1,1));
    }
  }
  g.gpu_software_render[(u32)(g.point.y * os_window_size().x + g.point.x)] = u32_from_rgba(v4(1,1,1,1));

  // Loop (i, 40000) {
  //   g.gpu_software_render[i] = u32_from_rgba(v4(1,1,0,0));
  // }

  // Resize?
  if (st->gfx.swapchain_resized || g.old_scale != g.scale) {
    g.old_scale = g.scale;
    gfx_idle();
    v2u win_size = os_window_size();
    r_recreate_render_target(&g.world_rt, win_size);
  }

  {
    // u64 base = gfx_buffer_base(g.vert_buffer_each_frame);
    // g.draw_base_lines = (base + ring_write_nowrap_array(g.vert_ring_buffer, g.draw_lines.data, g.draw_lines.count)) / sizeof(Vertex);
    // g.draw_base_persistent_lines = (base + ring_write_nowrap_array(g.vert_ring_buffer, g.draw_lines_persistent.data, g.draw_lines_persistent.count)) / sizeof(Vertex);
    // g.draw_base_rects = (base + ring_write_nowrap_array(g.vert_ring_buffer, g.draw_rects.data, g.draw_rects.count)) / sizeof(Vertex);
    // g.draw_base_quad = (base + ring_write_nowrap_array(g.vert_ring_buffer, g.draw_quads.data, g.draw_quads.count)) / sizeof(Vertex);

    var vert = [](void* data, u64 size, u64 round) {
      var& g = st->r;
      u64 base = gfx_buffer_base(g.vert_buffer_each_frame);
      // g.vert_ring_buffer.write_pos = RoundUp(g.vert_ring_buffer.write_pos, round);
      return (base + ring_write_nowrap(g.vert_ring_buffer, data, size, 4)) / round;
    };
    g.draw_base_lines = vert(g.draw_lines.data, slice_size(slice(g.draw_lines)), sizeof(R_Vertex));
    g.draw_base_persistent_lines = vert(g.draw_lines_persistent.data, slice_size(slice(g.draw_lines_persistent)), sizeof(R_Vertex));
    g.draw_base_rects = vert(g.draw_rects.data, slice_size(slice(g.draw_rects)), sizeof(R_Vertex));
    g.draw_base_rects_texture = vert(g.draw_rects_texture.data, slice_size(slice(g.draw_rects_texture)), sizeof(R_Vertex));

    // UI_Vertex v;
    // ring_write_nowrap_array(g.vert_ring_buffer, &v, 1);
    // var* arr = push_array(scratch, Vertex, 10);
    // g.draw_base_quad = (base + ring_write_nowrap_array(g.vert_ring_buffer, arr, 10)) / sizeof(Vertex);
    // g.draw_base_quad = (base + ring_write_nowrap_array(g.vert_ring_buffer, arr, 10)) / sizeof(Vertex);
  }

  {
    var& gpu_st = *g.gpu_state;
    gpu_st.projection_view = st->projection * st->view;
    gpu_st.projection = st->projection;
    gpu_st.view = st->view;
    gpu_st.ambient_color = st->ambient_color;

    u32 idx = 0;
    LoopINode (i, g.materials.first, g.materials.data) {
      var& mat = g.materials.data[i].elem;
      mat.idx = idx;
      R_MaterialProps props = mat.desc.props;
      g.gpu_materials[idx++] = {
        .ambient = props.ambient,
        .diffuse = props.diffuse,
        .specular = props.specular,
        .shininess = props.shininess,
        .tex = pool_get(g.textures, mat.tex).view.idx,
      };
    }

    gpu_st.cubemap = pool_get(g.textures, g.cur_cubemap).view.idx;
  }

  ///////////////////////////////////
  // World
  {
    gfx_begin_pass({.attachments = r_render_target_to_attachments(g.world_rt)});
    {
      gfx_apply_viewport(rng2_make(v2(), os_window_size()), true);
      gfx_apply_scissor(rng2_make(v2(), os_window_size()));

      gfx_bind_vert();
      gfx_bind_index();

      u32 drawcall_count = 0;
      Loop (i, g.entity_batches.count) {
        R_DrawBatch& batch = g.entity_batches[i];
        gfx_bind_pipeline(batch.pip);

        var make_draw = [&](Gfx_IndirectDrawcall draw, b32 indexed) {
          if (draw.count) {
            vk_push_constants({.drawcall_base = draw.base});
            if (indexed) {
              gfx_draw_indexed_indirect(draw);
            } else {
              gfx_draw_indirect(draw);
            }
          }
        };
        var process_push = [&](R_DrawCall push) {
          // mat4 model = mat4_translate(push.pos) * quat_to_mat4(push.rot) * mat4_scale(push.scale);
          mat4 model = mat4_translate(push.pos) * mat4_from_quat(push.rot) * mat4_scale(push.scale);
          var mat = pool_get(g.materials, push.mat);
          g.gpu_drawcall[drawcall_count] = {
            .model = model,
            .color = push.color,
            .mat = mat.idx,
          };
        };
        var emit_batch = [&](Slice<R_DrawCall> pushes, b32 indexed) {
          u32 base = gfx_begin_indirect();
          Loop (i, pushes.count) {
            R_DrawCall draw = pushes[i];
            process_push(draw);
            Gfx_Mesh mesh = pool_get(g.meshes, draw.mesh);
            gfx_draw_mesh_indirect(mesh, drawcall_count++);
          }
          Gfx_IndirectDrawcall draw = gfx_end_indirect(base);
          make_draw(draw, indexed);
        };
        emit_batch(slice(batch.draws), true);
        emit_batch(slice(batch.unindexed_draws), false);

        array_clear(batch.draws);
        array_clear(batch.unindexed_draws);
      }

      // Hello world
      // gfx_pipeline_bind(g.triangle_pip);
      // gfx_draw(0, 3);

      // Debug drawing
      gfx_bind_vert(Gfx_MemType_Cpu);
      if (g.draw_lines.count > 0) {
        gfx_bind_pipeline(g.debug_line_pip);
        gfx_draw(g.draw_base_lines, g.draw_lines.count);
        array_clear(g.draw_lines);
      }
      if (g.draw_lines_persistent.count > 0) {
        gfx_bind_pipeline(g.debug_line_pip);
        gfx_draw(g.draw_base_persistent_lines, g.draw_lines_persistent.count);
      }

      // Cube map
      gfx_bind_pipeline(g.cubemap_pip);
      gfx_bind_vert();
      Gfx_Mesh mesh = pool_get(g.meshes, get_mesh(Mesh_Cube));
      gfx_draw_mesh(mesh);

      gfx_apply_viewport(rng2_make(v2(), os_window_size()));
      // Rect drawing
      gfx_bind_vert(Gfx_MemType_Cpu);
      if (!array_empty(g.draw_rects)) {
        gfx_bind_pipeline(g.ui_pip);
        gfx_draw(g.draw_base_rects, g.draw_rects.count);
        array_clear(g.draw_rects);
      }

      // Quad drawing
      if (!array_empty(g.draw_quads)) {
        MemCopyArray(g.gpu_ui_rect, g.draw_quads.data, g.draw_quads.count);
        gfx_bind_pipeline(g.ui_rect_pip);
        gfx_draw(0, g.draw_quads.count*4, g.draw_quads.count);
        array_clear(g.draw_quads);
      }

      // Font
      {
        Loop (i, g.text_draws.count) {
          R_DrawText text = g.text_draws[i];
          R_FontData& font = pool_get(g.fonts, g.my_font);
          v2 pos = text.pos;
          Loop (i, text.str.size) {
            u8 c = text.str.str[i];
            R_Glyph& glyph = font.glyphs[c - 32];
  
            // screen pos
            f32 y0 = pos.y + glyph.yoff;
            f32 x0 = pos.x + glyph.xoff;
            f32 x1 = x0 + (glyph.x1 - glyph.x0);
            f32 y1 = y0 + (glyph.y1 - glyph.y0);
  
            // advance cursor
            pos.x += glyph.xadvance;
  
            // uv
            f32 u0 = glyph.x0 / (f32)512;
            f32 v0 = glyph.y0 / (f32)512;
            f32 u1 = glyph.x1 / (f32)512;
            f32 v1 = glyph.y1 / (f32)512;
  
            v3 min = v2_to_v3(v2_remap_01_to_11(v2(x0, y0), v2_of_v2u(os_window_size())), 0);
            v3 max = v2_to_v3(v2_remap_01_to_11(v2(x1, y1), v2_of_v2u(os_window_size())), 0);
            v4 color = text.color;
            R_Vertex vert[] = {
              {.pos = min,                 .uv = v2(u0,v0), .color = color},
              {.pos = v3(min.x, max.y, 0), .uv = v2(u0,v1), .color = color},
              {.pos = max,                 .uv = v2(u1,v1), .color = color},
              {.pos = max,                 .uv = v2(u1,v1), .color = color},
              {.pos = v3(max.x, min.y, 0), .uv = v2(u1,v0), .color = color},
              {.pos = min,                 .uv = v2(u0,v0), .color = color},
            };
            array_push_elems(g.draw_rects_texture, slice(vert));
          }
        }
        array_clear(g.text_draws);

        u64 base = gfx_buffer_base(g.vert_buffer_each_frame);
        g.draw_base_rects_texture = (base + ring_write_nowrap_array(g.vert_ring_buffer, g.draw_rects_texture.data, g.draw_rects_texture.count)) / sizeof(R_Vertex);

        if (g.draw_rects_texture.count > 0) {
          gfx_bind_pipeline(g.font_pip);
          R_FontData& f = pool_get(g.fonts, g.my_font);
          R_TextureData& t = pool_get(g.textures, f.texture);
          vk_push_constants({.image_index = t.view.idx});
          gfx_draw(g.draw_base_rects_texture, g.draw_rects_texture.count);
          array_clear(g.draw_rects_texture);
        }

      }

    }
    gfx_end_pass();
  }
  
  ///////////////////////////////////
  // Swapchain
  gfx_begin_pass({});
  {
    gfx_bind_pipeline(g.screen_pip);
    VK_PushConstant push = {g.world_rt.resolve.views[st->gfx.current_image_idx].idx};
    vk_push_constants(push);
    gfx_draw(0, 3);
    gfx_bind_pipeline(g.software_render_pip);
    gfx_draw(0, 3);
    imgui_end_frame();
  }
  gfx_end_pass();
  gfx_end();
}

void r_draw_mesh(R_Mesh mesh, R_Material mat, v3 pos) {
  var& g = st->r;
  R_DrawCall cmd = {
    .pos = pos,
    .scale = v3(1),
    .rot = quat_identity(),
    .mesh = mesh,
    .mat = mat,
  };
  u32 batch_idx = pool_get(g.materials, mat).entity_batch_idx;
  if (pool_get(g.meshes, mesh).index_count) {
    array_push(g.entity_batches[batch_idx].draws, cmd);
  } else {
    array_push(g.entity_batches[batch_idx].unindexed_draws, cmd);
  }
}

void r_draw_mesh_trs(R_Mesh mesh, R_Material mat, v3 pos, v4 rot, v3 scale) {
  var& g = st->r;
  R_DrawCall cmd = {
    .pos = pos,
    .scale = scale,
    .rot = rot,
    .mesh = mesh,
    .mat = mat,
  };
  u32 batch_idx = pool_get(g.materials, mat).entity_batch_idx;
  if (pool_get(g.meshes, mesh).index_count) {
    array_push(g.entity_batches[batch_idx].draws, cmd);
  } else {
    array_push(g.entity_batches[batch_idx].unindexed_draws, cmd);
  }
}

void r_draw_entity(ThingId id) {
  var& g = st->r;
  Thing& e = get_thing(id);
  R_DrawCall cmd = {
    .pos = e.pos,
    .scale = e.scale,
    .rot = e.rot,
    .color = e.color,
    .mesh = e.mesh,
    .mat = e.mat,
  };
  u32 batch_idx = pool_get(g.materials, e.mat).entity_batch_idx;
  if (pool_get(g.meshes, e.mesh).index_count) {
    array_push(g.entity_batches[batch_idx].draws, cmd);
  } else {
    array_push(g.entity_batches[batch_idx].unindexed_draws, cmd);
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
    // .texture = r_texture_get_descriptor_idx(st->textures_ids[Texture_Orange]),
    .src_p0 = v2(0),
    .src_p1 = v2(500),
    .color = color,
  };
  array_push(g.draw_quads, vert);
}

void r_draw_texture(Rng2 rect, R_Texture tex) {
  var& g = st->r;
  var t = pool_get(g.textures, tex);
  R_UI_Rect vert = {
    .dst_p0 = rect.min,
    .dst_p1 = rect.max,
    .texture = r_texture_get_descriptor_idx(tex),
    .color = ColorWhite,
    .src_p0 = v2(),
    .src_p1 = v2(t.width, t.height),
  };
  array_push(g.draw_quads, vert);
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

void r_draw_text(v2 pos, String str, v4 color) {
  var& g = st->r;
  array_push(g.text_draws, {
    .str = push_str_copy(st->frame_arena, str),
    .pos = pos,
    .color = color,
  });
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
    default: return 0;
    case MouseButton_Left:   return ImGuiMouseButton_Left;
    case MouseButton_Right:  return ImGuiMouseButton_Right;
    case MouseButton_Middle: return ImGuiMouseButton_Middle;
  }
}

OS_InputEvent last_key_event;
Timer _timer_type_repeat_speed = {.interval = 1.0f/20};
Timer _timer_type_repeat_delay = {.interval = 1.0f/6};

void imgui_impl_new_frame() {
  ImGuiIO& io = ImGui::GetIO();
  io.DeltaTime = get_dt();
  v2u win_size = os_window_size();
  io.DisplaySize = ImVec2(win_size.x, win_size.y);
  Slice<OS_InputEvent> events = os_get_input_events();
  if (last_key_event.is_pressed) {
    _timer_type_repeat_delay.acc += get_dt();
    if (_timer_type_repeat_delay.acc >= _timer_type_repeat_delay.interval) {
      if (timer_update(_timer_type_repeat_speed)) {
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
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), st->gfx.cmds_render[st->gfx.current_frame_idx]);
  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();
}

#endif

