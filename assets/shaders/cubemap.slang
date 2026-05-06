#include "lib.slang"

VS_Entry VSOutput vs_main(VSInput in) {
  VSOutput o;
  mat4 view = g.view;
  view[0][3] = 0;
  view[1][3] = 0;
  view[2][3] = 0;
  v4 pos = g.projection * view * v4(in.pos, 1);
  o.pos = v4(pos.xy, pos.w, pos.w);
  o.color = in.pos;
  return o;
}

FS_Entry v4 fs_main(VSOutput in) {
  v4 color = cube_texture.Sample(samplers[0], in.color);
  return color;
}
