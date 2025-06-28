#version 450 core
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.vert.glsl"

layout(location = 0) out vec4 out_color;
layout(location = 0) in in_data {
  vec3 in_color;
};

void main() {
  out_color = vec4(in_color, 1);
} 

