#version 450 core
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"
#include "defines/vertdef.glsl"

void main() {
  // gl_Position = st.projection * st.view * vec4(in_pos, 1.0);
  // out_pos = in_pos;
  // gl_Position = st.projection * mat4(mat3(st.view)) * vec4(in_pos, 1.0);
  // v4 pos = st.projection * st.view * vec4(in_pos, 1.0);


  v4 pos = st.projection * mat4(mat3(st.view)) * vec4(in_pos, 1.0);
  gl_Position = v4(pos.xy, pos.w, pos.w);
  out_pos = v3(in_pos);

}


