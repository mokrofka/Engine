#version 450 core

#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"

layout(location = 0) in vec3 in_pos;

void main() {

  gl_Position = st.projection * st.view * push.model * vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
}

