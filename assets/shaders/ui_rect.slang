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

  v2 cur_texture_size = v2(1,1);
  if (in.texture > 0) {
    textures[in.texture].GetDimensions(cur_texture_size.x, cur_texture_size.y);
  }
  UI_VSOutput o;
  o.vert = v4(2 * dst_pos.x / st.res.x - 1, 2 * dst_pos.y / st.res.y - 1, 0, 1);
  o.uv = v2(src_pos.x / cur_texture_size.x, src_pos.y / cur_texture_size.y);
  o.texture = in.texture;
  o.color = in.color;
  return o;
}

FS_Entry v4 fs_main(UI_VSOutput in) {
  v4 sampled = v4(1,1,1,1);
  if (in.texture > 0) {
    sampled = textures[in.texture].Sample(samplers[1], in.uv);
  }

  v4 color = v4_hadamard(in.color, sampled);
  return color;
}

