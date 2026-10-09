#include "com.h"

#include "gfx.cpp"
#include "meta.cpp"

#include "generated.h"

GlobalVar GlobalState* st;

DrawBatch make_draw_batch(Allocator alloc, Gfx_Pipeline pip) {
	DrawBatch res = {
		.draws = array_make<DrawCall>(alloc),
		.unindexed_draws = array_make<DrawCall>(alloc),
	};
	return res;
}

Attachment make_attachment(AttachmentDesc desc, v2u size) {
	Attachment res = {};
	Gfx_ImageUsage usage = 0;
	if(flag_has(desc.usage, AttachUsage_Color )) {
		usage |= Gfx_ImageUsage_ColorAttachment;
	}
	if(flag_has(desc.usage, AttachUsage_Depth)) {
		usage |= Gfx_ImageUsage_DepthStencilAttachment;
	}
	if(flag_has(desc.usage, AttachUsage_Resolve)) {
		usage |= Gfx_ImageUsage_ResolveAttachment;
	}
	res.image = gfx_make_image({
		.pixel_format  = desc.fmt,
		.samples = desc.samples,
		.usage   = usage,
		.width   = size.x,
		.height  = size.y,
	});
	res.view = gfx_make_view({
		.type = Gfx_ViewType_Attachment,
		.image = res.image,
	});
	if(flag_has(usage, AttachUsage_Sampled)) {
		res.sampled = gfx_make_view({Gfx_ViewType_Sampled, res.image});
	}
	return res;
}
void destroy_attachment(Attachment& a) {
	gfx_destroy_view(a.view);
	gfx_destroy_image(a.image);
	if(a.sampled.idx) {
		gfx_destroy_view(a.sampled);
	}
}
v2u resolve_size(RenderTargetDesc d) {
	switch(d.size_mode) {
		InvalidDefaultCase;
		case Size_Swapchain: return os_window_size();
		case Size_Absolute:  return d.size;
	}
}
RenderTarget make_render_target(RenderTargetDesc desc) {
	_DefSet(desc.samples, Gfx_DefaultSampleCount);
	RenderTarget res = {.desc = desc, .size = resolve_size(desc)};
	if(desc.color.usage) {
		res.color = make_attachment(desc.color, res.size);
	}
	if(desc.samples > 1) {
		AttachmentDesc r = desc.color;
		r.samples = 1;
		r.usage = AttachUsage_Resolve;
		res.resolve = make_attachment(r, res.size);
	}
	if(desc.depth.usage) {
		AttachmentDesc d = desc.depth;
		d.samples = desc.samples;
		res.depth = make_attachment(d, res.size);
	}
	return res;
}
void destroy_render_target(RenderTarget rt) {
	if(rt.color.image.idx) {
		destroy_attachment(rt.color);
	}
	if(rt.resolve.image.idx) {
		destroy_attachment(rt.resolve);
	}
	if(rt.depth.image.idx) {
		destroy_attachment(rt.depth);
	}
}
void resize_render_target(RenderTarget& rt) {
	v2u ns = resolve_size(rt.desc);
	if(ns.x == rt.size.x && ns.y == rt.size.y) return;
	destroy_render_target(rt);
	rt = make_render_target(rt.desc);
}
void begin_render_pass(RenderPassDesc p) {
	Gfx_Attachments att = {};
	att.color = p.target->color.view;
	att.resolve = p.target->resolve.view;
	att.depth_stencil = p.target->depth.view;
	gfx_begin_pass({
		.attachments = att,
		.action = {
			.color = p.color,
			.depth = p.depth,
		},
	});
}

TextureDesc load_texture(String name) {
	Scratch scratch;
	u32 required_channel_count = 4;
	u32 channel_count;
	String path = push_strf(scratch, "%s/%s", st->textures_dir, name);
	Slice buf = os_file_path_read_all(scratch, path);
	TextureDesc res = {};
	res.data = stbi_load_from_memory(buf.data, buf.count, (i32*)&res.width, (i32*)&res.height, (i32*)&channel_count, required_channel_count);
	Assert(res.data);
	return res;
}

TextureDesc load_cubemap(String dir) {
	Scratch scratch;
	String sides[] = {
			"right", "left",
			"top", "bottom",
			"front", "back",
	};
	TextureDesc cube = {};
	LoopArray (i, sides) {
		String name = push_strf(scratch, "%s/%s%s", dir, sides[i], S(".png"));
		TextureDesc tex = load_texture(name);
		cube.cube[i] = tex.data;
		cube.width = tex.width;
		cube.height = tex.height;
	}
	return cube;
}

MeshDesc load_mesh(Allocator arena, String name) {
	Scratch scratch(arena);
	String format = str_skip_last_dot(name);
	MeshDesc res = {};
	if(str_match(format, "glb")) {
		res = load_gltf(arena, name, true).mesh;
	} else if(str_match(format, "gltf")) {
		res = load_gltf(arena, name, false).mesh;
	} else if(str_match(format, "obj")) {
		res = load_obj(arena, name);
	} else {
		InvalidPath;
	}
	return res;
}

TextureId make_texture(TextureDesc desc) {
	ProfFunc;
	Scratch scratch;
	var& g = *st;
	_DefSet(desc.pixel_format, Gfx_PixelFormat_RGBA8);
	Texture dum = desc.is_cube ? g.textures[g.textures_ids[Texture_DummyCubeMap].idx] : g.textures[g.textures_ids[Texture_Dummy].idx];
	TextureId res = {array_push(g.textures, dum)};
	struct Ctx {
		TextureId id;
		TextureDesc desc;
	};
	var& ctx = thread_push_ctx(Ctx);
	ctx = {
		res,
		desc,
	};
	var f = [](void* c) {
		Scratch scratch;
		var& g = *st;
		var& ctx = *(Ctx*)c;
		var desc = ctx.desc;
		var res = ctx.id;
		if(desc.name.size) {
			TextureDesc loaded = desc.is_cube ? load_cubemap(desc.name) : load_texture(desc.name);
			desc.width = loaded.width;
			desc.height = loaded.height;
			ArrayCopy(desc.cube, loaded.cube);
		}
		Slice<Slice<u8>> data = push_slice(scratch, Slice<u8>, 6);
		Loop(i, 6) {
			data[i] = Slice(desc.cube[i], desc.width * desc.height * gfx_pixelformat_bytesize(desc.pixel_format));
		}
		u32 stage_off = desc.is_cube ? gfx_push_stage_buffers(data) : gfx_push_stage_buffer(data[0]);
		LockScope(g.texture_mutex);
		Gfx_Image image = gfx_make_image({
			.type = desc.is_cube ? Gfx_ImageType_Cube : Gfx_ImageType_2D,
			.width = desc.width,
			.height = desc.height,
			.pixel_format = desc.pixel_format,
			.mipmaps = true,
		});
		Gfx_View view = gfx_make_view({Gfx_ViewType_Sampled, image});
		u32 counter = gfx_push_stage_buffer_cmd({
			.type = Gfx_CmdType_Texture,
			.img = image,
			.stage_off = stage_off,
		});
		queue_push(g.finished_gpu_queue, {
			.texture_id = res,
			.texture = {
				image,
				view,
			},
			.counter = counter,
		});
	};
	if(desc.name.size) {
		thread_push({&ctx, f});
	} else {
		f(&ctx);
	}
	return res;
}

