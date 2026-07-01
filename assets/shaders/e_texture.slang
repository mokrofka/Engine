#include "lib.slang"

VS_Entry VSOutput vs_main(VSInput in) {
  VSOutput o;
  DrawCallData data = get_data(in.draw_id);
  o.pos = st().projection_view * data.model * v4(in.pos, 1.0);
  o.uv = in.uv;
  o.id = get_id(in.draw_id);
  return o;
}

FS_Entry v4 fs_main(VSOutput in) {
  DrawCallData data = get_data(in.id);
  u32 tex = materials[data.mat].texture;
  v4 color = textures[tex].Sample(samplers[1], in.uv);
  return color;
}

