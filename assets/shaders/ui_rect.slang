#include "lib.slang"

VS_Entry UI_VSOutput vs_main(u32 vert_id : SV_VertexID, u32 inst_id : SV_InstanceID) {
	UI_Rect in = ui_rects[inst_id];
	v2 vertices[] = {
		{-1, -1},
		{-1, +1},
		{+1, -1},
		{+1, +1},
	};

	v2 dst_half_size = (in.dst_p1 - in.dst_p0) / 2;
	v2 dst_center = (in.dst_p1 + in.dst_p0) / 2;
	v2 dst_pos = vertices[vert_id] * dst_half_size + dst_center;

	v2 src_half_size = (in.src_p1 - in.src_p0) / 2;
	v2 src_center = (in.src_p1 + in.src_p0) / 2;
	v2 src_pos = vertices[vert_id] * src_half_size + src_center;

	UI_VSOutput o;
	o.vert = v4(2 * dst_pos.x / st.res.x - 1, 2 * dst_pos.y / st.res.y - 1, 0, 1);
	o.uv = v2(src_pos.x / in.tex_size.x, src_pos.y / in.tex_size.y);
	o.color = in.colors[vert_id];

	o.dst_pos = dst_pos;
	o.dst_center = dst_center;
	o.dst_half_size = dst_half_size;
	o.corner_radius = in.corner_radius;
	o.edge_softness = in.edge_softness;
	o.border_thickness = in.border_thickness;

	o.flags = in.flags;
	o.texture = in.texture;
	return o;
}

f32 rounded_rect_sdf(v2 sample_pos, v2 rect_center, v2 rect_half_size, f32 r) {
	v2 d2 = abs(rect_center - sample_pos) - rect_half_size + v2(r, r);
	return min(max(d2.x, d2.y), 0.0) + length(max(d2, 0.0)) - r;
}

FS_Entry v4 fs_main(UI_VSOutput in) {
	v4 sample = v4(1);
	// Font?
	if(flag_has(in.flags, GpuUI_RectFlag_IsFont)) {
		f32 r = textures[in.texture].Sample(samplers[1], in.uv).r;
		sample = r;
	}
	// Texture?
	else {
		if(in.texture > 0) {
			sample = textures[in.texture].Sample(samplers[1], in.uv);
		}
	}

	f32 softness = in.edge_softness;
	v2 softness_padding = v2(max(0, softness*2-1));
	f32 dist = rounded_rect_sdf(in.dst_pos, in.dst_center, in.dst_half_size-softness_padding, in.corner_radius);
	f32 sdf_factor = 1.f - smoothstep(0, 2*softness, dist);

	f32 border_factor = 1;
	if(in.border_thickness != 0) {
		v2 interior_half_size = in.dst_half_size - v2(in.border_thickness);
		f32 interior_radius_reduce_f = min(interior_half_size.x/in.dst_half_size.x, interior_half_size.y/in.dst_half_size.y);
		f32 interior_corner_radius = in.corner_radius * Square(interior_radius_reduce_f);
		f32 inside_d = rounded_rect_sdf(in.dst_pos, in.dst_center, interior_half_size-softness_padding, interior_corner_radius);
		f32 inside_f = smoothstep(0, 2*softness, inside_d);
		border_factor = inside_f;
	}

	v4 color = in.color * sample * sdf_factor * border_factor;
	return color;
}


