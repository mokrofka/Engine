#include "lib.slang"

global v2 positions[] = {
  {-1.0, -1.0},
  { 3.0, -1.0},
  {-1.0,  3.0},
};

global v2 tex_coords[] = {
  { 0.0,  0.0},
  { 2.0,  0.0},
  { 0.0, -2.0} // flip y-coord since I have fliped y-coord in viewport
};

VS_Entry VSOutput vs_main(VSInput in) {
  VSOutput o;
  o.pos = v4(positions[in.vert_id], 0, 1);
  o.uv = tex_coords[in.vert_id];
  return o;
}

FS_Entry v4 fs_main(VSOutput in) {
  v4 color = textures[push.drawcall_offset].Sample(samplers[0], in.uv);
  return color;
}

