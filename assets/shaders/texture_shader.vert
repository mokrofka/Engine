#version 450 core
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

layout(location = 0) out out_data {
  vec3 out_frag_pos;
  vec3 out_normal;
  vec2 out_tex_coord;
};

void main() {
  gl_Position = g.projection_view * u_model * vec4(in_pos, 1.0);
  
  out_frag_pos = vec3(g.view * u_model * vec4(in_pos, 1));
  out_normal = mat3(g.view) * mat3(u_model) * in_normal;
  out_tex_coord = in_texcoord;
}
