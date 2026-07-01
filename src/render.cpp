#include "stb_image.h"
#include "stb_truetype.h"

u64 hash(R_KeyToShaderPipeline x) { return hash(x.name) + hash_memory(&x.pipeline_desc, sizeof(Gfx_PipelineDesc)); }
b32 equal(R_KeyToShaderPipeline a, R_KeyToShaderPipeline b) { return equal(a.name, b.name) && MemMatchStruct(&a.pipeline_desc, &b.pipeline_desc); }

v4& get_pos() { return st->r.gpu_global->ambient_color; }
mat4& get_mat() { return st->r.gpu_global->mat; }
v4& get_matrix() { return st->r.gpu_global->ambient_color; }

b32 r_texture_is_null(R_Texture tex) { return tex.idx == 0; }
b32 r_mesh_is_null(R_Mesh mesh)      { return mesh.idx == 0; }
b32 r_material_is_null(R_Mesh m)     { return m.idx == 0; }
b32 r_font_is_null(R_Mesh f)         { return f.idx == 0; }
b32 r_shader_is_null(R_Mesh shd)     { return shd.idx == 0; }

R_DrawBatch r_draw_batch_make(Allocator alloc, Gfx_Pipeline pip) {
  R_DrawBatch res = {
    .pip = pip,
    .draws = array_make(R_DrawCallData, alloc),
    .draws_unindexed = array_make(R_DrawCallData, alloc),
  };
  return res;
}

R_ShaderModuleEntry r_shader_module_entry_make(Allocator alloc) {
  R_ShaderModuleEntry res = {
    .track_pipelines = array_make(Gfx_Pipeline, alloc),
  };
  return res;
}

R_ArenaBuffer r_arena_buffer_make(u64 size, Gfx_MemType type) {
  R_ArenaBuffer res = {
    .buf = gfx_buffer_make(size, type),
    .cap = size,
  };
  return res;
}

R_ArenaBuffer r_arena_buffer_make_round(u64 size, u64 round, Gfx_MemType type) {
  R_ArenaBuffer res = {
    .buf = gfx_buffer_make_round_base(size, round, type),
    .cap = size,
  };
  return res;
}

u64 r_arena_buffer_push(R_ArenaBuffer* buf, u64 size) {
  u64 res = offset_push(buf->pos, size);
  Assert(buf->pos <= buf->cap);
  return res;
}

R_Attachment r_attachment_make(R_AttachmentDesc desc) {
  R_Attachment res = {.type = desc.type};
  Gfx_ImageUsage usage = 0;
  switch (desc.type) {
    InvalidDefaultCase;
    case R_AttachmentType_Color: usage = Gfx_ImageUsage_ColorAttachment; break;
    case R_AttachmentType_Resolve: usage = Gfx_ImageUsage_ResolveAttachment; break;
    case R_AttachmentType_Depth: usage = Gfx_ImageUsage_DepthStencilAttachment; break;
  }
  Loop (i, st->gfx.images_in_flight) {
    res.images[i] = gfx_image_make({
      .usage = usage,
      .width = desc.size.x,
      .height = desc.size.y
    });
    switch (desc.type) {
      InvalidDefaultCase;
      case R_AttachmentType_Color: res.views[i] = gfx_view_make({.color_attachment = {.image = res.images[i]}}); break;
      case R_AttachmentType_Resolve: res.views[i] = gfx_view_make({.resolve_attachment = {.image = res.images[i]}}); break;
      case R_AttachmentType_Depth: res.views[i] = gfx_view_make({.depth_stencil_attachment = {.image = res.images[i]}}); break;
    }
  }
  return res;
}
void r_attachment_destroy(R_Attachment attachment) {
  Loop (i, st->gfx.images_in_flight) {
    gfx_image_destroy(attachment.images[i]);
    gfx_view_destroy(attachment.views[i]);
  }
}
void r_attachment_recreate(R_Attachment* attachment, v2u size) {
  r_attachment_destroy(*attachment);
  *attachment = r_attachment_make({.type = attachment->type, size});
}

