#include "com.h"

u64 hash(R_KeyToShaderPipeline x) { return hash(x.name) + hash_memory(&x.pipeline_desc, sizeof(ShaderState)); }
b32 equal(R_KeyToShaderPipeline a, R_KeyToShaderPipeline b) { return equal(a.name, b.name) && MemMatchStruct(&a.pipeline_desc, &b.pipeline_desc); }

v4& get_pos() { return st->r.gpu_global->ambient_color; }
mat4& get_mat() { return st->r.gpu_global->mat; }
v4& get_matrix() { return st->r.gpu_global->ambient_color; }

R_MeshBatch r_mesh_batch_make(Allocator alloc) {
  R_MeshBatch res = {
    .entities = darray_make<OpaqueId>(alloc),
  };
  return res;
}

R_MeshesBatches r_shader_batch_make(Allocator alloc) {
  R_MeshesBatches res = {
    .mesh_batches = darray_make<R_MeshBatch>(alloc),
    .mesh_to_batch = map_make<u32, u32>(alloc),
  };
  return res;
}

R_EntityPipelineBatch r_render_batch_make(Allocator alloc) {
  R_EntityPipelineBatch res = {};
  for EachElement(i, res.batches) {
    res.batches[i] = r_shader_batch_make(alloc);
  }
  return res;
}

R_ShaderModuleEntry r_shader_module_entry_make(Allocator alloc) {
  R_ShaderModuleEntry res = {
    .track_pipelines = darray_make<Gfx_Pipeline>(alloc),
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
    .buf = gfx_buffer_make_round(size, round, type),
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
  if (FlagHas(rt.attachments, R_AttachmentType_Color)) {
    r_attachment_destroy(rt.color);
  }
  if (FlagHas(rt.attachments, R_AttachmentType_Resolve)) {
    r_attachment_destroy(rt.resolve);
  }
  if (FlagHas(rt.attachments, R_AttachmentType_Depth)) {
    r_attachment_destroy(rt.depth);
  }
}
void r_render_target_recreate(R_RenderTarget* rt, v2u size) {
  r_render_target_destroy(*rt);
  *rt = r_render_target_make(rt->attachments, size);
}
Gfx_Attachments r_render_target_to_attachments(R_RenderTarget rt) {
  Gfx_Attachments att = {};
  if (FlagHas(rt.attachments, R_AttachmentType_Color)) {
    att.color = rt.color.views[st->gfx.current_image_idx];
  }
  if (FlagHas(rt.attachments, R_AttachmentType_Resolve)) {
    att.resolve = rt.resolve.views[st->gfx.current_image_idx];
  }
  if (FlagHas(rt.attachments, R_AttachmentType_Depth)) {
    att.depth_stencil = rt.depth.views[st->gfx.current_image_idx];
  }
  return att;
}

////////////////////////////////////////////////////////////////////////
// @Shader

void r_shader_reload(String name) {
  Scratch scratch;
  R_State& g = st->r;
  Info("reload '%s' shader", name);
  u32 module_idx = map_get(g.shader_to_module_idx, name);
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
  String cur_dir = os_get_current_directory();
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
    OS_Handle time_stamp_file = os_file_open(saved_time_stamps, OS_AccessFlag_Write);
    FileData data = {
      .com_modified = com_modified,
      .lib_modified = lib_modified,
    };
    os_file_write(time_stamp_file, slice_struct_to_bytes(&data));
  }

  ///////////////////////////////////
  // Which files recompile?
  struct File {
    String file_path;
    String compiled_file_path;
    String shader_name;
    OS_Handle pid;
  };
  var files = darray_make<File>(scratch);
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
  var file_names = darray_make<String>(arena);
  g.shader_module_compilation_pids = push_slice(st->arena, OS_Handle, files.count);
  g.shaders_to_compile = push_slice(st->arena, String, files.count);
  Loop (i, files.count) {
    File& f = files[i];
    StringList list = {};
    str_list_push(scratch, &list, "slangc");
    str_list_push(scratch, &list, f.file_path);
    str_list_push(scratch, &list, "-target");
    str_list_push(scratch, &list, "spirv");
    str_list_push(scratch, &list, "-g");
    str_list_push(scratch, &list, "-o");
    str_list_push(scratch, &list, f.compiled_file_path);
    g.shader_module_compilation_pids[i] = os_process_launch(list);
    Debug("%s", f.file_path);
    array_push(file_names, f.shader_name);
  }
  g.shaders_to_compile = slice(file_names);
}

