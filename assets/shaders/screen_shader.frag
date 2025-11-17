#version 450 core

#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"

layout(location = 0) out v4 out_color;

layout(location = 0) in in_data {
  v2 in_texcoord;
};

layout(set = 0, binding = 1) uniform sampler2D texture_target[];

void main() {
  v4 texture_color = texture(texture_target[0], in_texcoord);
  out_color = texture_color;
}