R_RenderTarget r_render_target_make(R_RenderTargetUsage usage, v2u size) {
  R_RenderTarget res = {.attachments = usage};
  if (FlagHas(usage, R_RenderTargetUsage_Color)) {
    res.color = r_attachment_make({.type = R_AttachmentType_Color, .size = size});
  }
  if (FlagHas(usage, R_RenderTargetUsage_Resolve)) {
    res.resolve = r_attachment_make({.type = R_AttachmentType_Resolve, .size = size});
  }
  if (FlagHas(usage, R_RenderTargetUsage_Depth)) {
    res.depth = r_attachment_make({.type = R_AttachmentType_Depth, .size = size});
  }
  return res;
}
void r_render_target_destroy(R_RenderTarget rt) {
  if (FlagHas(rt.attachments, R_RenderTargetUsage_Color)) {
    r_attachment_destroy(rt.color);
  }
  if (FlagHas(rt.attachments, R_RenderTargetUsage_Resolve)) {
    r_attachment_destroy(rt.resolve);
  }
  if (FlagHas(rt.attachments, R_RenderTargetUsage_Depth)) {
    r_attachment_destroy(rt.depth);
  }
}
void r_render_target_recreate(R_RenderTarget* rt, v2u size) {
  r_render_target_destroy(*rt);
  *rt = r_render_target_make(rt->attachments, size);
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

Texture r_image_load(String name) {
  Scratch scratch;
  u32 required_channel_count = 4;
  u32 channel_count;
  String path = push_strf(scratch, "%s/%s", st->textures_dir, name);
  Slice buf = os_file_path_read_all(scratch, path);
  Texture res = {};
  res.data = stbi_load_from_memory(buf.data, buf.count, (i32*)&res.width, (i32*)&res.height, (i32*)&channel_count, required_channel_count);
  Assert(res.data);
  return res;
}

R_Texture r_texture_load(String name) {
  R_State& g = st->r;
  Texture texture = r_image_load(name);
  R_TextureData tex = {};
  tex.image = gfx_image_make({
    .width = texture.width,
    .height = texture.height,
    .data = texture.data,
    .mipmaps = true,
  }),
  tex.view = gfx_view_make({.texture = {.image = tex.image}});
  R_Texture res = pool_push(g.textures, tex);
  return res;
}

R_Texture r_texture_load_async(String name) {
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
    Texture texture = r_image_load(data->name);
    LockScope(g.async_stage_mutex);
    R_TextureData& img_data = pool_get(g.textures, data->tex);
    img_data.image = gfx_image_make({
      .width = texture.width,
      .height = texture.height,
      .data = texture.data,
      .mipmaps = true,
    }),
    img_data.view = gfx_view_make({.texture = {.image = img_data.image}});
  }});
  return res;
}

R_Texture r_texture_make(Texture tex) {
  var& g = st->r;
  R_TextureData texture = {};
  texture.image = gfx_image_make({
    .width = tex.width,
    .height = tex.height,
    .data = tex.data,
  }),
  texture.view = gfx_view_make({.texture = {.image = texture.image}});
  R_Texture res = pool_push(g.textures, texture);
  return res;
}

R_Texture r_texture_cube_load(String dir) {
  Scratch scratch;
  String sides[] = {
    "right", "left",
    "top", "bottom",
    "front", "back",
  };
  TextureDesc texture = {};
  LoopElement (i, sides) {
    String name = push_strf(scratch, "%s/%s%s", dir, sides[i], String(".png"));
    Texture tex = r_image_load(name);
    texture.cube[i] = tex.data;
    texture.width = tex.width;
    texture.height = tex.height;
  }
  Gfx_ImageDesc desc = {
    .type = Gfx_ImageType_Cube,
    .width = texture.width,
    .height = texture.height,
  };
  ArrayCopy(desc.cube, texture.cube);
  Gfx_Image img = gfx_image_make(desc);
  gfx_view_make({.texture = {.image = img}});
  return {};
}

