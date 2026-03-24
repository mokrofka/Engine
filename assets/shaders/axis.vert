#version 460
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"
#include "defines/vertdef.glsl"

void main() {
  Entity e = entities[drawinfo[gl_InstanceIndex].entity_id];
  gl_Position = st.projection * st.view * e.model * vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
  out_color = in_color;
}

