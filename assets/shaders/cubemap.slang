#include "lib.slang"

VS_Entry VSOutput vs_main(VSInput in) {
  VSOutput o;
  mat4 view = st().view;
  view[0][3] = 0; //
  view[1][3] = 0;
  view[2][3] = 0;

  // view[3][0] = 0; //
  // view[3][1] = 0;
  // view[3][2] = 0;
  v4 pos = st().projection * view * v4(in.pos, 1);
  o.pos = v4(pos.xy, pos.w, pos.w);
  o.color = v4(in.pos, 1);
  return o;
}

FS_Entry v4 fs_main(VSOutput in) {
  v4 color = cube_texture.Sample(samplers[1], in.color.xyz);
  return color;
}
