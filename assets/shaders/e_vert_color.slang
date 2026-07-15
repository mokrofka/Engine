#include "lib.slang"

VS_Entry VSOutput vs_main(VSInput in) {
  VSOutput o;
  u32 entity_id = drawinfo[push.drawcall_base + in.draw_id].base;
  mat4 model = drawcalls_data[entity_id].model;
  o.pos = st.projection_view * model * v4(in.pos, 1.0);
  o.color = in.color;
  return o;
}

FS_Entry v4 fs_main(VSOutput in) {
  v4 color = in.color;
  return color;
}
