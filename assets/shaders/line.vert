#version 460
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"
#include "defines/vertdef.glsl"

void main() {
  gl_Position = st.projection * st.view * vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
  out_color = in_color;
}

