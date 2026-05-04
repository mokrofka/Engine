#version 460
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"
#include "defines/vertdef.glsl"

void main() {
  u32 entities_offset_id = drawinfo[push.drawcall_offset + gl_DrawID].entity_id;
  u32 entity_id = st.entity_indices[entities_offset_id + gl_InstanceIndex];
  Entity e = entities[entity_id];

  gl_Position = st.projection * st.view * e.model * vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);

  out_uv = v2(in_uv.x, -in_uv.y);
  out_color = in_color;
  out_entity_id = entity_id;
}


