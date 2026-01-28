#version 450 core
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"
#include "defines/fragdef.glsl"

void main() {
  v4 texture_color = texture(sampler2D(textures[push.tex_id], samplers[0]), in_uv);
  // v4 texture_color = v4(in_color, 1);
  out_color = texture_color;
} 
