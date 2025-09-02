#version 450 core

#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"

layout(location = 0) in vec3 in_pos;

vec3 triangle[] = {
  {-1.0f, -1.0f, 0},
  {0.0f,  1.0f, 0},
  {1.0f, -1.0f, 0}
};

void main() {
  gl_Position = g.projection_view * u_model * vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
}
