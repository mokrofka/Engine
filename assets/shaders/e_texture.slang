#include "lib.slang"

VS_Entry VSOutput vs_main(VSInput in) {
  VSOutput o;
  u32 entity_inst_offset = drawinfo[push.drawcall_offset + in.draw_id].entity_id;
  u32 entity_id = g.entity_indices[entity_inst_offset + in.inst_id];
  Entity e = entities[entity_id];

  o.pos = g.projection_view * e.model * v4(in.pos, 1.0);
  v4 pos = v4(in.pos, 1);
  o.uv = in_uv(in.uv);
  o.entity_id = entity_id;
  return o;
}

FS_Entry v4 fs_main(VSOutput in) {
  Entity e = entities[in.entity_id];
  Material material = materials[e.material];
  v4 color = textures[material.texture].Sample(samplers[0], in.uv);
  return color;
}
