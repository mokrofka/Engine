#version 450 core
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_tex_coord;

void main() {
  gl_Position = g.projection_view * u_model * vec4(in_pos, 1.0);
}
