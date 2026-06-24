#include "lib.slang"

VS_Entry VSOutput vs_main(VSInput in) {
  VSOutput o;
  o.pos = v4(in.pos, 1);
  o.color = in.color;
  o.uv = in.uv;
  return o;
}

FS_Entry v4 fs_main(VSOutput in) {
  f32 r = textures[push.drawcall_offset].Sample(samplers[1], in.uv).r;
  if (r < 0.1) discard;
  v4 color = r * in.color;
  return color;
}