TextureId make_texture_idle(TextureDesc desc) {
	var& g = *st;
	if(desc.name.size) {
		TextureDesc loaded = desc.is_cube ? load_cubemap(desc.name) : load_texture(desc.name);
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
	Texture tex = {
		.image = gfx_make_image(img_desc),
		.view = gfx_make_view({Gfx_ViewType_Sampled, tex.image}),
	};
	TextureId res = {array_push(g.textures, tex)};
	return res;
}

MeshId make_mesh(MeshDesc desc) {
	ProfFunc;
	Scratch scratch;
	var& g = *st;
	MeshId res = {array_push(g.meshes, g.meshes[g.meshes_ids[Mesh_Dummy].idx])};
	if(desc.name.size) {
		desc = load_mesh(scratch, desc.name);
	}
	u32 vert_buf_off = gfx_push_buffer(g.gpu_vert_buf, slice_size(desc.vertices));
	u32 index_buf_off = gfx_push_buffer(g.gpu_index_buf, slice_size(desc.indices));
	u32 vert_base = vert_buf_off / sizeof(Vertex);
	u32 vert_stage_off = gfx_push_stage_buffer(slice_to_bytes(desc.vertices));
	u32 index_stage_off = 0;
	u32 index_base = 0;
	if(desc.indices.count) {
		index_base = index_buf_off / sizeof(u16);
		index_stage_off = gfx_push_stage_buffer(slice_to_bytes(desc.indices));
	}
	u32 counter = gfx_push_stage_buffer_cmd({
		.type = Gfx_CmdType_Mesh,
		.stage_off = vert_stage_off,
		.buf = g.gpu_vert_buf,
		.size = desc.vertices.count * sizeof(Vertex),
		.buf_offset = vert_base * sizeof(Vertex),
	});
	if(desc.indices.count) {
		gfx_push_stage_buffer_cmd({
			.type = Gfx_CmdType_Mesh,
			.stage_off = index_stage_off,
			.buf = g.gpu_index_buf,
			.size = desc.indices.count * sizeof(u16),
			.buf_offset = index_base * sizeof(u16),
		});
	}
	os_mutex_lock(g.finished_gpu_mutex);
	queue_push(g.finished_gpu_queue, {
		.type = PushToGpuType_Mesh,
		.mesh_id = res,
		.mesh = {
			.vert_count = (u32)desc.vertices.count,
			.base_vert = vert_base,
			.index_count = (u32)desc.indices.count,
			.base_index = index_base,
		},
		.counter = counter,
	});
	os_mutex_unlock(g.finished_gpu_mutex);
	return res;
}

MeshId make_mesh_idle(MeshDesc desc) {
	Scratch scratch;
	var& g = *st;
	if(desc.name.size) {
		desc = load_mesh(scratch, desc.name);
	}
	u64 buf_offset = gfx_push_buffer(g.gpu_vert_buf, slice_size(desc.vertices));
	u32 base_vert = buf_offset / sizeof(Vertex);
	gfx_update_buffer(g.gpu_vert_buf, buf_offset, slice_to_bytes(desc.vertices));
	u32 base_index = 0;
	if(desc.indices.count) {
		u64 buf_offset = gfx_push_buffer(g.gpu_index_buf, slice_size(desc.indices));
		base_index = buf_offset / sizeof(u32);
		gfx_update_buffer(g.gpu_index_buf, buf_offset, slice_to_bytes(desc.indices));
	}
	Gfx_Mesh mesh = {
		.base_vert = base_vert,
		.vert_count = (u32)desc.vertices.count,
		.base_index = base_index,
		.index_count = (u32)desc.indices.count,
	};
	MeshId res = {array_push(g.meshes, mesh)};
	return res;
}

u32 make_pipeline_state(Gfx_PipelineState s) {
	var& g = *st;
	u32 res = array_push(g.batches, make_draw_batch(g.gpa, {}));
	g.batches[res].state = s;
	return res;
}

MaterialId make_material(Material mat) {
	var& g = *st;
	if(!mat.base_color.idx) {
		mat.base_color = g.textures_ids[Texture_Dummy];
	}
	MaterialId res = MaterialId(array_push(g.materials, mat));
	return res;
}

Gfx_Pipeline make_pipeline(String name, Gfx_PipelineDesc2 desc) {
	ProfFunc;
	Scratch scratch;
	var& g = *st;
	ShaderModuleWithPipelines module = {};
	if(!map_get(g.shader_to_module, hash(name), &module)) {
		Slice code = os_file_path_read_all(scratch, push_strf(scratch, "%s/%s.spv", st->shader_compiled_dir, name));
		ProfBlock("make shader");
		module = {.shd = gfx_make_shader(code)};
		map_set(g.shader_to_module, hash(name), module);
	};
	desc.shader = module.shd;
	Gfx_Pipeline pip = gfx_make_pipeline(desc);
	array_push(module.pipelines, pip);
	return pip;
}

FontId make_font(FontDesc desc) {
	Scratch scratch;
	var& g = *st;
	FontId res = {array_push(g.fonts, g.fonts[g.fonts_ids[Font_Dummy].idx])};
	struct Ctx {
		FontDesc desc;
		FontId font_id;
	};
	var& ctx = thread_push_ctx(Ctx);
	ctx = {
		.desc = desc,
		.font_id = res,
	};
	thread_push(&ctx, [](void* in) {
		Scratch scratch;
		Ctx& ctx = *(Ctx*)in;
		var& g = *st;
		String path = push_strf(scratch, "%s/%s/%s", st->asset_dir, S("fonts"), ctx.desc.name);
		Slice data = os_file_path_read_all(scratch, path);
		stbtt_bakedchar characters_info[96];
		u32 font_height = ctx.desc.font_height;
		u32 pixels_size = 96 * Square(font_height);
		u32 dim = sqrt(pixels_size);
		u32 width = round_up(dim, font_height);
		u32 height = round_up(dim, font_height);
		u8* pixels = push_buffer(scratch, width*height);
		stbtt_BakeFontBitmap(data.data, 0, font_height, pixels, width, height, 32, 96, characters_info);
		TextureId texture_id = make_texture({.width = width, .height = height, .data = pixels, .pixel_format = Gfx_PixelFormat_R8});
		Font font = {.font_height = font_height, .texture = texture_id};
		LoopArray(i, font.glyphs) {
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
		LockScope(g.font_mutex);
		queue_push(g.waiting_fonts, {
			.id = ctx.font_id,
			.font = font,
		});
	});
	return res;
}

FontId make_font_idle(FontDesc desc) {
	Scratch scratch;
	var& g = *st;
	String path = push_strf(scratch, "%s/%s/%s", st->asset_dir, S("fonts"), desc.name);
	Slice data = os_file_path_read_all(scratch, path);
	stbtt_bakedchar characters_info[96];
	u32 font_height = desc.font_height;
	u32 pixels_size = 96 * Square(font_height);
	u32 dim = sqrt(pixels_size);
	u32 width = round_up(dim, font_height);
	u32 height = round_up(dim, font_height);
	u8* pixels = push_buffer(g.arena, width*height);
	stbtt_BakeFontBitmap(data.data, 0, font_height, pixels, width, height, 32, 96, characters_info);
	TextureId texture_id = make_texture_idle({.width = width, .height = height, .data = pixels, .pixel_format = Gfx_PixelFormat_R8});
	Font font = {.font_height = font_height, .texture = texture_id};
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
	FontId res = {array_push(g.fonts, font)};
	return res;
}

u32 texture_descriptor_idx(TextureId id) {
	var& g = *st;
	u32 res = g.textures[id.idx].view.idx;
	return res;
}

void set_cubemap(TextureId cubemap) {
	var& g = *st;
	g.cur_cubemap = cubemap;
}

void update_texture(TextureId t, u8* data) {
	var& g = *st;
	Texture tex = g.textures[t.idx];
	gfx_update_image(tex.image, data);
}

void update_mesh(MeshId mesh, MeshDesc desc) {
}

void reload_shader(String name) {
	Scratch scratch;
	var& g = *st;
	Info("reload '%s' shader", name);
	ShaderModuleWithPipelines module; AssertOk(map_get(g.shader_to_module, hash(name), &module));
	Slice code = os_file_path_read_all(scratch, push_strf(scratch, "%s/%s.spv", st->shader_compiled_dir, name));
	gfx_update_shader(module.shd, code);
	Loop(i, module.pipelines.count) {
		Gfx_Pipeline pip = module.pipelines[i];
		gfx_update_pipeline(pip, gfx_query_pipeline_desc2(pip));
	}
}

void shaders_compile(Allocator arena) {
	Scratch scratch(arena);
	String cur_dir = os_cur_directory();
	String shader_dir = st->shader_dir;
	String compiled_shader_dir = st->shader_compiled_dir;
	String saved_time_stamps = push_strf(scratch, "%s/%s", cur_dir, String("saved_time_stamps_for_shad"));
	if(!os_directory_path_exist(shader_dir)) {
		os_directory_make(shader_dir);
	}
	if(!os_directory_path_exist(compiled_shader_dir)) {
		os_directory_make(compiled_shader_dir);
	}
	String com_path = push_strf(scratch, "%s/%s", shader_dir, String("com.slang"));
	String lib_path = push_strf(scratch, "%s/%s", shader_dir, String("lib.slang"));
	String header_path = push_strf(scratch, "%s/../src/shader_header.h", os_cur_directory());
	u64 com_modified = os_file_path_mtime(com_path);
	u64 lib_modified = os_file_path_mtime(lib_path);
	u64 header_modified = os_file_path_mtime(header_path);
	FileProperties time_stamp_file_props = os_file_path_properties(saved_time_stamps);

	///////////////////////////////////
	// Recompile?
	struct FileData {
		u64 com_modified;
		u64 lib_modified;
		u64 header_modified;
	};
	b32 recompile = false;
	if(time_stamp_file_props.size == 0) {
		recompile = true;
	} else {
		Slice buf = os_file_path_read_all(scratch, saved_time_stamps);
		FileData* data = (FileData*)buf.data;
		if(com_modified != data->com_modified || lib_modified != data->lib_modified || header_modified != data->header_modified) {
			recompile = true;
		}
	}
	if(recompile) {
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
	var files = array_make<File>(scratch);
	Slice infos = os_file_iter_directory(scratch, shader_dir, OS_FileIterFlag_SkipFolders);
	Loop(i, infos.count) {
		String name = infos[i].name;
		String file_path = push_strf(scratch, "%s/%s", shader_dir, name);
		String shader_name = str_chop_last_dot(name);
		String compiled_file_path = push_strf(scratch, "%s/%s.spv", compiled_shader_dir, shader_name);
		if(str_match(name, "com.slang") || str_match(name, "lib.slang")) {
			continue;
		}
		FileProperties compiled_props = os_file_path_properties(compiled_file_path);
		if(infos[i].props.modified != compiled_props.modified || recompile) {
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
	var& g = *st;
	var file_names = array_make<String>(arena);
	g.shader_module_compilation_pids = push_slice(arena, OS_Handle, files.count);
	g.shaders_to_compile = push_slice(arena, String, files.count);
	Loop(i, files.count) {
		File& f = files[i];
		var arr = array_make<String>(scratch);
		String args[] = {S("slangc"), f.file_path, S("-target"), S("spirv"), S("-O0"), S("-g3"), S("-o"), f.compiled_file_path};
		// String args[] = {S("slangc"), f.file_path, S("-target"), S("spirv"), S("-o"), f.compiled_file_path};
		array_push_elems(arr, slice(args));
		g.shader_module_compilation_pids[i] = os_process_make(slice(arr));
		Debug("%s", f.file_path);
		array_push(file_names, f.shader_name);
	}
	g.shaders_to_compile = slice(file_names);
}

void shaders_compile_join() {
	Scratch scratch;
	var& g = *st;
	Loop(i, g.shaders_to_compile.count) {
		os_process_join(g.shader_module_compilation_pids[i]);
		String shader_file_path = push_strf(scratch, "%s/%s.slang", st->shader_dir, g.shaders_to_compile[i]);
		String compiled_file_path = push_strf(scratch, "%s/%s.spv", st->shader_compiled_dir, g.shaders_to_compile[i]);
		os_file_path_copy_mtime(shader_file_path, compiled_file_path);
	}
}

void r_bind_pipeline(Gfx_Pipeline pip) {
	var& g = *st;
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
	_DefSet(s->sample_count, Gfx_DefaultSampleCount);
}

void r_apply_state_raw(Gfx_PipelineState s = {}) {
	gfx_apply_primitive_type(s.primitive_type);
	gfx_apply_cull_mode(s.cull_mode);
	gfx_apply_face_winding(s.face_winding);
	gfx_apply_depth_test(s.depth.test_disable);
	gfx_apply_depth_write(s.depth.write_enabled);
	gfx_apply_depth_compare(s.depth.compare);
	gfx_apply_color_blend_enable(s.blend.enabled);
	if(s.blend.enabled) {
		gfx_apply_color_blend_equation(s.blend);
	}
	gfx_apply_color_blend_mask(s.blend);
	gfx_apply_sample(s.sample_count);
	gfx_apply_sample_mask(s.sample_count, U32_MAX);
	gfx_apply_alpha_to_coverage(false);
}

void apply_pip_state(Gfx_PipelineState s) {
	var& g = *st;
	r_default_pipeline_state(&s);
	if(g.cur_pip.idx != g.prev_pip.idx) {
		g.prev_pip = g.cur_pip;
		r_apply_state_raw(s);
	} else {
		var& prev = g.prev_pip_state;
		if(s.primitive_type != prev.primitive_type) {
			gfx_apply_primitive_type(s.primitive_type);
		}
		if(s.cull_mode != prev.cull_mode) {
			gfx_apply_cull_mode(s.cull_mode);
		}
		if(s.face_winding != prev.face_winding) {
			gfx_apply_face_winding(s.face_winding);
		}
		if(s.depth.test_disable != prev.depth.test_disable) {
			gfx_apply_depth_test(s.depth.test_disable);
		}
		if(s.depth.write_enabled != prev.depth.write_enabled) {
			gfx_apply_depth_write(s.depth.write_enabled);
		}
		if(s.depth.compare != prev.depth.compare) {
			gfx_apply_depth_compare(s.depth.compare);
		}
		if(s.depth.write_enabled != prev.depth.write_enabled) {
			gfx_apply_color_blend_enable(s.blend.enabled);
		}
		if(s.blend.enabled) {
			Gfx_BlendState b = s.blend;
			Gfx_BlendState pb = prev.blend;
			if(b.src_factor_rgb != pb.src_factor_rgb || b.dst_factor_rgb != pb.dst_factor_rgb || b.op_rgb != pb.op_rgb ||
						b.src_factor_alpha != pb.src_factor_alpha || b.dst_factor_alpha != pb.dst_factor_alpha || b.op_alpha != pb.op_alpha)
			{
				gfx_apply_color_blend_equation(s.blend);
			}
		}
		if(s.blend.write_mask != prev.blend.write_mask) {
			gfx_apply_color_blend_mask(s.blend);
		}
		if(s.sample_count != prev.sample_count) {
			gfx_apply_sample(s.sample_count);
		}
	}
	g.prev_pip_state = s;
}

void r_init() {
	ProfFunc;
	Scratch scratch;
	Arena arena = arena_make(.name = "render arena");
	var& g = *st;
	g.arena = arena;
	g.gpa = alloc_make(g.arena);
	g.scale = 1;
	gfx_init({.cpu_mem_size = MB(100), .gpu_mem_size = MB(10), .image_mem_size = MB(10)});

	///////////////////////////////////
	// Buffers
	{
		ProfBlock("buffers");
		g.cpu_vert_buf = gfx_make_buffer(MB(1), Gfx_MemType_Cpu);
		g.cpu_vertices = (Vertex*)gfx_buffer_base_ptr(g.cpu_vert_buf);
		g.gpu_vert_buf = gfx_make_buffer(MB(1), Gfx_MemType_Gpu);
		g.gpu_index_buf = gfx_make_buffer(MB(1), Gfx_MemType_Gpu);
	}

	///////////////////////////////////
	// Descriptors
	{
		ProfBlock("descriptors");
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
				.binding = Bindings::Materials,
				.size = sizeof(GpuMaterial) * MaxMaterials,
				.out_cpu_ptr = (void**)&g.gpu_materials,
			},
			{
				.binding = Bindings::DrawCtx,
				.size = sizeof(GpuDrawCall) * MaxThings,
				.out_cpu_ptr = (void**)&g.gpu_drawcalls,
			},
			{
				.binding = Bindings::SoftwareRender,
				.size = os_screen_size().x * os_screen_size().y * sizeof(u32),
				.out_cpu_ptr = (void**)&g.gpu_software_render,
			},
			{
				.binding = Bindings::UI_RectBinding,
				.size = sizeof(UI_Rect) * MaxThings,
				.out_cpu_ptr = (void**)&g.gpu_ui_rects,
			},
			{
				.binding = Bindings::SkinMatrices,
				.size = sizeof(UI_Rect) * MaxSkinMatrices,
				.out_cpu_ptr = (void**)&g.gpu_skin_matrices,
			}
		};
		gfx_make_binding_buffers(slice(buffers));
		gfx_flush();
		g.gpu_state->p = st->gfx.cpu_buf_address;
	}

	// Misc
	{
		ProfBlock("render targers");
		g.world_rt = make_render_target({
			.color = {.usage = AttachUsage_Color},
			.depth = {.usage = AttachUsage_Depth},
		});
	}
	g.sampler = gfx_make_sampler({});
	{
		ProfBlock("Waiting for compiling shaders");
		shaders_compile_join();
	}

	// Dummy
	{
		ProfBlock("making dummy");
		TextureDesc dum = load_texture("dummy.png");
		g.textures_ids[Texture_Dummy] = make_texture_idle({
			.data = dum.data,
			.width = dum.width,
			.height = dum.height,
		});
		TextureDesc cube = {
			.width = dum.width,
			.height = dum.width,
			.is_cube = true,
		};
		Loop(i, 6) {
			cube.cube[i] = (u8*)dum.data;
		}
		g.textures_ids[Texture_DummyCubeMap] = make_texture_idle(cube);
		g.gpu_state->cubemap = texture_descriptor_idx(g.textures_ids[Texture_DummyCubeMap]);
		g.meshes_ids[Mesh_Dummy] = make_mesh_idle({.name = "dummy.glb"});
		g.fonts_ids[Font_Dummy] = make_font_idle({"Roboto-Bold.ttf", 32});
	}

	// Shaders
	{
		ProfBlock("load shaders");
		g.uber_pip = make_pipeline("uber", {});
		g.ui_rect_pip = make_pipeline("ui_rect", {});
	}
	Info("Renderer initialized");
}

void r_shutdown() {
	gfx_shutdown();
}

void r_end() {
	ProfFunc;
	Scratch scratch;
	var& g = *st;
	gfx_begin();
	g.gpu_state->res.x = os_window_size().x;
	g.gpu_state->res.y = os_window_size().y;
	g.prev_pip = {};

	// Resize?
	if(st->gfx.swapchain_resized || g.old_scale != g.scale) {
		g.old_scale = g.scale;
		gfx_idle();
		v2u win_size = os_window_size();
		win_size.x *= g.scale;
		win_size.y *= g.scale;
		resize_render_target(g.world_rt);
	}

	// Update dummies?
	{
		Loop(i, queue_count(g.finished_gpu_queue)) {
			if(queue_front(g.finished_gpu_queue).counter <= gfx_ready_counter()) {
				var slot = queue_pop(g.finished_gpu_queue);
				switch(slot.type) {
					InvalidDefaultCase;
					case PushToGpuType_Texture: {
						var& t = g.textures[slot.texture_id.idx];
						t = slot.texture;
						t.is_ready = true;
					}break;
					case PushToGpuType_Mesh: {
						var& t = g.meshes[slot.mesh_id.idx];
						t = slot.mesh;
					}break;
				}
			} else break;
		} 

		// Update font?
		Loop(i, queue_count(g.waiting_fonts)) {
			if(g.textures[queue_front(g.waiting_fonts).font.texture.idx].is_ready) {
				var slot = queue_pop(g.waiting_fonts);
				g.fonts[slot.id.idx] = slot.font;
			} else break;
		}
	}

	// Update skeleton
	{
		g.global_matrices[0] = m4x4_transform(g.bones[0].local);
		g.skin_matrices[0] = g.global_matrices[0] * g.inv_matrices[0];
		// g.skin_matrices[0] = g.global_matrices[0];
		LoopOff(i, 1, g.bones.count) {
			g.global_matrices[i] = g.global_matrices[g.bones[i].parent] * m4x4_transform(g.bones[i].local);
			g.skin_matrices[i] = g.global_matrices[i] * g.inv_matrices[i];
			// g.skin_matrices[i] = g.global_matrices[i];
		}
	}

	// Write to shaders
	{
		var& gpu_st = *g.gpu_state;
		gpu_st.projection_view = g.projection * g.view;
		gpu_st.projection = g.projection;
		gpu_st.view = g.view;
		gpu_st.ambient_color = g.ambient_color;
		u32 idx = 0;
		Loop(i, g.materials.count) {
			var& mat = g.materials[i];
			mat.idx = idx;
			MaterialProps props = mat.props;
			g.gpu_materials[idx++] = {
				.ambient = props.ambient,
				.diffuse = props.diffuse,
				.specular = props.specular,
				.shininess = props.shininess,
				.tex = texture_descriptor_idx(mat.base_color),
			};
		}
		gpu_st.cubemap = texture_descriptor_idx(g.cur_cubemap);
		MemCopyArray(g.gpu_skin_matrices, g.skin_matrices.data, g.bones.count);
	}

	///////////////////////////////////
	// World
	u32 drawcall_count = 0;
	{
		gfx_begin_timestamp("world");
		begin_render_pass({&g.world_rt});
		{
			gfx_apply_viewport(rng2_make(v2(), os_window_size() * g.scale), true);
			gfx_apply_scissor(rng2_make(v2(), os_window_size()));
			gfx_bind_vert(g.gpu_vert_buf);
			gfx_bind_index(g.gpu_index_buf);
			r_bind_pipeline(g.uber_pip);
			for(var& batch : g.batches) {
				apply_pip_state(batch.state);
				var emit_batch = [&](Slice<DrawCall> pushes, b32 indexed) {
					u32 base = gfx_begin_indirect();
					Loop(i, pushes.count) {
						DrawCall draw = pushes[i];
						m4x4 model = m4x4_transform(draw.pos, draw.scale, draw.rot);
						var mat = g.materials[draw.mat.idx];
						g.gpu_drawcalls[drawcall_count] = {
							.model = model,
							.color = draw.color.v,
							.mat = mat.idx,
							.type = draw.type,
							.flags = draw.flags,
						};
						Gfx_Mesh mesh = g.meshes[draw.mesh.idx];
						gfx_push_indirect_mesh(mesh, drawcall_count++);
					}
					Gfx_IndirectDrawCall draw = gfx_end_indirect(base);
					if(draw.count) {
						vk_push_constants({draw.base});
						if(indexed) {
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
			gfx_bind_vert(g.cpu_vert_buf);
			if(g.draw_lines.count) {
				MemCopyArray(g.cpu_vertices, g.draw_lines.data, g.draw_lines.count);
				apply_pip_state({
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
			// if(g.draw_lines_persistent.count) {
			//   gfx_bind_pipeline(g.uber_pip_debug_line);
			//   gfx_draw(g.draw_base_persistent_lines, g.draw_lines_persistent.count);
			// }

			// Cube map
			apply_pip_state({.depth = {.compare = Gfx_CompareOp_LessEqual}});
			gfx_bind_vert(g.gpu_vert_buf);
			g.gpu_drawcalls[drawcall_count].type = ShaderType::Cube;
			vk_push_constants({drawcall_count++});
			gfx_draw_mesh(g.meshes[g.meshes_ids[Mesh_Cube].idx]);

			// UI drawing
			gfx_apply_viewport(rng2_make(v2(), os_window_size() * g.scale));
			if(g.draw_rects.count) {
				Loop(i, g.draw_rects.count) {
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
						.border_thickness = rect.border_thickness,
					};
					ArrayCopy(g.gpu_ui_rects[i].colors, rect.colors);
				}
				r_bind_pipeline(g.ui_rect_pip);
				apply_pip_state({
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
		gfx_end_timestamp();
	}
	
	///////////////////////////////////
	// Swapchain
	gfx_begin_timestamp("swapchain");
	gfx_begin_pass({});
	{
		gfx_apply_viewport(rng2_make(v2(), os_window_size()));
		gfx_bind_pipeline(g.uber_pip);
		apply_pip_state({.sample_count = 1});
		g.gpu_drawcalls[drawcall_count] = {
			.type = ShaderType::Screen,
			.cur_resolve_idx = g.world_rt.resolve.view.idx,
		};
		vk_push_constants({drawcall_count++});
		gfx_draw(0, 3);
		// g.gpu_drawcalls[drawcall_count] = {
		// 	.type = ShaderType::SoftwareRender,
		// 	.cur_resolve_idx = g.world_rt.resolve.views[st->gfx.current_image_idx].idx,
		// };
		// vk_push_constants({drawcall_count++});
		// gfx_draw(0, 3);
#if BUILD_DEV
		imgui_end_frame();
#endif
	}
	gfx_end_pass();
	gfx_end_timestamp();
	gfx_end();
}

void draw_mesh(MeshId mesh, MaterialId mat, v3 pos, u32 flags) {
	var& g = *st;
	var material = g.materials[mat.idx];
	u32 batch_idx = material.batch;
	DrawCall cmd = {
		.pos = pos,
		.scale = v3(1),
		.rot = quat_identity(),
		.mesh = mesh,
		.mat = mat,
		.type = material.type,
		.flags = flags,
	};
	if(g.meshes[mesh.idx].index_count) {
		array_push(g.batches[batch_idx].draws, cmd);
	} else {
		array_push(g.batches[batch_idx].unindexed_draws, cmd);
	}
}

void draw_mesh_trs(MeshId mesh, MaterialId mat, v3 pos, v4 rot, v3 scale) {
	var& g = *st;
	var material = g.materials[mat.idx];
	u32 batch_idx = material.batch;
	DrawCall cmd = {
		.pos = pos,
		.scale = scale,
		.rot = rot,
		.mesh = mesh,
		.mat = mat,
		.type = material.type,
	};
	if(g.meshes[mesh.idx].index_count) {
		array_push(g.batches[batch_idx].draws, cmd);
	} else {
		array_push(g.batches[batch_idx].unindexed_draws, cmd);
	}
}

void draw_thing(ThingId id) {
	var& g = *st;
	Thing& e = get_thing(id);
	var material = g.materials[e.mat.idx];
	u32 batch_idx = material.batch;
	DrawCall cmd = {
		.pos = e.pos,
		.scale = e.scale,
		.rot = e.rot,
		.color = e.color,
		.mesh = e.mesh,
		.mat = e.mat,
		.type = material.type,
	};
	if(g.meshes[e.mesh.idx].index_count) {
		array_push(g.batches[batch_idx].draws, cmd);
	} else {
		array_push(g.batches[batch_idx].unindexed_draws, cmd);
	}
}

void draw_line(v3 a, v3 b, Color color) {
	Vertex vert[] = {
		{.pos = a, .color = color.v},
		{.pos = b, .color = color.v},
	};
	array_push_elems(st->draw_lines, slice(vert));
}

void draw_line_persistent(v3 a, v3 b, Color color) {
	Vertex vert[] = {
		{.pos = a, .color = color.v},
		{.pos = b, .color = color.v},
	};
	array_push_elems(st->draw_lines_persistent, slice(vert));
}

void draw_grid(v3 center, u32 slices, f32 spacing, Color color) {
	var& g = *st;
	f32 half = slices * spacing * 0.5f;
	Loop(i, slices+1) {
		f32 t = i * spacing - half;
		// Horizontal line
		array_push(g.draw_lines, {.pos = center + v3(-half, 0, t), .color = color.v});
		array_push(g.draw_lines, {.pos = center + v3(+half, 0, t), .color = color.v});
		// Vertical line
		array_push(g.draw_lines, {.pos = center + v3(t, 0, -half), .color = color.v});
		array_push(g.draw_lines, {.pos = center + v3(t, 0, +half), .color = color.v});
	}
}

void draw_cuboid(Rng3 rng, Color color) {
	v3 p000 = {rng.min.x, rng.min.y, rng.min.z};
	v3 p001 = {rng.min.x, rng.min.y, rng.max.z};
	v3 p010 = {rng.min.x, rng.max.y, rng.min.z};
	v3 p011 = {rng.min.x, rng.max.y, rng.max.z};

	v3 p100 = {rng.max.x, rng.min.y, rng.min.z};
	v3 p101 = {rng.max.x, rng.min.y, rng.max.z};
	v3 p110 = {rng.max.x, rng.max.y, rng.min.z};
	v3 p111 = {rng.max.x, rng.max.y, rng.max.z};

	draw_line(p000, p001, color);
	draw_line(p000, p010, color);
	draw_line(p000, p100, color);

	draw_line(p111, p110, color);
	draw_line(p111, p101, color);
	draw_line(p111, p011, color);

	draw_line(p001, p011, color);
	draw_line(p001, p101, color);

	draw_line(p010, p011, color);
	draw_line(p010, p110, color);

	draw_line(p100, p101, color);
	draw_line(p100, p110, color);
}

void draw_rect(Rng2 rect, Color color, f32 corner_radius, f32 border_thickness, f32 edge_softness) {
	var& g = *st;
	UI_Rect vert = {
		.dst_p0 = rect.min,
		.dst_p1 = rect.max,
		.corner_radius = corner_radius,
		.border_thickness = border_thickness,
		.edge_softness = edge_softness,
	};
	for(var& x : vert.colors) x = color.v;
	array_push(g.draw_rects, vert);
}

void draw_texture(Rng2 dst, Rng2 src, TextureId tex, Color color, f32 corner_radius, f32 border_thickness, f32 edge_softness) {
	var& g = *st;
	var t = g.textures[tex.idx];
	var desc = gfx_query_image_desc(t.image);
	v2 tex_size = v2(desc.width, desc.height);
	UI_Rect vert = {
		.dst_p0 = dst.min,
		.dst_p1 = dst.max,
		.src_p0 = src.min,
		.src_p1 = src.max,
		.corner_radius = corner_radius,
		.border_thickness = border_thickness,
		.edge_softness = edge_softness,
		.tex_size = tex_size,
		.texture = texture_descriptor_idx(tex),
	};
	for(var& x : vert.colors) x = ColorWhite;
	array_push(g.draw_rects, vert);
}

void draw_texture(Rng2 rect, TextureId tex) {
	var& g = *st;
	var t = g.textures[tex.idx];
	var desc = gfx_query_image_desc(t.image);
	v2 tex_size = v2(desc.width, desc.height);
	UI_Rect vert = {
		.dst_p0 = rect.min,
		.dst_p1 = rect.max,
		.src_p0 = v2(),
		.src_p1 = tex_size,
		.tex_size = tex_size,
		.texture = texture_descriptor_idx(tex),
	};
	for(var& x : vert.colors) x = ColorWhite;
	array_push(g.draw_rects, vert);
}

void draw_text(FontId font, v2 pos, String str, Color color, u32 font_height) {
	var& g = *st;
	var fo = g.fonts[font.idx];
	var desc = gfx_query_image_desc(g.textures[fo.texture.idx].image);
	v2 tex_size = v2(desc.width, desc.height);
	v2 cursor = pos;
	f32 scale = (f32)font_height / fo.font_height;
	Loop(i, str.size) {
		u8 c = str.str[i];
		var glyph = fo.glyphs[c - 32];
		f32 y0 = cursor.y + glyph.yoff * scale;
		f32 x0 = cursor.x + glyph.xoff * scale;
		f32 x1 = x0 + rng2u_width(glyph.rect) * scale;
		f32 y1 = y0 + rng2u_height(glyph.rect) * scale;
		cursor.x += glyph.xadvance * scale;
		UI_Rect rect = {
			.dst_p0 = v2(x0,y0),
			.dst_p1 = v2(x1,y1),
			.src_p0 = glyph.rect.min,
			.src_p1 = glyph.rect.max,
			.tex_size = tex_size,
			.texture = texture_descriptor_idx(fo.texture),
			.flags = GpuUI_RectFlag_IsFont,
		};
		for(var& x : rect.colors) x = color;
		array_push(g.draw_rects, rect);
	}
}

////////////////////////////////////////////////////////////////////////
// @Dear imgui

#if BUILD_DEV
#include "../vendor/imgui/imgui_impl_vulkan.h"

ImGuiKey imgui_keycode_translate(Key key) {
	switch(key) {
		default: return ImGuiKey_None;
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
	}
}

u32 imgui_mouse_button_translate(MouseButton button) {
	switch(button) {
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
	io.DisplaySize = v2(os_window_size());
	Slice<OS_InputEvent> events = os_get_input_events();
	if(last_key_event.is_pressed) {
		_timer_type_repeat_delay.acc += time_dt;
		if(_timer_type_repeat_delay.acc >= _timer_type_repeat_delay.interval) {
			if(time_on_interval(1.0/20)) {
				io.AddInputCharacter(os_key_to_character(last_key_event.key, last_key_event.modifier));
			}
		}
	}
	for(var event : events) switch(event.type) {
		case OS_EventType_Key: {
			if(event.key < Key_COUNT && event.key != Key_Super) {
				last_key_event = event;
				_timer_type_repeat_delay.acc = 0;
			}
			ImGuiKey key = imgui_keycode_translate(event.key);
			io.AddKeyEvent(key, event.is_pressed);
			// Info("%u %u", event.key, event.is_pressed);
			if(event.is_pressed) {
				io.AddInputCharacter(os_key_to_character(event.key, event.modifier));
			}
		}break;
		case OS_EventType_MouseButton: {
			io.AddMouseButtonEvent(imgui_mouse_button_translate(event.mouse_button), event.is_pressed);
		}break;
		case OS_EventType_MouseMove: {
			io.AddMousePosEvent(event.x, event.y);
		}break;
		case OS_EventType_Scroll: {
			io.AddMouseWheelEvent(0, event.scroll);
		}break;
		case OS_EventType_Modifier: {
			if(flag_has(event.modifier, OS_Modifier_Shift)) {
				io.AddKeyEvent(ImGuiMod_Shift, true);
				// Info("mod shift true");
			} else {
				io.AddKeyEvent(ImGuiMod_Shift, false);
				// Info("mod shift false");
			}
			if(flag_has(event.modifier, OS_Modifier_Alt)) {
				io.AddKeyEvent(ImGuiMod_Alt, true);
			} else {
				io.AddKeyEvent(ImGuiMod_Alt, false);
			}
			if(flag_has(event.modifier, OS_Modifier_Ctrl)) {
				io.AddKeyEvent(ImGuiMod_Ctrl, true);
			} else {
				io.AddKeyEvent(ImGuiMod_Ctrl, false);
			}break;
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
		{VK_DESCRIPTOR_TYPE_SAMPLER, 100},
		{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100},
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
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), st->gfx.render_cmds[st->gfx.cur_frame_idx]);
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}

void imgui_image(TextureId id, v2 pos) {
	var& g = *st;
	var& texture = g.textures[id.idx];
	if(texture.imgui_id == 0) {
		var vkview = pool_get(g.gfx.views, g.textures[id.idx].view);
		texture.imgui_id = (ImTextureID)ImGui_ImplVulkan_AddTexture(vkview.h, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	ImGui::Image(texture.imgui_id, pos);
}

void imgui_draw_rect(ImDrawList* draw, Rng2 rect, Color col, f32 rounding, ImDrawFlags flags, f32 thickness) {
	draw->AddRect(rect.min, rect.max, col.v, rounding, thickness, flags);
}
void imgui_draw_rect_filled(ImDrawList* draw, Rng2 rect, Color col, f32 rounding, ImDrawFlags flags) {
	draw->AddRectFilled(rect.min, rect.max, col.v);
}
void imgui_draw_line(ImDrawList* draw, v2 p0, v2 p1, Color col, f32 thickness) {
	draw->AddLine(p0, p1, col.v);
}
void imgui_draw_text(ImDrawList* draw, v2 pos, Color col, String fmt, ...) {
	Scratch scratch;
	VaList args;
	va_start(args, fmt);
	String formateted = push_strfv(scratch, fmt, args);
	va_end(args);
	draw->AddText(pos, col.v, (char*)formateted.str, (char*)(formateted.str + formateted.size));
}
void imgui_draw_text(ImDrawList* draw, f32 font_size, v2 pos, Color col, String fmt, ...) {
	Scratch scratch;
	VaList args;
	va_start(args, fmt);
	String formateted = push_strfv(scratch, fmt, args);
	va_end(args);
	draw->AddText(null, font_size, pos, col.v, (char*)formateted.str, (char*)(formateted.str + formateted.size));
}
void imgui_text(String fmt, ...) {
	Scratch scratch;
	VaList args;
	va_start(args, fmt);
	String formateted = push_strfv(scratch, fmt, args);
	va_end(args);
	ImGui::TextUnformatted((char*)formateted.str, (char*)(formateted.str + formateted.size));
}
v2 imgui_calc_text_size(String str) {
	return ImGui::CalcTextSize((char*)str.str, (char*)str.str+str.size);
}

#endif

Vertex cube_vertices[] = {
	// Front face (0, 0, 1)
	{.pos = v3(-1, -1,  1), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(0, 0)},
	{.pos = v3( 1, -1,  1), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(1, 0)},
	{.pos = v3( 1,  1,  1), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(1, 1)},
	{.pos = v3( 1,  1,  1), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(1, 1)},
	{.pos = v3(-1,  1,  1), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(0, 1)},
	{.pos = v3(-1, -1,  1), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(0, 0)},

	// Back face (0, 0, -1)
	{.pos = v3( 1, -1, -1), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(0, 0)},
	{.pos = v3(-1, -1, -1), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(1, 0)},
	{.pos = v3(-1,  1, -1), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(1, 1)},
	{.pos = v3(-1,  1, -1), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(1, 1)},
	{.pos = v3( 1,  1, -1), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(0, 1)},
	{.pos = v3( 1, -1, -1), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(0, 0)},

	// Left face (-1, 0, 0)
	{.pos = v3(-1, -1, -1),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(0, 0)},
	{.pos = v3(-1, -1,  1),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(1, 0)},
	{.pos = v3(-1,  1,  1),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(1, 1)},
	{.pos = v3(-1,  1,  1),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(1, 1)},
	{.pos = v3(-1,  1, -1),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(0, 1)},
	{.pos = v3(-1, -1, -1),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(0, 0)},

	// Right face (1, 0, 0)
	{.pos = v3(1, -1,  1),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(0, 0)},
	{.pos = v3(1, -1, -1),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(1, 0)},
	{.pos = v3(1,  1, -1),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(1, 1)},
	{.pos = v3(1,  1, -1),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(1, 1)},
	{.pos = v3(1,  1,  1),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(0, 1)},
	{.pos = v3(1, -1,  1),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(0, 0)},

	// Bottom face (0, -1, 0)
	{.pos = v3(-1, -1, -1),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(0, 1)},
	{.pos = v3( 1, -1, -1),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(1, 1)},
	{.pos = v3( 1, -1,  1),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(1, 0)},
	{.pos = v3( 1, -1,  1),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(1, 0)},
	{.pos = v3(-1, -1,  1),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(0, 0)},
	{.pos = v3(-1, -1, -1),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(0, 1)},

	// Top face (0, 1, 0)
	{.pos = v3(-1,  1,  1), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(0, 0)},
	{.pos = v3( 1,  1,  1), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(1, 0)},
	{.pos = v3( 1,  1, -1), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(1, 1)},
	{.pos = v3( 1,  1, -1), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(1, 1)},
	{.pos = v3(-1,  1, -1), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(0, 1)},
	{.pos = v3(-1,  1,  1), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(0, 0)},
};

Vertex triangle_vertices[] = {
	{.pos = v3( 0.0f,   0.5f, 0), .uv = v2(0.5f, 0), .color = u32_from_rgba(v4(1,0,0,1))},
	{.pos = v3(-0.5f,  -0.5f, 0), .uv = v2(0.0f, 1), .color = u32_from_rgba(v4(0,1,0,1))},
	{.pos = v3( 0.5f,  -0.5f, 0), .uv = v2(1.0f, 1), .color = u32_from_rgba(v4(0,0,1,1))},
};

Vertex axis_vertices[] = {
	{.pos = v3(),      .color = u32_from_rgba(v4(1,0,0,1))},
	{.pos = v3(1,0,0), .color = u32_from_rgba(v4(1,0,0,1))},
	{.pos = v3(),      .color = u32_from_rgba(v4(0,1,0,1))},
	{.pos = v3(0,1,0), .color = u32_from_rgba(v4(0,1,0,1))},
	{.pos = v3(),      .color = u32_from_rgba(v4(0,0,1,1))},
	{.pos = v3(0,0,1), .color = u32_from_rgba(v4(0,0,1,1))},
};

global_var String meshes_strs[] = {
#define X(name) [Glue(Mesh_, name)] = Stringify(name),
	MESH_LIST
#undef X
};

global_var String textures_strs[] = {
#define X(name) [Glue(Texture_, name)] = Stringify(name),
	TEXTURE_LIST
#undef X
};

global_var String materials_strs[] = {
#define X(name) [Glue(Material_, name)] = Stringify(name),
	MATERIAL_LIST
#undef X
};

global_var String things_enum_strs[] = {
#define X(name) [name] = Stringify(name),
	THING_LIST
#undef X
};

const u32 TEST_SAMPLES = 100;
global_var i32 test_alignments[] = { 8, 16, 32, 64 };

void test_arena_alloc() {
	Arena arena = arena_make();
	Array<u8*, TEST_SAMPLES> arr = {};
	Array<u32, TEST_SAMPLES> sizes = {};
	Array<u32, TEST_SAMPLES> values = {};
	Loop(i, TEST_SAMPLES) {
		u32 size = rand_u32(8, KB(1));
		u64 align = ArrayRand(test_alignments);
		arr[i] = push_buffer(arena, size, align);
		sizes[i] = size;
		values[i] = rand_u32(0, 255);
		MemSet(arr[i], values[i], size);
	}
	Loop(i, TEST_SAMPLES) {
		u8* buf = arr[i];
		u32 size = sizes[i];
		u32 value = values[i];
		Loop(j, size) {
			AssertAlways(buf[j] == value);
		}
	}
	arena_destroy(arena);
}

void test_arena_list_alloc() {
	Scratch scratch;
	ArenaList arena(scratch);
	Array<u8*, TEST_SAMPLES> arr = {};
	Array<u32, TEST_SAMPLES> sizes = {};
	Array<u32, TEST_SAMPLES> values = {};

	Loop(i, TEST_SAMPLES) {
		u32 size = rand_u32(8, KB(1));
		u64 align = ArrayRand(test_alignments);
		arr[i] = push_buffer(arena, size, align);
		sizes[i] = size;
		values[i] = rand_u32(0, 255);
		MemSet(arr[i], values[i], size);
	}
	Loop(i, TEST_SAMPLES) {
		u8* buf = arr[i];
		u32 size = sizes[i];
		u32 value = values[i];
		Loop(j, size) {
			AssertAlways(buf[j] == value);
		}
	}
	alloc_arena_list_clear(arena);

	Loop(i, TEST_SAMPLES) {
		u32 size = rand_u32(8, KB(1));
		u64 align = ArrayRand(test_alignments);
		arr[i] = push_buffer(arena, size, align);
		sizes[i] = size;
		values[i] = rand_u32(0, 255);
		MemSet(arr[i], values[i], size);
	}
	Loop(i, TEST_SAMPLES) {
		u8* buf = arr[i];
		u32 size = sizes[i];
		u32 value = values[i];
		Loop(j, size) {
			AssertAlways(buf[j] == value);
		}
	}
	alloc_arena_list_clear(arena);
}

void test_alloc() {
	Scratch scratch;
	Alloc alloc = alloc_make(scratch);
	defer{alloc_destroy(alloc);};
	struct AllocCtx {
		u8* data;
		u64 size;
	};
	Array<AllocCtx, TEST_SAMPLES> arr = {};

	Loop(i, TEST_SAMPLES) {
		u64 size = rand_u32(8, KB(1));
		u64 align = ArrayRand(test_alignments);
		array_push(arr, {mem_alloc(alloc, size, align), size});
		MemZero(arr[i].data, size);
	}
	Array<u32, TEST_SAMPLES> indices = {};
	Loop(i, TEST_SAMPLES) array_push(indices, (u32)i);
	rand_shuffle(slice(indices));
	Loop(i, TEST_SAMPLES) {
		mem_free(alloc, arr[indices[i]].data, arr[indices[i]].size);
	}

	array_clear(arr);
	Loop(i, TEST_SAMPLES) {
		u64 size = rand_u32(8, KB(1));
		u64 align = ArrayRand(test_alignments);
		array_push(arr, {mem_alloc(alloc, size, align), size});
		MemZero(arr[i].data, size);
	}
	array_clear(indices);
	Loop(i, TEST_SAMPLES) array_push(indices, (u32)i);
	rand_shuffle(slice(indices));
	Loop(i, TEST_SAMPLES) {
		mem_free(alloc, arr[indices[i]].data, arr[indices[i]].size);
	}
}

void test_gpu_seglist_alloc() {
	Scratch scratch;
	GpuAllocSegList alloc = {.cap = MB(1)};
	alloc = gpu_alloc_seglist_make(scratch);
	Array<GpuMemId, TEST_SAMPLES> arr = {};

	Loop(i, TEST_SAMPLES) {
		u64 size = rand_u32(8, KB(1));
		u64 align = ArrayRand(test_alignments);
		array_push(arr, gpu_alloc_seglist_alloc(alloc, size, align));
	}
	Array<u32, TEST_SAMPLES> indices = {};
	Loop(i, TEST_SAMPLES) array_push(indices, (u32)i);
	rand_shuffle(slice(indices));
	Loop(i, TEST_SAMPLES) {
		gpu_alloc_seglist_free(alloc, arr[indices[i]]);
	}

	array_clear(arr);
	Loop(i, TEST_SAMPLES) {
		u64 size = rand_u32(8, KB(1));
		u64 align = ArrayRand(test_alignments);
		array_push(arr, gpu_alloc_seglist_alloc(alloc, size, align));
	}
	array_clear(indices);
	Loop(i, TEST_SAMPLES) array_push(indices, (u32)i);
	rand_shuffle(slice(indices));
	Loop(i, TEST_SAMPLES) {
		gpu_alloc_seglist_free(alloc, arr[indices[i]]);
	}
}

void test_object_pool() {
	Scratch scratch;
	struct A {
		u32 a;
		u32 b;
	};
	var pool = pool_make<A, OpaqueId>(scratch);
	Array<A, TEST_SAMPLES> values = {};
	Array<OpaqueId, TEST_SAMPLES> handlers = {};

	Loop(i, TEST_SAMPLES) {
		values[i].a = rand_u32(0, TEST_SAMPLES);
		values[i].b = rand_u32(0, TEST_SAMPLES);
	};
	Loop(i, TEST_SAMPLES) {
		handlers[i] = pool_push(pool, values[i]);
	}
	Loop(i, TEST_SAMPLES) {
		AssertAlways(MemMatchStruct(&values[i], &pool_get(pool, handlers[i])));
	}
	Array<u32, TEST_SAMPLES> indices = {};
	Loop(i, TEST_SAMPLES) array_push(indices, (u32)i);
	rand_shuffle(slice(indices));
	Loop(i, TEST_SAMPLES) {
		pool_remove(pool, handlers[i]);
	}

	array_clear(indices);
	Loop(i, TEST_SAMPLES) {
		values[i].a = rand_u32(0, TEST_SAMPLES);
		values[i].b = rand_u32(0, TEST_SAMPLES);
	};
	Loop(i, TEST_SAMPLES) {
		handlers[i] = pool_push(pool, values[i]);
	}
	Loop(i, TEST_SAMPLES) {
		AssertAlways(MemMatchStruct(&values[i], &pool_get(pool, handlers[i])));
	}
	Loop(i, TEST_SAMPLES) array_push(indices, (u32)i);
	rand_shuffle(slice(indices));
	Loop(i, TEST_SAMPLES) {
		pool_remove(pool, handlers[i]);
	}
}

void test_id_pool() {
	Scratch scratch;
	{
		DIdPool id_pool = id_pool_make(scratch);
		Array<u32, TEST_SAMPLES> arr = {};
		Loop(i, TEST_SAMPLES) {
			u32 id = id_pool_push(id_pool);
			array_push(arr, id);
		}
		rand_shuffle(slice(arr));
		Loop(i, arr.count) {
			id_pool_remove(id_pool, arr[i]);
		}
		Array<u32, TEST_SAMPLES> new_arr = {};
		Loop(i, arr.count) {
			u32 id = id_pool_push(id_pool);
			array_push(new_arr, id);
		}
		Loop(i, arr.count) {
			b32 exists = false;
			Loop(j, arr.count) {
				if(arr[i] == new_arr[j]) {
					AssertAlways(exists == false);
					exists = true;
				}
			}
			AssertAlways(exists);
		}
		Loop(i, arr.count) {
			id_pool_remove(id_pool, new_arr[i]);
		}
	}

	{
		IdPool<TEST_SAMPLES> static_id_pool = {};
		id_pool_init(static_id_pool);
		Array<u32, TEST_SAMPLES> arr = {};
		Loop(i, TEST_SAMPLES) {
			u32 id = id_pool_push(static_id_pool);
			array_push(arr, id);
		}
		rand_shuffle(slice(arr));
		Loop(i, arr.count) {
			id_pool_remove(static_id_pool, arr[i]);
		}
		Array<u32, TEST_SAMPLES> new_arr = {};
		Loop(i, arr.count) {
			u32 id = id_pool_push(static_id_pool);
			array_push(new_arr, id);
		}
		Loop(i, arr.count) {
			b32 exists = false;
			Loop(j, arr.count) {
				if(arr[i] == new_arr[j]) {
					AssertAlways(exists == false);
					exists = true;
				}
			}
			AssertAlways(exists);
		}
		Loop(i, arr.count) {
			id_pool_remove(static_id_pool, new_arr[i]);
		}
	}
}

void test_profiler_bar() {
	ProfFunc;
	{
		ProfBlock("block in bar");
		os_sleep_ms(1);
	}
	os_sleep_ms(2);
}

void test_profiler_der() {
	ProfFunc;
	os_sleep_ms(10);
}

void test_profiler_die(i32 i) {
	ProfFunc;
	os_sleep_ms(1);
	if(--i) {
		test_profiler_die(i);
	}
}

// intern void test_profile_print_time_elapsed(u64 total_tsc_elapsed, ProfileAnchor anchor) {
//   Scratch scratch;
//   String label_c = push_str_copy(scratch, anchor.label);
//   f64 percent = 100.0 * ((f64)anchor.tsc_elapsed_exclusive / (f64)total_tsc_elapsed);
//   print("  %s[%u64]: %u64 (%.2f%%)", label_c.str, anchor.hit_count, anchor.tsc_elapsed_exclusive, percent);
//   if(anchor.tsc_elapsed_inclusive != anchor.tsc_elapsed_exclusive) {
//     f64 percent_with_children = 100.0 * ((f64)anchor.tsc_elapsed_inclusive / (f64)total_tsc_elapsed);
//     print(", %.2f%% w/children", percent_with_children);
//   }
//   print(")\n");
// }

// intern void test_profiler_print() {
	// u64 cpu_freq = cpu_frequency();
	// u64 total_cpu_elapsed = g_st->profiler.prev_tsc_elapsed;
	// if(cpu_freq) {
	//   print("\nTotal time: %0.4fms (CPU freq %lu)\n", 1000.0 * (f64)total_cpu_elapsed / (f64)cpu_freq, cpu_freq);
	// }
	// Slice<ProfileAnchor> anchors = profiler_get_current_frame_anchors();
	// Loop(anchor_idx, anchors.count) {
	//   ProfileAnchor anchor = anchors[anchor_idx];
	//   if(anchor.tsc_elapsed_inclusive) {
	//     test_profile_print_time_elapsed(total_cpu_elapsed, anchor);
	//   }
	// }
// }

void test_profiler() {
	// profiler_begin();
	test_profiler_bar();
	// test_profiler_bar();
	// test_profiler_der();
	// test_profiler_die(10);
}

void test_sort() {
	Scratch scratch;
	{
		Info("insert");
		i32 arr[] = {0, -4, -3, 7, 3};
		sort_insert(slice(arr), [](var a, var b) { return a < b; });
		LoopArray (i, arr) {
			print("%i ", arr[i]);
		}
		print("\n");
	}
	{
		Info("quick");
		i32 arr[] = {0, -4, -3, 7, 3};
		sort_quick(slice(arr), [](var a, var b) { return a < b;});
		LoopArray (i, arr) {
			print("%i ", arr[i]);
		}
		print("\n");
	}
	{
		Scratch scratch;
		Info("merge");
		i32 arr[] = {0, -4, -3, 7, 3};
		sort_merge(scratch, slice(arr), [](var a, var b) { return a < b;});
		LoopArray (i, arr) {
			print("%i ", arr[i]);
		}
		print("\n");
	}
	{
		Scratch scratch;
		Info("radix");
		i32 arr[] = {0, -4, -3, 7, 3};
		Slice<SortEntry> entries = push_slice(scratch, SortEntry, ArrayCount(arr));
		LoopArray (i, arr) {
			entries[i] = {sort_i32_key_to_u32(arr[i]), (u32)i};
		}
		sort_radix(entries);
		LoopArray (i, arr) {
			print("%i ", arr[entries[i].idx]);
		}
		print("\n");
	}

	u32 counts[] = {32, 64, 128, 512, KB(1), KB(10), KB(100), MB(1)};
	Slice<u32> arrs[ArrayCount(counts)];
	LoopArray (i, counts) {
		arrs[i] = push_slice(scratch, u32, counts[i]);
		Loop(j, counts[i]) {
			arrs[i][j] = rand_i32();
		}
	}
	print("\n/////////////////\n");
	Info("\nQuick test:");
	Loop(i, 2) {
		LoopArray (i, counts) {
			Info("Count %i", counts[i]);
			rand_shuffle(arrs[i]);
			{
				u64 s = cpu_now();
				sort_quick(arrs[i], [](var a, var b) {return a < b;});
				Info("%fms", tsc_to_ms(cpu_now()-s));
			}
		}
	}
	Info("\nmerge test:");
	Loop(i, 2) {
		LoopArray (i, counts) {
			Scratch scratch;
			Info("Count %i", counts[i]);
			rand_shuffle(arrs[i]);
			{
				u64 s = cpu_now();
				sort_merge(scratch, arrs[i], [](var a, var b) {return a < b;});
				Info("%fms", tsc_to_ms(cpu_now()-s));
			}
		}
	}
	Info("\nradix test:");
	Loop(i, 2) {
		LoopArray (i, counts) {
			Scratch scratch;
			Info("Count %i", counts[i]);
			rand_shuffle(arrs[i]);
			Slice<SortEntry> entries = push_slice(scratch, SortEntry, counts[i]);
			Loop(j, counts[i]) {
				entries[j] = {arrs[i][j], (u32)j};
			}
			{
				u64 s = cpu_now();
				Scratch scratch;
				sort_radix(entries);
				Info("%fms", tsc_to_ms(cpu_now()-s));
			}
		}
	}
}

void test_js() {
	Scratch scratch;
	String d = R"(
	{
		"name": "cube",
		"visible": true,
		"pos": [1.0, 2.0, 3.0],
		"material": {
			"color": [1.0, 0.0, 0.0],
			"roughness": 0.5
		}
	}
	)";

	{
		JsParser p = js_parse_make(scratch, d);
		JsObj root = js_parse(p).obj;
		String name = {}; AssertOk(js_str(root, "name", &name));
		Info("%s", name);
		b32 visible = 0; AssertOk(js_bool(root, "visible", &visible));
		Info("%u", visible);
		Slice<JsVal*> pos = {}; AssertOk(js_array(root, "pos", &pos));
		Loop(i, pos.count) {
			Info("%f", pos[i]->number);
		}
		{
			JsObj material = {}; AssertOk(js_obj(root, "material", &material));
			Slice<JsVal*> color = {}; AssertOk(js_array(material, "color", &color));
			Loop(i, color.count) {
				Info("%f", color[i]->number);
			}
			f64 roughness = 0; AssertOk(js_f64(material, "roughness", &roughness));
			Info("%f", roughness);
		}
	}
}

void test_co1(Coroutine& co, f32 dt) {
	co_begin(co);

	Info("child");
	co_wait(1);
	co_yield();
	Info("child waits");
	co_wait(1);

	co_end();
}

void test_co(Coroutine& co, f32 dt) {
	f32 wait_time = 1;
	var& a = co_var(co, u32);
	co_begin(co);

	Info("start");
	co_wait(wait_time);
	co_yield();

	Info("waited 0, %i", a++);
	co_wait(wait_time);
	co_call(test_co1(co, dt));
	Info("waited 1, %i", a);
	co_wait(wait_time);
	Info("waited 2, %i", a);

	co_end();
}

void test() {
	ProfFunc;
	// test_sort();
	// os_exit(0);
	test_alloc();
	test_arena_alloc();
	test_arena_list_alloc();
	test_gpu_seglist_alloc();
	test_object_pool();
	// test_handle_darray();
	test_id_pool();
}

f64 tsc_to_ms(u64 tsc) { return (f64)tsc/cpu_frequency*1000; }
b32 time_on_interval(f64 time, f32 delta, f32 interval, f32 offset) {
	u32 last = (time - offset - delta) / interval;
	u32 next = (time - offset) / interval;
	return last < next;
}
u32 time_on_interval_steps(f64 time, f32 delta, f32 interval, f32 offset) {
	u32 last = (time - offset - delta) / interval;
	u32 next = (time - offset) / interval;
	return next - last;
}
f64 time_next_interval(f64 time, f32 interval, f32 offset) {
	u32 next = (time - offset) / interval + 1;
	return offset + next * interval;
}
f64 time_prev_interval(f64 time, f32 interval, f32 offset) {
	u32 prev = (time - offset) / interval;
	return offset + prev * interval;
}
b32 time_on_time(f64 time, f64 timestamp, f64 dt)                  { return time >= timestamp && (time - dt) < timestamp; }
b32 time_on_between_interval(f64 time, f32 interval, f32 offset)   { return mod(time - offset, interval*2) >= interval; }
f32 time_percent(f64 time, f64 start, f64 duration)                { return (time - start) / duration; }
f32 time_lerp_delta(f32 current, f32 target, f32 rate, f32 delta)  { return target + (current - target) * exp(-rate * delta); }
b32 time_on_frame_interval(u32 frame, u32 n, u32 offset = 0)       { return (frame + offset) % n == 0; }
f64 time_saw_wave(f64 time, f32 interval, f32 offset) {
	f64 t = (time - offset) / interval;
	return t - floor(t);
}
f32 time_sine_wave(f64 time, f32 period) {
	return sin(time / period * 2.0f * PI);
}
f32 time_smooth_wave(f64 time, f32 period) {
	f32 p = time_saw_wave(time, period, 0);
	return 0.5f - 0.5f * cos(p * 2.0f * PI);
}
f32 time_triangle_wave(f64 time, f32 period) {
	f32 p = time_saw_wave(time, period, 0);
	return (1.0f - Abs(2.0f * p - 1.0f));
}
b32 time_pulse_wave(f64 time, f32 period, f32 duration, f32 offset) {
	return mod(time - offset, period) < duration;
}
u32 time_frame(f64 time, f32 frame_duration, u32 frame_count) {
	return u32(time / frame_duration) % frame_count;
}

b32 time_elapsed(f64 time, f32 start, f32 duration) { 
	return time > start && start + duration > time;
}
b32 time_between(f64 time, f32 start, f32 end) {
	return rng1_contains({start, end}, time);
}

b32 time_on_interval(f32 interval, f32 offset)         { return time_on_interval(time_now, time_dt, interval, offset); }
b32 time_on_between_interval(f32 interval, f32 offset) { return time_on_between_interval(time_now, interval, offset); }
f64 time_since(f64 timestamp) { return time_now - timestamp; }
f64 time_until(f64 timestamp) { return timestamp - time_now; }

void debug_window_apply_state(DebugWindow& win) {
	if(win.toggle_fullscreen) {
		if(win.fullscreen) {
			win.fullscreen = false;
			ImGui::SetNextWindowPos(win.pos);
			ImGui::SetNextWindowSize(win.size);
			win.flags = NoFlags;
		} else {
			win.fullscreen = true;
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
			win.flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
		}
		win.toggle_fullscreen = false;
	}
}

void debug_window_track_state(DebugWindow& win) {
	if(!win.fullscreen) {
		win.pos = ImGui::GetWindowPos();
		win.size = ImGui::GetWindowSize();
	}
}

void debug_window_toggle_fullscreen(DebugWindow& win) {
	win.toggle_fullscreen = 1;
}

void dev_init() {
#if BUILD_DEV
	var& g = *st;
	g.prof_win = {
		.win.open = false,
		.root_cam = {.zoom = 1, .zoom2 = v2(1)},
		.frames_cam = {.zoom = 1, .zoom2 = v2(1)},
		.launch_cam = {.zoom = 1, .zoom2 = v2(1)},
		.mem_cam = {.zoom = 1, .zoom2 = v2(1)},
	};
	g.imgui_demo_open = false;
	imgui_init();
	g.prof_win.colors = {
		.work = ColorGreyDark,
		.sleep = ColorGreenUi,
		.job = ColorOrangeUi,
		.async = ColorBlueUi,
		.current_frame = ColorGrey3,
		.frame_ok = ColorGreen,
		.frame_warn = ColorYellow,
		.frame_bad = ColorRed,
		.mem_used = ColorGreenUi,
		.mem_committed = ColorBlueUi,
		.mem_cap = ColorRedUi,
	};
#endif
}

void dev_update() {
#if BUILD_DEV
	Scratch scratch;
	var& g = *st;
	if(g.imgui_demo_open) ImGui::ShowDemoWindow();
	if(os_key_is_pressed(Key_F1)) g.prof_win.win.open = !g.prof_win.win.open;
	if(os_key_is_pressed(Key_F2)) g.imgui_demo_open = !g.imgui_demo_open;
	if(os_key_is_pressed(Key_F3)) g.game_win.open = !g.game_win.open;

	///////////////////////////////////
	// Game
	{
		var& win = st->game_win;
		if(win.open) {
			debug_window_apply_state(win);
			ImGuiWindow("Game") {
				if(ImGui::IsWindowHovered() && os_key_is_pressed(Key_V)) {
					debug_window_toggle_fullscreen(win);
				}
				if(ImGui::Button("save state")) {
					save_game_state();
				}
				if(ImGui::Button("load state")) {
					load_game_state();
				}
				ImGui::DragFloat3("cam pos", g.cam.pos.v);
				ImGui::DragFloat("cam vel fric", &g.cam.vel_friction, 0.1, 0, 100);

				imgui_image(g.textures_ids[Texture_Dummy], v2(100));
			}
		}
	}

	///////////////////////////////////
	// Profiler
	{
		struct  {
			f32 tabs_height = 30;
			f32 top_bar_height = 48;
			f32 thread_name_text_size = 20;
			f32 time_bar_text_size = 15;
			f32 bar_height = 30;
			f32 thread_height_off = 200;
			f32 bar_race_height = 10;
		}l;

		ProfBlock("Profiler");
		var& prof = prof_st;
		var& prof_win = st->prof_win;

		// Avg, min, max
		u64 tsc_elapsed_sum = 0;
		u64 tsc_elapsed_max = 0;
		u64 tsc_elapsed_min = U64_MAX;
		for(var frame : prof.frames_times) {
			u64 elapsed = frame.tsc_end - frame.tsc_start;
			tsc_elapsed_sum += elapsed;
			tsc_elapsed_max = Max(tsc_elapsed_max, elapsed);
			tsc_elapsed_min = Min(tsc_elapsed_min, elapsed);
		}
		f32 frame_avg_time = tsc_to_ms(tsc_elapsed_sum / ProfRecordHistoryNum);
		f32 frame_max_time = tsc_to_ms(tsc_elapsed_max);
		f32 frame_min_time = tsc_to_ms(tsc_elapsed_min);

		// Prev frame
		ProfFrame prev_frame = prof_get_prev_frame();
		u64 prev_frame_tsc_start = prev_frame.frame_time.tsc_start;
		u64 prev_frame_tsc_end = prev_frame.frame_time.tsc_end;
		u64 prev_frame_tsc_elapsed = prev_frame_tsc_end - prev_frame_tsc_start;
		ProfColors colors = prof_win.colors;
		if(os_key_is_pressed(Key_H)) {
			ImGui::SetNextWindowFocus(); 
		}
		if(prof_win.win.open) {
			debug_window_apply_state(prof_win.win);
			ImGuiWindow("Profiler", prof_win.win.flags) {
				debug_window_track_state(prof_win.win);
				if(os_key_is_pressed(Key_1)) prof_win.active_tab = ProfileTabActive_Root;
				if(os_key_is_pressed(Key_2)) prof_win.active_tab = ProfileTabActive_Frames;
				if(os_key_is_pressed(Key_3)) prof_win.active_tab = ProfileTabActive_Time;
				if(os_key_is_pressed(Key_4)) prof_win.active_tab = ProfileTabActive_LaunchTime;
				if(os_key_is_pressed(Key_5)) prof_win.active_tab = ProfileTabActive_Memory;
				if(os_key_is_pressed(Key_P)) prof.paused = !prof.paused;
				if(ImGui::IsWindowHovered() && os_key_is_pressed(Key_V)) {
					debug_window_toggle_fullscreen(prof_win.win);
				}
				var draw = ImGui::GetWindowDrawList();
				Rng2 win_rect = rng2_make(ImGui::GetCursorScreenPos(), ImGui::GetContentRegionAvail());
				ImGuiPushClipRect2(win_rect);
				ImGuiBeginTabBar("MyTabBar") {
					rng2_cut_top(&win_rect, l.tabs_height);
					var top_bar_rect = rng2_cut_top(&win_rect, l.top_bar_height);
					var info_rect = win_rect;
					ImGui::SetCursorScreenPos(top_bar_rect.min);
					local_persist f32 fps;
					if(time_on_interval(0.1)) fps = 1000 / tsc_to_ms(prev_frame_tsc_elapsed);
					imgui_text("%.1ffps %.1fms CPU %.1fGhz, Recording: %s", fps, tsc_to_ms(prev_frame_tsc_elapsed), (f64)cpu_frequency / Billion(1), prof.paused ? S("off") : S("on"));
					imgui_text("avg %.1fms, max %.1f, min %.1f", frame_avg_time, frame_max_time, frame_min_time);

					///////////////////////////////////
					// Tab mouse click
					var tab_mouse_click_handle = [&](ProfTabActive tab) {
						Rng2 tab_rect = Rng2(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
						if(os_mouse_is_button_pressed(MouseButton_Left)) {
							if(rng2_contains(tab_rect, os_mouse_pos())) switch(tab) {
								case ProfileTabActive_Root: prof_win.active_tab = ProfileTabActive_Root; break;
								case ProfileTabActive_Frames: prof_win.active_tab = ProfileTabActive_Frames; break;
								case ProfileTabActive_Time: prof_win.active_tab = ProfileTabActive_Time; break;
								case ProfileTabActive_LaunchTime: prof_win.active_tab = ProfileTabActive_LaunchTime; break;
								case ProfileTabActive_Memory: prof_win.active_tab = ProfileTabActive_Memory; break;
							}
						}
					};

					var draw_thread_names = [&](Rng2 rect, Camera2 cam) {
						f32 thread_height = 200;
						f32 thread_height_offset = 0;
						f32 text_height_off = -30;
						String str = push_strf(scratch, "Main thread");
						v2 text_pos = {0, text_height_off};
						text_pos = {rect.min.x, world_to_screen2(cam, text_pos, rect.min).y};
						imgui_draw_text(draw, l.thread_name_text_size, text_pos, ColorWhite, str);
						Loop(i, Thread_NumWorkers) {
							thread_height_offset += thread_height;
							String str = push_strf(scratch, "Worker %i", i);
							v2 text_pos = {0, thread_height_offset + text_height_off};
							text_pos = {rect.min.x, world_to_screen2(cam, text_pos, rect.min).y};
							imgui_draw_text(draw, l.thread_name_text_size, text_pos, ColorWhite, str);
						}
					};

					var draw_frame_graph = [&](Rng2 info_rect, Slice<Slice<ProfAnchor>> slices, ProfFrameTime time, f32 width_off, Camera2 cam, b32 wrap = false) {
						ProfFunc;
						Scratch scratch;
						Loop(i, slices.count) {
							var anchors = slices[i];
							u64 tsc_elapsed = time.tsc_end - time.tsc_start;
							for(var anchor : anchors) {

								// Handle async anchors
								if(wrap) {
									if(anchor.tsc_end == 0) {
										anchor.tsc_elapsed_excl = time.tsc_end - anchor.tsc_start;
										anchor.tsc_end = time.tsc_end;
									}
									if(anchor.tsc_start < time.tsc_start) {
										anchor.tsc_start = time.tsc_start;
									}
								} else if(anchor.tsc_end == 0) continue;
								f64 width = remap_f64(anchor.tsc_end-anchor.tsc_start, tsc_elapsed, rng2_width(info_rect));
								f64 start_off = remap_f64(anchor.tsc_start, time.tsc_start, time.tsc_end, 0, rng2_width(info_rect));
								Rng2 item_rect = rng2_make(v2(start_off, anchor.depth*l.bar_height), v2(width, l.bar_height));

								// Offset
								item_rect = rng2_shift(item_rect, v2(width_off, l.thread_height_off * i));

								// To Screen
								item_rect.min = world_to_screen2(cam, item_rect.min, info_rect.min);
								item_rect.max = world_to_screen2(cam, item_rect.max, info_rect.min);

								///////////////////////////////////
								// Draw
								Color color = {};
								String str = {};
								switch(anchor.type) {
									case ProfType_Default: {
										color = colors.work;
										str = "work";
									}break;
									case ProfType_Sleep: {
										color = colors.sleep;
										str = "sleep";
									}break;
									case ProfType_Worker: {
										color = colors.job;
										str = "job";
									}break;
									case ProfType_Async: {
										color = colors.async;
										str = "async";
									}break;
								}
								imgui_draw_rect_filled(draw, item_rect, color);
								imgui_draw_rect(draw, item_rect, ColorGreyLight);
								if(rng2_contains(item_rect, os_mouse_pos())) ImGuiBeginToolTip() {
									imgui_text("Label: %s", anchor.label);
									imgui_text("Percent: %f%%", rng2_dim(item_rect).x / rng2_dim(info_rect).x * 100);
									imgui_text("Time: %fms", tsc_to_ms(anchor.tsc_end - anchor.tsc_start));
									imgui_text("Time exclusive: %fms", tsc_to_ms(anchor.tsc_elapsed_excl));
									imgui_text("Type: %s", str);
								}

								// Text
								str = push_strf(scratch, "%s %.3f", anchor.label, tsc_to_ms(anchor.tsc_end - anchor.tsc_start));
								v2 text_size = imgui_calc_text_size(str);
								if(rng2_dim(item_rect).x < 30.1 || cam.zoom2.y < 0.3) {
									continue;
								}
								v2 text_pos = {};
								if(text_size.x > rng2_dim(item_rect).x) {
									text_pos.x = item_rect.min.x;
									text_pos.y = rng1_align_center(rng2_rng_y(item_rect), text_size.y);
								} else {
									text_pos = rng2_align_dim_at_center(item_rect, text_size).min;
								}
								ImGuiPushClipRect2(item_rect);
								imgui_draw_text(draw, l.time_bar_text_size, text_pos, ColorWhite, str);
							}
						}
					};

					///////////////////////////////////
					// Root
					var active_tab = prof_win.active_tab;
					ImGuiBeginTabItem("root", active_tab == ProfileTabActive_Root ? ImGuiTabItemFlags_SetSelected : 0) {
						if(ImGui::IsWindowHovered()) {
							camera_zoom_and_pan(prof_win.root_cam, info_rect.min);
						}
						draw_thread_names(info_rect, prof_win.root_cam);
						Slice<ProfAnchor> slices[ArrayCount(prof.prof_threads)] = {};
						u32 idx = (current_frame-1) % ArrayCount(prof.frames_times);
						local_persist i32 frequency = 1;
						ImGui::SetCursorScreenPos(rng2_subrng_x01(top_bar_rect, {0.5,1}).min);
						ImGui::PushItemWidth((50));
						ImGui::DragInt("show freq", &frequency, 0.1f, 1, ProfRecordHistoryNum); ImGui::SameLine();
						ImGui::DragInt("fps max", (i32*)&g.fps, 1.0f, 10, 2000); ImGui::SameLine();
						ImGui::SliderFloat("scale", &st->scale, 0.01, 1);
						ImGui::PopItemWidth();
						idx /= ProfRecordHistoryNum/frequency;

						// Little bars
						Rng2 race_bar_rect = rng2_cut_top(&info_rect, l.bar_race_height);
						Loop(i, ArrayCount(prof.frames_times)) {
							ProfFrameTime frame_time = prof.frames_times[i];
							f32 ms = tsc_to_ms(frame_time.tsc_end - frame_time.tsc_start);
							f32 width = rng2_width(race_bar_rect) / ArrayCount(prof.frames_times);
							v2 max = race_bar_rect.min + v2(width * i + width, l.bar_race_height);
							v2 s = {width, ms};
							var rect = rng2_make(max - s, s);
							if(rng2_contains(rect, os_mouse_pos())) {
								if(os_mouse_is_button_pressed(MouseButton_Left)) {
									prof_win.frames_cam.pos = v2(rng2_width(race_bar_rect) * i, 0);
									prof_win.frames_cam.zoom2 = v2(1);
									prof_win.frames_cam.zoom = 1;
								}
							}
							Color color = {};
							if(i == current_frame % ArrayCount(prof.frames_times)) {
								color = colors.current_frame;
							} else {
								color = colors.frame_ok;
								if(rng1_contains({17, 21}, ms)) {
									color = colors.frame_warn;
								} else if(ms >= 21) {
									color = colors.frame_bad;
								}
							}
							imgui_draw_rect_filled(draw, rect, color);
							Color c = ColorGrey0;
							c.a = 0.3f * 255;
							imgui_draw_rect(draw, rect, c);
						}

						LoopArray(i, slices) slices[i] = slice(prof.prof_threads[i].recorded_anchors[idx]);
						draw_frame_graph(info_rect, slice(slices), prof.frames_times[idx], 0, prof_win.root_cam, true);

						// Gpu flame graph
						{
							local_persist Gfx_ProfileFrame pf;
							if(time_on_interval(0.5)) pf = st->gfx.cur_prof_frame;
							var& zones = pf.zones;
							f64 elapsed = pf.end_ms-pf.start_ms;
							f64 time_start = pf.start_ms;
							f64 time_end = pf.end_ms;
							f32 gpu_offset_height = -100;

							Rng2 item_rect = rng2_make(v2(0), v2(rng2_width(info_rect), l.bar_height));
							item_rect = rng2_shift(item_rect, v2(0, gpu_offset_height - l.bar_height));
							item_rect.min = world_to_screen2(prof_win.root_cam, item_rect.min, info_rect.min);
							item_rect.max = world_to_screen2(prof_win.root_cam, item_rect.max, info_rect.min);
							imgui_draw_rect_filled(draw, item_rect, colors.work);
							imgui_draw_rect(draw, item_rect, ColorGreyLight);

							{
								String str = push_strf(scratch, "gpu frame %.3f", elapsed);
								v2 text_size = imgui_calc_text_size(str);
								v2 text_pos = rng2_align_dim_at_center(item_rect, text_size).min;
								ImGuiPushClipRect2(item_rect);
								imgui_draw_text(draw, l.time_bar_text_size, text_pos, ColorWhite, str);
							}

							for(var zone : zones) {
								f64 width = remap_f64(zone.end - zone.start, elapsed, rng2_width(info_rect));
								f64 start_off = remap_f64(zone.start, time_start, time_end, 0, rng2_width(info_rect));
								Rng2 item_rect = rng2_make(v2(start_off, zone.depth*l.bar_height), v2(width, l.bar_height));

								item_rect = rng2_shift(item_rect, v2(0, gpu_offset_height));

								item_rect.min = world_to_screen2(prof_win.root_cam, item_rect.min, info_rect.min);
								item_rect.max = world_to_screen2(prof_win.root_cam, item_rect.max, info_rect.min);

								imgui_draw_rect_filled(draw, item_rect, colors.work);
								imgui_draw_rect(draw, item_rect, ColorGreyLight);
								
								if(rng2_contains(item_rect, os_mouse_pos())) ImGuiBeginToolTip() {
									imgui_text("vert count: %u, frag count: %u", zone.vert_invocations, zone.frag_invocations);
								}

								// Text
								String str = push_strf(scratch, "%s %.3f", zone.name, zone.end - zone.start);
								v2 text_size = imgui_calc_text_size(str);
								if(rng2_dim(item_rect).x < 30.1 || prof_win.root_cam.zoom2.y < 0.3) {
									continue;
								}
								v2 text_pos = {};
								if(text_size.x > rng2_dim(item_rect).x) {
									text_pos.x = item_rect.min.x;
									text_pos.y = rng1_align_center(rng2_rng_y(item_rect), text_size.y);
								} else {
									text_pos = rng2_align_dim_at_center(item_rect, text_size).min;
								}
								ImGuiPushClipRect2(item_rect);
								imgui_draw_text(draw, l.time_bar_text_size, text_pos, ColorWhite, str);
							}
						}
					}tab_mouse_click_handle(ProfileTabActive_Root);
					ImGuiBeginTabItem("frames", active_tab == ProfileTabActive_Frames ? ImGuiTabItemFlags_SetSelected : 0) {
						if(ImGui::IsWindowHovered()) {
							camera_zoom_and_pan(prof_win.frames_cam, info_rect.min);
						}
						// Little bars
						Rng2 race_bar_rect = rng2_cut_top(&info_rect, l.bar_race_height);
						Loop(i, ArrayCount(prof.frames_times)) {
							ProfFrameTime frame_time = prof.frames_times[i];
							f32 ms = tsc_to_ms(frame_time.tsc_end - frame_time.tsc_start);
							f32 width = rng2_width(race_bar_rect) / ArrayCount(prof.frames_times);
							v2 max = race_bar_rect.min + v2(width * i + width, l.bar_race_height);
							v2 s = {width, ms};
							var rect = rng2_make(max - s, s);
							if(rng2_contains(rect, os_mouse_pos())) {
								if(os_mouse_is_button_pressed(MouseButton_Left)) {
									prof_win.frames_cam.pos = v2(rng2_width(info_rect) * i, 0);
									prof_win.frames_cam.zoom2 = v2(1);
									prof_win.frames_cam.zoom = 1;
								}
							}
							Color color = {};
							if(i == current_frame % ArrayCount(prof.frames_times)) {
								color = colors.current_frame;
							} else {
								color = colors.frame_ok;
								if(rng1_contains({17, 21}, ms)) {
									color = colors.frame_warn;
								} else if(ms >= 21) {
									color = colors.frame_bad;
								}
							}
							imgui_draw_rect_filled(draw, rect, color);
							var c = ColorGrey0;
							c.a = 0.3f * 255;
							imgui_draw_rect(draw, rect, c.a);
						}
						draw_thread_names(info_rect, prof_win.frames_cam);
						// Draw graph per thread
						LoopArray(j, prof.frames_times) {
							Slice<ProfAnchor> slices[ArrayCount(prof.prof_threads)] = {};
							LoopArray(i, prof.prof_threads) slices[i] = slice(prof.prof_threads[i].recorded_anchors[j]);
							draw_frame_graph(info_rect, slice(slices), prof.frames_times[j], j * rng2_width(info_rect), prof_win.frames_cam);
						}
					}tab_mouse_click_handle(ProfileTabActive_Frames);

					///////////////////////////////////
					// Time
					ImGuiBeginTabItem("time", active_tab == ProfileTabActive_Time ? ImGuiTabItemFlags_SetSelected : 0) {
						var sort_entries = push_slice(scratch, SortEntry64, prev_frame.anchors.count);
						Loop(i, sort_entries.count) {
							var& entry = sort_entries[i];
							entry.idx = i;
							entry.sort_key = prev_frame.anchors[i].tsc_elapsed_excl;
						}
						sort_radix64_msd(sort_entries);
						Loop(i, prev_frame.anchors.count) {
							ProfAnchor anchor = prev_frame.anchors[sort_entries[i].idx];
							f32 width = remap_f64(anchor.tsc_elapsed_excl, prev_frame_tsc_elapsed, rng2_width(info_rect));
							f32 height = 30;
							var rect = rng2_make(info_rect.min+v2(0,height*i), v2(width,height));
							imgui_draw_rect_filled(draw, rect, ColorGreyDark);
							imgui_draw_rect(draw, rect, ColorGreyLight);
							v2 text_size = imgui_calc_text_size(anchor.label);
							v2 pos = {
								rect.min.x,
								rng1_align_center(rng2_rng_y(rect), text_size.y),
							};
							imgui_draw_text(draw, pos, ColorWhite, anchor.label);
							String ms_str = push_strf(scratch, "%.3fms", tsc_to_ms(anchor.tsc_elapsed_excl));
							pos = {
								rng2_width(info_rect) * 0.92f,
								info_rect.min.y + height * i,
							},
							imgui_draw_text(draw, pos, ColorWhite, ms_str);
						}
					}tab_mouse_click_handle(ProfileTabActive_Time);

					///////////////////////////////////
					// Launch
					ImGuiBeginTabItem("launch", active_tab == ProfileTabActive_LaunchTime ? ImGuiTabItemFlags_SetSelected : 0) {
						if(ImGui::IsWindowHovered()) {
							camera_zoom_and_pan(prof_win.launch_cam, info_rect.min);
						}
						draw_thread_names(info_rect, prof_win.launch_cam);
						Slice<ProfAnchor> slices[ArrayCount(prof.prof_threads)] = {};
						LoopArray(i, prof.prof_threads) slices[i] = slice(prof.prof_threads[i].launch_anchors);
						draw_frame_graph(info_rect, slice(slices), prof.launch_time, 0, prof_win.launch_cam, true);
					}tab_mouse_click_handle(ProfileTabActive_LaunchTime);

					///////////////////////////////////
					// Memory
					ImGuiBeginTabItem("memory", active_tab == ProfileTabActive_Memory ? ImGuiTabItemFlags_SetSelected : 0) {
						if(ImGui::IsWindowHovered()) {
							camera_zoom_and_pan(prof_win.mem_cam, info_rect.min, true);
						}
						var mem_info_top = rng2_cut_top(&info_rect, 20);
						AllocatorInfoList infos = mem_track_info();
						var sort_entries = push_slice(scratch, SortEntry64, infos.count);
						u64 mem_usage = 0;
						u32 idx = 0;
						LoopNode(it, infos.first) {
							var& entry = sort_entries[idx++];
							entry.p = it;
							entry.sort_key = it->pos;
							mem_usage += it->cap;
						}
						sort_radix64_msd(sort_entries);
						u64 max_pos = ((AllocatorInfo*)sort_entries[0].p)->pos;
						f32 mem_levels[] = {KB(1), KB(10), KB(100), MB(1), MB(10), MB(100), GB(1)};
						DArray<AllocatorInfo*> mem_arrays[ArrayCount(mem_levels)] = {};
						for(var& arr : mem_arrays) arr = array_make<AllocatorInfo*>(scratch);
						u32 max_mem_level = 0;
						LoopArray(i, mem_levels) {
							if(max_pos < mem_levels[i]) { 
								max_mem_level = i;
								break;
							}
						}
						Loop(i, sort_entries.count) {
							var info = (AllocatorInfo*)sort_entries[i].p;
							u32 max_level = 0;
							LoopArray(j, mem_levels) {
								if(info->pos < mem_levels[j]) {
									max_level = j;
									break;
								}
							}
							array_push(mem_arrays[max_level], info);
						}
						f32 height_off = 0;
						LoopReverse(mem_idx, max_mem_level+1) {
							if(mem_arrays[mem_idx].count) {
								v2 rect_size = {50, 25};
								// var pos = rng1_align_center(rng2_rng_x(info_rect), rect_size.x);
								var pos = rng1_align_center(Rng1(0, rng2_width(info_rect)), rect_size.x);
								// var rect = rng2_make(v2(pos, info_rect.min.y) + v2(0,height_off), rect_size);
								var rect = rng2_make(v2(pos, 0) + v2(0,height_off), rect_size);
								rect = world_to_screen(prof_win.mem_cam, rect, info_rect.min);

								// rect = world_to_screen(prof_win.mem_cam, rect, info_rect.min);

								imgui_draw_rect_filled(draw, rect, ColorBlueUi);
								imgui_draw_rect(draw, rect, ColorGreyLight);
								MemFormatSize mem_fmt = mem_format_size(mem_levels[mem_idx]);
								String str = push_strf(scratch, "%.0f%s", mem_fmt.size, mem_fmt.format);
								v2 text_size = imgui_calc_text_size(str);
								Rng2 text_rect = rng2_align_dim_at_center(rect, text_size);
								imgui_draw_text(draw, text_rect.min, ColorWhite, str);
								height_off += 40;
							}
							Loop(parent_idx, mem_arrays[mem_idx].count) {
								var info = mem_arrays[mem_idx][parent_idx];
								f32 excl_width = remap(info->pos - info->children_size, mem_levels[mem_idx], rng2_width(info_rect));
								f32 incl_width = remap(info->pos, mem_levels[mem_idx], rng2_width(info_rect));
								f32 cap_width = remap(info->cap, mem_levels[mem_idx], rng2_width(info_rect));
								f32 height = 30;
								// var r = rng2_make(info_rect.min + v2(0, height_off), {rng2_width(info_rect), height});
								var r = rng2_make(v2(0, height_off), {rng2_width(info_rect), height});
								var excl_rect = rng2_cut_left(&r, excl_width);
								var incl_rect = rng2_cut_left(&r, incl_width-excl_width);
								var cap_rect =  rng2_cut_left(&r, cap_width-incl_width);
								excl_rect = world_to_screen(prof_win.mem_cam, excl_rect, info_rect.min);
								incl_rect = world_to_screen(prof_win.mem_cam, incl_rect, info_rect.min);
								cap_rect = world_to_screen(prof_win.mem_cam, cap_rect, info_rect.min);
								height_off += 30;
								imgui_draw_rect_filled(draw, excl_rect, ColorGreenUi);
								imgui_draw_rect(draw, excl_rect, ColorGreyLight);
								imgui_draw_rect_filled(draw, incl_rect, ColorBlueUi);
								imgui_draw_rect(draw, incl_rect, ColorGreyLight);
								imgui_draw_rect_filled(draw, cap_rect, ColorRedUi);
								imgui_draw_rect(draw, cap_rect, ColorGreyLight);
								MemFormatSize incl_mem = mem_format_size(info->pos);
								MemFormatSize excl_mem = mem_format_size(info->pos - info->children_size);
								MemFormatSize cmt_mem = mem_format_size(info->cap);
								if(rng2_contains(rng2_union(excl_rect, cap_rect), os_mouse_pos())) ImGuiBeginToolTip() {
									imgui_text(push_strf(scratch, "inclusive: %.2f%s", incl_mem.size, incl_mem.format));
									imgui_text(push_strf(scratch, "exclusive: %.2f%s", excl_mem.size, excl_mem.format));
									imgui_text(push_strf(scratch, "file: %s, line: %i", String(info->file), info->line));
								}
								String name_str = push_strf(scratch, "%s", String(info->name));
								String mem_str = push_strf(scratch, "%.2f%s pos, %.2f%s cap", incl_mem.size, incl_mem.format, cmt_mem.size, cmt_mem.format);
								imgui_draw_text(draw, excl_rect.min, ColorWhite, name_str);
								imgui_draw_text(draw, excl_rect.min + v2(200, 0), ColorWhite, mem_str);
								var sort_entries = push_slice(scratch, SortEntry64, info->child_count);
								u32 idx = 0;
								LoopNode(it, info->first) {
									var& entry = sort_entries[idx++];
									entry.p = it;
									entry.sort_key = it->pos;
								}
								sort_radix64_msd(sort_entries);
								Loop(child_idx, sort_entries.count) {
									var info = (AllocatorInfo*)sort_entries[child_idx].p;
									f32 c_width = remap(info->pos, mem_levels[mem_idx], rng2_width(info_rect));
									f32 c_cap_width = remap(info->cap, mem_levels[mem_idx], rng2_width(info_rect));
									f32 height = 30;
									var r = rng2_make(v2(0, height_off), {rng2_width(info_rect), height});
									var rect = rng2_cut_left(&r, c_width);
									var cap_rect =  rng2_cut_left(&r, c_cap_width - c_width);
									r = world_to_screen(prof_win.mem_cam, r, info_rect.min);
									rect = world_to_screen(prof_win.mem_cam, rect, info_rect.min);
									cap_rect = world_to_screen(prof_win.mem_cam, cap_rect, info_rect.min);
									height_off += 30;
									imgui_draw_rect_filled(draw, rect, ColorGreyDark);
									imgui_draw_rect(draw, rect, ColorGreyLight);
									imgui_draw_rect_filled(draw, cap_rect, ColorRedUi);
									imgui_draw_rect(draw, cap_rect, ColorGreyLight);
									MemFormatSize mem = mem_format_size(info->pos);
									MemFormatSize cap_mem = mem_format_size(info->cap);
									if(rng2_contains(rng2_union(rect, cap_rect), os_mouse_pos())) ImGuiBeginToolTip() {
										imgui_text(push_strf(scratch, "pos: %.2f%s", mem.size, mem.format));
										imgui_text(push_strf(scratch, "file: %s, line: %i", String(info->file), info->line));
									}
									String name_str = push_strf(scratch, "%s", String(info->name));
									String mem_str = push_strf(scratch, "%.2f%s pos, %.2f%s cap", mem.size, mem.format, cap_mem.size, cap_mem.format);
									imgui_draw_text(draw, rect.min, ColorWhite, name_str);
									imgui_draw_text(draw, rect.min + v2(200, 0), ColorWhite, mem_str);
								}
								if(sort_entries.count) height_off += 10;
								height_off+=2;
							}
						}

						// overall usage
						ImGui::SetCursorScreenPos(mem_info_top.min);
						MemFormatSize mem_fmt = mem_format_size(mem_usage);
						MemFormatSize os_commited_fmt = mem_format_size(os_commited_size());
						MemFormatSize os_address_reserved_fmt = mem_format_size(os_reserved_size());
						imgui_text("os commited: %.2f%s, address reserved %.2f%s, mem usage: %.2f%s",
							os_commited_fmt.size, os_commited_fmt.format,
							os_address_reserved_fmt.size, os_address_reserved_fmt.format,
							mem_fmt.size, mem_fmt.format);

						// Gpu memory
						{
							var& gfx = st->gfx;
							u64 cpu_cap = st->gfx.cpu_mem.cap;
							u64 gpu_cap = st->gfx.gpu_mem.cap;
							u64 images_size = st->gfx.images_mem_usage;
							u64 target_images_size = st->gfx.target_images_mem_usage;
							u64 max = Max(Max3(cpu_cap, gpu_cap, images_size), target_images_size);
							struct Info {
								u64 pos;
								u64 cap;
								String name;
							} infos[] = {
								{gfx.cpu_buf.pos, gfx.cpu_buf.cap, "cpu"},
								{gfx.gpu_buf.pos, gfx.gpu_buf.cap, "gpu"},
								{gfx.stage_buffer.pos, gfx.stage_buffer.size, "stage"},
								{gfx.images_mem_usage, gfx.images_mem_usage, "images"},
								{gfx.target_images_mem_usage, gfx.target_images_mem_usage, "targets images"},
							};

							f32 height_off = -150;
							LoopArray(i, infos) {
								f32 height = 30;
								f32 pos_width = remap(infos[i].pos, max, rng2_width(info_rect));
								f32 cap_width = remap(infos[i].cap, max, rng2_width(info_rect));
	
								var r = rng2_make(v2(), v2(rng2_width(info_rect), height));
								var pos_r = rng2_cut_left(&r, pos_width);
								var cap_r = rng2_cut_left(&r, cap_width - pos_width);
	
								pos_r = rng2_shift(pos_r, v2(0, height_off));
								cap_r = rng2_shift(cap_r, v2(0, height_off));
								height_off += height;
	
								pos_r = world_to_screen(prof_win.mem_cam, pos_r, info_rect.min);
								cap_r = world_to_screen(prof_win.mem_cam, cap_r, info_rect.min);
	
								imgui_draw_rect_filled(draw, pos_r, ColorGreenUi);
								imgui_draw_rect(draw, pos_r, ColorGreyLight);
								imgui_draw_rect_filled(draw, cap_r, ColorRedUi);
								imgui_draw_rect(draw, cap_r, ColorGreyLight);
	
								var pos_fmt = mem_format_size(infos[i].pos);
								var cap_fmt = mem_format_size(infos[i].cap);
								imgui_draw_text(draw, pos_r.min, ColorWhite, push_strf(scratch, "%s pos: %.2f%s cap: %.2f%s", infos[i].name, pos_fmt.size, pos_fmt.format, cap_fmt.size, cap_fmt.format));
							}
						}
					}tab_mouse_click_handle(ProfileTabActive_Memory);
				}
			}
		}
	}
#endif
}

MeshDesc load_obj(Allocator arena, String name) {
	Scratch scratch(arena);
	var positions = array_make<v3>(scratch);
	var normals = array_make<v3>(scratch);
	var uvs = array_make<v2>(scratch);
	var indexes = array_make<v3u>(scratch);

	// String str = R"(
	//   v  -4.4   14 4.1
	//   v   1.4  -14 4.1
	//   vt -4.4   14
	//   vt  1.4  -14
	//   vn -4.4   14 4.1
	//   vn  1.4  -14 4.1
	//   f 10/4/1 1/2/3 11/22/33
	//   f 20/2/2 91/42/13 141/22/33
	// )";

	struct WordLexer {
		String str;
		u32 cursor;
	};
	var word_lexer_next = [](WordLexer& l) {
		while(l.cursor < l.str.size && char_is_ws(l.str.str[l.cursor])) {
			l.cursor++;
		}
		u32 word_base = l.cursor;
		while(l.cursor < l.str.size && !char_is_ws(l.str.str[l.cursor])) {
			l.cursor++;
		}
		String res = {l.str.str + word_base, l.cursor - word_base};
		return res;
	};
	WordLexer l = {os_file_path_read_all_str(scratch, name)};
	for(String word = {}; (word = word_lexer_next(l)).size;) {
		// Info("%s", word);
		if(word.str[0] == 'v' && word.str[1] == ' ') {
			v3 pos = {
			 f32_from_str(word_lexer_next(l)),
			 f32_from_str(word_lexer_next(l)),
			 f32_from_str(word_lexer_next(l)),
			};
			array_push(positions, pos);
			// Info("%f %f %f", pos.x, pos.y, pos.z);
		} else if(word.str[0] == 'v' && word.str[1] == 'n') {
			v3 norm = {
				f32_from_str(word_lexer_next(l)),
				f32_from_str(word_lexer_next(l)),
				f32_from_str(word_lexer_next(l)),
			};
			array_push(normals, norm);
			// Info("%f %f %f", norm.x, norm.y, norm.z);
		} else if(word.str[0] == 'v' && word.str[1] == 't') {
			v2 uv = {
				f32_from_str(word_lexer_next(l)),
				f32_from_str(word_lexer_next(l)),
			};
			array_push(uvs, uv);
			// Info("%f %f", uv.x, uv.y);
		} else if(word.str[0] == 'f' && word.str[1] == ' ') {
			Loop(i, 3) {
				v3u raw = {};
				String word = word_lexer_next(l);
				u32 cursor = 0;
				String r = {};
				Loop(i, 3) {
					u32 num_base = cursor;
					while(cursor < word.size && char_is_digit(word.str[cursor])) {
						cursor++;
					}
					r = {word.str+num_base, cursor - num_base};
					raw.v[i] = u64_from_str(r) - 1;
					cursor++;
				}
				// Info("%u %u %u", raw.x, raw.y, raw.z);
				v3u v = {raw.x, raw.z, raw.y};
				array_push(indexes, v);
			}
		}
	}

	var vertices = array_make<Vertex>(arena);
	var final_indices = array_make<u16>(arena);
	var map = map_make<u16>(scratch);
	Loop(i, indexes.count) {
		v3u idx = indexes[i];
		u64 h = hash_bytes(&idx, sizeof(v3u));
		u16 value = 0;
		if(map_get(map, h, &value)) {
			array_push(final_indices, value);
		} else {
			Vertex v = {
			 positions[idx.x],
			 normals[idx.y],
			 uvs[idx.z],
			};
			u16 new_index = vertices.count;
			array_push(vertices, v);
			array_push(final_indices, new_index);
			map_set(map, h, new_index);
		} 
	}
	MeshDesc mesh = {
		.vertices = slice(vertices),
		.indices = slice(final_indices),
	};
	return mesh;
}

#define Gltf_i8  5120
#define Gltf_u8  5121
#define Gltf_i16 5122
#define Gltf_u16 5123
#define Gltf_u32 5125
#define Gltf_f32 5126

GltfInfo load_gltf(Allocator arena, String name, b32 is_glb) {
	Scratch scratch(arena);

	String path = push_strf(scratch, "%s/%s", st->models_dir, name);

	String json = {};
	Slice<u8> cursor = {};
	if(is_glb) {
		struct FileHeader {
			u32 magic;
			u32 version;
			u32 size;
		};
		struct ChunkHeader {
			u32 chunk_length;
			u32 chunk_type;
		};
		cursor = os_file_path_read_all(scratch, path);

		FileHeader* header = (FileHeader*)cursor.data;
		NoOp(header);
		Assert(str_match(String((u8*)&header->magic, 4), "glTF"));
		cursor = slice_skip(cursor, sizeof(FileHeader));

		ChunkHeader* json_chunk = (ChunkHeader*)cursor.data;
		Assert(str_match(String((u8*)&json_chunk->chunk_type, 4), "JSON"));
		cursor = slice_skip(cursor, sizeof(ChunkHeader));
		json = str_make(slice_prefix(cursor, json_chunk->chunk_length));
		cursor = slice_skip(cursor, json_chunk->chunk_length);

		ChunkHeader* bin_chunk = (ChunkHeader*)cursor.data;
		NoOp(bin_chunk);
		Assert(str_match(String((u8*)&bin_chunk->chunk_type, 3), "BIN"));
		cursor = slice_skip(cursor, sizeof(ChunkHeader));
	} else {
		json = os_file_path_read_all_str(scratch, path);
	}

	String file_path = str_chop_last_dot(path);
	String file_path_bin = push_strf(scratch, "%s.bin", file_path);
	Slice data = is_glb ? cursor : os_file_path_read_all(scratch, file_path_bin);

	u32 vert_count = 0;
	u32 indices_count = 0;
	Slice<v3> pos = {};
	Slice<v3> norm = {};
	Slice<v2> texcoord = {};
	Slice<u16> indices_u16 = {};
	Slice<v4u8> joints = {};
	Slice<v4> weights = {};
	Slice<m4x4> inv_matrices = {};
	Slice<Bone> bones = {};
	Slice<Channel> channels = {};
	Slice<Sampler> samplers = {};

	JsParser p = js_parse_make(scratch, json);
	JsObj root = js_parse(p).obj;
	Slice<JsVal*> nodes; AssertOk(js_array(root, "nodes", &nodes));
	Slice<JsVal*> accessors; AssertOk(js_array(root, "accessors", &accessors));
	Slice<JsVal*> buffer_views; AssertOk(js_array(root, "bufferViews", &buffer_views));

	///////////////////////////////////
	// Mesh
	{
		Slice<JsVal*> meshes; AssertOk(js_array(root, "meshes", &meshes));
		JsObj first_mesh = meshes[0]->obj;
		Slice<JsVal*> primitives; AssertOk(js_array(first_mesh, "primitives", &primitives));
		JsObj first_primitive = primitives[0]->obj;
		JsObj attributes; AssertOk(js_obj(first_primitive, "attributes", &attributes));
		{
			u32 pos_attribute; AssertOk(js_u32(attributes, "POSITION", &pos_attribute));
			JsObj pos_accessor = accessors[pos_attribute]->obj;
			u32 buffer_view; AssertOk(js_u32(pos_accessor, "bufferView", &buffer_view));
			AssertOk(js_u32(pos_accessor, "count", &vert_count));
			JsObj pos_buffer_view = buffer_views[buffer_view]->obj;
			u64 size; AssertOk(js_u64(pos_buffer_view, "byteLength", &size));
			u64 off; AssertOk(js_u64(pos_buffer_view, "byteOffset", &off));
			pos = slice_reinterpret<v3>(slice_n(data, off, size));
		}
		{
			u32 norm_attribute; AssertOk(js_u32(attributes, "NORMAL", &norm_attribute));
			JsObj norm_accessor = accessors[norm_attribute]->obj;
			u32 buffer_view; AssertOk(js_u32(norm_accessor, "bufferView", &buffer_view));
			JsObj norm_buffer_view = buffer_views[buffer_view]->obj;
			u64 size; js_u64(norm_buffer_view, "byteLength", &size);
			u64 off; js_u64(norm_buffer_view, "byteOffset", &off);
			norm = slice_reinterpret<v3>(slice_n(data, off, size));
		}
		{
			u32 texcoord_attribute; AssertOk(js_u32(attributes, "TEXCOORD_0", &texcoord_attribute));
			JsObj texcoord_accessor = accessors[texcoord_attribute]->obj;
			u32 buffer_view; AssertOk(js_u32(texcoord_accessor, "bufferView", &buffer_view));
			JsObj texcoord_buffer_view = buffer_views[buffer_view]->obj;
			u64 size; js_u64(texcoord_buffer_view, "byteLength", &size);
			u64 off; js_u64(texcoord_buffer_view, "byteOffset", &off);
			texcoord = slice_reinterpret<v2>(slice_n(data, off, size));
		}
		{
			u32 indices_attribute; AssertOk(js_u32(first_primitive, "indices", &indices_attribute));
			JsObj indices_accessor = accessors[indices_attribute]->obj;
			u32 buffer_view; AssertOk(js_u32(indices_accessor, "bufferView", &buffer_view));
			AssertOk(js_u32(indices_accessor, "count", &indices_count));
			JsObj indices_buffer_view = buffer_views[buffer_view]->obj;
			u64 size; Assert(js_u64(indices_buffer_view, "byteLength", &size));
			u64 off; Assert(js_u64(indices_buffer_view, "byteOffset", &off));
			indices_u16 = slice_reinterpret<u16>(slice_n(data, off, size));
		}
		{
			u32 joints_attribute = 0;
			if(js_u32(attributes, "JOINTS_0", &joints_attribute)) {
				JsObj joints_accessor = accessors[joints_attribute]->obj;
				u32 buffer_view; AssertOk(js_u32(joints_accessor, "bufferView", &buffer_view));
				JsObj joints_buffer_view = buffer_views[buffer_view]->obj;
				u64 size; AssertOk(js_u64(joints_buffer_view, "byteLength", &size));
				u64 offset; AssertOk(js_u64(joints_buffer_view, "byteOffset", &offset));
				joints = slice_reinterpret<v4u8>(slice_n(data, offset, size));
			}
		}
		{
			u32 weights_attribute = 0;
			if(js_u32(attributes, "WEIGHTS_0", &weights_attribute)) {
				JsObj weights_accessor = accessors[weights_attribute]->obj;
				u32 buffer_view; AssertOk(js_u32(weights_accessor, "bufferView", &buffer_view));
				JsObj weights_buffer_view = buffer_views[buffer_view]->obj;
				u64 size; AssertOk(js_u64(weights_buffer_view, "byteLength", &size));
				u64 off; AssertOk(js_u64(weights_buffer_view, "byteOffset", &off));
				weights = slice_reinterpret<v4>(slice_n(data, off, size));
			}
		}
	}

	///////////////////////////////////
	// Skin
	{
		Slice<JsVal*> skins = {};
		if(js_array(root, "skins", &skins)) {
			JsObj first_skin = skins[0]->obj;
			u32 inv_matrices_attribute; AssertOk(js_u32(first_skin, "inverseBindMatrices", &inv_matrices_attribute));
			JsObj inv_matrices_accessor = accessors[inv_matrices_attribute]->obj;
			u32 buffer_view; AssertOk(js_u32(inv_matrices_accessor, "bufferView", &buffer_view));
			JsObj inv_matrices_buffer_view = buffer_views[buffer_view]->obj;
			u64 size; AssertOk(js_u64(inv_matrices_buffer_view, "byteLength", &size));
			u64 off; AssertOk(js_u64(inv_matrices_buffer_view, "byteOffset", &off));
			var matrices = slice_reinterpret<m4x4>(slice_n(data, off, size));
			inv_matrices = push_slice(arena, m4x4, matrices.count);
			Slice<JsVal*> joints_indices; AssertOk(js_array(first_skin, "joints", &joints_indices));
			bones = push_slice(arena, Bone, joints_indices.count);
			u32 bone_count = 0;
			u32 node_to_joint[255] = {};
			u32 node_to_bone[255] = {};
			Loop(joint_idx, joints_indices.count) {
				u32 node_idx = joints_indices[joint_idx]->number;
				node_to_joint[node_idx] = joint_idx;
			}
			var dfs = [&](var dfs, u32 node_idx, u32 parent_idx) -> void {
				JsObj node = nodes[node_idx]->obj;
				u32 bone_idx = bone_count++;
				node_to_bone[node_idx] = bone_idx;
				inv_matrices[bone_idx] = matrices[node_to_joint[node_idx]];
				Transform trans = {
					.pos = v3(),
					.rot = quat_identity(),
					.scale = v3(1),
				};
				js_v3(node, "translation", &trans.pos);
				js_v4(node, "rotation", &trans.rot);
				js_v3(node, "scale", &trans.scale);
				bones[bone_idx].local = trans;
				bones[bone_idx].parent = parent_idx;
				Slice<JsVal*> children = {}; js_array(node, "children", &children);
				Loop(i, children.count) {
					dfs(dfs, children[i]->number, bone_idx);
				}
			};
			dfs(dfs, joints_indices[0]->number, 0);

			// Animation
			Slice<JsVal*> animations = {};
			if(js_array(root, "animations", &animations)) {
				var first_animation = animations[0]->obj;
				Slice<JsVal*> js_channels; AssertOk(js_array(first_animation, "channels", &js_channels));
				channels = push_slice(arena, Channel, js_channels.count);
				Loop(i, js_channels.count) {
					var channel = js_channels[i]->obj;
					u32 sampler; AssertOk(js_u32(channel, "sampler", &sampler));
					JsObj target; AssertOk(js_obj(channel, "target", &target));
					u32 node; AssertOk(js_u32(target, "node", &node));
					String path; AssertOk(js_str(target, "path", &path));
					channels[i].bone = node_to_bone[node];
					channels[i].sampler = sampler;
					AnimPath anim_path = {};
					if(str_match(path, "translation")) {
						anim_path = AnimPath_Pos;
					} else if(str_match(path, "rotation")) {
						anim_path = AnimPath_Rot;
					} else if(str_match(path, "scale")) {
						anim_path = AnimPath_Scale;
					} else InvalidPath;
					channels[i].path = anim_path;
				}
				Slice<JsVal*> js_samplers; AssertOk(js_array(first_animation, "samplers", &js_samplers));
				samplers = push_slice(arena, Sampler, js_samplers.count);
				Loop(i, js_samplers.count) {
					var sampler = js_samplers[i]->obj;
					u32 input; AssertOk(js_u32(sampler, "input", &input));
					JsObj input_accessor = accessors[input]->obj;
					u32 input_buffer_view; AssertOk(js_u32(input_accessor, "bufferView", &input_buffer_view));
					JsObj js_input_buffer_view = buffer_views[input_buffer_view]->obj;
					u64 input_size; AssertOk(js_u64(js_input_buffer_view, "byteLength", &input_size));
					u64 input_off; AssertOk(js_u64(js_input_buffer_view, "byteOffset", &input_off));
					var timestamps = slice_reinterpret<f32>(slice_n(data, input_off, input_size));
					samplers[i].times = slice_clone(arena, timestamps).data;
					samplers[i].count = timestamps.count;

					u32 output; AssertOk(js_u32(sampler, "output", &output));
					JsObj output_accessor = accessors[output]->obj;
					u32 output_buffer_view; AssertOk(js_u32(output_accessor, "bufferView", &output_buffer_view));
					String output_type; AssertOk(js_str(output_accessor, "type", &output_type));
					JsObj js_output_buffer_view = buffer_views[output_buffer_view]->obj;
					u64 output_size; AssertOk(js_u64(js_output_buffer_view, "byteLength", &output_size));
					u64 output_off; AssertOk(js_u64(js_output_buffer_view, "byteOffset", &output_off));
					if(str_match(output_type, "VEC3")) {
						var values = slice_reinterpret<v3>(slice_n(data, output_off, output_size));
						samplers[i].values = slice_clone(arena, values).data;
					} else if(str_match(output_type, "VEC4")) {
						var values = slice_reinterpret<v4>(slice_n(data, output_off, output_size));
						samplers[i].values = slice_clone(arena, values).data;
					} else InvalidPath;
				}
			}
		}
	}

	Slice vertices = push_slice(arena, Vertex, vert_count);
	Slice indices = push_slice(arena, u16, indices_count);
	Loop(i, indices.count) {
		indices[i] = indices_u16[i];
	}
	Loop(i, vertices.count) {
		vertices[i] = {
			.pos = pos[i],
			.norm = norm[i],
			.uv = texcoord[i],
		};
	}
	if(joints.count) {
		Loop(i, vertices.count) {
			// vertices[i].joints = joints[i];
			vertices[i].joints.v[0] = joints[i].v[0];
			vertices[i].joints.v[1] = joints[i].v[1];
			vertices[i].joints.v[2] = joints[i].v[2];
			vertices[i].joints.v[3] = joints[i].v[3];
			// vertices[i].weights = weights[i];
		}
	}
	GltfInfo res = {
		.mesh = {
			.vertices = vertices,
			.indices = indices,
		},
		.bones = bones,
		.inv_matrices = inv_matrices,
		.animation = {
			.channels = channels,
			.samplers = samplers,
		},
	};
	res.animation.duration = animation_duration(res.animation);
	return res;
}

v3 sample_v3(Sampler s, f32 time) {
	u32 found = -1;
	LoopOff(i, 1, s.count) {
		if(time < s.times[i]) {
			found = i-1;
			break;
		}
	}
	if(found == -1)
		return ((v3*)s.values)[s.count - 1];
	f32 t0 = s.times[found];
	f32 t1 = s.times[found + 1];
	f32 t = unlerp(t0, time, t1);
	v3 a = ((v3*)s.values)[found];
	v3 b = ((v3*)s.values)[found + 1];
	return v3_lerp(a, t, b);
}
v4 sample_quat(Sampler s, f32 time) {
	u32 found = -1;
	LoopOff(i, 1, s.count) {
		if(time < s.times[i]) {
			found = i-1;
			break;
		}
	}
	if(found == -1)
		return ((v4*)s.values)[s.count - 1];
	f32 t0 = s.times[found];
	f32 t1 = s.times[found + 1];
	f32 t = unlerp(t0, time, t1);
	v4 a = ((v4*)s.values)[found];
	v4 b = ((v4*)s.values)[found + 1];
	return quat_slerp(a, t, b);
}

void update_animation(Animation anim, Pose pose, f32 time) {
	for(var channel : anim.channels) {
		Sampler sampler = anim.samplers[channel.sampler];
		Transform& bone = pose.bones[channel.bone].local;
		switch(channel.path) {
			case AnimPath_Pos: {
				bone.pos = sample_v3(sampler, time);
			} break;
			case AnimPath_Rot: {
				bone.rot = sample_quat(sampler, time);
			} break;
			case AnimPath_Scale: {
				bone.scale = sample_v3(sampler, time);
			} break;
		}
	}
}

f32 animation_duration(Animation anim) {
	f32 duration = 0;
	for(var sampler : anim.samplers) {
		f32 last_time = sampler.times[sampler.count - 1];
		duration = Max(duration, last_time);
	}
	return duration;
}

JsParser js_parse_make(Allocator arena, String str) {
	JsParser res = {
		.arena = arena,
		.str = str,
	};
	return res;
}
b32 js_is_end(JsParser& p) {
	return p.cursor >= p.str.size;
}
u8 js_peek(JsParser& p) {
	if(js_is_end(p)) return 0;
	return p.str.str[p.cursor];
}
void js_skip_ws(JsParser& p) {
	while(char_is_ws(js_peek(p))) {
		p.cursor++;
	}
}
String js_parse_str(JsParser& p) {
	Assert(js_peek(p) == '\"');
	p.cursor++;
	u32 start = p.cursor;
	while(js_peek(p) != '\"' && !js_is_end(p)) {
		p.cursor++;
	}
	Assert(js_peek(p) == '\"'); 
	String res = str_substr(p.str, Rng1u(start, p.cursor));
	p.cursor++;
	return res;
}
f64 js_parse_number(JsParser& p) {
	u32 start = p.cursor;
	if(js_peek(p) == '-')
		p.cursor++;
	while(char_is_digit(js_peek(p)))
		p.cursor++;
	if(js_peek(p) == '.') {
		p.cursor++;
		while(char_is_digit(js_peek(p)))
			p.cursor++;
	}
	if(js_peek(p) == 'e' || js_peek(p) == 'E') {
		p.cursor++;
		if(js_peek(p) == '+' || js_peek(p) == '-')
			p.cursor++;
		while(char_is_digit(js_peek(p)))
			p.cursor++;
	}
	String str = str_substr(p.str, Rng1u(start, p.cursor));
	f64 res = f64_from_str(str);
	return res;
}
JsVal js_parse(JsParser& p) {
	js_skip_ws(p);
	switch(js_peek(p)) {
		default:   return {.type = JsType_Number, .number = js_parse_number(p)};
		case '\"': return {.type = JsType_Str, .str = js_parse_str(p)};
		case 't': p.cursor += 4; return {.type = JsType_Bool, .boolean = true};
		case 'f': p.cursor += 5; return {.type = JsType_Bool, .boolean = false};
		case 'n': p.cursor += 4; return {.type = JsType_Null};
		case '[': {
			p.cursor++;
			var arr = array_make<JsVal*>(p.arena);
			For {
				js_skip_ws(p);
				if(js_peek(p) == ']') {
					p.cursor++;
					break;
				}
				JsVal* val = push_struct(p.arena, JsVal);
				*val = js_parse(p);
				// switch(val->type) {
				//   case JsType_Null: Info("null"); break;
				//   case JsType_Bool: Info("bool: %u", val->boolean); break;
				//   case JsType_Number: Info("num: %f", val->number); break;
				//   case JsType_Str: Info("str: %s", val->str); break;
				//   case JsType_Array: Info("array"); break;
				//   case JsType_Obj:  Info("obj"); break;
				// }
				array_push(arr, val);
				js_skip_ws(p);
				if(js_peek(p) == ',') {
					p.cursor++;
				}
			}
			JsVal res = {
				.type = JsType_Array,
				.array = slice(arr),
			};
			return res;
		}break;
		case '{': {
			p.cursor++;
			var fields = array_make<JsField>(p.arena);
			For {
				js_skip_ws(p);
				if(js_peek(p) == '}') {
					p.cursor++;
					break;
				}
				String key = js_parse_str(p);
				js_skip_ws(p);
				Assert(js_peek(p) == ':');
				p.cursor++;
				JsVal* val = push_struct(p.arena, JsVal);
				*val = js_parse(p);
				// switch(val->type) {
				//   case JsType_Null: Info("null"); break;
				//   case JsType_Bool: Info("bool: %u", val->boolean); break;
				//   case JsType_Number: Info("num: %f", val->number); break;
				//   case JsType_Str: Info("str: %s", val->str); break;
				//   case JsType_Array: Info("array"); break;
				//   case JsType_Obj:  Info("obj"); break;
				// }
				array_push(fields, {.key = key, .val = val});
				js_skip_ws(p);
				if(js_peek(p) == ',') {
					p.cursor++;
				}
			}
			JsVal res = {
				.type = JsType_Obj,
				.obj = slice(fields),
			};
			return res;
		}break;
	}
}

b32 js_val(JsVal val, String key, JsVal* out) {
	for(var& field : val.obj.fields) {
		if(str_match(field.key, key)) {
			*out = *field.val;
			return true;
		}
	}
	return false;
}
b32 js_bool(JsObj obj, String key, b32* out) {
	JsVal v = {};
	if(js_val({.obj = obj}, key, &v)) {
		*out = v.boolean;
		return true;
	}
	return false;
}
b32 js_f64(JsObj obj, String key, f64* out) {
	JsVal v = {};
	if(js_val({.obj = obj}, key, &v)) {
		*out = v.number;
		return true;
	}
	return false;
}
b32 js_f32(JsObj obj, String key, f32* out) {
	f64 r = 0;
	if(js_f64(obj, key, &r)) {
		*out = r;
		return true;
	}
	return false;
}
b32 js_u64(JsObj obj, String key, u64* out) {
	f64 r = 0;
	if(js_f64(obj, key, &r)) {
		*out = r;
		return true;
	}
	return false;
}
b32 js_u32(JsObj obj, String key, u32* out) {
	f64 r = 0;
	if(js_f64(obj, key, &r)) {
		*out = r;
		return true;
	}
	return false;
}
b32 js_str(JsObj obj, String key, String* out) {
	JsVal v = {};
	if(js_val({.obj = obj}, key, &v)) {
		*out = v.str;
		return true;
	}
	return false;
}
b32 js_array(JsObj obj, String key, Slice<JsVal*>* out) {
	JsVal v = {};
	if(js_val({.obj = obj}, key, &v)) {
		*out = v.array;
		return true;
	}
	return false;
}
b32 js_obj(JsObj obj, String key, JsObj* out) {
	JsVal v = {};
	if(js_val({.obj = obj}, key, &v)) {
		*out = v.obj;
		return true;
	}
	return false;
}
b32 js_v3(JsObj obj, String key, v3* out) {
	JsVal v = {};
	if(js_val({.obj = obj}, key, &v)) {
		v3 res = {};
		Loop(i, 3) {
			res.v[i] = v.array[i]->number;
		}
		*out = res;
		return true;
	}
	return false;
}
b32 js_v4(JsObj obj, String key, v4* out) {
	JsVal v = {};
	if(js_val({.obj = obj}, key, &v)) {
		v4 res = {};
		Loop(i, 4) {
			res.v[i] = v.array[i]->number;
		}
		*out = res;
		return true;
	}
	return false;
}

f32 parse_f32(Parser& p) {
	b32 negative = false;
	if(tok_match(p, TokenType_Minus)) {
		negative = true;
	}
	Token tok = tok_expect(p, TokenType_Number);
	f32 v = f32_from_str(tok.str);
	return negative ? -v : v;
}
f32 parse_u32(Parser& p) {
	b32 negative = false;
	if(tok_match(p, TokenType_Minus)) {
		negative = true;
	}
	Token tok = tok_expect(p, TokenType_Number);
	i32 v = u32_from_str(tok.str);
	return negative ? -v : v;
}
f32 parse_i32(Parser& p) {
	b32 negative = false;
	if(tok_match(p, TokenType_Minus)) {
		negative = true;
	}
	Token tok = tok_expect(p, TokenType_Number);
	i32 v = i32_from_str(tok.str);
	return negative ? -v : v;
}
v3 parse_v3(Parser& p) {
	return v3(parse_f32(p), parse_f32(p), parse_f32(p));
}
v4 parse_v4(Parser& p) {
	return v4(parse_f32(p), parse_f32(p), parse_f32(p), parse_f32(p));
}
String write_v3(Allocator alloc, v3 v) { return push_strf(alloc, "%f %f %f", v.x, v.y, v.z); }
String write_v4(Allocator alloc, v4 v) { return push_strf(alloc, "%f %f %f %f", v.x, v.y, v.z, v.w); }

v2 world_to_screen(Camera2 c, v2 p, v2 screen_center) {
	return screen_center + (p - c.pos) * c.zoom;
}
Rng2 world_to_screen(Camera2 c, Rng2 rect, v2 screen_center) {
	Rng2 res = {
		screen_center + (rect.min - c.pos) * c.zoom,
		screen_center + (rect.max - c.pos) * c.zoom,
	};
	return res;
}
v2 world_to_screen2(Camera2 c, v2 p, v2 screen_center) {
	v2 res = {
		screen_center.x + (p.x - c.pos.x) * c.zoom2.x,
		screen_center.y + (p.y - c.pos.y) * c.zoom2.y,
	};
	return res;
}
v2 screen_to_world(Camera2 c, v2 p, v2 screen_center) {
	return c.pos + (p - screen_center) / c.zoom;
}
v2 screen_to_world2(Camera2 c, v2 p, v2 screen_center) {
	v2 res = {
		c.pos.x + (p.x - screen_center.x) / c.zoom2.x,
		c.pos.y + (p.y - screen_center.y) / c.zoom2.y,
	};
	return res;
}
void camera_zoom_at(Camera2& c, v2 mouse, v2 screen_center, f32 factor) {
	v2 before = screen_to_world(c, mouse, screen_center);
	c.zoom *= factor;
	v2 after = screen_to_world(c, mouse, screen_center);
	c.pos += before - after;
}
void camera_zoom_at2(Camera2& c, v2 mouse, v2 screen_center, f32 factor) {
	v2 before = screen_to_world2(c, mouse, screen_center);
	c.zoom *= factor;
	v2 after = screen_to_world2(c, mouse, screen_center);
	c.pos += before - after;
}
void camera_zoom_at_pow_clamp(Camera2& c, v2 mouse, v2 screen_center, f32 factor) {
	v2 before = screen_to_world2(c, mouse, screen_center);
	c.zoom *= factor;
	c.zoom = ClampBot(c.zoom, 0.02);
	c.zoom2.y = Clamp(0.01, c.zoom, 3);
	f32 t = pow(c.zoom + 1, 2);
	f32 ratio = t * 0.3;
	c.zoom2.x = c.zoom2.y * ratio;
	if(c.zoom2.y > 1) {
		f32 inv = 1.0f / c.zoom;
		f32 target = inv / (1.0f + inv);
		c.zoom2.y = lerp(1.0f, 0.3f, target);
	}
	v2 after = screen_to_world2(c, mouse, screen_center);
	c.pos += before - after;
}
void camera_pan(Camera2& camera, v2 screen_delta) {
	camera.pos -= screen_delta / camera.zoom;
}
void camera_pan2(Camera2& camera, v2 screen_delta) {
	camera.pos.x -= screen_delta.x / camera.zoom2.x;
	camera.pos.y -= screen_delta.y / camera.zoom2.y;
}
void camera_zoom_and_pan(Camera2& c, v2 screen_center, b32 linear) {
	f32 wheel = os_mouse_wheel();
	if(wheel) {
		if(os_key_is_down(Key_LControl)) {
			if(!linear) {
				camera_zoom_at_pow_clamp(c, os_mouse_pos(), screen_center, wheel > 0 ? 1.3f : 1 / 1.3f);
			} else {
				camera_zoom_at(c, os_mouse_pos(), screen_center, wheel > 0 ? 1.3f : 1 / 1.3f);
			}
		} else {
			f32 wheel_h = os_mouse_wheel_horizontal();
			camera_pan2(c, v2(wheel_h * 100, wheel * 100));
		}
	} else {
		f32 wheel_h = os_mouse_wheel_horizontal();
		camera_pan2(c, v2(wheel_h * 100, wheel * 100));
	}
}

void watch_add(String watch_name, WatchOp op) {
	var& g = *st;
	FileProperties props = os_file_path_properties(watch_name);
	WatchFile file_watch = {
		.path = watch_name,
		.modified = props.modified,
		.op = op,
	};
	array_push(g.watches, file_watch);
}

void watch_directory_add(String watch_name, WatchOp op, OS_WatchFlags flags) {
	var& g = *st;
	String dir_path = push_strf(st->arena, "%s", watch_name);
	OS_Watch watch = os_watch_open(flags);
	os_watch_attach(watch, dir_path);
	WatchDirectory dir_watch = {
		.path = dir_path,
		.watch = watch,
		.op = op,
	};
	array_push(g.watch_directories, dir_watch);
}

void watch_update() {
	var& g = *st;
	Scratch scratch;
	Loop(i, g.watches.count) {
		WatchFile& x = g.watches[i];
		FileProperties props = os_file_path_properties(x.path);
		if(props.modified > x.modified) {
			switch(x.op) {
				case WatchOp_NotifyHotreload: {
					st->should_hotreload = true;
				}break;
				InvalidDefaultCase;
			}
			x.modified = props.modified;
		}
	}
	for(var dir : g.watch_directories) {
		Slice strs = os_watch_check(scratch, dir.watch);
		Loop(i, strs.count) {
			String name = strs[i];
			switch(dir.op) {
				case WatchOp_RecompileShader: {
					GlobalState& g = *st;
					Scratch scratch;
					String shader_filepath = push_strf(scratch, "%s/%s", g.shader_dir, name);
					String shader_compiled_filepath = push_strf(scratch, "%s/%s%s", g.shader_compiled_dir, str_chop_last_dot(name), String(".spv"));
					StringList list = {};
					str_list_push(scratch, list, "slangc");
					str_list_push(scratch, list, shader_filepath);
					str_list_push(scratch, list, "-target");
					str_list_push(scratch, list, "spirv");
					str_list_push(scratch, list, "-g");
					str_list_push(scratch, list, "-o");
					str_list_push(scratch, list, shader_compiled_filepath);
					os_process_make(list);
				}break;
				case WatchOp_ShaderReload: {
					GlobalState& g = *st;
					String shader_name = str_chop_last_dot(name);
					String shader_name_slang = push_strf(scratch, "%s.slang", shader_name);
					String shader_filepath = push_strf(scratch, "%s/%s", g.shader_dir, shader_name_slang);
					String shader_compiled_filepath = push_strf(scratch, "%s/%s", g.shader_compiled_dir, name);
					os_file_path_copy_mtime(shader_filepath, shader_compiled_filepath);
					reload_shader(shader_name);
				}break;
				InvalidDefaultCase;
			}
		}
	}
}

b32 iter_next(ThingIter& it) {
	if(it.idx >= it.count) return false;
	u32 dense_idx = it.dense[it.idx];
	it.id = {dense_idx, it.gens[dense_idx]};
	it.idx++;
	return true;
}

ThingIter things_begin() {
	var& g = *st;
	ThingIter res = {
		.data = g.things.data,
		.count = g.things_set.count,
		.dense = g.things_set.dense,
		.gens = g.things.gens,
	};
	return res;
}

ThingDesc default_thing_desc() {
	var& g = *st;
	ThingDesc res = {
		.scale = v3(1),
		.rot = quat_identity(),
		.color = ColorWhite,
		.mesh_id = g.meshes_ids[Mesh_Cube],
		.mat_id = g.materials_ids[Material_Orange],
	};
	return res;
}

MaterialProps default_material_props() {
	MaterialProps props = {
		.ambient = v3(1),
		.diffuse = v3(1),
		.specular = v3(1),
		.shininess = 1,
	};
	return props;
}

Thing& get_thing(ThingId id) { return pool_get(st->things, id); }

void push_child_thing(ThingId parent, ThingId id) {
	var& g = *st;
	var& t = get_thing(parent);
	hdll_push_back(g.things.data, t.first, t.last, id);
}

String dumb_struct(Allocator arena, Slice<MemberDefinition> members, void* ptr, EntityFlags flags) {
	Scratch scratch(arena);
	var string = dstr_make(arena);
	for(var member : members) {
		u8* member_ptr = Offset(ptr, member.offset);
		switch(member.type) {
			default:break;
			case MetaType_u32: {
				dstr_push(string, push_strf(scratch, "%s %u\n", member.name, *(u32*)member_ptr));
			}break;
			case MetaType_i32: {
				dstr_push(string, push_strf(scratch, "%s %u\n", member.name, *(i32*)member_ptr));
			}break;
			case MetaType_b32: {
				dstr_push(string, push_strf(scratch, "%s %u\n", member.name, *(b32*)member_ptr));
			}break;
			case MetaType_f32: {
				dstr_push(string, push_strf(scratch, "%s %f\n", member.name, *(f32*)member_ptr));
			}break;
			case MetaType_v2: {
				v2 v = *(v2*)member_ptr;
				dstr_push(string, push_strf(scratch, "%s %f %f\n", member.name, v.x, v.y));
			}break;
			case MetaType_v3: {
				v3 v = *(v3*)member_ptr;
				dstr_push(string, push_strf(scratch, "%s %f %f %f\n", member.name, v.x, v.y, v.z));
			}break;
			case MetaType_v4: {
				v4 v = *(v4*)member_ptr;
				dstr_push(string, push_strf(scratch, "%s %f %f %f %f\n", member.name, v.x, v.y, v.z, v.w));
			}break;
			case MetaType_Rng3: {
				Rng3 v = *(Rng3*)member_ptr;
				dstr_push(string, push_strf(scratch, "%s %f %f %f %f %f %f\n", member.name, v.min.x,v.min.y,v.min.z, v.max.x,v.max.y,v.max.z));
			}break;
			case MetaType_String: {
				String v = *(String*)member_ptr;
				dstr_push(string, push_strf(scratch, "%s \"%s\"\n", member.name, v));
			}break;
			case MetaType_R_MeshId: {
				MeshId v = *(MeshId*)member_ptr;
				dstr_push(string, push_strf(scratch, "%s \"%s\"\n", member.name, st->mesh_id_to_str[v.idx]));
			}break;
			case MetaType_R_MaterialId: {
				MaterialId v = *(MaterialId*)member_ptr;
				dstr_push(string, push_strf(scratch, "%s \"%s\"\n", member.name, st->material_id_to_str[v.idx]));
			}break;
		}
	}
	return string;
}

void dumb_struct_load(Slice<MemberDefinition> members, void* ptr, Parser* parser) {
	Parser& p = *parser;
	while(!tok_match(p, TokenType_CloseBrace)) {
		MemberDefinition member = {};
		Token ident_token = tok_advance(p);
		if(ident_token.type == TokenType_Identifier) {
			Loop(i, members.count) {
				if(str_match(members[i].name, ident_token.str)) {
					member = members[i];
					break;
				}
			}
		}
		u8* mem = Offset(ptr, member.offset);
		switch(member.type) {
			default:break;
			case MetaType_u32: {
				Token tok = tok_advance(p);
				*(u32*)mem = u32_from_str(tok.str);
			}break;
			case MetaType_i32: {
				*(u32*)mem = parse_i32(p);
			}break;
			case MetaType_b32: {
				Token tok = tok_advance(p);
				*(u32*)mem = u32_from_str(tok.str);
			}break;
			case MetaType_f32: {
				*(f32*)mem = parse_f32(p);
			}break;
			case MetaType_v2: {
				*(v2*)mem = v2(parse_f32(p), parse_f32(p));
			}break;
			case MetaType_v3: {
				*(v3*)mem = v3(parse_f32(p), parse_f32(p), parse_f32(p));
			}break;
			case MetaType_v4: {
				*(v4*)mem = v4(parse_f32(p), parse_f32(p), parse_f32(p), parse_f32(p));
			}break;
			case MetaType_Rng3: {
				*(Rng3*)mem = Rng3(v3(parse_f32(p), parse_f32(p), parse_f32(p)), v3(parse_f32(p), parse_f32(p), parse_f32(p)));
			}break;
			// case MetaType_R_MeshId: {
			// 	Token tok = tok_expect(p, TokenType_String);
			// 	var [mesh, ok] = map_get(st->str_to_mesh, hash(tok.str));
			// 	Assert(ok);
			// 	*(R_MeshId*)mem = mesh;
			// }break;
			// case MetaType_MaterialId: {
			// 	Token tok = tok_expect(p, TokenType_String);
			// 	var [material, ok] = map_get(st->str_to_material, hash(tok.str));
			// 	Assert(ok);
			// 	*(R_MaterialId*)mem = material;
			// }break;
			// case MetaType_String: {
			// 	Token tok = tok_expect(p, TokenType_String);
			// 	*(String*)mem = push_str_copy(st->arena, tok.str);
			// }break;
			// case MetaType_EntityFlags: {
			// 	*(EntityFlags*)mem = parse_u32(p);
			// }break;
		}
	}
}

struct Observation {
	ThingId observed;
	ThingId observer;
	u32 next_observed, prev_observed;
	u32 next_observer, prev_observer;
};

Poolu32<Observation, KB(1)> observations;
Map<u32, KB(1)> first_observed;
Map<u32, KB(1)> first_observer;

Slice<ThingId> find_observed(Allocator arena, ThingId observer) {
	var res = array_make<ThingId>(arena);
	u64 key = observer.idx;
	u32 first_idx = 0;
	if(!map_get(first_observed, key, &first_idx)) {
		return {};
	}
	u32 idx = first_idx;
	do {
		Observation o = observations.data[idx];
		array_push(res, o.observed);
		idx = o.next_observed;
	}while(idx != first_idx);
	return slice(res);
}

Slice<ThingId> find_observers(Allocator arena, ThingId observed) {
	var res = array_make<ThingId>(arena);
	u64 key = observed.idx;
	u32 first_idx = 0;
	if(!map_get(first_observed, key, &first_idx)) {
		return {};
	}
	u32 idx = first_idx;
	do {
		Observation o = observations.data[idx];
		array_push(res, o.observer);
		idx = o.next_observer;
	}while(idx != first_idx);
	return slice(res);
}

u32 add_observation(ThingId observer, ThingId observed) {
	u32 idx = pool_push(observations, {.observed = observed, .observer = observer});
	{
		u64 key = observer.idx;
		u32 first = 0;
		if(map_get(first_observed, key, &first)) {
			DLL_list_push(observations.data, first, idx, next_observed, prev_observed);
		} else {
			DLL_list_init(observations.data, idx, next_observed, prev_observed);
			map_set(first_observed, key, idx);
		}
	}
	{
		u64 key = observed.idx;
		u32 first = 0;
		if(map_get(first_observer, key, &first)) {
			DLL_list_push(observations.data, first, idx, next_observer, prev_observer);
		} else {
			DLL_list_init(observations.data, idx, next_observer, prev_observer);
			map_set(first_observer, key, idx);
		}
	}
	return idx;
}

void remove_observation(u32 idx) {
	var o = observations.data[idx];
	{
		DLL_list_remove(observations.data, idx, next_observed, prev_observed);
		u64 key = o.observer.idx;
		u32 first = 0;
		map_get(first_observed, key, &first);
		if(first == idx) {
			if(o.next_observed != idx) {
				map_set(first_observed, key, o.next_observed);
			} else {
				map_remove(first_observed, key);
			}
		}
	}
	{
		DLL_list_remove(observations.data, idx, next_observer, prev_observer);
		u64 key = o.observed.idx;
		u32 first = 0;
		map_get(first_observer, key, &first);
		if(first == idx) {
			if(o.next_observer != idx) {
				map_set(first_observer, key, o.next_observer);
			} else {
				map_remove(first_observer, key);
			}
		}
	}
	pool_remove(observations, idx);
}

void print_observed(ThingId id) {
	Scratch scratch;
	var s = find_observed(scratch, id);
	print("Thing %i is observed by: ", id.idx);
	for(var idx : s) {
		print("%i ", idx);
	}
	println("");
}

void print_observers(ThingId id) {
	Scratch scratch;
	var s = find_observers(scratch, id);
	print("Thing %i is observing: ", id.idx);
	for(var idx : s) {
		print("%i ", idx);
	}
	println("");
}

void check_observer_observed() {
	Scratch scratch;

	add_observation({2}, {1}); /* Enemy A observes Player      -> idx 0 */
	uint32_t enemyB_watches_player =
		add_observation({3}, {1}); /* Enemy B observes Player      -> idx 1 */
	add_observation({4}, {1});	 /* Enemy C observes Player      -> idx 2 */
	add_observation({2}, {5});	 /* Enemy A also observes QuestGiver -> idx 3 */

	Info("-- before removal --");
	print_observers({1}); /* expect: 2 3 4 (some order) */
	print_observed({2});	 /* expect: 1 5 */
	print_observed({3});	 /* expect: 1 */
	print_observers({5}); /* expect: 2 */

	/* Enemy B stops observing Player */
	remove_observation(enemyB_watches_player);

	Info("\n-- after Enemy B stops observing Player --");
	print_observers({1}); /* expect: 2 4 only, no 3 */
	print_observed({3});	 /* expect: (nothing) - B's chain is now empty */
}

template <typename T> struct NodeIter2 {
	T* nodes;
	u32 idx;
	u32 cur;
	T* val;
};

template <typename T> b32 node_iter_next(NodeIter2<T>& it) {
	if(it.idx == 0) return false;
	it.cur = it.idx;
	it.val = &it.nodes[it.idx];
	it.idx = it.val->next;
	return true;
}

void init() {
	Scratch scratch;
	var& g = *st;

	g.asset_dir = push_strf(g.arena, "%s/%s", os_cur_directory(), String("../assets"));
	g.shader_dir = push_str_cat(g.arena, g.asset_dir, "/shaders");
	g.shader_compiled_dir = push_str_cat(g.arena, g.shader_dir, "/compiled");
	g.models_dir = push_str_cat(g.arena, g.asset_dir, "/models");
	g.textures_dir = push_str_cat(g.arena, g.asset_dir, "/textures");

	for(var s : os_args()) {
		if(str_match(s, "compile_shaders")) {
			u64 s = os_now_ns();
			shaders_compile(scratch);
			shaders_compile_join();
			Info("took: %fms", f64(os_now_ns()-s) / Million(1));
			os_exit(0);
		}
	}

	g.gpa = alloc_make(g.arena);
	g.frame_arena = arena_make();
	shaders_compile(scratch);
	cpu_find_frequency();
	prof_init(g.arena);
	prof_launch_begin();
	{
		ProfBlock("init");
		os_gfx_init();
		thread_pool_init();
		test();
		r_init();
		dev_init();
		init_game();
		watch_directory_add(g.shader_dir, WatchOp_RecompileShader);
		watch_directory_add(g.shader_compiled_dir, WatchOp_ShaderReload);
	}
	prof_launch_end();
}

shared_function void update(HotReloadData* data) {
	Scratch scratch;
if(data->ctx == null) {
		Arena arena = arena_make(.name = "common arena");
		data->ctx = st = push_struct_zero(arena, GlobalState);
		st->arena = arena;
		u64 start = cpu_now();
		init();
		Debug("init time: %fms", tsc_to_ms(cpu_now() - start));
#if HOTRELOAD_BUILD
		watch_add(data->lib_path, WatchOp_NotifyHotreload);
#endif
	}
	if(!st) {
		st = (GlobalState*)data->ctx;
		st->should_hotreload = false;
	}

	var& g = *st;
	g.fps = 60;
	u64 prev_ns = os_now_ns();
	while(!os_window_should_close()) {
		if(st->should_hotreload) {
			goto hotreload;
		}
		u64 target_fps = Billion(1) / g.fps;
		prof_begin_frame();
		{
			ProfBlock("frame");
			os_pump_messages();
			u64 now_ns = os_now_ns();
			time_dt = f64(now_ns - prev_ns) / Billion(1);
			time_now += time_dt;
			prev_ns = now_ns;
#if BUILD_DEV
			imgui_begin_frame();
#endif
			update_game();
			r_end();
			watch_update();

			u64 frame_duration = os_now_ns() - now_ns;
			if(frame_duration < target_fps) {
				u64 sleep_time = target_fps - frame_duration;
				ProfBlock("main sleep", ProfType_Sleep);
				os_sleep_ms(sleep_time / Million(1));
			}
		}
		prof_end_frame();
		current_frame++;
		arena_clear(st->frame_arena);
		mem_track_end();
	}

	// vk_shutdown();
	// os_gfx_shutdown();
	os_exit(0);

	hotreload:
	thread_wait_remanings();
}

MeshDesc generate_sphere(Allocator arena) {
	u32 lat_steps = 10;
	u32 lon_steps = 10;
	u32 vert_count = lat_steps*lon_steps;
	u32 index_count = (lat_steps - 1) * lon_steps * 6;
	var vertices = push_slice(arena, Vertex, vert_count);
	var indices = push_slice(arena, u16, index_count);
	f32 lat_step_angle = PI / lat_steps;
	f32 lon_step_angle = 2*PI / lon_steps;
	for(u32 i = 0; i < lat_steps; i++) {
		f32 lat_angle = -PI/2 + i*lat_step_angle;
		for(u32 j = 0; j < lon_steps; j++) {
			f32 lon_angle = j * lon_step_angle;
			Vertex vert = {
				.pos.x = cos(lat_angle) * cos(lon_angle),
				.pos.y = sin(lat_angle),
				.pos.z = cos(lat_angle) * sin(lon_angle),
				.uv.x = (f32)j / (lon_steps - 1),  // 0 → 1 across longitude
				.uv.y = (f32)i / (lat_steps - 1),  // 0 → 1 from bottom to top
			};
			vertices[i*lon_steps + j] = vert;
		}
	}
	var idx = [&](u32 i, u32 j) {
		return i * lon_steps + j;
	};
	u32 k = 0;
	for(u32 i = 0; i < lat_steps - 1; i++) {
		for(u32 j = 0; j < lon_steps; j++) {
			u32 next_j = (j + 1) % lon_steps; // wrap around
			u32 v0 = idx(i, j);
			u32 v1 = idx(i, next_j);
			u32 v2 = idx(i + 1, j);
			u32 v3 = idx(i + 1, next_j);
			indices[k++] = v0;
			indices[k++] = v2;
			indices[k++] = v1;
			indices[k++] = v1;
			indices[k++] = v2;
			indices[k++] = v3;
		}
	}
	// u32 north_pole_index = vert_count - 1; // last vertex
	// for(u32 j = 0; j < lon_steps; ++j) {
	//   u32 next_j = (j + 1) % lon_steps;
	//   indices[k++] = idx(lat_steps - 2, j); // last row before pole
	//   indices[k++] = north_pole_index;      // pole
	//   indices[k++] = idx(lat_steps - 2, next_j);
	// }
	MeshDesc mesh = {
		.vertices = vertices,
		.indices = indices,
	};
	return mesh;
}

MeshDesc generate_grid(Allocator arena, u32 size, f32 step) {
	var vertices = push_slice(arena, Vertex, size*4);
	v3 pos_offset = v3(-(i32)size/2, 0, -(i32)size/2);
	for(i32 i = 0; i < size; i++) {
		vertices[i*2].pos = pos_offset + v3(0, 0, i*step);
		vertices[i*2+1].pos = pos_offset + v3(size*step, 0, i*step);
	}
	var vertical_vertices = slice_skip(vertices, size*2);
	for(i32 i = 0; i < size; i++) {
		vertical_vertices[i*2].pos = pos_offset + v3(i*step, 0, 0);
		vertical_vertices[i*2+1].pos = pos_offset + v3(i*step, 0, size*step);
	}
	MeshDesc mesh = {
		.vertices = vertices,
	};
	return mesh;
}

// b32 ray_intersect_AABB(Ray ray, AABB aabb) {
//   v3 tMin = v3_hadamard_div(aabb.min - ray.origin, ray.dir);
//   v3 tMax = v3_hadamard_div(aabb.max - ray.origin, ray.dir);
//   v3 t1 = v3_less(tMin, tMax);
//   v3 t2 = v3_greater(tMin, tMax);
//   f32 tNear = Max3(t1.x, t1.y, t1.z);
//   f32 tFar = Min3(t2.x, t2.y, t2.z);
//   if(tNear > tFar) {
//     return false;
//   }
//   return true;
// };

ThingId make_thing(ThingDesc desc) {
	var& g = *st;
	Thing e = {
		.pos = desc.pos,
		.rot = desc.rot,
		.scale = desc.scale,
		.mesh = desc.mesh_id,
		.mat = desc.mat_id,
		.aabb = desc.aabb,
		.color = desc.color,
	};
	ThingId id = pool_push(g.things, e);
	sparse_set_push(g.things_set, id.idx);
	return id;
}

void destroy_thing(ThingId id) {
	var& g = *st;
	pool_remove(g.things, id);
	sparse_set_remove(g.things_set, id.idx);
}

void select_obj() {
	var& g = *st;
	v3 dir = ray_from_screen(os_mouse_pos(), os_window_size(), g.cam.pos, st->view, st->projection).dir;
	var desc = default_thing_desc();
	desc.pos = g.cam.pos;
	desc.scale = v3(0.3);
	desc.vel = dir * 4;
	make_thing(desc);
	// e.pos() = st->cam.pos + v3_norm(m4x4_forward(st->cam.view));
}

void save_game_state() {
	Scratch scratch;
	var& g = *st;
	Dstring data = dstr_make(scratch);

	{
		var& cam = g.cam;
		dstr_push(data, "Camera {\n");
		dstr_pushf(data, "pos %s\n", write_v3(scratch, cam.pos));
		dstr_pushf(data, "dir %f %f %f\n", write_v3(scratch, cam.dir));
		dstr_pushf(data, "yaw %f\n", cam.yaw);
		dstr_pushf(data, "pitch %f\n", cam.pitch);
		dstr_pushf(data, "fov %f\n", cam.fov);
		dstr_pushf(data, "accel %f\n", cam.accel);
		dstr_pushf(data, "vel %s\n", write_v3(scratch, cam.vel));
		dstr_pushf(data, "vel_friction %f\n", cam.vel_friction);
		dstr_push(data, "}\n");
	}

	{
		dstr_push(data, "Things {\n");
		u32 idx = 0;
		LoopIter(it, things_begin()) {
			Thing& e = get_thing(it.id);
			e.serialized_idx = idx;
			dstr_push(data, "{\n");
			dstr_pushf(data, "pos %s\n", write_v3(scratch, e.pos));
			dstr_pushf(data, "rot %s\n", write_v4(scratch, e.rot));
			dstr_pushf(data, "scale %s\n", write_v3(scratch, e.scale));
			dstr_pushf(data, "mesh \"%s\"\n", g.mesh_id_to_str[e.mesh.idx]);
			dstr_pushf(data, "mat \"%s\"\n", g.material_id_to_str[e.mat.idx]);
			dstr_push(data, "}\n");
			idx++;
		}
		dstr_push(data, "}\n");
		dstr_push(data, "ThingEnum {\n");
		LoopEnumNonZero(i, ThingEnum) {
			// var id = g.thing_enums[i];
			// if(pool_is_valid_handle(g.things.pool, id)) {
			// 	var& t = get_thing(id);
			// 	dstr_pushf(data, "%s %u\n", things_enum_strs[i], t.serialized_idx);
			// }
		}
		dstr_push(data, "}\n");
	}

	// {
	// 	dstr_push(data, "Camera {\n");
	// 	dstr_push(data, dumb_struct(scratch, slice(members_of_Camera), &g.cam));
	// 	dstr_push(data, "}\n");
	// }

	// // Things
	// dstr_push(data, "Things {\n");
	// {
	// 	Thing e = get_thing(g.cube1);
	// 	dstr_push(data, "cube1 {\n");
	// 	dstr_push(data, dumb_struct(scratch, slice(members_of_Thing), &e));
	// 	dstr_push(data, "}\n");
	// }
	// dstr_push(data, "}\n");

	// {
	// 	LoopIter(it, things_begin()) {
	// 		Thing& e = *it;
	// 		dstr_push(data, "Thing {\n");
	// 		dstr_push(data, dumb_struct(scratch, slice(members_of_Thing), &e, e.flags));
	// 		dstr_push(data, "}\n");
	// 	}
	// }
	os_file_path_write_all(push_strf(scratch, "%s/saved", os_cur_directory(), String("saved")), dstr_slice(data));
}

void load_game_state() {
	var& g = *st;
	Scratch scratch;
	LoopIter(it, things_begin()) {
		destroy_thing(it.id);
	}
	pool_clear(g.things);
	g.things_set.count = 0;
	String str = os_file_path_read_all_str(scratch, push_strf(scratch, "%s/saved", os_cur_directory(), String("saved")));
	Slice tokens = tokens_from_str(scratch, str);
	Parser p = parser_make(tokens);
	while(p.cur < p.tokens.count) {
		Token tok = tok_advance(p);
		if(tok.type == TokenType_Identifier) {
			if(str_match(tok.str, "Camera")) {
				var& cam = g.cam;
				tok_expect(p, TokenType_OpenBrace);
				tok_expect(p, TokenType_Identifier); cam.pos = parse_v3(p);
				tok_expect(p, TokenType_Identifier); cam.dir = parse_v3(p);
				tok_expect(p, TokenType_Identifier); cam.yaw = parse_f32(p);
				tok_expect(p, TokenType_Identifier); cam.pitch = parse_f32(p);
				tok_expect(p, TokenType_Identifier); cam.fov = parse_f32(p);
				tok_expect(p, TokenType_Identifier); cam.accel = parse_f32(p);
				tok_expect(p, TokenType_Identifier); cam.vel = parse_v3(p);
				tok_expect(p, TokenType_Identifier); cam.vel_friction = parse_f32(p);
				tok_expect(p, TokenType_CloseBrace);
			} else if(str_match(tok.str, "Things")) {
				tok_expect(p, TokenType_OpenBrace);
				while(tok_match(p, TokenType_OpenBrace)) {
					ThingDesc desc = {};
					tok_expect(p, TokenType_Identifier); desc.pos = parse_v3(p);
					tok_expect(p, TokenType_Identifier); desc.rot = parse_v4(p);
					tok_expect(p, TokenType_Identifier); desc.scale = parse_v3(p);
					tok_expect(p, TokenType_Identifier); map_get(g.str_to_mesh_id, hash(tok_advance(p).str), &desc.mesh_id);
					tok_expect(p, TokenType_Identifier); map_get(g.str_to_material_id, hash(tok_advance(p).str), &desc.mat_id);
					make_thing(desc);
					tok_expect(p, TokenType_CloseBrace);
				}
				tok_expect(p, TokenType_CloseBrace);
			} else if(str_match(tok.str, "ThingEnum")) {
				tok_expect(p, TokenType_OpenBrace);
				while(!tok_match(p, TokenType_CloseBrace)) {
					// var t = tok_expect(p, TokenType_Identifier);
					// u32 thing_idx = parse_u32(p) + 1;
					// u32 enum_idx = map_get(g.str_to_thing_enum, hash(t.str)).value;
					// g.thing_enums[enum_idx] = pool_get_handle(g.things.pool, thing_idx);
				}
			}
		}
	}

	// while(p.cur < p.tokens.count) {
	// 	Token tok = tok_advance(p);
	// 	if(tok.type == TokenType_Identifier) {
	// 		if(str_match(tok.str, "Camera")) {
	// 			dumb_struct_load(slice(members_of_Camera), &g.cam, &p);
	// 		} else if(str_match(tok.str, "Entity")) {
	// 			ThingId id = make_thing({});
	// 			Thing& e = get_thing(id);
	// 			dumb_struct_load(slice(members_of_Thing), &e, &p);
	// 			if(flag_has(e.flags, EntityFlag_Referenced)) {
	// 				if(str_match("monkey", e.name)) {
	// 					g.monkey0 = id;
	// 				} else if(str_match("axis_attached_to_cam", e.name)) {
	// 					g.axis_attached_to_cam_id = id;
	// 				} else if(str_match("rotating_cube", e.name)) {
	// 					g.cube0 = id;
	// 				} 
	// 			}
	// 		} else if(str_match(tok.str, "e")) {
	// 			ThingId e_id = make_thing({});
	// 			Thing& e = get_thing(e_id);
	// 			dumb_struct_load(slice(members_of_Thing), &e, &p);
	// 			g.cube1 = e_id;
	// 		}
	// 	}
	// }
}

void init_game() {
	ProfFunc;
	Scratch scratch;
	var& g = *st;
	g.arena = arena_make(.name = "game arena");
	g.gpa = alloc_make(g.arena);

	///////////////////////////////////
	// Assets
	{
		set_cubemap(make_texture({.name = "night_cubemap", .is_cube = true}));

		// Known assets
		var mesh_set = [](MeshEnum mesh_enum, MeshId id) { 
			GlobalState& g = *st;
			g.meshes_ids[mesh_enum] = id;
			String str = push_str_copy(g.arena, meshes_strs[mesh_enum]);
			map_set(g.str_to_mesh_id, hash(str), id);
			g.mesh_id_to_str[id.idx] = str;
		};
		mesh_set(Mesh_Triangle, make_mesh({.vertices = slice(triangle_vertices)}));
		mesh_set(Mesh_Grid, make_mesh(generate_grid(scratch, 100, 1)));
		mesh_set(Mesh_Axis, make_mesh({.vertices = slice(axis_vertices)}));
		mesh_set(Mesh_Sphere, make_mesh(generate_sphere(scratch)));

		// Mesh
		var load_mesh = [&](MeshEnum enum_name, String name) {
			MeshId id = make_mesh({.name = name});
			g.meshes_ids[enum_name] = id;
			String str = push_str_copy(g.arena, name);
			map_set(g.str_to_mesh_id, hash(str), id);
			g.mesh_id_to_str[id.idx] = str;
		};
		load_mesh(Mesh_Cube, "cube_ok_uv.glb");
		load_mesh(Mesh_MonkeyGlb, "monkey.glb");
		load_mesh(Mesh_CubeGlft, "cube.gltf");
		load_mesh(Mesh_Barrack, "castle.gltf");
		// load_mesh(Mesh_T, "t.gltf");
		{
			var info = load_gltf(scratch, "t.gltf", false);
			mesh_set(Mesh_T, make_mesh(info.mesh));
			array_push_elems(g.bones, info.bones);
			array_push_elems(g.inv_matrices, info.inv_matrices);
		}

		// Texture
		var load_texture = [&](TextureEnum enum_name, String name) {
			TextureId id = make_texture({.name = name});
			g.textures_ids[enum_name] = id;
			String str = push_str_copy(g.arena, name);
			map_set(g.str_to_texture_id, hash(str), id);
			g.texture_id_to_str[id.idx] = str;
		};
		load_texture(Texture_Orange, "orange_lines_512.png");
		load_texture(Texture_Container, "container.jpg");
		load_texture(Texture_Barrack, "castle_diffuse.png");
		load_texture(Texture_Bricks, "bricks.png");

		// Materials
		var default_state = make_pipeline_state({
			.depth = {
				.compare = Gfx_CompareOp_Less,
				.write_enabled = true,
			}
		});
		var default_state_line = make_pipeline_state({
			.primitive_type = Gfx_PrimitiveType_Line,
			.depth = {
				.compare = Gfx_CompareOp_Less,
				.write_enabled = true,
			}
		});
		var load_mat = [&](MaterialEnum enum_name, Material desc) {
			MaterialId id = make_material(desc);
			g.materials_ids[enum_name] = id;
			String str = push_str_copy(g.arena, materials_strs[enum_name]);
			map_set(g.str_to_material_id, hash(str), id);
			g.material_id_to_str[id.idx] = str;
		};
		load_mat(Material_Dummy, {
			.type = ShaderType::Texture,
			.batch = default_state,
			.props = default_material_props(),
			.base_color = g.textures_ids[Texture_Dummy],
		});
		load_mat(Material_Orange, {
			.type = ShaderType::Texture,
			.batch = default_state,
			.props = default_material_props(),
			.base_color = g.textures_ids[Texture_Orange],
		});
		load_mat(Material_Container, {
			.type = ShaderType::Texture,
			.batch = default_state,
			.props = default_material_props(),
			.base_color = g.textures_ids[Texture_Container],
		});
		load_mat(Material_Axis, {
			.type = ShaderType::VertColor,
			.batch = default_state_line,
		});
		load_mat(Material_Line, {
			.type = ShaderType::E_Color,
			.batch = default_state_line,
		});
		load_mat(Material_Barrack, {
			.type = ShaderType::Texture,
			.batch = default_state,
			.props = default_material_props(),
			.base_color = g.textures_ids[Texture_Barrack],
		});
	}

	///////////////////////////////////
	// Camera
	{
		Camera& cam = g.cam;
		cam = {
			.pos = v3(0,0,5),
			.yaw = 180,
			.fov = 45,
			.vel_friction = 7.7f,
			.accel = 300,
		};
		cam.dir = {
			cosd(cam.yaw) * cosd(cam.pitch),
			sind(cam.pitch),
			sind(cam.yaw) * cosd(cam.pitch)
		};
		g.view = m4x4_look_at(cam.pos, cam.dir, v3_up());
	}
}

void update_game() {
	ProfFunc;
	Scratch scratch;
	var& g = *st;
	dev_update();

	draw_rect(rng2_make(v2(400, 100), v2(50)), RED);
	draw_rect(rng2_make(v2(600, 100), v2(50)), RED, 10, 2, 1);


	// Test jobs
	#if 0
	{
		ProfBlock("push jobs");
		Loop(i, 2) {
			thread_push({.fn = [](void* ctx) {os_sleep_ms(rand_u32()%6);}, .priority = TaskPriority_Low});
		}
		WaitGroup id0 = thread_push({.fn = [](void* ctx) { os_sleep_ms(2); }});
		WaitGroup id1 = thread_push({.fn = [](void* ctx) { os_sleep_ms(2); }});
		WaitGroup id2 = thread_push({.fn = [](void* ctx) { os_sleep_ms(2); }});
		WaitGroup id3 = thread_push({.fn = [](void* ctx) { os_sleep_ms(2); }});
		TaskDesc tasks[] = {
			{.fn = [](void* ctx) { os_sleep_ms(2); }},
			{.fn = [](void* ctx) { os_sleep_ms(2); }},
			{.fn = [](void* ctx) { os_sleep_ms(2); }},
			{.fn = [](void* ctx) { os_sleep_ms(2); }},
		};
		WaitGroup batch = thread_push_batch(slice(tasks));
		thread_wg_wait(id0);
		thread_wg_wait(id1);
		thread_wg_wait(id2);
		thread_wg_wait(id3);
		thread_wg_wait(batch);
	}
	#endif

	// return;

	///////////////////////////////////
	// Hotkeys
	if(os_key_is_down(Key_Escape)) {
		os_window_close();
	}
	if(os_key_is_pressed(Key_U)) {
		if(g.fps_camera) {
			os_cursor_unlock();
		} else {
			os_cursor_lock();
		}
		g.fps_camera = !g.fps_camera;
	}

	///////////////////////////////////
	// Camera
	{
		Camera& cam = g.cam;
		v2 win_size = v2_of_v2u(os_window_size());
		m4x4& projection = st->projection;
		m4x4& view = st->view;
		projection = m4x4_perspective(deg2rad(cam.fov), win_size.x / win_size.y, 0.1f, 1000.0f);

		f32 rotation_speed = 180.0f * time_dt;
		if(os_key_is_down(Key_A)) {
			cam.yaw += rotation_speed;
		}
		if(os_key_is_down(Key_D)) {
			cam.yaw -= rotation_speed;
		}
		if(os_key_is_down(Key_R)) {
			cam.pitch += rotation_speed;
		}
		if(os_key_is_down(Key_F)) {
			cam.pitch -= rotation_speed;
		}
		if(g.fps_camera) {
			f32 rot_speed = 10;
			cam.pitch -= os_mouse_dt().y * time_dt * rot_speed;
			cam.yaw -= os_mouse_dt().x * time_dt * rot_speed;
		}
		f32 speed = 1;
		v3 mov = {};
		if(os_key_is_down(Key_W)) {
			mov += m4x4_forward(view);
		}
		if(os_key_is_down(Key_S)) {
			mov += m4x4_backward(view);
		}
		if(os_key_is_down(Key_Q)) {
			mov += m4x4_left(view);
		}
		if(os_key_is_down(Key_E)) {
			mov += m4x4_right(view);
		}
		if(os_key_is_down(Key_Space)) {
			mov.y += 1.0f;
		}
		if(os_key_is_down(Key_X)) {
			mov.y -= 1.0f;
		}
		if(os_key_is_down(Key_Shift)) {
			speed *= 10;
		}
		if(os_key_is_down(Key_LAlt)) {
			speed *= 0.1;
		}
		mov *= cam.accel * speed;
		f32 dt = time_dt;

		cam.vel += mov * dt;
		cam.pos += cam.vel * dt;
		cam.vel -= cam.vel * cam.vel_friction * dt;
		v4 yaw = quat_axis_angle(v3_up(), deg2rad(cam.yaw));
		v4 pitch = quat_axis_angle(v3_right(), deg2rad(-cam.pitch));
		v4 quat = quat_mul(yaw, pitch);
		cam.dir = quat_forward(quat);
		view = m4x4_look_at(cam.pos, cam.pos + cam.dir, quat_up(quat));
	}

	// XYZ Axis at center
	{
		f32 line_len = 5;
		v3 pos = v3(0, 0.1, 0);
		draw_line(pos, line_len * v3_right(), ColorRed);
		draw_line(pos, line_len * v3_up(), ColorGreen);
		draw_line(pos, line_len * v3_forward(), ColorBlue);
	}

	// Axis aligned at edge of screen
	{
		m4x4& view = st->view;
		v3 forward = m4x4_forward(view);
		v3 right   = m4x4_right(view);
		v3 up      = m4x4_up(view);
		f32 dist = 1.0f;
		f32 xoff = 0.3f;
		f32 yoff = 0.3f;
		v3 pos = g.cam.pos + forward*dist + right*xoff + up*yoff;
		draw_mesh_trs(g.meshes_ids[Mesh_Axis], g.materials_ids[Material_Axis], pos, quat_identity(), v3(0.1));
	}
	draw_mesh(g.meshes_ids[Mesh_Barrack], g.materials_ids[Material_Barrack], v3(0,0,-40));
	draw_grid(v3(0), 40, 2, u32_from_rgba(v4(0.8)));
	draw_mesh(g.meshes_ids[Mesh_T], g.materials_ids[Material_Container], v3(), DrawCallFlag_Skeleton);
	// r_draw_mesh(g.meshes_ids[Mesh_T], g.materials_ids[Material_Container], v3());

	draw_rect(rng2_make(v2(100), v2(100)), RED);
}
