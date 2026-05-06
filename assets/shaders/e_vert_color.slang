#include "lib.slang"

VS_Entry VSOutput vs_main(VSInput in) {
  VSOutput o;

  u32 entities_offset_id = drawinfo[push.drawcall_offset + in.draw_id].entity_id;
  u32 entity_id = g.entity_indices[entities_offset_id + in.inst_id];
  Entity e = entities[entity_id];
  o.pos = g.projection_view * e.model * v4(in.pos, 1.0);
  o.color = in.color;
  return o;
}

FS_Entry v4 fs_main(VSOutput in) {
  v4 color = v4(in.color, 0.6);
  return color;
}
