#include "lib.slang"

global v2 positions[] = {
  {-1.0, -1.0},
  {-1.0,  3.0},
  { 3.0, -1.0},
};

global v2 tex_coords[] = {
  {0.0, 0.0},
  {0.0, 2.0},
  {2.0, 0.0}
};

VS_Entry VSOutput vs_main(VSInput in) {
  VSOutput o;
  o.pos = v4(positions[in.vert_id], 0, 1);
  o.uv = tex_coords[in.vert_id];
  return o;
}

FS_Entry v4 fs_main(VSOutput in) {
  f32 x = in.pos.x;
  f32 y = in.pos.y;
  v4 color = rgba_from_u32(software_render[(u32)(y * st.res.x + x)]);
  if (color.w < 0.1) discard;
  // v4 color = v4(x / 800, y / 800, 0, 0);
  return color;
}

