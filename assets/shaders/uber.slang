#include "lib.slang"
// #include "com.slang"
// printf("hello %v4f", in.pos);


CONST v2 positions[] = {
	{-1.0, -1.0},
	{-1.0,  3.0},
	{ 3.0, -1.0},
};

CONST v2 tex_coords[] = {
	{0.0, 0.0},
	{0.0, 2.0},
	{2.0, 0.0},
};

void foo(u32 b, u32 c) {
}

// how to make hotkey in vscode that for example: void foo(u32 <cursor> a, u32 b, u32 c) { in place of cursor by pressing key, it'd become: void foo(u32 b, u32 c) { } ?

// VS_Entry VSOutput vs_main(Vertex in, u32 vert_id: SV_VertexID, u32 draw_id: SV_DrawIndex, u32 inst_id: SV_InstanceID) {
VS_Entry VSOutput vs_main(u32 vert_id: SV_VertexID, u32 draw_id: SV_DrawIndex, u32 inst_id: SV_InstanceID) {
	u32 drawcall_idx = push.idx + draw_id;
	DrawCall draw = drawcalls[drawcall_idx];
	VSOutput o;
	Vertex in;
	if(flag_any(draw.flags, DrawCallFlag_Indexed)) {
		in = vertices[draw.mesh.vert_off + indices[draw.mesh.base + vert_id]];
	} else {
		if(flag_any(draw.flags, DrawCallFlag_CpuVertices)) {
			in = cpu_vertices[vert_id];
		} else {
			in = vertices[draw.mesh.base + vert_id];
		}
	}
	switch(draw.type) {
		case ShaderType::Cube: {
			m4x4 view = st.view;
			view[0][3] = 0;
			view[1][3] = 0;
			view[2][3] = 0;
			v4 pos = st.projection * view * v4(in.pos, 1);
			o.pos = v4(pos.xy, pos.w, pos.w);
			o.norm = in.pos;
		}break;
		case ShaderType::E_Color: {
			o.pos = st.projection_view * draw.model * v4(in.pos, 1.0);
		}break;
		case ShaderType::Texture: {
			v4 pos = v4(in.pos, 1);
			if(flag_any(draw.flags, DrawCallFlag_Skeleton)) {
				m4x4 skin =
					in.weights.x * skin_matrices[in.joints.x] +
					in.weights.y * skin_matrices[in.joints.y] +
					in.weights.z * skin_matrices[in.joints.z] +
					in.weights.w * skin_matrices[in.joints.w];
				pos = skin * pos;
			}
			o.pos = st.projection_view * draw.model * pos;
			o.uv = in.uv;
		}break;
		case ShaderType::VertColor: {
			o.pos = st.projection_view * draw.model * v4(in.pos, 1.0);
			o.color = rgba_from_u32(in.color);
		}break;
		case ShaderType::Line: {
			o.pos = st.projection_view * v4(in.pos, 1);
			o.color = rgba_from_u32(in.color);
		}break;
		case ShaderType::Screen: {
			o.pos = v4(positions[vert_id], 0, 1);
			o.uv = tex_coords[vert_id];
		}break;
		case ShaderType::SoftwareRender: {
			o.pos = v4(positions[vert_id], 0, 1);
			o.uv = tex_coords[vert_id];
		}break;
	}
	o.id = drawcall_idx;
	return o;
}

FS_Entry v4 fs_main(VSOutput in) {
	v4 o = {};
	DrawCall draw = drawcalls[in.id];
	switch(draw.type) {
		case ShaderType::Cube: {
			o = sample_cube(st.cubemap, in.norm);
			// sample = sample_texture(in.texture, in.uv);
		}break;
		case ShaderType::E_Color: {
			o = drawcalls[in.id].color;
		}break;
		case ShaderType::Texture: {
			u32 tex = materials[draw.mat].tex;
			o = sample_texture(tex, in.uv);
		}break;
		case ShaderType::VertColor: {
			o = in.color;
		}break;
		case ShaderType::Line: {
			o = in.color;
		}break;
		case ShaderType::Screen: {
			o = sample_texture(draw.cur_resolve_idx, in.uv);
		}break;
		case ShaderType::SoftwareRender: {
			u32 x = (u32)in.pos.x;
			u32 y = (u32)in.pos.y;
			o = rgba_from_u32(software_render[(u32)(y * st.res.x + x)]);
			// if(o.w < 0.1) discard;
		}break;
	}
	return o;
}

