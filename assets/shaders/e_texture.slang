#include "lib.slang"

VS_Entry VSOutput vs_main(VSInput in) {
  VSOutput o;
  u32 entity_id = drawinfo[push.drawcall_offset + in.draw_id].entity_inst_offset;
  mat4 model = drawcalls_ctx[entity_id].model;

  o.pos = g.projection_view * model * v4(in.pos, 1.0);

  Entity e = entities[entity_id];
  // o.pos = g.projection_view * e.model * v4(in.pos, 1.0);
  v4 pos = v4(in.pos, 1);
  o.uv = in.uv;
  o.entity_id = entity_id;
  return o;
}

FS_Entry v4 fs_main(VSOutput in) {
  Entity e = entities[in.entity_id];
  // Material material = materials[e.material];
  // v4 color = textures[material.texture].Sample(samplers[1], in.uv);

  u32 tex = drawcalls_ctx[in.entity_id].tex;
  v4 color = textures[tex].Sample(samplers[1], in.uv);

  // color = drawcalls_ctx[in.entity_id].color;
  // color = v4(0,0,0,0);

  return color;
}

