#version 460

#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"

layout(location = 0) out v4 out_color;

layout(location = 0) in in_data {
  v2 in_texcoord;
};

void main() {
  v4 texture_color = texture(sampler2D(textures[push.drawcall_offset], samplers[0]), in_texcoord);
  out_color = texture_color;
}
