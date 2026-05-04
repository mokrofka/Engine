#version 460
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"
#include "defines/vertdef.glsl"

void main() {
  gl_Position = v4(in_pos, 1.0);
  out_color = in_color;
}
