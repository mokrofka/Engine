#version 450 core
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"
#include "defines/fragdef.glsl"

void main() {
  v4 texture_color = texture(samplerCube(cube_texture, samplers[0]), in_pos);
  out_color = texture_color;

} 
