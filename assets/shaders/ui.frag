#version 460
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"
#include "defines/fragdef.glsl"

void main() {
  out_color = vec4(in_color, 1.0);
}
