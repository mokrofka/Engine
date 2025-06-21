#version 450 core
#extension GL_GOOGLE_include_directive : enable
#include "global.vert.glsl"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;

layout(location = 0) out out_data {
  vec3 out_color;
};

void main() {
  gl_Position = g.projection_view * u_model * vec4(in_pos, 1.0);
  
  out_color = in_color;
}
