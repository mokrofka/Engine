#version 460
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"
#include "defines/vertdef.glsl"
void main() {

  // Entity e = st.entities[drawinfo[gl_DrawID].entity_id];
  // Entity e = st.entities[drawinfo[0].entity_id];
  // gl_Position = st.projection * st.view * e.model * vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
  gl_Position = st.projection * st.view * st.entities[push.id].model * vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
  out_uv = v2(in_uv.x, -in_uv.y);
  out_color = in_color;

}