void r_shader_compile_join() {
  Scratch scratch;
  R_State& g = st->r;
  Loop (i, g.shaders_to_compile.count) {
    os_process_join(g.shader_module_compilation_pids[i]);
    String shader_file_path = push_strf(scratch, "%s/%s.slang", st->shader_dir, g.shaders_to_compile[i]);
    String compiled_file_path = push_strf(scratch, "%s/%s.spv", st->shader_compiled_dir, g.shaders_to_compile[i]);
    os_file_path_copy_mtime(shader_file_path, compiled_file_path);
  }
}

GpuMeshId r_mesh_load(Mesh mesh) {
  R_State& g = st->r;
  u64 offset = r_arena_buffer_push(&g.vert_arena, slice_size(mesh.vertices));
  u32 base_vert = (gfx_buffer_base(g.vert_arena.buf) + offset) / sizeof(Vertex);
  gfx_buffer_upload(g.vert_arena.buf, offset, slice_to_bytes(mesh.vertices));

  u32 base_index = 0;
  if (mesh.indices.count) {
    u64 offset = r_arena_buffer_push(&g.vert_arena, slice_size(mesh.vertices));
    base_index = (gfx_buffer_base(g.index_arena.buf) + offset) / sizeof(u32);
    gfx_buffer_upload(g.index_arena.buf, offset, slice_to_bytes(mesh.indices));
  }
  Gfx_Mesh vk_mesh = {
    .vert_count = (u32)mesh.vertices.count,
    .base_vert = base_vert,
    .index_count = (u32)mesh.indices.count,
    .base_index = base_index,
  };
  GpuMeshId res = pool_push(g.meshes, vk_mesh);
  return res;
}

u32 r_get_texture_descriptor_idx(GpuTextureId id) {
  R_State& g = st->r;
  u32 res = pool_get(g.textures, id).view.idx;
  return res;
}

GpuTextureId r_texture_load(Texture texture) {
  R_State& g = st->r;
  R_Texture tex = {};
  tex.image = gfx_image_make({
    .width = texture.width,
    .height = texture.height,
    .data = texture.data,
    .mipmaps = true,
  }),
  tex.view = gfx_view_make({.texture = {.image = tex.image}});
  GpuTextureId res = pool_push(g.textures, tex);
  return res;
}

GpuCubemapId r_cubemap_load(Texture* textures) {
  Gfx_ImageDesc desc = {
    .type = Gfx_ImageType_Cube,
    .width = textures[0].width,
    .height = textures[0].height,
  };
  Loop (i, 6) {
    desc.cube[i] = textures->data;
  }
  Gfx_Image img = gfx_image_make(desc);
  gfx_view_make({.texture = {.image = img}});
  return {};
}

GpuMaterialId r_material_load(MaterialDesc material) {
  R_State& g = st->r;
  R_KeyToShaderPipeline key = {material.shader_name, material.pipeline_desc};
  var pip_r = map_get(g.shader_to_pipeline, key);
  if (pip_r.err) {
    pip_r = r_pipeline_make(material.shader_name, material.pipeline_desc);
  }
  Gfx_Pipeline pip = pip_r;
  var batch_idx_r = map_get(g.pip_idx_to_entity_batch_idx, pip.idx);
  if (batch_idx_r.err) {
    u32 batch_idx = array_push(g.entity_pipelines, {.pip = pip, .batch_idx = array_push(g.batches, r_render_batch_make(g.gpa))});
    map_set(g.pip_idx_to_entity_batch_idx, pip.idx, batch_idx);
    batch_idx_r = batch_idx;
  }
  u32 batch_idx = batch_idx_r;
  GpuTextureId texture_id = map_get(st->str_to_texture_id, material.texture);
  u32 texture_descriptor_idx = 0;
  if (pool_is_valid_slot(g.textures, texture_id)) {
    texture_descriptor_idx = r_get_texture_descriptor_idx(texture_id);
  }
  g.gpu_materials[g.materials.count] = {
    .ambient = material.props.ambient,
    .diffuse = material.props.diffuse,
    .specular = material.props.specular,
    .shininess = material.props.shininess, 
    .texture_idx = texture_descriptor_idx,
  };
  R_GpuMaterial mat = {
    .entity_pipeline_idx = batch_idx,
    .texture_idx = texture_descriptor_idx,
  };
  GpuMaterialId result = {g.materials.count};
  array_push(g.materials, mat);
  return result;
}

