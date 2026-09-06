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

	// printf("tex: %u, x: %v2f", in.texture, cur_texture_size);

	UI_VSOutput o;
	o.vert = v4(2 * dst_pos.x / st.res.x - 1, 2 * dst_pos.y / st.res.y - 1, 0, 1);
	o.uv = v2(src_pos.x / in.tex_size.x, src_pos.y / in.tex_size.y);
	o.color = in.colors[vert_id];

	o.dst_pos = dst_pos;
	o.dst_center = dst_center;
	o.dst_half_size = dst_half_size;
	o.corner_radius = in.corner_radius;
	o.edge_softness = in.edge_softness;

	o.flags = in.flags;
	o.texture = in.texture;
	return o;
}

f32 rounded_rect_sdf(v2 sample_pos, v2 rect_center, v2 rect_half_size, f32 r) {
	v2 d2 = abs(rect_center - sample_pos) - rect_half_size + v2(r, r);
	return min(max(d2.x, d2.y), 0.0) + length(max(d2, 0.0)) - r;
}

FS_Entry v4 fs_main(UI_VSOutput in) {
	// Font?
	if (flag_has(in.flags, GpuUI_RectFlag_IsFont)) {
		f32 r = textures[in.texture].Sample(samplers[1], in.uv).r;
		if (r < 0.1) discard;
		v4 color = in.color * r;
		return color;
	}

	// Texture?
	if (in.texture > 0) {
		// return v4(in.uv, 0, 1);

		// u32 tex_idx = NonUniformResourceIndex(in.texture);
		// v4 sampled = textures[tex_idx].Sample(samplers[1], in.uv);
		v4 sampled = textures[in.texture].Sample(samplers[1], in.uv);
		// v4 sampled = textures[15].Sample(samplers[1], in.uv);
		v4 color = v4_hadamard(in.color, sampled);
		return color;
	}

	f32 softness = in.edge_softness;
	v2 softness_padding = v2(max(0, softness*2-1));

	// sample distance
	f32 dist = rounded_rect_sdf(in.dst_pos, in.dst_center, in.dst_half_size-softness_padding, in.corner_radius);

	// map distance => a blend factor
	f32 sdf_factor = 1.f - smoothstep(0, 2*softness, dist);

	v4 color = in.color * sdf_factor;
	return color;
}


