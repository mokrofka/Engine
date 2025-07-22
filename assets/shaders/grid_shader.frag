#version 450 core
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"

layout(location = 0) out vec4 out_color;

void main() {
  float val = 0.3;
  out_color = vec4(val, val, val, 0.4f);
}