void r_texture_update(R_Texture t, u8* data) {
  var& g = st->r;
  R_TextureData tex = pool_get(g.textures, t);
  gfx_image_update(tex.image, data);
}

void r_texture_readback(R_Texture t, u8* dst) {
  var& g = st->r;
  R_TextureData texture = pool_get(g.textures, t);
  gfx_image_readback(texture.image, dst);
}

void r_texture_destroy(R_Texture t) {
  var& g = st->r;
  R_TextureData texture = pool_get(g.textures, t);
  gfx_image_destroy(texture.image);
  gfx_view_destroy(texture.view);
  pool_remove(g.textures, t);
}

R_Mesh r_mesh_load(String name) {
  Scratch scratch;
  String filepath = push_strf(scratch, "%s/%s", st->models_dir, name);
  String format = str_skip_last_dot(name);
  MeshDesc mesh = {};
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
    MeshDesc desc = {};
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
      u64 offset = r_arena_buffer_push(&g.vert_arena, slice_size(desc.vertices));
      u32 base_vert = (gfx_buffer_base(g.vert_arena.buf) + offset) / sizeof(Vertex);
      gfx_buffer_update(g.vert_arena.buf, offset, slice_to_bytes(desc.vertices));
      u32 base_index = 0;
      if (desc.indices.count) {
        u64 offset = r_arena_buffer_push(&g.vert_arena, slice_size(desc.vertices));
        base_index = (gfx_buffer_base(g.index_arena.buf) + offset) / sizeof(u32);
        gfx_buffer_update(g.index_arena.buf, offset, slice_to_bytes(desc.indices));
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

R_Mesh r_mesh_make(MeshDesc desc) {
  R_State& g = st->r;
  u64 offset = r_arena_buffer_push(&g.vert_arena, slice_size(desc.vertices));
  u32 base_vert = (gfx_buffer_base(g.vert_arena.buf) + offset) / sizeof(Vertex);
  gfx_buffer_update(g.vert_arena.buf, offset, slice_to_bytes(desc.vertices));
  u32 base_index = 0;
  if (desc.indices.count) {
    u64 offset = r_arena_buffer_push(&g.vert_arena, slice_size(desc.vertices));
    base_index = (gfx_buffer_base(g.index_arena.buf) + offset) / sizeof(u32);
    gfx_buffer_update(g.index_arena.buf, offset, slice_to_bytes(desc.indices));
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

void r_mesh_update(R_Mesh mesh, MeshDesc desc) {
}

void r_mesh_destroy(R_Mesh mesh) {
}

R_Material r_material_make(MaterialDesc desc) {
  R_State& g = st->r;
  R_KeyToShaderPipeline key = {desc.shader_name, desc.pipeline_desc};
  var pip_r = map_get(g.shader_to_pipeline, key);
  if (pip_r.err) {
    pip_r = r_pipeline_make(desc.shader_name, desc.pipeline_desc);
  }
  Gfx_Pipeline pip = pip_r;
  var batch_idx_r = map_get(g.pip_idx_to_entity_batch_idx, pip.idx);
  if (batch_idx_r.err) {
    u32 batch_idx = array_push(g.entity_batches, r_draw_batch_make(g.gpa, pip));
    map_set(g.pip_idx_to_entity_batch_idx, pip.idx, batch_idx);
    batch_idx_r = batch_idx;
  }
  u32 batch_idx = batch_idx_r;
  R_Texture texture_id = map_get(st->str_to_texture_id, desc.texture);
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
  Result module_idx = map_get(g.shader_to_module_idx, name);
  Assert(!module_idx.err);
  R_ShaderModuleEntry entry = g.modules[module_idx];
  String shader = os_file_path_read_all_str(scratch, push_strf(scratch, "%s/%s.spv", st->shader_compiled_dir, name));
  gfx_shader_update(entry.shd, {.shader = shader});
  Loop (i, entry.track_pipelines.count) {
    Gfx_Pipeline pip = entry.track_pipelines[i];
    gfx_pipeline_update(pip, gfx_query_pipeline_desc(pip));
  }
}

Gfx_Pipeline r_pipeline_make(String name, Gfx_PipelineDesc desc) {
  Scratch scratch;
  R_State& g = st->r;
  R_KeyToShaderPipeline key = {name, desc};
  Result module_idx_r = map_get(g.shader_to_module_idx, name);
  if (module_idx_r.err) {
    R_ShaderModuleEntry entry = r_shader_module_entry_make(g.gpa);
    String shader = os_file_path_read_all_str(scratch, push_strf(scratch, "%s/%s.spv", st->shader_compiled_dir, name));
    entry.shd = gfx_shader_make({.shader = shader});
    u32 module_idx = array_push(g.modules, entry);
    map_set(g.shader_to_module_idx, name, module_idx);
    module_idx_r = module_idx;
  }
  u32 module_idx = module_idx_r;
  desc.shader = g.modules[module_idx].shd;
  Gfx_Pipeline pip = gfx_pipeline_make(desc);
  map_set(g.shader_to_pipeline, key, pip);
  R_ShaderModuleEntry& entry = g.modules[module_idx];
  array_push(entry.track_pipelines, pip);
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
    StringList list = {};
    str_list_push(scratch, &list, "slangc");
    str_list_push(scratch, &list, f.file_path);
    str_list_push(scratch, &list, "-target");
    str_list_push(scratch, &list, "spirv");
    str_list_push(scratch, &list, "-O0");
    str_list_push(scratch, &list, "-g");
    str_list_push(scratch, &list, "-o");
    str_list_push(scratch, &list, f.compiled_file_path);
    g.shader_module_compilation_pids[i] = os_process_launch(list);
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

R_Font r_font_load(String name, f32 size) {
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
  stbtt_BakeFontBitmap(data.data, 0, 32, pixels, width, height, first_char, characters_num, characters_info);

  R_TextureData texture = {
    .width = width,
    .height = height,
    .data = pixels,
  };
  texture.image = gfx_image_make({
    .width = texture.width,
    .height = texture.height,
    .data = texture.data,
    .pixel_format = Gfx_PixelFormat_R8,
  }),
  texture.view = gfx_view_make({.texture = {.image = texture.image}});
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
  g.async_stage_mutex = os_mutex_alloc();

  gfx_init({.cpu_mem_size = MB(100), .gpu_mem_size = MB(10), .image_mem_size = MB(10)});

  ///////////////////////////////////
  // Buffers
  {
    g.vert_buffer_each_frame = gfx_buffer_make_round_base(MB(1), sizeof(Vertex), Gfx_MemType_Cpu);
    g.vert_ring_buffer = ring_make(gfx_buffer_base_ptr(g.vert_buffer_each_frame), MB(1));
    g.vert_arena = r_arena_buffer_make_round(MB(2) + sizeof(Vertex), sizeof(Vertex));
    g.index_arena = r_arena_buffer_make_round(MB(2) + sizeof(u32), sizeof(u32));
  }

  ///////////////////////////////////
  // Descriptors
  {
    gfx_bind_make({.type = Gfx_BindType_Image, .binding = Bindings::Textures, .count = Gfx_MaxImages});
    gfx_bind_make({.type = Gfx_BindType_Sampler, .binding = Bindings::Samplers, .count = Gfx_MaxSamplers});
    gfx_bind_make({.type = Gfx_BindType_Image, .binding = Bindings::CubeTextures, .count = Gfx_MaxCubeTextures});
    gfx_bind_make({.binding = Bindings::State});
    gfx_bind_make({.binding = Bindings::Entities});
    gfx_bind_make({.binding = Bindings::Materials});
    gfx_bind_make({.binding = Bindings::DrawCtx});
    g.gpu_global_buf = gfx_buffer_make(sizeof(R_GlobalStateGPU), Gfx_MemType_Cpu);
    g.gpu_entities_buf = gfx_buffer_make((MaxEntities) * sizeof(R_EntityGPU), Gfx_MemType_Cpu);
    g.gpu_materials_buf = gfx_buffer_make(R_MaxMaterials * sizeof(R_MaterialGPU), Gfx_MemType_Cpu);
    g.gpu_drawcall_ctx_buf = gfx_buffer_make(MaxEntities * sizeof(R_DrawCallDataGPU), Gfx_MemType_Cpu);

    g.gpu_entities = (R_EntityGPU*)gfx_buffer_base_ptr(g.gpu_entities_buf);
    g.gpu_global = (R_GlobalStateGPU*)gfx_buffer_base_ptr(g.gpu_global_buf);
    g.gpu_materials = (R_MaterialGPU*)gfx_buffer_base_ptr(g.gpu_materials_buf);
    g.gpu_entities_indices = g.gpu_global->entity_indices;
    g.gpu_drawcall = (R_DrawCallDataGPU*)gfx_buffer_base_ptr(g.gpu_drawcall_ctx_buf);

    gfx_instance_set_indices(g.gpu_entities_indices);

    gfx_bind_buffer(g.gpu_global_buf, Bindings::State);
    gfx_bind_buffer(g.gpu_entities_buf, Bindings::Entities);
    gfx_bind_buffer(g.gpu_materials_buf, Bindings::Materials);
    gfx_bind_buffer(g.gpu_drawcall_ctx_buf, Bindings::DrawCtx);
    gfx_flush();
  }

  v2u win_size = os_get_window_size();
  g.world_rt = r_render_target_make(R_RenderTargetUsage_Color | R_RenderTargetUsage_Resolve | R_RenderTargetUsage_Depth, win_size);
  g.com_sampler = gfx_sampler_make({});

  {
    ProfBlock("Waiting for compiling shaders");
    r_shaders_compile_join();
  }

  ///////////////////////////////////
  // Dummy
  {
    u32 width = 128;
    u32 height = 128;
    u32* pixels = push_array(scratch, u32, width*height);
    MemSet(pixels, 255, width*height*4);
    // Loop (y, height) {
    //   Loop (x, width) {
    //   }
    // }
    Texture tex = {
      .data = (u8*)pixels,
      .width = width,
      .height = height,
    };
    g.dummy_texture = r_texture_make(tex);
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
  g.cubemap_pip = r_pipeline_make("cubemap", {
    .depth = {
      .compare = Gfx_CompareOp_LessEqual,
    },
  });
  g.ui_pip = r_pipeline_make("ui", {});
  g.font_pip = r_pipeline_make("font", {});

  g.my_font = r_font_load("arial.ttf", 32);
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

  // Resize?
  if (st->gfx.swapchain_resized || g.old_scale != g.scale) {
    g.old_scale = g.scale;
    gfx_idle();
    v2u win_size = os_get_window_size();
    r_render_target_recreate(&g.world_rt, win_size);
  }

  {
    u64 base = gfx_buffer_base(g.vert_buffer_each_frame);
    g.draw_base_lines = base + ring_write_nowrap_array(g.vert_ring_buffer, g.draw_lines.data, g.draw_lines.count);
    g.draw_base_persistent_lines = base + ring_write_nowrap_array(g.vert_ring_buffer, g.draw_lines_persistent.data, g.draw_lines_persistent.count);
    g.draw_base_rects = base + ring_write_nowrap_array(g.vert_ring_buffer, g.draw_rects.data, g.draw_rects.count);
    g.draw_base_lines /= sizeof(Vertex);
    g.draw_base_persistent_lines /= sizeof(Vertex);
    g.draw_base_rects /= sizeof(Vertex);
  }

  {
    R_GlobalStateGPU& gpu_st = *g.gpu_global;
    gpu_st.projection_view = st->projection * st->view;
    gpu_st.projection = st->projection;
    gpu_st.view = st->view;
    gpu_st.ambient_color = st->ambient_color;

    u32 idx = 0;
    LoopINode (i, g.materials.first, g.materials.data) {
      var& mat = g.materials.data[i].elem;
      mat.idx = idx;
      MaterialProps props = mat.desc.props;
      g.gpu_materials[idx++] = {
        .ambient = props.ambient,
        .diffuse = props.diffuse,
        .specular = props.specular,
        .shininess = props.shininess,
        .tex = pool_get(g.textures, mat.tex).view.idx,
      };
    }
  }

  ///////////////////////////////////
  // World
  {
    gfx_pass_begin({.attachments = r_render_target_to_attachments(g.world_rt)});
    {
      gfx_apply_viewport(rng2_make(v2_zero(), v2_of_v2u(os_get_window_size())), true);
      gfx_apply_scissor(rng2_make(v2_zero(), v2_of_v2u(os_get_window_size())));

      gfx_bind_vert();
      gfx_bind_index();

      u32 drawcall_count = 0;
      Loop (i, g.entity_batches.count) {
        R_DrawBatch& batch = g.entity_batches[i];
        gfx_pipeline_bind(batch.pip);

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
        var process_push = [&](R_DrawCallData push) {
          // mat4 model = mat4_translate(push.pos) * quat_to_mat4(push.rot) * mat4_scale(push.scale);
          mat4 model = mat4_translate(push.pos) * mat4_from_quat(push.rot) * mat4_scale(push.scale);
          var mat = pool_get(g.materials, push.mat);
          g.gpu_drawcall[drawcall_count] = {
            .model = model,
            .color = push.color,
            .mat = mat.idx,
          };
        };
        var emit_batch = [&](Slice<R_DrawCallData> pushes, b32 indexed) {
          u32 base = gfx_indirect_begin();
          Loop (i, pushes.count) {
            R_DrawCallData draw = pushes[i];
            process_push(draw);
            Gfx_Mesh mesh = pool_get(g.meshes, draw.mesh);
            gfx_draw_mesh_indirect(mesh, drawcall_count++);
          }
          Gfx_IndirectDrawcall draw = gfx_indirect_end(base);
          make_draw(draw, indexed);
        };
        emit_batch(slice(batch.draws), true);
        emit_batch(slice(batch.draws_unindexed), false);

        array_clear(batch.draws);
        array_clear(batch.draws_unindexed);
      }

      // Hello world
      // gfx_pipeline_bind(g.triangle_pip);
      // gfx_draw(0, 3);

      // Debug drawing
      gfx_bind_vert(Gfx_MemType_Cpu);
      if (g.draw_lines.count > 0) {
        gfx_pipeline_bind(g.debug_line_pip);
        gfx_draw(g.draw_base_lines, g.draw_lines.count);
        array_clear(g.draw_lines);
      }
      if (g.draw_lines_persistent.count > 0) {
        gfx_pipeline_bind(g.debug_line_pip);
        gfx_draw(g.draw_base_persistent_lines, g.draw_lines_persistent.count);
      }

      // Cube map
      gfx_pipeline_bind(g.cubemap_pip);
      gfx_bind_vert();
      Gfx_Mesh mesh = pool_get(g.meshes, mesh_get(Mesh_Cube));
      gfx_draw_mesh(mesh);

      // Rect drawing
      gfx_bind_vert(Gfx_MemType_Cpu);
      if (g.draw_rects.count > 0) {
        gfx_pipeline_bind(g.ui_pip);
        // gfx_draw(g.draw_base_rects, g.draw_rects.count);
        array_clear(g.draw_rects);
      }

      // Font
      {
        gfx_apply_viewport(rng2_make(v2_zero(), v2_of_v2u(os_get_window_size())));
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
  
            v3 min = v2_to_v3(v2_remap_01_to_11(v2(x0, y0), v2_of_v2u(os_get_window_size())), 0);
            v3 max = v2_to_v3(v2_remap_01_to_11(v2(x1, y1), v2_of_v2u(os_get_window_size())), 0);
            v4 color = text.color;
            Vertex vert[] = {
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
        g.draw_base_rects_texture = (base + ring_write_nowrap_array(g.vert_ring_buffer, g.draw_rects_texture.data, g.draw_rects_texture.count)) / sizeof(Vertex);

        if (g.draw_rects_texture.count > 0) {
          gfx_pipeline_bind(g.font_pip);
          R_FontData& f = pool_get(g.fonts, g.my_font);
          R_TextureData& t = pool_get(g.textures, f.texture);
          vk_push_constants({.image_index = t.view.idx});
          gfx_draw(g.draw_base_rects_texture, g.draw_rects_texture.count);
          array_clear(g.draw_rects_texture);
        }

      }

    }
    gfx_pass_end();
  }
  
  ///////////////////////////////////
  // Swapchain
  gfx_pass_begin({});
  {
    gfx_apply_viewport(rng2_make(v2_zero(), v2_of_v2u(os_get_window_size())));
    gfx_apply_scissor(rng2_make(v2_zero(), v2_of_v2u(os_get_window_size())));
    gfx_pipeline_bind(g.screen_pip);
    VK_PushConstant push = {g.world_rt.resolve.views[st->gfx.current_image_idx].idx};
    vk_push_constants(push);
    gfx_draw(0, 3);
    imgui_end_frame();
  }
  gfx_pass_end();
  gfx_end();
}

void r_draw_mesh(R_Mesh mesh, R_Material mat, v3 pos) {
  var& g = st->r;
  R_DrawCallData cmd = {
    .pos = pos,
    .scale = v3_splat(1),
    .rot = quat_identity(),
    .mesh = mesh,
    .mat = mat,
  };
  u32 batch_idx = pool_get(g.materials, mat).entity_batch_idx;
  if (pool_get(g.meshes, mesh).index_count) {
    array_push(g.entity_batches[batch_idx].draws, cmd);
  } else {
    array_push(g.entity_batches[batch_idx].draws_unindexed, cmd);
  }
}

void r_draw_mesh_trs(R_Mesh mesh, R_Material mat, v3 pos, v4 rot, v3 scale) {
  var& g = st->r;
  R_DrawCallData cmd = {
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
    array_push(g.entity_batches[batch_idx].draws_unindexed, cmd);
  }
}

void r_draw_entity(EntityId id) {
  var& g = st->r;
  Entity& e = get_entity(id);
  R_DrawCallData cmd = {
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
    array_push(g.entity_batches[batch_idx].draws_unindexed, cmd);
  }
}

void r_draw_line(v3 a, v3 b, v4 color) {
  Vertex vert[] = {
    {.pos = a, .color = color},
    {.pos = b, .color = color},
  };
  array_push_elems(st->r.draw_lines, slice(vert));
}

void r_draw_line_persistent(v3 a, v3 b, v4 color) {
  Vertex vert[] = {
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
  v2 size = v2_of_v2u(os_get_window_size());
  rect.min = v2_remap_01_to_11(rect.min, size);
  rect.min.y = -rect.min.y;
  rect.max = v2_remap_01_to_11(rect.max, size);
  rect.max.y = -rect.max.y;
  Vertex vert[] = {
    {.pos = v2_to_v3(rect.min, 0), .color = color},
    {.pos = v2_to_v3(v2(rect.min.x, rect.max.y), 0), .color = color},
    {.pos = v2_to_v3(rect.max, 0), .color = color},
    {.pos = v2_to_v3(rect.max, 0), .color = color},
    {.pos = v2_to_v3(v2(rect.max.x, rect.min.y), 0), .color = color},
    {.pos = v2_to_v3(rect.min, 0), .color = color},
  };
  array_push_elems(st->r.draw_rects, slice(vert));
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
  v2u win_size = os_get_window_size();
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

