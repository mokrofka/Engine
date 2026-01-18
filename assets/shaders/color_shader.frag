#version 450 core
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"
#include "defines/fragdef.glsl"

void main() {
  v4 texture_color = texture(textures[push.tex_id], in_uv);

  out_color = texture_color;
} 

