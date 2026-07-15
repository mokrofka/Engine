#include "lib.slang"

VS_Entry VSOutput vs_main(VSInput in) {
  VSOutput o;
  o.pos = st.projection_view * v4(in.pos, 1);
  o.color = in.color;
  return o;
}

FS_Entry v4 fs_main(VSOutput in) {
  v4 color = in.color;
  return color;
}


