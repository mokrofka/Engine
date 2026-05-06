#include "lib.slang"

VS_Entry VSOutput vs_main(VSInput in) {
  VSOutput o;
  o.pos = v4(in.pos, 1);
  o.color = in.color;
  return o;
}

FS_Entry v4 fs_main(VSOutput in) {
  v4 color = v4(in.color, 1);
  return color;
}