void r_init() {
  ProfFunc;
  Scratch scratch;
  Arena arena = arena_make_named("render arena");
  R_State& g = st->r;
  g.arena = arena;
  g.gpa = alloc_seglist_make(g.arena, "vk gpa");
  g.scale = 1;
  g.entity_pipelines = darray_make<R_EntityPipeline>(g.gpa);
  g.modules = darray_make<R_ShaderModuleEntry>(g.gpa);
  g.batches = darray_make<R_EntityPipelineBatch>(g.gpa);

  gfx_init({.cpu_mem_size = MB(100), .gpu_mem_size = MB(10), .image_mem_size = MB(10)});

  ///////////////////////////////////
  // Buffers
  {
    g.gpu_mem = vk_mem_make(Gfx_MemType_Gpu, MB(100));
    g.vert_buffer_each_frame = gfx_buffer_make_round(MB(1), sizeof(Vertex), Gfx_MemType_Cpu);
    g.vert_ring_buffer = ring_make(gfx_buffer_base_ptr(g.vert_buffer_each_frame), MB(1));
    g.vert_arena = r_arena_buffer_make_round(MB(1) + sizeof(Vertex), sizeof(Vertex));
    g.index_arena = r_arena_buffer_make_round(MB(1) + sizeof(u32), sizeof(u32));
  }

  ///////////////////////////////////
  // Descriptors
  {
    gfx_make_bind({.type = Gfx_BindType_Image, .binding = Bindings::Textures, .count = Gfx_MaxImages});
    gfx_make_bind({.type = Gfx_BindType_Sampler, .binding = Bindings::Samplers, .count = Gfx_MaxSamplers});
    gfx_make_bind({.type = Gfx_BindType_Image, .binding = Bindings::CubeTextures, .count = Gfx_MaxCubeTextures});
    gfx_make_bind({.binding = Bindings::State});
    gfx_make_bind({.binding = Bindings::Entities});
    gfx_make_bind({.binding = Bindings::Materials});
    g.gpu_global_buf = gfx_buffer_make(sizeof(R_GlobalStateGPU), Gfx_MemType_Cpu);
    g.gpu_entities_buf = gfx_buffer_make((MaxEntities+MaxStaticEntities) * sizeof(R_EntityGPU), Gfx_MemType_Cpu);
    g.gpu_materials_buf = gfx_buffer_make(MaxMaterials * sizeof(R_EntityGPU), Gfx_MemType_Cpu);

    g.gpu_entities = (R_EntityGPU*)gfx_buffer_base_ptr(g.gpu_entities_buf);
    g.gpu_global = (R_GlobalStateGPU*)gfx_buffer_base_ptr(g.gpu_global_buf);
    g.gpu_materials = (R_MaterialGPU*)gfx_buffer_base_ptr(g.gpu_materials_buf);
    g.gpu_entities_indices = g.gpu_global->entity_indices;

    gfx_bind_buffer(g.gpu_global_buf, Bindings::State);
    gfx_bind_buffer(g.gpu_entities_buf, Bindings::Entities);
    gfx_bind_buffer(g.gpu_materials_buf, Bindings::Materials);
    gfx_flush();
  }

  v2u win_size = os_get_window_size();
  g.world_rt = r_render_target_make(R_RenderTargetUsage_Color | R_RenderTargetUsage_Resolve | R_RenderTargetUsage_Depth, win_size);
  g.com_sampler = gfx_sampler_make({});

  {
    ProfBlock("Waiting for compiling shaders");
    r_shader_compile_join();
  }

  ///////////////////////////////////
  // Load basic shaders
  g.triangle_pip = gfx_pipeline_make({
    .shader = gfx_shader_make({.shader = os_file_path_read_all_str(scratch, "../assets/shaders/compiled/triangle.spv")}),
    .depth = {
      .compare = Gfx_CompareOp_Less,
      .write_enabled = true,
    },
  });
  g.debug_line_pip = gfx_pipeline_make({
    .shader = gfx_shader_make({.shader = os_file_path_read_all_str(scratch, "../assets/shaders/compiled/line.spv")}),
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
  g.screen_pip = gfx_pipeline_make({
    .shader = gfx_shader_make({.shader = os_file_path_read_all_str(scratch, "../assets/shaders/compiled/screen.spv")}),
    .sample_count = 1,
  });
  g.cubemap_pip = gfx_pipeline_make({
    .shader = gfx_shader_make({.shader = os_file_path_read_all_str(scratch, "../assets/shaders/compiled/cubemap.spv")}),
    .depth = {
      .compare = Gfx_CompareOp_LessEqual,
    },
  });
  g.ui_pip = gfx_pipeline_make({
    .shader = gfx_shader_make({.shader = os_file_path_read_all_str(scratch, "../assets/shaders/compiled/ui.spv")}),
  });
  
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
    g.draw_base_consistent_lines = base + ring_write_nowrap_array(g.vert_ring_buffer, g.draw_lines_consistent.data, g.draw_lines_consistent.count);
    g.draw_base_rects = base + ring_write_nowrap_array(g.vert_ring_buffer, g.draw_rects.data, g.draw_rects.count);
    g.draw_base_lines /= sizeof(Vertex);
    g.draw_base_consistent_lines /= sizeof(Vertex);
    g.draw_base_rects /= sizeof(Vertex);
  }

  ///////////////////////////////////
  // World
  {
    gfx_pass_begin({.attachments = r_render_target_to_attachments(g.world_rt)});
    {
      gfx_apply_viewport(rng2_make(v2_zero(), v2_of_v2u(os_get_window_size())));
      gfx_apply_scissor(rng2_make(v2_zero(), v2_of_v2u(os_get_window_size())));
      b32 rebuild_static_buffer = false;
      if (g.static_entities_count != g.static_entities_count_old) {
        g.static_entities_count_old = g.static_entities_count;
        rebuild_static_buffer = true;
      }

      gfx_bind_vert();
      gfx_bind_index();

      R_GlobalStateGPU& shader_st = *g.gpu_global;
      shader_st.projection_view = st->projection * st->view;
      shader_st.projection = st->projection;
      shader_st.view = st->view;

      gfx_instance_set_indices(g.gpu_entities_indices);
      Loop (i, g.entity_pipelines.count) {
        R_EntityPipeline pipeline = g.entity_pipelines[i];
        gfx_pipeline_bind(pipeline.pip);
        R_EntityPipelineBatch& batch = g.batches[pipeline.batch_idx];

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

        var fill = [&](R_MeshesBatches shader_batch) -> Gfx_IndirectDrawcall {
          u32 base = gfx_indirect_begin();
          Loop (i, shader_batch.mesh_batches.count) {
            R_MeshBatch& mb = shader_batch.mesh_batches[i];
            if (mb.entities.count == 0) continue;
            u32* indices = gfx_indirect_indices();
            Loop (j, mb.entities.count) {
              EntityId e_id = Transmute(EntityId, mb.entities[j]);
              indices[j] = e_id.idx;
              g.gpu_entities[e_id.idx].model = mat4_transform(get_entity_transform(e_id));
            }
            Gfx_Mesh mesh = pool_get(g.meshes, mb.mesh_id);
            gfx_push_indirect_instanced(mesh, mb.entities.count);
          }
          Gfx_IndirectDrawcall drawcall = gfx_indirect_end(base);
          return drawcall;
        };

        var fill_static = [&](R_MeshesBatches shader_batch) -> Gfx_IndirectDrawcall {
          u32 base = gfx_indirect_begin();
          Loop (i, shader_batch.mesh_batches.count) {
            R_MeshBatch& mb = shader_batch.mesh_batches[i];
            if (mb.entities.count == 0) continue;
            u32* indices = gfx_indirect_indices();
            Loop (j, mb.entities.count) {
              StaticEntityId e_id = Transmute(StaticEntityId, mb.entities[j]);
              u32 gpu_entity_idx = e_id.idx + MaxEntities;
              indices[j] = gpu_entity_idx;
            }
            Gfx_Mesh mesh = pool_get(g.meshes, mb.mesh_id);
            gfx_push_indirect_instanced(mesh, mb.entities.count);
          }
          Gfx_IndirectDrawcall drawcall = gfx_indirect_end(base);
          return drawcall;
        };

        var update_static_entities = [&](R_MeshesBatches shader_batch) {
          Loop (i, shader_batch.mesh_batches.count) {
            R_MeshBatch& mb = shader_batch.mesh_batches[i];
            if (mb.entities.count == 0) continue;
            Loop (j, mb.entities.count) {
              StaticEntityId e_id = Transmute(StaticEntityId, mb.entities[j]);
              u32 gpu_entity_idx = e_id.idx + MaxEntities;
              g.gpu_entities[gpu_entity_idx].model = mat4_transform(get_static_entity_transform(e_id));
            }
          }
        };

        Gfx_IndirectDrawcall indexed_drawcall = fill(batch.batches[VK_BatchType_Indexed]);
        Gfx_IndirectDrawcall drawcall = fill(batch.batches[VK_BatchType_Unindexed]);
        make_draw(indexed_drawcall, true);
        make_draw(drawcall, false);

        ///////////////////////////////////
        // Static entities
        if (rebuild_static_buffer) {
          update_static_entities(batch.batches[VK_BatchType_StaticIndexed]);
          update_static_entities(batch.batches[VK_BatchType_StaticUnindexed]);
        }
        Gfx_IndirectDrawcall static_drawcall_indexed = fill_static(batch.batches[VK_BatchType_StaticIndexed]);
        Gfx_IndirectDrawcall static_drawcall = fill_static(batch.batches[VK_BatchType_StaticUnindexed]);
        make_draw(static_drawcall_indexed, true);
        make_draw(static_drawcall, false);
      }

      // Hello world
      gfx_pipeline_bind(g.triangle_pip);
      gfx_draw(0, 3);

      // Debug drawing
      gfx_bind_vert(Gfx_MemType_Cpu);
      if (g.draw_lines.count > 0) {
        gfx_pipeline_bind(g.debug_line_pip);
        gfx_draw(g.draw_base_lines, g.draw_lines.count * 2);
        array_clear(g.draw_lines);
      }
      if (g.draw_lines_consistent.count > 0) {
        gfx_pipeline_bind(g.debug_line_pip);
        gfx_draw(g.draw_base_consistent_lines, g.draw_lines_consistent.count * 2);
      }

      // Cube map
      gfx_pipeline_bind(g.cubemap_pip);
      gfx_bind_vert();
      Gfx_Mesh mesh = pool_get(g.meshes, mesh_get(Mesh_Cube));
      if (mesh.index_count) {
        gfx_draw_indexed(mesh.base_index, mesh.index_count, mesh.base_vert);
      } else {
        gfx_draw(mesh.base_vert, mesh.vert_count);
      }

      // Rect drawing
      gfx_bind_vert(Gfx_MemType_Cpu);
      if (g.draw_rects.count > 0) {
        gfx_pipeline_bind(g.ui_pip);
        gfx_draw(g.draw_base_rects, g.draw_rects.count * 6);
        array_clear(g.draw_rects);
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

////////////////////////////////////////////////////////////////////////
// Entity

void vk_register_entity(OpaqueId entity_id, GpuMeshId mesh_id, GpuMaterialId material_id, b32 is_static) {
  R_State& g = st->r;
  u32 offset = 0;
  if (is_static) {
    offset = MaxEntities;
    ++g.static_entities_count;
  }
  u32 entity_idx = entity_id.idx + offset;
  Assert(g.entities[entity_idx].is_init == false);
  DebugDo(g.entities[entity_idx].is_init = true);
  u32 material_idx = material_id.idx;
  u32 pipeline_idx = g.materials[material_idx].entity_pipeline_idx;
  u32 mesh_idx = mesh_id.idx;
  Gfx_Mesh mesh = pool_get(g.meshes, mesh_id);
  R_BatchType type = (mesh.index_count == 0) | (is_static << 1);
  R_MeshesBatches& shader_batch = g.batches[g.entity_pipelines[pipeline_idx].batch_idx].batches[type];
  Result mesh_idx_in_array_r = map_get(shader_batch.mesh_to_batch, mesh_idx);
  if (mesh_idx_in_array_r.err) {
    R_MeshBatch mesh_batch = r_mesh_batch_make(g.gpa);
    mesh_batch.mesh_id = mesh_id;
    u32 mesh_idx_in_array = array_push(shader_batch.mesh_batches, mesh_batch);
    map_set(shader_batch.mesh_to_batch, mesh_id.idx, mesh_idx_in_array);
    mesh_idx_in_array_r = mesh_idx_in_array;
  }
  u32 mesh_idx_in_array = mesh_idx_in_array_r;
  R_MeshBatch& mesh_batch = shader_batch.mesh_batches[mesh_idx_in_array];
  u32 entity_idx_in_array = array_push(mesh_batch.entities, entity_id);
  g.entities[entity_idx].entity_idx_in_mesh_batch = entity_idx_in_array;
  g.gpu_entities[entity_idx].material_idx = material_idx;
}

void vk_make_renderable(EntityId entity_id, GpuMeshId mesh_id, GpuMaterialId material_id) {
  vk_register_entity(Transmute(OpaqueId, entity_id), mesh_id, material_id, false);
}
void vk_make_renderable_static(StaticEntityId entity_id, GpuMeshId mesh_id, GpuMaterialId material_id) {
  vk_register_entity(Transmute(OpaqueId, entity_id), mesh_id, material_id, true);
}

void vk_unregister_entity(OpaqueId entity_id, b32 is_static) {
  R_State& g = st->r;
  u32 offset = 0;
  if (is_static) {
    offset = MaxEntities;
    --g.static_entities_count;
  }
  u32 entity_idx = entity_id.idx + offset;
  Assert(g.entities[entity_idx].is_init == true);
  DebugDo(g.entities[entity_idx].is_init = false);
  u32 pipeline_idx = 0;
  GpuMeshId mesh_id;
  if (is_static) {
    StaticEntity& entity = get_static_entity(Transmute(StaticEntityId, entity_id));
    pipeline_idx = g.materials[entity.material_id.idx].entity_pipeline_idx;
    mesh_id = entity.mesh_id;
  } else {
    Entity& entity = get_entity(Transmute(EntityId, entity_id));
    pipeline_idx = g.materials[id_idx(entity.material_id.idx)].entity_pipeline_idx;
    mesh_id = entity.mesh_id;
  }
  Gfx_Mesh mesh = pool_get(g.meshes, mesh_id);
  R_BatchType type = (mesh.index_count == 0) | (is_static << 1);
  R_MeshesBatches& shader_batch = g.batches[g.entity_pipelines[pipeline_idx].batch_idx].batches[type];

  Result mesh_batch_idx = map_get(shader_batch.mesh_to_batch, mesh_id.idx);
  Assert(!mesh_batch_idx.err);
  R_MeshBatch& mesh_batch = shader_batch.mesh_batches[mesh_batch_idx];

  u32 idx = g.entities[entity_idx].entity_idx_in_mesh_batch;
  u32 last_idx = mesh_batch.entities.count-1;
  OpaqueId swapped = mesh_batch.entities[last_idx];
  u32 swapped_idx = swapped.idx;

  mesh_batch.entities[idx] = swapped;
  array_pop(mesh_batch.entities);
  g.entities[swapped_idx+offset].entity_idx_in_mesh_batch = idx;
}

void vk_remove_renderable(EntityId entity_id) {
  vk_unregister_entity(Transmute(OpaqueId, entity_id), false);
}

void vk_remove_static_renderable(StaticEntityId entity_id) {
  vk_unregister_entity(Transmute(OpaqueId, entity_id), true);
}

void vk_set_entity_color(EntityId entity_id, v4 color) {
  st->r.gpu_entities[entity_id.idx].color = color;
}

void r_debug_line(v3 a, v3 b, v4 color) {
  Vertex vert[] = {
    // {.pos = a, .color = v3_of_v4(color)},
    // {.pos = b, .color = v3_of_v4(color)},
    {.pos = a, .color = color},
    {.pos = b, .color = color},
  };
  array_push(st->r.draw_lines, {vert[0], vert[1]});
}

void r_debug_line_persistent(v3 a, v3 b, v4 color) {
  Vertex vert[] = {
    {.pos = a, .color = color},
    {.pos = b, .color = color},
  };
  array_push(st->r.draw_lines_consistent, {vert[0], vert[1]});
}

void r_debug_cuboid(Rng3 rng, v4 color) {
  v3 p000 = {rng.min.x, rng.min.y, rng.min.z};
  v3 p001 = {rng.min.x, rng.min.y, rng.max.z};
  v3 p010 = {rng.min.x, rng.max.y, rng.min.z};
  v3 p011 = {rng.min.x, rng.max.y, rng.max.z};

  v3 p100 = {rng.max.x, rng.min.y, rng.min.z};
  v3 p101 = {rng.max.x, rng.min.y, rng.max.z};
  v3 p110 = {rng.max.x, rng.max.y, rng.min.z};
  v3 p111 = {rng.max.x, rng.max.y, rng.max.z};

  r_debug_line(p000, p001, color);
  r_debug_line(p000, p010, color);
  r_debug_line(p000, p100, color);

  r_debug_line(p111, p110, color);
  r_debug_line(p111, p101, color);
  r_debug_line(p111, p011, color);

  r_debug_line(p001, p011, color);
  r_debug_line(p001, p101, color);

  r_debug_line(p010, p011, color);
  r_debug_line(p010, p110, color);

  r_debug_line(p100, p101, color);
  r_debug_line(p100, p110, color);
}

void r_draw_rect(Rng2 rect, v4 color) {
  v2 size = v2_of_v2u(os_get_window_size());
  rect.min = v2_remap_01_to_11(rect.min, size);
  rect.min.y = -rect.min.y;
  rect.max = v2_remap_01_to_11(rect.max, size);
  rect.max.y = -rect.max.y;
  DebugDrawRect square = {
    .vert = {
      // {.pos = v2_to_v3(rect.min, 0), .color = v3_of_v4(color)},
      // {.pos = v2_to_v3(v2(rect.min.x, rect.max.y), 0), .color = v3_of_v4(color)},
      // {.pos = v2_to_v3(rect.max, 0), .color = v3_of_v4(color)},
      // {.pos = v2_to_v3(rect.max, 0), .color = v3_of_v4(color)},
      // {.pos = v2_to_v3(v2(rect.max.x, rect.min.y), 0), .color = v3_of_v4(color)},
      // {.pos = v2_to_v3(rect.min, 0), .color = v3_of_v4(color)},
      {.pos = v2_to_v3(rect.min, 0), .color = color},
      {.pos = v2_to_v3(v2(rect.min.x, rect.max.y), 0), .color = color},
      {.pos = v2_to_v3(rect.max, 0), .color = color},
      {.pos = v2_to_v3(rect.max, 0), .color = color},
      {.pos = v2_to_v3(v2(rect.max.x, rect.min.y), 0), .color = color},
      {.pos = v2_to_v3(rect.min, 0), .color = color},
    }
  };
  array_push(st->r.draw_rects, square);
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

