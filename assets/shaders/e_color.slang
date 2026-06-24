#include "lib.slang"

VS_Entry VSOutput vs_main(VSInput in) {
  VSOutput o;
  u32 entity_id = drawinfo[push.drawcall_offset + in.draw_id].entity_inst_offset;
  Entity e = entities[entity_id];
  o.pos = g.projection_view * e.model * v4(in.pos, 1.0);
  o.entity_id = entity_id;
  return o;
}

FS_Entry v4 fs_main(VSOutput in) {
  Entity e = entities[in.entity_id];
  v4 color = e.color;
  return color;
}
